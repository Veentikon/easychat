#include <stdio.h> // printf, fgets
#include <stdlib.h> // malloc, free, exit
#include <string.h> // strcpy, strcmp, strtok
#include <uinstd.h> // close, read, write
#include <pthread.h> // threads
#include <sys/socket.h> // socket, bind, listen, accept, connect
#include <netinet/in.h> // sockaddr_in


int main(int argc, char *argv[]) {
    printf("Hello %s!\n", argv[1]);
    return 0;
}
