#include "Raft.hpp"
#include "network/Node.hpp"
#include "raft/RaftLog.hpp"
#include "raft/RaftMessage.hpp"
#include <chrono>
#include <cmath>
#include <thread>
#include <threads.h>



Raft::Raft(std::vector<std::string> peers) :
    m_term(0, peers.size()) {
    m_peers = peers;
    for(int i = 0; i < m_peers.size(); i++) {
        if(m_peers[i] == address) {
            m_peers.erase(m_peers.begin() + i);
        }
    }

    m_raft_state = FOLLOWER;
    m_random =
        std::mt19937_64(std::chrono::steady_clock::now().time_since_epoch().count());
    m_log.add_entry(LogEntry(0, 0, ""));
}



void Raft::start_election() {
    // set random wait
    auto wait = m_random() % 2000;
    std::this_thread::sleep_for(std::chrono::milliseconds(wait));

    new_term(log_idx() + 1);

    m_raft_state = CANDIDATE;

    for(auto& peer : m_peers) {
        m_future_votes.push_back(send_message(m_network, peer, new RaftRequestVoteMessage(m_log.last().term(), m_log.last().idx(), m_term.idx())));
    }
}


void Raft::set_to_leader() {
    m_raft_state = LEADER;

    for(auto& peer : m_peers) {
        if(peer == address) {
            continue;
        }

        m_next_idxs[peer] = m_log.last().idx() + 1;
        m_match_idxs[peer] = 0;

        send_heartbeat();
    }
}


void Raft::check_votes() {
    for(auto& vote : m_future_votes) {
        if(!vote->is_replied() || m_term.is_voter_counted(vote->response()->from())) {
            continue;
        }

        auto response = (RaftRequestVoteResponse*)vote->response()->message();
        if(response->has_accepted()) {
            m_term.add_vote_for_me(vote->response()->from());
        } else {
            m_term.add_vote_against_me(vote->response()->from());
        }

        if(response->voter_is_leader() && !response->has_accepted()) {
            m_raft_state = FOLLOWER;
            m_term.reset_voting();
        }
    }

    if(m_term.get_voting_state() != WAITING) {
        switch(m_term.get_voting_state()) {
            case WIN: {
                set_to_leader();
                break;
            } case LOSE: {
                m_raft_state = FOLLOWER;
                break;
            } case MATCH: {
                start_election();
                break;
            }
        }
    }
}

void Raft::send_heartbeat() {
    for(auto& peer : m_peers) {
        if(peer == address || m_responses.count(peer)) {
            continue;
        }

        std::optional<LogEntry> entry = {};
        if(m_next_idxs[peer] <= m_log.last().idx()) {
            entry = m_log.get(m_next_idxs[peer]);
            std::cout << "Getting entry " << entry.has_value() << std::endl;
        }

        m_responses[peer] = send_message(m_network, peer, new RaftAppendEntriesMessage(
            m_term.idx(),
            m_log.last().idx(),
            m_log.last().term(),
            entry,
            m_commit_idx));
    }
}

bool Raft::should_vote(std::string candidate, RaftRequestVoteMessage* request) {
    if(request->candidate_term_idx() < m_term.idx()) {
        return false;
    }

    if(m_log.is_up_to_date(request->last_log_term_idx(), request->last_log_idx())) {
        new_term(request->candidate_term_idx());
        if(!m_term.has_voted() || m_term.voted_for() == candidate) {
            m_term.set_has_voted(candidate);
            return true;
        }
    }

    return false;
}


void Raft::append_entries(std::string leader, NetworkMessage* network_message, RaftAppendEntriesMessage* message) {
    if(!m_leader.has_value() || m_term.idx() < message->leader_term()) {
        m_leader = leader;
        m_term = Term(message->leader_term(), m_peers.size());
        m_term.set_has_voted(m_leader.value());
    }

    if(message->leader_term() < m_term.idx()) {
        std::cout << "Leader term is lower than current term" << std::endl;
        m_network->reply(network_message, new RaftAppendEntriesResponse(false, m_term.idx()));
        return;
    }

    if(!m_log.contains(message->prev_log_term_idx(), message->prev_log_idx())) {
        std::cout << "Syncing" << std::endl;
        m_network->reply(network_message, new RaftAppendEntriesResponse(false, m_term.idx()));
        m_log.sync(message->prev_log_term_idx(), message->prev_log_idx());
        return;
    }

    if(message->entry().has_value() && !m_log.contains(message->entry()->term(), message->entry()->idx())) {
        std::cout << "Adding entry" << std::endl;
        m_log.add_entry(message->entry().value());
    }

    if(message->leader_commit() > m_commit_idx) {
        m_commit_idx = std::min(message->leader_commit(), m_log.commit_idx());
    }

    m_network->reply(network_message, new RaftAppendEntriesResponse(true, m_term.idx()));
}

