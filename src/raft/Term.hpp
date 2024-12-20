#pragma once

#include <cstdint>
#include <optional>
#include <set>
#include <string>

enum VotingState {
    WIN,
    LOSE,
    MATCH,
    WAITING
};

class Term {
    uint32_t m_idx;
    std::optional<std::string> m_voted;

    std::set<std::string> m_couted_voters;
    uint32_t m_votes;
    uint32_t m_votes_against;

    uint32_t m_num_peers;

public:
    uint32_t votes_for() const { return m_votes; }
    uint32_t votes_againts() const { return m_votes_against; }

    uint32_t idx() const {return m_idx;}
    bool has_voted() const {return m_voted.has_value();}
    std::string voted_for() const { return m_voted.value(); }

    const std::set<std::string>& counted_voters() const { return m_couted_voters; }

    bool is_voter_counted(std::string voter) {
        return m_couted_voters.count(voter) > 0;
    }

    void set_has_voted(std::string candidate) {
        m_voted = candidate;
    }

    Term(uint32_t idx, uint32_t num_peers) :
        m_idx(idx),
        m_num_peers(num_peers),
        m_voted({}),
        m_votes(0),
        m_votes_against(0) {
    }

    void add_vote_for_me(std::string from) {
        m_couted_voters.insert(from);
        m_votes++;
    }

    void add_vote_against_me(std::string from) {
        m_couted_voters.insert(from);
        m_votes_against++;
    }

    void candidate() {

    }

    void reset_voting() {
        m_votes = 0;
        m_votes_against = 0;
        m_couted_voters.clear();
    }

    VotingState get_voting_state() {
        if(m_votes + m_votes_against == m_num_peers) {
            if(m_votes == m_votes_against) {
                return MATCH;
            } else if(m_votes > m_votes_against) {
                return WIN;
            } else {
                return LOSE;
            }
        } else {
            return WAITING;
        }
    }
};
