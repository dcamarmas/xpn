
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

#include <dirent.h>
#include <sys/stat.h>
#include <sys/statvfs.h>

#include "base_cpp/filesystem.hpp"
#include "xpn/xpn_metadata.hpp"

/* Operations */

namespace XPN {

enum class xpn_server_ops {
    // File operations
    OPEN_FILE,
    CREAT_FILE,
    READ_FILE,
    WRITE_FILE,
    CLOSE_FILE,
    RM_FILE,
    RM_FILE_ASYNC,
    RENAME_FILE,
    GETATTR_FILE,
    SETATTR_FILE,

    // Directory operations
    MKDIR_DIR,
    RMDIR_DIR,
    RMDIR_DIR_ASYNC,
    OPENDIR_DIR,
    READDIR_DIR,
    CLOSEDIR_DIR,

    // FS Operations
    STATVFS_DIR,

    // Metadata
    READ_MDATA,
    WRITE_MDATA,
    WRITE_MDATA_FILE_SIZE,

    // Connection operatons
    FINALIZE,
    DISCONNECT,
    END,

    // For enum count
    size,
};

static const std::array<std::string, static_cast<size_t>(xpn_server_ops::size) + 1> xpn_server_ops_names = {
    // File operations
    "OPEN_FILE",
    "CREAT_FILE",
    "READ_FILE",
    "WRITE_FILE",
    "CLOSE_FILE",
    "RM_FILE",
    "RM_FILE_ASYNC",
    "RENAME_FILE",
    "GETATTR_FILE",
    "SETATTR_FILE",

    // Directory operations
    "MKDIR_DIR",
    "RMDIR_DIR",
    "RMDIR_DIR_ASYNC",
    "OPENDIR_DIR",
    "READDIR_DIR",
    "CLOSEDIR_DIR",

    // FS Operations
    "STATVFS_DIR",

    // Metadata
    "READ_MDATA",
    "WRITE_MDATA",
    "WRITE_MDATA_FILE_SIZE",

    // Connection operatons
    "FINALIZE",
    "DISCONNECT",
    "END",

    // For enum count
    "size",
};

static inline const std::string &xpn_server_ops_name(xpn_server_ops op) {
    return xpn_server_ops_names[static_cast<size_t>(op)];
}

/* Message struct */
struct xpn_server_path {
    uint32_t size;
    char path[PATH_MAX];

    size_t get_size() { return offsetof(std::remove_pointer<decltype(this)>::type, path) + size; }
};

struct xpn_server_double_path {
    uint32_t size1;
    uint32_t size2;
    char path[PATH_MAX * 2];

    size_t get_size() { return offsetof(std::remove_pointer<decltype(this)>::type, path) + size1 + size2; }
    char *path1() { return path; }
    char *path2() { return path + size1; }
    const char *path1() const { return path; }
    const char *path2() const { return path + size1; }
};

struct st_xpn_server_status {
    int ret;
    int server_errno;

    size_t get_size() { return sizeof(*this); }
};

struct st_xpn_server_path_flags {
    int flags;
    mode_t mode;
    char xpn_session;
    xpn_server_path path;

    size_t get_size() { return offsetof(std::remove_pointer<decltype(this)>::type, path) + path.get_size(); }
};
struct st_xpn_server_path {
    xpn_server_path path;

    size_t get_size() { return path.get_size(); }
};

struct st_xpn_server_close {
    int fd;
    ::DIR *dir;

    size_t get_size() { return sizeof(*this); }
};

struct st_xpn_server_rw {
    int64_t offset;
    uint64_t size;
    int fd;
    char xpn_session;
    // uint64_t new_file_size;
    xpn_server_path path;

    size_t get_size() { return offsetof(std::remove_pointer<decltype(this)>::type, path) + path.get_size(); }
};

struct st_xpn_server_rw_req {
    int64_t size;
    char last;
    struct st_xpn_server_status status;

    size_t get_size() { return sizeof(*this); }
};

struct st_xpn_server_rename {
    xpn_server_double_path paths;

    size_t get_size() { return paths.get_size(); }
};

struct st_xpn_server_setattr {
    struct ::stat attr;
    xpn_server_path path;

