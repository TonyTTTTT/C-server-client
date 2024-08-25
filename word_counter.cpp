#include <iostream>
#include <fstream>
#include <unordered_map>
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

#include "word_counter.h"

using namespace std;


unordered_map<string, int> occurrence;

string read_file(string path) {
    ifstream input_file(path);

    if (!input_file.is_open()) {
        cerr << "Error opening the file: " << path <<endl;
    }

    string res;
    string line;

    while (getline(input_file, line)) {
        res += line;
    }

    input_file.close();

    return res;
}

void parallel_tokenize_and_count(string &str, int num_of_threads) {
    int a = 0;
    
    printf("max threads: %d\n", omp_get_max_threads());
    // omp_set_dynamic(0);
    if (num_of_threads > omp_get_max_threads()) {
        num_of_threads = omp_get_max_threads();
    }
    omp_set_num_threads(num_of_threads);
    #pragma omp parallel
    {
        printf("%d threads executing!\n", omp_get_num_threads());

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
            // local_words[thread_id].push_back(word);
            // cout << word << endl;
            // #pragma omp critical
            // {
            //     occurrence[word]++;
            // }

            __sync_add_and_fetch(&occurrence[word], 1);
            // InterlockedIncrement((volatile long *)&occurrence[word]);
            // atomic<int> *cnt = &occurrence[word];
            // cnt++;
            
            // atomic_fetch_add(occurrence[word], 1);
            
        }
    }

    // for (int i=0; i<omp_get_max_threads(); i++) {
    //     words.insert(words.end(), local_words[i].begin(), local_words[i].end());
    // }

    return;
}

void print_occurrence() {
    printf("==============================\n");
    for (auto itr : occurrence) {
        printf("%s: %d\n", itr.first.c_str(), itr.second);
    }
}

void tokenize_and_count(string &str) {
    static unordered_map<string, int> occurrence;
    // vector<string> words;

    istringstream stream(str);
    string word;

    while (stream >> word) {
        // words.push_back(word);
        // cout << word << endl;
        occurrence[word]++;
    }

    return;
}

// void word_occurrence_count(string str) {
//     char delimit[] = " .,";
//     char *word;
//     char *cstr = &str[0];
    
//     word = strtok(cstr, delimit);
//     #pragma omp parallel
//     {
//         while (word != NULL) {
//             cout << word << endl;
//             word = strtok(NULL, delimit);
//         }
//     }

//     vector<int> v = {2, 3, 4, 8};
//     int sum = reduce(v.begin(), v.end());

//     return;
// }



void word_occurrence_count(char *path, int num_of_threads) {
    string path_str(path);
    string content = read_file(path_str);

    auto startTime = std::chrono::high_resolution_clock::now();
    tokenize_and_count(content);
    auto endTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> serialDuration = endTime - startTime;

    startTime = std::chrono::high_resolution_clock::now();
    parallel_tokenize_and_count(content, num_of_threads);
    endTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> parallelDuration = endTime - startTime;

    std::cout << "Serial duration: " << serialDuration.count() << " seconds" << std::endl; 
    std::cout << "Parallel duration: " << parallelDuration.count() << " seconds" << std::endl; 
    std::cout << "Speedup: " << serialDuration.count() / parallelDuration.count() << std::endl; 
}

// int main() {
//     char path[] = "./directory_big/file1";
//     string path_str(path);
//     string content = read_file(path_str);

//     // word_occurrence_count(path);
//     // word_occurrence_count("./directory_big/file1");

//     auto startTime = std::chrono::high_resolution_clock::now();
//     tokenize_and_count(content);
//     auto endTime = std::chrono::high_resolution_clock::now();
//     std::chrono::duration<double> serialDuration = endTime - startTime;

//     startTime = std::chrono::high_resolution_clock::now();
//     parallel_tokenize_and_count(content);
//     endTime = std::chrono::high_resolution_clock::now();
//     std::chrono::duration<double> parallelDuration = endTime - startTime;

//     // std::cout << "Serial result: " << resSerial << std::endl;
//     // std::cout << "Parallel result: " << resParallel << std::endl; 
//     std::cout << "Serial duration: " << serialDuration.count() << " seconds" << std::endl; 
//     std::cout << "Parallel duration: " << parallelDuration.count() << " seconds" << std::endl; 
//     std::cout << "Speedup: " << serialDuration.count() / parallelDuration.count() << std::endl; 
    
//     // print_occurrence();
//     print_occurrence();
//     return 0;
// }