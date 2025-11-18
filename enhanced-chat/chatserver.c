#include "chatroom.h"

#include <stdio.h> // printf, fgets
#include <stdlib.h> // malloc, free, exit
#include <errno.h> // error messages and stuff

#include <sys/types.h>
#include <sys/socket.h> // socket, bind, listen, accept, connect
#include <netinet/in.h> // sockaddr_in

#include <string.h> // strcpy, strcmp, strtok
#include <unistd.h>
//#include <pthread.h> // threads


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
    strncpy(users[user_count].username, username, USERNAME_LEN -1);
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
    int n_bytes = 0;
    strcat(buffer, from_user);
    n_bytes += strlen(from_user);
    strcat(buffer, ": ");
    strcat(buffer, msg);
    n_bytes += strlen(msg); // ===================================================== Error?
    n_bytes += 3; // add all the previous characters to length
    for (int i = 0; i < user_count; i ++) {
        if (users[i].socket != from_socket) {
            send(users[i].socket, buffer, n_bytes, 0);
        }
    }
    pthread_mutex_unlock(&user_mutex);
}


// Send a notification to a user such as when other user joins or leaves the chat
void notify_users(const char *msg, int from_socket, char *from_user) {
    pthread_mutex_lock(&user_mutex);
    char buffer[256] = "\033[1;31m";
    int n_bytes = 10;
    strcat(buffer, from_user);
    n_bytes += strlen(from_user);
    strcat(buffer, " ");
    n_bytes += 1;
    strcat(buffer, msg);
    n_bytes += strlen(msg) + 7;
    strcat(buffer, "\033[0m");
    buffer[n_bytes] = '\0';
    
    for (int i = 0; i < user_count; i ++) {
        if (users[i].socket != from_socket) {
	    send(users[i].socket, buffer, n_bytes, 0);
	}
    }
    pthread_mutex_unlock(&user_mutex);
}

// Send a list of active users to the requesting user
void list_users(int client_socket) {
    pthread_mutex_lock(&user_mutex);
    char buffer[256] = "online users: ";

    // make sure to send only n_bytes of data, not the whole buffer   
    int n_bytes = 14;
    for (int i = 0; i < user_count; i ++) {
        strcat(buffer, users[i].username);
	strcat(buffer, " | ");
	n_bytes = n_bytes + sizeof(users[i].username) + 3;
    }
    strcat(buffer, "\0"); // =============================
    send(client_socket, buffer, n_bytes + 1, 0);
    pthread_mutex_unlock(&user_mutex);
}

// Test whether a user with specified username already exists
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
    User *user = (User *)arg;
    char client_message[256]; // full client message
    char command[32]; // command received
    char *message; // the message if user wants to send it

    char users[6] = "users";
    char write[6] = "write";
    char exit[5] = "exit";
    char joined[7] = "joined";
    char left[5] = "left";

    notify_users(joined, user->socket, user->username); // notify others that this user joined the chat

    while(1) { // continuously read and serve client requests

        int n_bytes = recv(user->socket, &client_message, sizeof(client_message), 0);
        //printf("%s\n", client_message); // ==================================================
	if (n_bytes > 0) { // socket live, message received
            client_message[n_bytes] = '\0'; //ensure null termination
        } else if (n_bytes == 0) { // peer closed connection
	    remove_user(user->socket);
	    free(user);
	    return 0;
	} else if (n_bytes == -1) {
	    if (errno == EINTR) {
		// Interrupted by signal, retry
		continue;
	    } else {
		perror("recv");
		notify_users(left, user->socket, user->username);
		remove_user(user->socket);
		free(user);
		return 0;
	    }
	}

	// extract the command
	char *space = strchr(client_message, ' ');
	if (space != NULL) {
	    size_t len = space - client_message;
	    strncpy(command, client_message, len);
	    command[len] = '\0';

	    // remove trailing newline
	    message = space + 1; // message starts after space
	    message[strcspn(message, "\n")] = '\0';
	} else {
	    strcpy(command, client_message);
	    command[strcspn(command, "\n")] = '\0';
	    message = NULL;
	}

	if (strcmp(command, exit) == 0) {
	    remove_user(user->socket);   
            send(user->socket, exit, sizeof(exit), 0);
	    notify_users(left, user->socket, user->username);
	    remove_user(user->socket);
	    free(user);
	    return 0;
        } else if (strcmp(command, users) == 0) {
	    list_users(user->socket);
	} else if (strcmp(command, write) == 0) {
	    broadcast_message(message, user->socket, user->username);
	}
    }
    free(user);
    return NULL;
}


int main(int argc, char *argv[]) {
    //struct User *chatroom[10]; // logged in users
    char client_message[256]; // client message bufferi
    char delim[10] = " ";
    pthread_t thread_id;
    int client_socket;
    
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

    while (1 == 1) { 
	client_socket = accept(server_socket, NULL, NULL);    
	recv(client_socket, &client_message, sizeof(client_message), 0); // First message should contain username

	// use strtok() to separate string into tokens, if first token is "HELLO", second token should be username
	// restricitions? username cannot be \t or \n or \0 or " ". Should be between 1 and 10 characters long
	// strtok() is fine in non-multithreaded env, otherwise use re-entrant version strtok_r()
	    
	char *username = strtok(client_message, delim);
	if (!user_exists(username)) {
		
	    int result;
	    //char buff[256] = "Hello ";
	    //strcat(buff, username);
	    add_user(username, client_socket);

	    // Create new user struct to pass to thread
	    User *new_user = malloc(sizeof(User));
	    strcpy(new_user->username, username);
	    new_user->socket = client_socket;
	    // =====================================
	   
	    result = pthread_create(&thread_id, NULL, serve_thread, (void *) new_user);
	    send(client_socket, username, sizeof(username), 0);// send back username confirming successful login
	    if (result != 0) {
	        fprintf(stderr, "Error creating thread: %d\n", result);
	        return 1;
	    }
	} else {
	    char msg[256] = "Error, this user already exists\n";
	    send(client_socket, msg, sizeof(msg), 0);
	}
    }
   
    //pthread_join(thread_id, NULL); // wait for the new thread to finish
    //printf("Hello from the main thread!\n");

    close(server_socket);
    return 0;
}


/*
Instead of having a separate messaging loop, what if the user have to type "write" for each message to be
broadcast

which is simpler to implement? the second option
which is easier otion to use? the first option
*/




