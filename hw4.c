
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

void* thread_work(void* file_path) {
    
}

int main(int argc, char** argv) {
    char* out_file = argv[1];
    int i, active_threads, rc, rc_offline, rc_cv, out_size;
    pthread_t *threads;
    int num_input_files = argc - 2;
    // init current active threads count
    active_threads = num_input_files;
    char buffer[CHUNK_SIZE], res[CHUNK_SIZE];
    printf("Hello, creating %s from %d input files\n", out_file, num_input_files);
    int out_fd = open(argv[1], O_WRONLY|O_CREAT|O_TRUNC, 00777);
    if (out_fd < 0){
        perror("ERROR: ");
        exit(-1);
    }
    // init the buffer
    memset(result_buffer, 0, sizeof(result_buffer));
    // --- Initialize mutex  and condition variable objects -----------
    rc = pthread_mutex_init(&buffer_mutex, NULL);
    if (rc) {
        perror("ERROR in pthread_mutex_init");
        exit(-1);
    }
    rc_offline = pthread_mutex_init(&num_of_offline, NULL);
    if (rc_offline) {
        perror("ERROR in pthread_mutex_init");
        exit(-1);
    }
    rc_cv = pthread_cond_init(&finish_cv, NULL);
    if (rc_cv) {
        perror("Error in cv init");
        exit(-1);
    }
    // ----- init threads ------
    threads = malloc(num_input_files * sizeof(pthread_t*));
    if (!threads) {
        perror("Error in malloc threads");
        exit(-1);
    }
    // --- create reader thread for every input file ---- 
    for (int j=2; j < argc ; j++) {
        if (pthread_create(&threads[j-2], NULL, thread_work, argv[i]) != 0) {
            perror("Error creating threads");
            exit(-1);
        }
    }
    // --- Wait for all threads to complete --- 
    for (int k=0; k < num_input_files; k++) {
        if (pthread_join(threads[i], NULL) != 0) {
            perror("Error joining threads");
            exit(-1);
        }
    }
    // --- check ouput file
    out_size = get_size_file(output_fd);
    if (out_size < 0) {
        perror("ERROR:");
        exit(-1);
    }
    if (close(output_fd) < 0) {
        perror("ERROR:");
        exit(-1);
    }
    prinf("Created %s with size %d bytes\n", out_file, out_size);
    // --- Clean up and exit ---- 
    free(threads);
    if (pthread_mutex_destroy(&buffer_mutex) != 0){
        perror("Error destoy mutex");
        exit(-1);
    }
    if (pthread_mutex_destroy(&num_of_offline) != 0){
        perror("Error destroy mutex");
        exit(-1);
    }
    if (pthread_cond_destroy(&finish_cv) != 0) {
        perror("Error destroy cv");
        exit(-1);
    }
    return 0; 
}