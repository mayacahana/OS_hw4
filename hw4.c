
/*
 *  hw4.c
 *
 *  Created on: Dec 27, 2017
 *  Author: Maya Cahana
 * 
*/

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <fcntl.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/stat.h>


#define CHUNK_SIZE 1024000

// Global parameters
// locks
pthread_mutex_t buffer_mutex, num_of_offline;
int output_fd;
pthread_cond_t finish_cv;
char result_buffer[CHUNK_SIZE];

// Helpers functions
// * assume length is the minimal size of these two buffers
void xor_op(char* buffer_1,char* buffer_2,char* out_buffer, int length){
    while (length--){
        *out_buffer++ = *buffer_1++ ^ *buffer_2++;
    }
}

int get_size_file(int file_desc) {
    struct stat st;
    if (fstat(file_desc, &st) == -1) {
        perror("ERROR: fstat failed");
        return -1;
    }
    return(st.st_size);
}

int main(int argc, char** argv) {
    char* out_file = argv[1];
    int i, active_threads;
    pthread_t *threads;
    int num_input_files = argc - 2;
    // init current active threads count
    active_threads = num_input_files;
    char buffer[CHUNK_SIZE], res[CHUNK_SIZE];
    int out_fd = open(argv[1], O_WRONLY|O_CREAT|O_TRUNC, 00777);
    if (out_fd < 0){
        perror("ERROR: ");
        exit(-1);
    }
    // init the buffer
    memset(result_buffer, 0, sizeof(result_buffer));

}