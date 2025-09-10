#include <stdio.h> // printf, fgets
#include <stdlib.h> // malloc, free, exit

#include <sys/types.h>
#include <sys/socket.h> // socket, bind, listen, accept, connect
#include <netinet/in.h> // sockaddr_in

#include <string.h> // strcpy, strcmp, strtok
//#include <uinstd.h>

#include <pthread.h> // threads


int main(int argc, char *argv[]) {
    printf("Hello, server is running...\n");    
    char server_message[256] = "you have reached the server";

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

    printf("%d\n", client_socket);
    send(client_socket, server_message, sizeof(server_message), 0);

    close(server_socket);

    return 0;
}