    size_t get_size() { return offsetof(std::remove_pointer<decltype(this)>::type, path) + path.get_size(); }
};

struct st_xpn_server_attr_req {
    char status;
    struct ::stat attr;
    struct st_xpn_server_status status_req;

    size_t get_size() { return sizeof(*this); }
};

struct st_xpn_server_readdir {
    long telldir;
    ::DIR *dir;
    char xpn_session;
    xpn_server_path path;

    size_t get_size() { return offsetof(std::remove_pointer<decltype(this)>::type, path) + path.get_size(); }
};

struct st_xpn_server_opendir_req {
    ::DIR *dir;
    struct st_xpn_server_status status;

    size_t get_size() { return sizeof(*this); }
};

struct st_xpn_server_readdir_req {
    int end;  // If end = 1 exist entry; 0 not exist
    struct ::dirent ret;
    long telldir;
    struct st_xpn_server_status status;

    size_t get_size() { return sizeof(*this); }
};

struct st_xpn_server_read_mdata_req {
    xpn_metadata::data mdata;
    struct st_xpn_server_status status;

    size_t get_size() { return sizeof(*this); }
};

struct st_xpn_server_write_mdata {
    xpn_metadata::data mdata;
    xpn_server_path path;

    size_t get_size() { return offsetof(std::remove_pointer<decltype(this)>::type, path) + path.get_size(); }
};

struct st_xpn_server_write_mdata_file_size {
    uint64_t size;
    xpn_server_path path;

    size_t get_size() { return offsetof(std::remove_pointer<decltype(this)>::type, path) + path.get_size(); }
};

struct st_xpn_server_statvfs_req {
    struct ::statvfs attr;
    struct st_xpn_server_status status_req;

    size_t get_size() { return sizeof(*this); }
};

constexpr size_t get_xpn_server_max_msg_size() {
    size_t size = 0;
    if (size < sizeof(st_xpn_server_status)) size = sizeof(st_xpn_server_status);
    if (size < sizeof(st_xpn_server_status)) size = sizeof(st_xpn_server_status);
    if (size < sizeof(st_xpn_server_path_flags)) size = sizeof(st_xpn_server_path_flags);
    if (size < sizeof(st_xpn_server_path)) size = sizeof(st_xpn_server_path);
    if (size < sizeof(st_xpn_server_close)) size = sizeof(st_xpn_server_close);
    if (size < sizeof(st_xpn_server_rw)) size = sizeof(st_xpn_server_rw);
    if (size < sizeof(st_xpn_server_rw_req)) size = sizeof(st_xpn_server_rw_req);
    if (size < sizeof(st_xpn_server_rename)) size = sizeof(st_xpn_server_rename);
    if (size < sizeof(st_xpn_server_setattr)) size = sizeof(st_xpn_server_setattr);
    if (size < sizeof(st_xpn_server_attr_req)) size = sizeof(st_xpn_server_attr_req);
    if (size < sizeof(st_xpn_server_readdir)) size = sizeof(st_xpn_server_readdir);
    if (size < sizeof(st_xpn_server_opendir_req)) size = sizeof(st_xpn_server_opendir_req);
    if (size < sizeof(st_xpn_server_readdir_req)) size = sizeof(st_xpn_server_readdir_req);
    if (size < sizeof(st_xpn_server_read_mdata_req)) size = sizeof(st_xpn_server_read_mdata_req);
    if (size < sizeof(st_xpn_server_write_mdata)) size = sizeof(st_xpn_server_write_mdata);
    if (size < sizeof(st_xpn_server_write_mdata_file_size)) size = sizeof(st_xpn_server_write_mdata_file_size);
    if (size < sizeof(st_xpn_server_statvfs_req)) size = sizeof(st_xpn_server_statvfs_req);
    return size;
}

struct xpn_server_msg {
    int op = 0;
    int tag = 0;
    uint32_t msg_size = 0;
    char msg_buffer[get_xpn_server_max_msg_size()] = {};

    uint32_t get_size() { return offsetof(std::remove_pointer<decltype(this)>::type, msg_buffer) + msg_size; }
    uint32_t get_header_size() { return offsetof(std::remove_pointer<decltype(this)>::type, msg_buffer); }
};

}  // namespace XPN