void Raft::handle_append_entry_responses() {
    for(auto& peer : m_peers) {
        if(m_responses.count(peer) == 0) {
            continue;
        }

        auto reply = m_responses[peer];
        if(reply.get() == 0) {
            continue;
        }

        if(reply->is_replied()) {
            auto response = (RaftAppendEntriesResponse*)reply->response()->message();
            if(response->is_success()) {
                if(m_next_idxs[peer] < m_log.last().idx()) {
                    m_match_idxs[peer] = m_next_idxs[peer];
                    m_next_idxs[peer] = m_next_idxs[peer] + 1;

                    auto entry = m_log.get(m_next_idxs[peer]);
                    if(!entry.has_value()) {
                        entry = m_log.get(0);
                    }
                    auto prev = m_log.get(entry.value().idx() - 1);

                    m_responses[peer] = send_message(m_network, peer, new RaftAppendEntriesMessage(
                                m_term.idx(),
                                prev.has_value() ? prev.value().idx() : 0,
                                prev.has_value() ? prev.value().term() : 0,
                                entry.value(),
                                m_commit_idx));
                } else {
                    m_responses.erase(peer);
                }
                m_last_success[peer] = true;
            } else {
                // if failed, try decrease the m_next_idxs
                // and try again
                m_last_success[peer] = false;
                m_next_idxs[peer] = m_next_idxs[peer] - 1;
                auto entry = m_log.get(m_next_idxs[peer]);
                if(!entry.has_value()) {
                    entry = m_log.get(0);
                }

                auto prev = m_log.get(entry.value().idx() - 1);
                m_responses[peer] = send_message(m_network, peer, new RaftAppendEntriesMessage(
                            m_term.idx(),
                            prev.has_value() ? prev.value().idx() : 0,
                            prev.has_value() ? prev.value().term() : 0,
                            entry.value(),
                            m_commit_idx));
            }
        }
    }
}

void Raft::leader_runtime() {
    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    if(std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() > 50 / *m_network->timescale_ptr()) {
        std::cout << "Sending heartbeat" << std::endl;
        send_heartbeat();
        start = end;
    }

    handle_append_entry_responses();
}

void Raft::runtime(Network* network)  {
    m_network = network;
    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
    m_last_frame = std::chrono::steady_clock::now();
    m_elapsed = std::chrono::milliseconds(0);


    while(true) {
        if(m_state == STOPPED) {
            continue;
        }

        // limit the frame rate
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        auto now = std::chrono::steady_clock::now();
        m_elapsed += std::chrono::duration_cast<std::chrono::nanoseconds>((now - m_last_frame) * *m_network->timescale_ptr());
        m_last_frame = now;
        if(m_elapsed < std::chrono::milliseconds(10)) {
            continue;
        }

        m_elapsed = std::chrono::milliseconds(0);

        if (m_raft_state == FOLLOWER &&
            m_raft_state != CANDIDATE &&
            !m_leader.has_value()) {
            start_election();
        }

        NetworkMessage* message = nullptr;
        while((message = receive_message())) {
            switch(message->message()->message_type()) {
                case RAFT_APPEND_ENTRIES_MESSAGE: {
                    auto candidateMessage = (RaftAppendEntriesMessage*)message->message();
                    append_entries(message->from(), message, candidateMessage);
                    break;
                }
                case RAFT_REQUEST_VOTE_MESSAGE: {
                    auto candidateMessage = (RaftRequestVoteMessage*)message->message();
                    m_network->reply(message, new RaftRequestVoteResponse(should_vote(message->from(), candidateMessage), false));
                    break;
                } case RAFT_REQUEST_MESSAGE: {
                    if(m_leader.has_value() && m_leader.value() != address) {
                        send_message(m_network, m_leader.value(), (Message*)message->message());
                    } else {
                        m_log.add_entry(m_term.idx(), ((RaftRequest*)message->message())->content());
                    }

                    break;
                }
            }
        }

        if(m_raft_state == LEADER) {
            leader_runtime();
        }
        else if(m_raft_state == CANDIDATE) {
            check_votes();
        }
    }
}
