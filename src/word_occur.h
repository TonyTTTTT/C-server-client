#ifndef WORD_OCCUR_H
#define WORD_OCCUR_H

#ifdef __cplusplus
extern "C" {
#endif
void word_occurrence_count(char *path, int num_of_threads);
void print_occurrence();
#ifdef __cplusplus
}
#endif

#endif