#pragma once

#include "mutex/Mutex.hpp"

class MutexGui {
    std::shared_ptr<MutexSheriff> m_sheriff;
    std::vector<std::shared_ptr<Mutex>> m_mutexes;

public:
    MutexGui(std::shared_ptr<MutexSheriff> sheriff,
        std::vector<std::shared_ptr<Mutex>> mutexes);

    void draw();
};
