
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
#define SUCCESS 0
#define ERROR -1

void xor_op(char* buffer_1,char* buffer_2, int length);
int get_size_file(int file_desc);
void* thread_work(void* in_file_path);
// Global parameters
// locks
pthread_mutex_t buffer_mutex, num_of_offline;
pthread_cond_t finish_cv;
char result_buffer[CHUNK_SIZE];
int output_fd, count_curr_offline,num_curr_active, xor_cnt, total_threads_read=0;

// Helpers functions
// * assume length is the minimal size of these two buffers
void xor_op(char* buffer_1,char* buffer_2, int length){
    while (length--){
        buffer_1++;
        *buffer_1 = *buffer_1 ^ *buffer_2++;
    }
}

int get_size_file(int file_desc) {
    struct stat st;
    if (fstat(file_desc, &st) == -1) {
        perror("ERROR: fstat failed");
        exit(-1);
    }
    return(st.st_size);
}

void* thread_work(void* in_file_path) {
    char* file_path = (char *)in_file_path;
    int total, num_read, size_file, fd;
    char buffer[CHUNK_SIZE];
    fd = open(file_path, O_RDONLY);
    if (fd < 0) {
        perror("Error while opening input file");
        exit(-1);
    }
    size_file = get_size_file(fd);
    if (size_file < 0) {
        perror("Error file size");
        exit(-1);
    }
    total = 0;
    bool flag = true;
    while (flag) {
        // read 1024kb from the file
        num_read = read(fd, buffer, CHUNK_SIZE);
        if (num_read < 0) {
            perror("Error with read");
            exit(-1);
        }
        total += num_read;
        // check if we end the file! 
        if (total == size_file) {
            if (pthread_mutex_lock(&num_of_offline) != 0) {
                perror("Error lock");
                exit(-1);
            }
            count_curr_offline++;
            if (pthread_mutex_unlock(&num_of_offline) != 0){
                perror("Error unlock");
                exit(-1);
            }
            flag=false;
        }
        //=======Critical section===============
        if (pthread_mutex_lock(&buffer_mutex) != 0) {
            perror("Error mutex lock");
            exit(-1);
        }
        xor_op(result_buffer, buffer,num_read);
        xor_cnt++;
        // cnt num of bytes read in any thread
        if (num_read > total_threads_read) {
            total_threads_read = num_read;
        }
        if (xor_cnt != num_curr_active) { // NOT last thread
            if (pthread_cond_wait(&finish_cv, &buffer_mutex) != 0){
                perror("ERROR:");
                exit(-1);
            }
        } else {
            if (write(output_fd, result_buffer, total_threads_read) < 0){
                perror("ERROR:");
                exit(-1);
            }
            // init variables
            num_curr_active -= count_curr_offline;
            count_curr_offline = 0;
            xor_cnt=0;
            total_threads_read = 0;
            memset(result_buffer, 0, sizeof(result_buffer));
            // broadcasting
            if (pthread_cond_broadcast(&finish_cv) != 0) {
                perror("ERROR:");
                exit(-1);  
            }
        }
        if(pthread_mutex_unlock(&buffer_mutex) != 0){
            perror("ERROR:");
            exit(-1);
        }
    }
    //=======Critical section===============
    if (close(fd) < 0){
        perror("ERROR:");
        exit(-1);
    }
    //exit thread!
    pthread_exit(NULL);

}


int main(int argc, char** argv) {
    char* out_file = argv[1];
    int rc, rc_offline, rc_cv, out_size;
    pthread_t *threads;
    num_curr_active = argc - 2;
    // init current active threads count
    // char buffer[CHUNK_SIZE], res[CHUNK_SIZE];
    printf("Hello, creating %s from %d input files\n", out_file, num_curr_active);
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
    threads = malloc((argc-2) * sizeof(pthread_t*));
    if (!threads) {
        perror("Error in malloc threads");
        exit(-1);
    }
    // --- create reader thread for every input file ---- 
    for (int j=2; j < argc ; j++) {
        if (pthread_create(&threads[j-2], NULL, thread_work, argv[j]) != 0) {
            perror("Error creating threads");
            exit(-1);
        }
    }
    // --- Wait for all threads to complete --- 
    for (int k=0; k < argc-2; k++) {
        if (pthread_join(threads[k], NULL) != 0) {
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
    printf("Created %s with size %d bytes\n", out_file, out_size);
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
    exit(0); 
}