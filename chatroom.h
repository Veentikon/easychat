#ifndef CHATROOM_H
#define CHATROOM_H

#include <pthread.h>

#define MAX_USERS 10
#define USERNAME_LEN 32

typedef struct {
    char username[USERNAME_LEN];
    int socket;
} User;

extern User users[MAX_USERS];
extern int user_count;
extern pthread_mutex_t user_mutex;

int add_user(const char *username, int socket);
void remove_user(int socket);
void broadcast_message(const char *msg, int from_socket, char *from_user);
void list_users(int client_socket);

#endif
