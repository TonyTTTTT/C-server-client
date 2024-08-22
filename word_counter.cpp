#include <iostream>
#include <fstream>
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

#include "word_counter.h"

using namespace std;

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

vector<string> parallel_tokenize(string &str) {
    vector<string> words;
    vector<string> local_words[omp_get_max_threads()];

    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        string local_str;

        size_t chunk_size = str.size() / omp_get_num_threads();
        size_t start = thread_id * chunk_size;
        size_t end = thread_id == omp_get_num_threads()-1 ? str.size() : start + chunk_size;

        while (start>0 && str[start-1] != ' ') start++;

        while (end<str.size() && str[end] != ' ') end++;

        local_str = str.substr(start, end-start);
        istringstream stream(local_str);
        string word;

        while (stream >> word) {
            local_words[thread_id].push_back(word);
            cout << word << endl;
        }
    }    

    for (int i=0; i<omp_get_max_threads(); i++) {
        words.insert(words.end(), local_words[i].begin(), local_words[i].end());
    }

    return words;
}

void word_occurrence_count(string str) {
    char delimit[] = " .,";
    char *word;
    char *cstr = &str[0];
    
    word = strtok(cstr, delimit);
    #pragma omp parallel
    {
        while (word != NULL) {
            cout << word << endl;
            word = strtok(NULL, delimit);
        }
    }

    vector<int> v = {2, 3, 4, 8};
    int sum = reduce(v.begin(), v.end());

    return;
}

void word_occurrence_count(char *path) {
    string path_str(path);
    string content = read_file(path_str);

    parallel_tokenize(content);

    // cout << content << endl;
}

int main() {
    char path[] = "./directory_big/file1";
    word_occurrence_count(path);


    return 0;
}