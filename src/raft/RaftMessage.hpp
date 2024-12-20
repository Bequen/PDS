#pragma once

#include <cstdint>
#include <iostream>
#include <optional>

#include "network/NetworkIO.hpp"
#include "raft/RaftLog.hpp"

#define RAFT_APPEND_ENTRIES_MESSAGE 1
#define RAFT_REQUEST_VOTE_MESSAGE 2
#define RAFT_REQUEST_MESSAGE 3

class RaftRequestVoteMessage : public Message {
private:
    uint32_t m_last_log_idx;
    uint32_t m_last_log_term_idx;

    uint32_t m_candidate_term_idx;

public:
    uint32_t last_log_idx() const {
        return m_last_log_idx;
    }

    uint32_t last_log_term_idx() const {
        return m_last_log_term_idx;
    }

    uint32_t candidate_term_idx() const {
        return m_candidate_term_idx;
    }

    RaftRequestVoteMessage(uint32_t last_log_term_idx, uint32_t last_log_idx, uint32_t candidate_term_idx) :
        m_last_log_idx(last_log_idx),
        m_last_log_term_idx(last_log_term_idx),
        m_candidate_term_idx(candidate_term_idx) {
    }

    std::string display() override {
        return "Candidate message for [" + std::to_string(m_candidate_term_idx) + "]";
    }

    unsigned int message_type() const override {
        return RAFT_REQUEST_VOTE_MESSAGE;
    }
};



class RaftRequestVoteResponse : public Message {
private:
    bool m_accepted;
    bool m_isLeader;

public:
    bool voter_is_leader() const {
        return m_isLeader;
    }

    bool has_accepted() const {
        return m_accepted;
    }

    RaftRequestVoteResponse(bool accepted, bool isLeader) :
        m_accepted(accepted),
        m_isLeader(isLeader) {

    }

    std::string display() override {
        return "Candidate message";
    }

    unsigned int message_type() const override {
        return RAFT_REQUEST_VOTE_MESSAGE;
    }
};




class RaftAppendEntriesMessage : public Message {
    uint32_t m_leader_term;
    uint32_t m_prev_log_idx;
    uint32_t m_prev_log_term_idx;
    std::optional<LogEntry> m_log_entry;
    uint32_t m_leader_commit;

public:
    std::optional<LogEntry> entry() const {
        return m_log_entry;
    }

    uint32_t leader_term() const {
        return m_leader_term;
    }

    uint32_t prev_log_idx() const {
        return m_prev_log_idx;
    }

    uint32_t prev_log_term_idx() const {
        return m_prev_log_term_idx;
    }

    uint32_t leader_commit() const {
        return m_leader_commit;
    }

    RaftAppendEntriesMessage(uint32_t leader_term, uint32_t prev_log_idx, uint32_t prev_log_term_idx, std::optional<LogEntry> entry, uint32_t leader_commit) :
        m_leader_term(leader_term),
        m_prev_log_idx(prev_log_idx),
        m_prev_log_term_idx(prev_log_term_idx),
        m_log_entry(entry),
        m_leader_commit(leader_commit)
        {

    }

    std::string display() override {
        return "Append Entries";
    }

    unsigned int message_type() const override {
        return RAFT_APPEND_ENTRIES_MESSAGE;
    }
};


class RaftAppendEntriesResponse : public Message {
    uint32_t m_term_idx;
    bool m_isSuccess;

public:
    bool is_success() const {
        return m_isSuccess;
    }

    uint32_t term_idx() const {
        return m_term_idx;
    }

    RaftAppendEntriesResponse(bool is_success, uint32_t term_idx) :
        m_isSuccess(is_success),
        m_term_idx(term_idx) {

    }

    std::string display() override {
        return "Append Entries Response";
    }

    unsigned int message_type() const override {
        return RAFT_APPEND_ENTRIES_MESSAGE;
    }
};




class RaftRequest : public Message {
    std::string m_content;

public:
    const std::string& content() const {
        return m_content;
    }

    RaftRequest(std::string content) :
        m_content(content) {

    }

    std::string display() override {
        return "Append Entries Response";
    }

    unsigned int message_type() const override {
        return RAFT_REQUEST_MESSAGE;
    }
};
