
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

#include "xpn/xpn_file_table.hpp"
#include "xpn/xpn_api.hpp"

namespace XPN
{
    xpn_file_table::~xpn_file_table()
    {   
        std::vector<int> keys;
        for (auto [key,file] : m_files)
        {
            keys.emplace_back(key);
        }
        for (auto &key : keys)
        {
            XPN_DEBUG("Forgot to close: "<<key);
            remove(key);
        }
    }

    int xpn_file_table::insert(std::shared_ptr<xpn_file> file)
    {
        int fd;
        if (m_free_keys.empty()){
            fd = secuencial_key++;
        }else{
            fd = m_free_keys.front();
            m_free_keys.pop();
        }
        m_files.emplace(std::make_pair(fd, file));
        return fd;
    }

    bool xpn_file_table::remove(int fd)
    {   
        int res = m_files.erase(fd);
        if (res == 1){
            m_free_keys.push(fd);
        }
        return res == 1 ? true : false;
    }

    // It must be checked if fd is in the file_table with has(fd)
    int xpn_file_table::dup(int fd, int new_fd)
    {   
        int ret = -1;
        auto file = get(fd);
        if (!file){
            return -1;
        }
        if (new_fd != -1){
            // Like posix dup2 close silently if its open
            if (has(new_fd)){
                xpn_api::get_instance().close(new_fd);
            }
            m_files.emplace(std::make_pair(new_fd, file));
            ret = new_fd;
        }else{
            ret = insert(file);
        }
        return ret;
    }

    std::string xpn_file_table::to_string()
    {
        std::stringstream out;
        for (auto &[key, file] : m_files)
        {
            out << "fd: " << key << " : " << (*file).m_path << std::endl;
        }
        return out.str();
    }
    
    void xpn_file_table::clean()
    {
        std::vector<int> fds_to_close;
        fds_to_close.reserve(m_files.size());
        for (auto &[key, file] : m_files)
        {
            fds_to_close.emplace_back(key);
        }
        for (auto &fd : fds_to_close)
        {
            xpn_api::get_instance().close(fd);
        }
        
        m_files.clear();
        decltype(m_free_keys) empty;
        m_free_keys.swap(empty);
        secuencial_key = 1;
    }
}