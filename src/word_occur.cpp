#include <iostream>
#include <fstream>
#include <unordered_map>
#include <map>
#include <string>
#include <string.h>
#include <sstream>
#include <omp.h>
#include <algorithm>
#include <execution>
#include <numeric>
#include <vector>
#include <omp.h>
#include <chrono>
#include <atomic>
#include <filesystem>

#include "word_occur.h"
#include "util.h"

using namespace std;

string read_file_to_str(string path);
void prune_word(string& word);
void parallel_tokenize_and_count(string &str, int num_of_threads);
void serial_tokenize_and_count(string &str);
void print_occurrence();

static unordered_map<string, int> occurrence;
static unordered_map<string, int> occurrence_for_serial;

void word_occurrence_count(char *path, int num_of_threads) {
    string path_str(path);
    string content = read_file_to_str(path_str);
    
    std::chrono::time_point<std::chrono::high_resolution_clock> startTime;
    std::chrono::time_point<std::chrono::high_resolution_clock> endTime;
    std::chrono::duration<double> serialDuration;
    if (debug) {
        startTime = std::chrono::high_resolution_clock::now();
        serial_tokenize_and_count(content);
        endTime = std::chrono::high_resolution_clock::now();
        serialDuration = endTime - startTime;
    }

    startTime = std::chrono::high_resolution_clock::now();
    parallel_tokenize_and_count(content, num_of_threads);
    endTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> parallelDuration = endTime - startTime;

    if (debug) {
        print_debug("Serial execute duration: %f seconds\n", serialDuration.count());
        print_debug("Parallel execute duration: %f seconds\n", parallelDuration.count()); 
        print_debug("Speedup: %f\n", serialDuration.count() / parallelDuration.count()); 
    }
}

string read_file_to_str(string path) {
    ifstream input_file(path);
    if (!input_file.is_open()) {
        cerr << "Error opening the file: " << path <<endl;
    }

    string res;
    string line;

    while (getline(input_file, line)) {
        res += line;
        res += " ";
    }

    input_file.close();

    return res;
}

void prune_word(string& word) {
    // for (auto& c : word) c = tolower(c);

    while (!isalpha(word.back())) {
        word.pop_back();
        if (word.empty()) return;
    }

    int start = 0;
    while (!isalpha(word[start])) start++;
    word = word.substr(start);
}

void parallel_tokenize_and_count(string &str, int num_of_threads) {
    omp_set_dynamic(0);
    if (num_of_threads > omp_get_max_threads()) num_of_threads = omp_get_max_threads();
    omp_set_num_threads(num_of_threads);

    #pragma omp parallel
    {
        print_debug("thread %d executing!\n", omp_get_thread_num());

        int thread_id = omp_get_thread_num();
        string local_str;

        size_t chunk_size = str.size() / omp_get_num_threads();
        size_t start = thread_id * chunk_size;
        size_t end = thread_id == omp_get_num_threads()-1 ? str.size() : start + chunk_size;

        while (start>0 && str[start-1] != ' ') start++;

        while (end<str.size() && str[end-1] != ' ') end++;

        local_str = str.substr(start, end-start);
        istringstream stream(local_str);
        string word;
    
        while (stream >> word) {
            prune_word(word);

            if (word.empty()) continue;

            // critical code for syncronization
            if (occurrence.count(word)) {
                auto addr = &occurrence[word];
                __sync_add_and_fetch(addr, 1);
            } else {
                // if word haven't occur in map, using __sync_add_and_fetch() will crash the map
                #pragma omp critical
                {
                    auto itr = occurrence.insert({word, 1});
                    // there might be case when one thread enter this critical region right 
                    // after another one with the same word, so we still need to check if 
                    // word already exist in occurrence in this case, we still need to add it by one
                    if (!itr.second) occurrence[word]++;
                }
            }
        }
    }
}

void serial_tokenize_and_count(string &str) {
    istringstream stream(str);
    string word;

    while (stream >> word) {
        prune_word(word);

        if (word.empty()) continue;

        occurrence_for_serial[word]++;
    }
}

void print_occurrence() {
    printf("%sWord Occurrence among all files:\n%s", SEP_LINE, SEP_LINE);

    map<string, int> ordered_occurrence(occurrence.begin(), occurrence.end());
    for (auto itr : ordered_occurrence) {
        printf("%s: %d\n", itr.first.c_str(), itr.second);
    }

    occurrence.clear();
    occurrence_for_serial.clear();
}