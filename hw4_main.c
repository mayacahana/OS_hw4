
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

#define CHUNK_SIZE 1024

void xor(char* buf1, char* buf2, int len1, int len2) {
    int length = len1 < len2 ? len1 : len2;
    for (int i=0; i<length; i++) {
        
    }
}
int main(int argc, char** argv) {
    char* output_file = argv[1];
    int num_of_files = argc - 2;
    int out_fd = open(argv[1], O_WRONLY|O_CREAT|O_TRUNC, 00777);
    if (out_fd < 0) {
        perror("ERROR: ");
    }
    int* in_fds = malloc((argc - 1)*sizeof(int));
    char buffer[CHUNK_SIZE], res[CHUNK_SIZE];
    // get the fd for all input files
    for (int i=2; i < argc; i++) {
        in_fds[i] = open(argv[i], O_RDONLY);
        if (in_fds < 0) {
            exit(1);
        }
    }
    int cur_in_fd;



}

