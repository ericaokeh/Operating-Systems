#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "list.h"

// external locks
extern int numReaders;
extern pthread_mutex_t rw_lock;
extern pthread_mutex_t mutex;

// ----- internal helpers for RW-locking around lists -----
static void start_read() {
    pthread_mutex_lock(&mutex);
    numReaders++;
    if (numReaders == 1) {
        pthread_mutex_lock(&rw_lock);
    }
    pthread_mutex_unlock(&mutex);
}

static void end_read() {
    pthread_mutex_lock(&mutex);
    numReaders--;
    if (numReaders == 0) {
        pthread_mutex_unlock(&rw_lock);
    }
    pthread_mutex_unlock(&mutex);
}

static void start_write() {
    pthread_mutex_lock(&rw_lock);
}

static void end_write() {
    pthread_mutex_unlock(&rw_lock);
}

/********************  USER LIST  *************************/

//insert node at the first location (only if username not duplicate)
struct node* insertFirstU(struct node *head, int socket, char *username) {
    start_write();

    // check duplicate
    struct node *cur = head;
    while (cur) {
        if (strcmp(cur->username, username) == 0) {
            printf("Duplicate: %s\n", username);
            end_write();
            return head;
        }
        cur = cur->next;
    }

    struct node *link = (struct node*) malloc(sizeof(struct node));
    link->socket = socket;
    strncpy(link->username, username, sizeof(link->username));
    link->username[sizeof(link->username)-1] = '\0';

    link->next = head;
    head = link;

    end_write();
    return head;
}

//find a node with given username
struct node* findU(struct node *head, char* username) {
    start_read();
    struct node* current = head;

    while (current != NULL) {
        if (strcmp(current->username, username) == 0) {
            end_read();
            return current;
        }
        current = current->next;
    }

    end_read();
    return NULL;
}

struct node* findBySocket(struct node *head, int socket) {
    start_read();
    struct node *current = head;
    while (current != NULL) {
        if (current->socket == socket) {
            end_read();
            return current;
        }
        current = current->next;
    }
    end_read();
    return NULL;
}

struct node* deleteBySocket(struct node *head, int socket) {
    start_write();
    struct node **pp = &head;
    while (*pp) {
        if ((*pp)->socket == socket) {
            struct node *tmp = *pp;
            *pp = (*pp)->next;
            free(tmp);
            break;
        }
        pp = &(*pp)->next;
    }
    end_write();
    return head;
}

/********************  ROOMS  *************************/

struct room* findRoom(struct room *rooms, const char *name) {
    struct room *r = rooms;
    while (r) {
        if (strcmp(r->name, name) == 0) return r;
        r = r->next;
    }
    return NULL;
}

struct room* createRoom(struct room **rooms, const char *name) {
    start_write();
    struct room *existing = findRoom(*rooms, name);
    if (existing) {
        end_write();
        return existing;
    }

    struct room *r = malloc(sizeof(struct room));
    strncpy(r->name, name, sizeof(r->name));
    r->name[sizeof(r->name)-1] = '\0';
    r->members = NULL;
    r->next = *rooms;
    *rooms = r;

    end_write();
    return r;
}

void addUserToRoom(struct room **rooms, struct node *user, const char *roomname) {
    if (!user) return;
    start_write();

    struct room *r = findRoom(*rooms, roomname);
    if (!r) {
        r = createRoom(rooms, roomname);  // creates under lock
    }

    // already member?
    struct roomMember *m = r->members;
    while (m) {
        if (m->user == user) {
            end_write();
            return;
        }
        m = m->next;
    }

    struct roomMember *nm = malloc(sizeof(struct roomMember));
    nm->user = user;
    nm->next = r->members;
    r->members = nm;

    end_write();
}

