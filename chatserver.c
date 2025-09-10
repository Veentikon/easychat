#include <stdio.h> // printf, fgets
#include <stdlib.h> // malloc, free, exit

#include <sys/types.h>
#include <sys/socket.h> // socket, bind, listen, accept, connect
#include <netinet/in.h> // sockaddr_in

#include <string.h> // strcpy, strcmp, strtok
//#include <uinstd.h>

#include <pthread.h> // threads


void *serve_thread(void * arg) {
    char msg[256] = "Hello from the new thread\n";
    int* client_socket = (int *) arg; 
    printf("thread created, sending message on socket: %d\n", *client_socket);
    send(*client_socket, msg, sizeof(msg), 0);

    return NULL;
}


int main(int argc, char *argv[]) {
    char server_message[256]; // server message buffer
    char client_message[256]; // client message buffer
    pthread_t thread_id;
    
    // create server socket
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);

    // define the server address structure
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(9000);
    server_address.sin_addr.s_addr = INADDR_ANY;

    // bind the socket to our specified IP and port
    bind(server_socket, (struct sockaddr*) &server_address, sizeof(server_address));

    printf("socket bound and listening for incoming connections...\n");
    listen(server_socket, 4);

    int client_socket;
    client_socket = accept(server_socket, NULL, NULL);
    recv(client_socket, &client_message, sizeof(client_message), 0);

    // TODO: test username uniqueness, if unique :
    // create entry with socket and username and spawn a thread to handle the connection
    // If not, send an error message
    if (1 == 1) {
        
        int result;
        result = pthread_create(&thread_id, NULL, serve_thread, (void *) &client_socket);
        if (result != 0) {
            fprintf(stderr, "Error creating thread: %d\n", result);
            return 1;
        }
    }
    // TODO: send error message if username is taken
    //send(client_socket, server_message, sizeof(server_message), 0);
    
    pthread_join(thread_id, NULL); // wait for the new thread to finish

    printf("Hello from the main thread!\n");

    close(server_socket);

    return 0;
}

