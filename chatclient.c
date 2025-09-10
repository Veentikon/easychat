#include <stdio.h> // printf, fgets
#include <stdlib.h> // malloc, free, exit
#include <string.h> // strcpy, strcmp, strtok
//#include <uinstd.h> // close, read, write
#include <pthread.h> // threads
#include <sys/socket.h> // socket, bind, listen, accept, connect
#include <netinet/in.h> // sockaddr_in


int main(int argc, char *argv[]) {
    char username[256] = "Eva";   

    // Create a socket
    int netsocket;
    netsocket = socket(AF_INET, SOCK_STREAM, 0);

    // specify an adress for the socket
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(9000);
    server_address.sin_addr.s_addr = INADDR_ANY;

    int connection_status = connect(netsocket, (struct sockaddr *) &server_address, sizeof(server_address));
    if (connection_status == -1) {
        printf("There was an error making the connection to server \n\n");
    }

    // receive data from server 
    char server_response[256];
    send(netsocket, username, sizeof(username), 0);
    recv(netsocket, &server_response, sizeof(server_response), 0);
    
    printf("server %s\n", server_response);

    close(netsocket);
    return 0;
}
