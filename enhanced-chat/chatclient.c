#include <stdio.h> // printf, fgets
#include <stdlib.h> // malloc, free, exit
#include <string.h> // strcpy, strcmp, strtok
//#include <uinstd.h> // close, read, write
#include <pthread.h> // threads
#include <sys/socket.h> // socket, bind, listen, accept, connect
#include <netinet/in.h> // sockaddr_in


//TODO 1 (in separatate method)
// Create thread to listen for server messages
// All it does is run a while true loop and repeatedly call recv()

//TODO 2 (in main function)
// Create a REPL to read, evaluate, print in a loop
// User input support: users, write, options, exit

//TODO 3
// Read username from argv, validate and send it in a message to the server.

void *listen_thread(void *arg) {
    int *server_socket = (int *) arg;
    char server_message[1024];
    char exit[5] = "exit";

    while (1) {
        //int res = poll(*server_socket)
	int n_bytes = recv(*server_socket, &server_message, sizeof(server_message), 0); 	
	   
        if (n_bytes > 0 && server_message[n_bytes-1] == '\n') {
            server_message[n_bytes-1] = '\0';
        } else if (n_bytes == 0) { // peer closed the connection
	    printf("Server closed the connection\n");
	    return 0;
	} else if (n_bytes == -1) {
	    printf("Socket error occured, exiting...\n");
	    return 0;
	}
	
	printf("%s\n", server_message);

        if (strcmp(server_message, exit) == 0) {
            return 0;
        }
    }
    return 0;
}


int main(int argc, char *argv[]) {
    // Valiate user input
    if (argc != 2) {
        printf("Error: expected 1 argument <username>, received %d\n", argc);
        return -1;
    }
 
    char *username = argv[1];
    char user_input[1024];
    char command[32];
    pthread_t thread_id;   
 
    // User input options. I could keem tham in an array instead.
    char exit[5] = "exit"; // How to properly terminate child thread?
    char options[8] = "options";
    char write[6] = "write";
    char users[6] = "users";
    
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

    // Perform handshake with the server
    char server_response[256];
    send(netsocket, username, sizeof(username), 0);
    recv(netsocket, &server_response, sizeof(server_response), 0);
   
    // If handshake failed, stop
    if (strcmp(server_response, username) != 0) {
	fprintf(stderr, "%s\n", server_response);
	return -1;
    }

    printf("Welcome %s\n", username);

    // Initialize listener thread to listen and print server responses/messages
    int result = pthread_create(&thread_id, NULL, listen_thread, (void *) &netsocket);
    if (result != 0) {
        fprintf(stderr, "Error creating thread: %d\n", result);
    	return 1;
    }
    
    
    // REPL
    while (1) {
	fgets(user_input, sizeof(user_input), stdin);
	int length = strlen(user_input); // relevant bytes to be sent
	
	// find first space ===================
	char *space = strchr(user_input, ' ');
	if (space != NULL) {
	    size_t len = space - user_input;
	    strncpy(command, user_input, len);
            command[len] = '\0';
	    
	} else {
	    strcpy(command, user_input);
	    command[strcspn(command, "\n")] = '\0';
	}
	// ====================================


        // Remove trailing newline character
        if (length > 0 && user_input[length-1] == '\n') {
            user_input[length-1] = '\0';
        }

	// Test input command and send it to the server
	if (strcmp(user_input, exit) == 0) {
            printf("Bye %s\n", username);
            send(netsocket, exit, sizeof(exit), 0);
	    break; // Exit the loop
        } else if (strcmp(command, options) == 0) {
            printf("Options: options, write, users, exit\n");
        } else if (strcmp(command, write) == 0) {
            send(netsocket, user_input, sizeof(user_input), 0);
        } else if (strcmp(command, users) == 0) {
            send(netsocket, users, sizeof(users), 0);
        } else {
            printf("Unexpected input\n");
        }
    } 
    
    // Wait for thread to finish, clean up socket and exit
    printf("waiting for child thread to finish ...\n");
    pthread_join(thread_id, NULL);
    close(netsocket);
    return 0;
}
