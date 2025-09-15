#include <fcntl.h>
#include <unistd.h>

#include <fstream>
#include <iostream>
#include <random>
#include <string>
#include <vector>

#include "base_cpp/debug.hpp"
#include "setup.hpp"
#include "xpn.h"

std::string generate_random_string(size_t length) {
    const std::string characters = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
    std::string result;
    result.reserve(length);

    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_int_distribution<size_t> distribution(0, characters.length() - 1);

    for (size_t i = 0; i < length; ++i) {
        result += characters[distribution(generator)];
    }
    return result;
}

void run_test() {
    XPN_scope xpn;

    size_t data_size_mb = 1;
    const std::string filename = "/xpn/mega_data_test.bin";
    size_t total_bytes = data_size_mb * 1024 * 1024;
    std::string original_data = generate_random_string(total_bytes);
    std::cout << "Generated data of " << data_size_mb << " MB." << std::endl;

    FILE* file_w = xpn_fopen(filename.c_str(), "w+");
    if (file_w == nullptr) {
        perror("Error opening file for writing");
        exit(EXIT_FAILURE);
    }
    size_t written_bytes = xpn_fwrite(original_data.data(), sizeof(char), original_data.size(), file_w);
    xpn_fclose(file_w);

    if (written_bytes != original_data.size()) {
        std::cerr << "Error writing data to file: " << filename << std::endl;
        exit(EXIT_FAILURE);
    }
    std::cout << "Successfully wrote " << written_bytes << " bytes to " << filename << std::endl;

    FILE* file_r = xpn_fopen(filename.c_str(), "r");
    if (file_r == nullptr) {
        perror("Error opening file for reading");
        exit(EXIT_FAILURE);
    }

    xpn_fseek(file_r, 0, SEEK_END);
    long file_size = xpn_ftell(file_r);
    xpn_fseek(file_r, 0, SEEK_SET);

    std::string read_data;
    read_data.resize(file_size);
    size_t read_bytes = xpn_fread(read_data.data(), sizeof(char), file_size, file_r);
    xpn_fclose(file_r);

    if (read_bytes != (size_t)file_size) {
        std::cerr << "Error reading data from file: " << filename << std::endl;
        exit(EXIT_FAILURE);
    }
    std::cout << "Successfully read " << read_bytes << " bytes from " << filename << std::endl;

    bool is_same = (original_data == read_data);
    if (is_same) {
        std::cout << "Test Passed: The written data is identical to the read data." << std::endl;
    } else {
        std::cerr << "Test Failed: The written data is NOT identical to the read data." << std::endl;
        exit(EXIT_FAILURE);
    }
}

int main() {
    std::string tmp_dir = "/tmp/" + std::to_string(::getpid());
    auto cleanup_tmp_dir = setup::create_empty_dir(tmp_dir);
    auto cleanup_data_dir1 = setup::create_empty_dir(tmp_dir + "/xpn1");
    auto cleanup_data_dir2 = setup::create_empty_dir(tmp_dir + "/xpn2");
    auto cleanup_data_dir3 = setup::create_empty_dir(tmp_dir + "/xpn3");
    XPN::xpn_conf::partition part;
    {
        part.server_urls = {
            "file://localhost/" + tmp_dir + "/xpn1",
            "file://localhost/" + tmp_dir + "/xpn2",
            "file://localhost/" + tmp_dir + "/xpn3",
        };
        part.bsize = 1024;
        auto cleanup_conf = setup::create_xpn_conf(tmp_dir + "/xpn.conf", part);
        run_test();
    }
    {
        part.server_urls = {
            "file://localhost/" + tmp_dir + "/xpn1",
            "file://localhost/" + tmp_dir + "/xpn2",
        };
        part.bsize = 1024 * 1024;
        auto cleanup_conf = setup::create_xpn_conf(tmp_dir + "/xpn.conf", part);
        run_test();
    }
    setup::env({{"XPN_LOCALITY", "0"}, {"XPN_CONNECT_RETRY_TIME_MS", "0"}});
    {
        part.server_urls = {
            "sck_server://localhost/" + tmp_dir + "/xpn1",
        };
        part.bsize = 1024 * 1024;
        auto cleanup_conf = setup::create_xpn_conf(tmp_dir + "/xpn.conf", part);
        auto cleanup_srvs = setup::start_srvs(part);
        print("end start_srvs");
        run_test();
    }
}