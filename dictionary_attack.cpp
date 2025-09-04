// Compile with:
// g++ -std=c++17 -pthread dictionary_attack.cpp -o attack -lssl -lcrypto

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <openssl/sha.h>
#include <openssl/md5.h>
#include <chrono>
#include <random>
#include <functional>

std::atomic<bool> found(false);
std::string result;
std::mutex result_mutex;

std::string md5(const std::string& input) {
    unsigned char hash[MD5_DIGEST_LENGTH];
    MD5(reinterpret_cast<const unsigned char*>(input.c_str()), input.size(), hash);
    std::ostringstream oss;
    for (int i = 0; i < MD5_DIGEST_LENGTH; ++i)
        oss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    return oss.str();
}

std::string sha1(const std::string& input) {
    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA1(reinterpret_cast<const unsigned char*>(input.c_str()), input.size(), hash);
    std::ostringstream oss;
    for (int i = 0; i < SHA_DIGEST_LENGTH; ++i)
        oss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    return oss.str();
}

std::string sha256(const std::string& input) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char*>(input.c_str()), input.size(), hash);
    std::ostringstream oss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i)
        oss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    return oss.str();
}

std::string sha512(const std::string& input) {
    unsigned char hash[SHA512_DIGEST_LENGTH];
    SHA512(reinterpret_cast<const unsigned char*>(input.c_str()), input.size(), hash);
    std::ostringstream oss;
    for (int i = 0; i < SHA512_DIGEST_LENGTH; ++i)
        oss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    return oss.str();
}

void load_file(const std::string& filename, std::vector<std::string>& list) {
    std::ifstream file(filename);
    std::string line;
    while (std::getline(file, line)) {
        if (!line.empty())
            list.push_back(line);
    }
}

std::string sequential_crack(const std::string& target_hash, const std::vector<std::string>& wordlist,
                             std::function<std::string(const std::string&)> hash_func) {
    for (const auto& word : wordlist) {
        if (hash_func(word) == target_hash)
            return word;
    }
    return "";
}

void worker(const std::vector<std::string>& wordlist, const std::string& target_hash,
            std::function<std::string(const std::string&)> hash_func, size_t start, size_t end) {
    for (size_t i = start; i < end && !found; ++i) {
        if (hash_func(wordlist[i]) == target_hash) {
            std::lock_guard<std::mutex> lock(result_mutex);
            result = wordlist[i];
            found = true;
            break;
        }
    }
}

std::string parallel_crack(const std::string& target_hash, const std::vector<std::string>& wordlist,
                           std::function<std::string(const std::string&)> hash_func, int thread_count) {
    std::vector<std::thread> threads;
    size_t chunk = wordlist.size() / thread_count;

    for (int i = 0; i < thread_count; ++i) {
        size_t start = i * chunk;
        size_t end = (i == thread_count - 1) ? wordlist.size() : start + chunk;
        threads.emplace_back(worker, std::cref(wordlist), std::cref(target_hash), hash_func, start, end);
    }

    for (auto& t : threads) t.join();
    return result;
}



int main() {
    std::vector<std::pair<std::string, std::function<std::string(const std::string&)>>> hash_algorithms = {
        {"SHA-1", sha1},
        {"SHA-256", sha256},
        {"SHA-512", sha512},
        {"MD5", md5}
    };

    std::vector<int> thread_counts = {1, 2, 4, 8, 16};

    std::vector<std::string> wordlist;
    load_file("rockyou.txt", wordlist);

    if (wordlist.empty()) {
        std::cerr << "Error: Empty wordlist.\n";
        return 1;
    }

    // Pick random password
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(0, wordlist.size() - 1);

    std::string password = wordlist[dist(gen)];
    std::cout << "Randomly selected password: " << password << "\n\n";

    for (const auto& [name, hash_func] : hash_algorithms) {
        std::string target_hash = hash_func(password);
        std::cout << "=== Hash Algorithm: " << name << " ===\n";
        std::cout << "Target hash: " << target_hash << "\n";

        // Sequential crack
        auto start_seq = std::chrono::high_resolution_clock::now();
        std::string seq_result = sequential_crack(target_hash, wordlist, hash_func);
        auto end_seq = std::chrono::high_resolution_clock::now();
        double seq_time = std::chrono::duration<double>(end_seq - start_seq).count();

        std::cout << "Sequential result: " << seq_result
                  << ", Time: " << seq_time << "s\n";

        // Parallel cracks
        for (int tc : thread_counts) {
            found = false;
            result.clear();

            auto start_par = std::chrono::high_resolution_clock::now();
            std::string par_result = parallel_crack(target_hash, wordlist, hash_func, tc);
            auto end_par = std::chrono::high_resolution_clock::now();
            double par_time = std::chrono::duration<double>(end_par - start_par).count();

            std::cout << "Parallel (" << tc << " threads) result: " << par_result
                      << ", Time: " << par_time << "s\n";
        }

        std::cout << "==============================\n\n";
    }

    return 0;
}
