#include "chatroom.h"

#include <stdio.h> // printf, fgets
#include <stdlib.h> // malloc, free, exit

#include <sys/types.h>
#include <sys/socket.h> // socket, bind, listen, accept, connect
#include <netinet/in.h> // sockaddr_in

#include <string.h> // strcpy, strcmp, strtok
//#include <uinstd.h>


//#include <pthread.h> // threads


// should I implement chat for only 2 people or for more?
// if more, then I need a proper structure
// a simple array could work if the expected number of users is not too large

User users[MAX_USERS]; // Record of active users
int user_count = 0;
pthread_mutex_t user_mutex = PTHREAD_MUTEX_INITIALIZER;

// Add user if user with given username does not yet exist
// and on failure return -1
int add_user(const char *username, int socket) {
    pthread_mutex_lock(&user_mutex);
    if (user_count >= MAX_USERS) { // cannot add new user, limit reached
        pthread_mutex_unlock(&user_mutex);
        return -1; 
    }
    strncpy(users[user_count].username, username, USERNAME_LEN -1 );
    users[user_count].socket = socket;
    user_count ++;
    pthread_mutex_unlock(&user_mutex);
    return 0;
}

void remove_user(int socket) {
    pthread_mutex_lock(&user_mutex);
    for (int i = 0; i < user_count; i ++) {
        if (users[i].socket == socket) {
            users[i] = users[user_count -1]; // swap this user with last user since we cannot just delete
            user_count --;
            break;
        }
    }
    pthread_mutex_unlock(&user_mutex);
}

// Broadcast message to all users but the sender
void broadcast_message(const char *msg, int from_socket, char *from_user) {
    pthread_mutex_lock(&user_mutex);
    char buffer[1024] = "";
    strcat(buffer, from_user);
    strcat(buffer, msg);
    strcat(buffer, "\n");
    for (int i = 0; i < user_count; i ++) {
        if (users[i].socket != from_socket) {
            send(users[i].socket, buffer, sizeof(buffer), 0);
        }
    }
    pthread_mutex_unlock(&user_mutex);
}

// Send a list of active users to the requesting user
void list_users(int client_socket) {
    pthread_mutex_lock(&user_mutex);
    char buffer[1024] = "";
    for (int i = 0; i < user_count; i ++) {
        strcat(buffer, users[i].username);
        strcat(buffer, "\n");
    }
    send(client_socket, buffer, sizeof(buffer), 0);
    pthread_mutex_unlock(&user_mutex);
}

int user_exists(char *username) {
    pthread_mutex_lock(&user_mutex);
    for (int i = 0; i < user_count; i ++) {
        if (strcmp(users[i].username, username) == 0) { // strcmp returns 0 if they are identical
            pthread_mutex_unlock(&user_mutex);
            return 1;
        }
    }
    pthread_mutex_unlock(&user_mutex);
    return 0;
}


// thread that communicates with the user
// TODO: Finish implementation
void *serve_thread(void *arg) {
    char msg[256] = "Hello from the child thread";
    int* client_socket = (int *) arg; 
    printf("thread created, sending message on socket: %d\n", *client_socket);
    send(*client_socket, msg, sizeof(msg), 0);

    return NULL;
}


int main(int argc, char *argv[]) {
    struct user * chatroom[10]; // logged in users
    char server_message[256]; // server message buffer
    char client_message[256]; // client message bufferi
    char delim[10] = " ";
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
    
    recv(client_socket, &client_message, sizeof(client_message), 0); // First message should contain username

    // TODO: set up hanshake proceure, recv HELLO <username>, send, Welcome <username>
    // Need a way to parse arguments. This is a custom "handshake" to confirm successful user "login"
    // Only create a new user if the username was not taken
    // use strtok() to separate string into tokens, if first token is "HELLO", second token should be username
    // restricitions? username cannot be \t or \n or \0 or " ". Should be between 1 and 10 characters long
    // strtok() is fine in non-multithreaded env, otherwise use re-entrant version strtok_r()
    
    char *username = strtok(client_message, delim);
    if (!user_exists(username)) {
        int result;
	char buff[256] = "Hello ";
	strcat(buff, username);
        
	send(client_socket, buff, sizeof(buff), 0);
        result = pthread_create(&thread_id, NULL, serve_thread, (void *) &client_socket);
        if (result != 0) {
            fprintf(stderr, "Error creating thread: %d\n", result);
            return 1;
        }
    } else {
        char msg[256] = "Error, this user already exists\n";
        send(client_socket, msg, sizeof(msg), 0);
    }
    
    pthread_join(thread_id, NULL); // wait for the new thread to finish
    printf("Hello from the main thread!\n");

    close(server_socket);
    return 0;
}

