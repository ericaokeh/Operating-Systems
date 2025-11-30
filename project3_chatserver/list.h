#ifndef LIST_H
#define LIST_H

#include <stdbool.h>

// ===== USER LIST =====
struct node {
    char username[30];
    int  socket;
    struct node *next;
};

// room membership node
struct roomMember {
    struct node *user;
    struct roomMember *next;
};

// room
struct room {
    char name[30];
    struct roomMember *members;
    struct room *next;
};

// direct message connection (undirected)
struct dm {
    struct node *u1;
    struct node *u2;
    struct dm *next;
};

// from server.c — used for locking inside list.c
extern int numReaders;
extern pthread_mutex_t rw_lock;
extern pthread_mutex_t mutex;

// USER list helpers
struct node* insertFirstU(struct node *head, int socket, char *username);
struct node* findU(struct node *head, char* username);
struct node* findBySocket(struct node *head, int socket);
struct node* deleteBySocket(struct node *head, int socket);

// ROOM helpers
struct room* findRoom(struct room *rooms, const char *name);
struct room* createRoom(struct room **rooms, const char *name);
void addUserToRoom(struct room **rooms, struct node *user, const char *roomname);
void removeUserFromRoom(struct room *rooms, struct node *user, const char *roomname);
bool usersShareRoom(struct room *rooms, struct node *a, struct node *b);

// DM helpers
void connectUsers(struct dm **dms, struct node *a, struct node *b);
void disconnectUsers(struct dm **dms, struct node *a, struct node *b);
bool areDMConnected(struct dm *dms, struct node *a, struct node *b);

// list -> text helpers
void buildUserListBuffer(struct node *head, char *buf, size_t buflen);
void buildRoomListBuffer(struct room *rooms, char *buf, size_t buflen);

// cleanup for shutdown
void freeAllUsers(struct node *head);
void freeAllRooms(struct room *rooms);
void freeAllDMs(struct dm *dms);

#endif
