#include "network/Node.hpp"
#include "network/Network.hpp"
#include "raft/RaftLog.hpp"
#include "raft/RaftMessage.hpp"
#include "raft/Term.hpp"

#include <chrono>
#include <random>

enum RaftState {
    FOLLOWER,
    CANDIDATE,
    LEADER
};

class Raft : public Node {
private:
    RaftState m_raft_state;
    std::vector<std::string> m_peers;
    std::mt19937_64 m_random;

    RaftLog m_log;
    Term m_term;

    std::vector<std::shared_ptr<FutureReply>> m_future_votes;
    std::map<std::string, uint32_t> m_next_idxs;
    std::map<std::string, uint32_t> m_match_idxs;
    std::map<std::string, std::shared_ptr<FutureReply>> m_responses;
    std::map<std::string, bool> m_last_success;

    std::optional<std::string> m_leader;
    std::chrono::steady_clock::time_point m_last_frame;
    std::chrono::nanoseconds m_elapsed;

    uint32_t m_commit_idx;

    void start_election();

    void new_term(uint32_t log_idx) {
        if(m_term.idx() < log_idx) {
            m_term = Term(log_idx, m_peers.size());
        }
    }

    void check_votes();

    void send_heartbeat();

    void handle_elections();
    bool should_vote(std::string candidate, RaftRequestVoteMessage* request);

    void append_entries(std::string leader, NetworkMessage* network_message, RaftAppendEntriesMessage* message);

    void set_to_leader();

public:
    const std::vector<std::string>& peers() const { return m_peers; }
    const std::vector<std::shared_ptr<FutureReply>>& future_votes() const { return m_future_votes; }
    uint32_t votes_for() const { return m_term.votes_for(); }
    uint32_t votes_againts() const {return m_term.votes_againts(); }

    uint32_t log_idx() const {return m_term.idx();}
    bool has_voted() const {return m_term.has_voted();}

    void time_out() {
        m_leader = {};
        start_election();
    }

    uint32_t next_idx_for(std::string peer) {
        if(m_next_idxs.count(peer)) {
            return m_next_idxs[peer];
        } return m_log.last().idx() + 1;
    }

    uint32_t match_idx_for(std::string peer) {
        if(m_match_idxs.count(peer)) {
            return m_match_idxs[peer];
        } return m_log.last().idx() + 1;
    }

    std::optional<std::shared_ptr<FutureReply>> response_for(std::string peer) {
        if(m_responses.count(peer)) {
            return m_responses[peer];
        } return {};
    }

    bool last_success_for(std::string peer) {
        if(m_last_success.count(peer)) {
            return m_last_success[peer];
        } return false;
    }

    RaftLog& log() { return m_log; }

    void handle_append_entry_responses();
    void leader_runtime();

    RaftState raft_state() const {return m_raft_state;}

    Raft(std::vector<std::string> peers);

    void runtime(Network* network) override;
};
