#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

class LogEntry {
private:
    uint32_t m_idx;
    uint32_t m_term;
    std::string m_command;

public:
    uint32_t idx() const { return m_idx; }
    uint32_t term() const { return m_term; }
    const std::string& command() const { return m_command; }

    LogEntry(uint32_t idx, uint32_t term_idx, std::string command) :
        m_idx(idx),
        m_term(term_idx),
        m_command(command) {

    }
};

class RaftLog {
    std::vector<LogEntry> m_entries;
    uint32_t m_idx;

public:
    std::vector<LogEntry>& entries() {
        return m_entries;
    }

    LogEntry last() {
        return m_entries.back();
    }

    uint32_t commit_idx() {
        return m_entries.size() - 1;
    }

    bool is_up_to_date(uint32_t term_idx, uint32_t log_idx) {
        return (last().term() <= term_idx && last().idx() <= log_idx);
    }

    std::optional<LogEntry> get(uint32_t log_idx) {
        for(auto& entry : m_entries) {
            if(entry.idx() == log_idx) {
                return entry;
            }
        }

        return {};
    }

    bool contains(uint32_t term_idx, uint32_t log_idx) {
        for(int i = m_entries.size() - 1; i >= 0; i--) {
            if(m_entries[i].idx() == log_idx) {
                return m_entries[i].term() == term_idx;
            }
        }

        return false;
    }

    /**
     * Erases every entry until it finds entry with log_idx and term_idx
     */
    void sync(uint32_t term_idx, uint32_t log_idx) {
        for(int i = m_entries.size() - 1; i >= 0; i--) {
            if(m_entries[i].idx() == log_idx) {
                if(m_entries[i].term() != term_idx) {
                    for(int x = i; x < m_entries.size(); x++) {
                        m_entries.erase(m_entries.begin() + x);
                    }
                }
                break;
            }
        }
    }

    RaftLog() {

    }

    void set_idx(uint32_t idx) {
        m_idx = idx;
    }

    void add_entry(uint32_t term_idx, std::string command) {
        m_entries.push_back(LogEntry(++m_idx, term_idx, command));
    }

    void add_entry(LogEntry logEntry) {
        m_entries.push_back(logEntry);
        m_idx = logEntry.idx() + 1;
    }
};
