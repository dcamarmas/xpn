
/*
 *  Copyright 2020-2024 Felix Garcia Carballeira, Diego Camarmas Alonso, Alejandro Calderon Mateos, Dario Muñoz Muñoz
 *
 *  This file is part of Expand.
 *
 *  Expand is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Expand is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with Expand.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

 #pragma once
 
 
#include <queue>
#include <mutex>
#include <condition_variable>

template <typename T>
class queue_pool {
public:
    queue_pool(uint64_t initialSize = 10) {
        for (uint64_t i = 0; i < initialSize; ++i) {
            m_queue.push(new T());
        }
    }

    ~queue_pool() {
        std::unique_lock lock(m_mutex);
        while (!m_queue.empty()) {
            delete m_queue.front();
            m_queue.pop();
        }
    }

    T* acquire() {
        std::unique_lock lock(m_mutex);
        if (m_queue.empty()) {
            return new T();
        }
        T* obj = m_queue.front();
        m_queue.pop();
        return obj;
    }

    void release(T* obj) {
        std::unique_lock lock(m_mutex);
        m_queue.push(obj);
    }

private:
    std::queue<T*> m_queue;
    std::mutex m_mutex;
};