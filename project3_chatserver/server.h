#ifndef SERVER_H
#define SERVER_H

/* System Header Files */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <signal.h>
#include <sys/wait.h>
#include <netdb.h>
#include <ctype.h>
#include <pthread.h>

#include "list.h"

#define TRUE          1
#define FALSE         0
#define PORT          8888
#define BACKLOG       2
#define MAXBUFF       2096
#define DEFAULT_ROOM  "Lobby"
#define delimiters    " "

// global server socket
extern int chat_serv_sock_fd;

// reader/writer sync
extern int numReaders;
extern pthread_mutex_t mutex;      // protects numReaders
extern pthread_mutex_t rw_lock;    // writer lock

// global lists
extern struct node *head;          // user list
extern struct room *rooms_head;    // room list
extern struct dm   *dms_head;      // DM list

// server text
extern char const *server_MOTD;

// prototypes
int  get_server_socket(void);
int  start_server(int serv_socket, int backlog);
int  accept_client(int serv_sock);
void sigintHandler(int sig_num);
void *client_receive(void *ptr);

#endif