void removeUserFromRoom(struct room *rooms, struct node *user, const char *roomname) {
    if (!user) return;
    start_write();

    struct room *r = findRoom(rooms, roomname);
    if (!r) {
        end_write();
        return;
    }

    struct roomMember **mp = &r->members;
    while (*mp) {
        if ((*mp)->user == user) {
            struct roomMember *tmp = *mp;
            *mp = (*mp)->next;
            free(tmp);
            break;
        }
        mp = &(*mp)->next;
    }

    end_write();
}

// do users share at least one room?
bool usersShareRoom(struct room *rooms, struct node *a, struct node *b) {
    start_read();
    struct room *r = rooms;
    while (r) {
        bool hasA = false, hasB = false;
        struct roomMember *m = r->members;
        while (m) {
            if (m->user == a) hasA = true;
            if (m->user == b) hasB = true;
            m = m->next;
        }
        if (hasA && hasB) {
            end_read();
            return true;
        }
        r = r->next;
    }
    end_read();
    return false;
}

/********************  DMs  *************************/

void connectUsers(struct dm **dms, struct node *a, struct node *b) {
    if (!a || !b || a == b) return;
    start_write();

    struct dm *d = *dms;
    while (d) {
        if ((d->u1 == a && d->u2 == b) || (d->u1 == b && d->u2 == a)) {
            end_write();
            return;
        }
        d = d->next;
    }

    struct dm *nd = malloc(sizeof(struct dm));
    nd->u1 = a;
    nd->u2 = b;
    nd->next = *dms;
    *dms = nd;

    end_write();
}

void disconnectUsers(struct dm **dms, struct node *a, struct node *b) {
    if (!a || !b) return;
    start_write();

    struct dm **pp = dms;
    while (*pp) {
        if (((*pp)->u1 == a && (*pp)->u2 == b) ||
            ((*pp)->u1 == b && (*pp)->u2 == a)) {
            struct dm *tmp = *pp;
            *pp = (*pp)->next;
            free(tmp);
            break;
        }
        pp = &(*pp)->next;
    }

    end_write();
}

bool areDMConnected(struct dm *dms, struct node *a, struct node *b) {
    start_read();
    struct dm *d = dms;
    while (d) {
        if ((d->u1 == a && d->u2 == b) ||
            (d->u1 == b && d->u2 == a)) {
            end_read();
            return true;
        }
        d = d->next;
    }
    end_read();
    return false;
}

/********************  LISTING HELPERS  *************************/

void buildUserListBuffer(struct node *head, char *buf, size_t buflen) {
    start_read();
    buf[0] = '\0';
    strncat(buf, "Users:\n", buflen-1);
    struct node *cur = head;
    char line[128];
    while (cur) {
        snprintf(line, sizeof(line), "  %s\n", cur->username);
        strncat(buf, line, buflen - strlen(buf) - 1);
        cur = cur->next;
    }
    end_read();
}

void buildRoomListBuffer(struct room *rooms, char *buf, size_t buflen) {
    start_read();
    buf[0] = '\0';
    strncat(buf, "Rooms:\n", buflen-1);
    struct room *r = rooms;
    char line[128];
    while (r) {
        snprintf(line, sizeof(line), "  %s\n", r->name);
        strncat(buf, line, buflen - strlen(buf) - 1);
        r = r->next;
    }
    end_read();
}

/********************  CLEANUP  *************************/

void freeAllUsers(struct node *head) {
    struct node *cur = head;
    while (cur) {
        struct node *next = cur->next;
        close(cur->socket);
        free(cur);
        cur = next;
    }
}

void freeAllRooms(struct room *rooms) {
    struct room *r = rooms;
    while (r) {
        struct roomMember *m = r->members;
        while (m) {
            struct roomMember *mn = m->next;
            free(m);
            m = mn;
        }
        struct room *rn = r->next;
        free(r);
        r = rn;
    }
}

void freeAllDMs(struct dm *dms) {
    struct dm *d = dms;
    while (d) {
        struct dm *dn = d->next;
        free(d);
        d = dn;
    }
}
