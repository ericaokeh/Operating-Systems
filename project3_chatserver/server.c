#include "server.h"

/* Global server socket */
int chat_serv_sock_fd;

/* Reader–writer synchronization state */
int numReaders = 0;
pthread_mutex_t mutex   = PTHREAD_MUTEX_INITIALIZER;   // protects numReaders
pthread_mutex_t rw_lock = PTHREAD_MUTEX_INITIALIZER;   // writer lock

/* Server MOTD and global lists */
char const *server_MOTD = "Thanks for connecting to the BisonChat Server.\n\nchat>";

struct node *head       = NULL;   // user list head
struct room *rooms_head = NULL;   // room list head
struct dm   *dms_head   = NULL;   // DM list head


int main(int argc, char **argv) {

    /* Install SIGINT handler for graceful shutdown */
    signal(SIGINT, sigintHandler);

    /* Create the default room (Lobby) that all clients join initially */
    rooms_head = NULL;
    createRoom(&rooms_head, DEFAULT_ROOM);

    /* Open server socket */
    chat_serv_sock_fd = get_server_socket();

    /* Get ready to accept connections */
    if (start_server(chat_serv_sock_fd, BACKLOG) == -1) {
        printf("start server error\n");
        exit(1);
    }

    printf("Server Launched! Listening on PORT: %d\n", PORT);

    /* Main accept loop */
    while (1) {
        int new_client = accept_client(chat_serv_sock_fd);
        if (new_client != -1) {
            /* Allocate a separate int for the thread argument */
            int *pclient = malloc(sizeof(int));
            if (pclient == NULL) {
                perror("malloc");
                close(new_client);
                continue;
            }
            *pclient = new_client;

            pthread_t new_client_thread;
            if (pthread_create(&new_client_thread, NULL, client_receive,
                               (void *)pclient) != 0) {
                perror("pthread_create");
                close(new_client);
                free(pclient);
                continue;
            }
            /* We don't need to join client threads in the server main loop */
            pthread_detach(new_client_thread);
        }
    }

    /* Not normally reached, but for completeness */
    close(chat_serv_sock_fd);
    return 0;
}


/* Create, bind, and return a listening socket on PORT */
int get_server_socket(void) {
    int opt = TRUE;
    int master_socket;
    struct sockaddr_in address;

    /* Create a TCP socket */
    if ((master_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    /* Allow address reuse */
    if (setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR,
                   (char *)&opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    /* Bind to all interfaces on PORT */
    address.sin_family      = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port        = htons(PORT);

    if (bind(master_socket, (struct sockaddr *)&address,
             sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    return master_socket;
}


/* Put the socket into listening mode */
int start_server(int serv_socket, int backlog) {
    int status = 0;
    if ((status = listen(serv_socket, backlog)) == -1) {
        printf("socket listen error\n");
    }
    return status;
}


/* Accept a new client connection */
int accept_client(int serv_sock) {
    int reply_sock_fd = -1;
    socklen_t sin_size = sizeof(struct sockaddr_storage);
    struct sockaddr_storage client_addr;

    reply_sock_fd = accept(serv_sock, (struct sockaddr *)&client_addr,
                           &sin_size);
    if (reply_sock_fd == -1) {
        printf("socket accept error\n");
    }

    return reply_sock_fd;
}


/* Handle SIGINT (CTRL+C) for graceful shutdown */
void sigintHandler(int sig_num) {
    (void)sig_num;  /* unused */

    printf("\nSIGINT received, shutting down server...\n");

    /* Close all client sockets and free all lists */
    freeAllUsers(head);
    freeAllRooms(rooms_head);
    freeAllDMs(dms_head);

    printf("--------CLOSING ACTIVE USERS AND FREEING RESOURCES--------\n");

    /* Close the listening socket */
    if (chat_serv_sock_fd >= 0) {
        close(chat_serv_sock_fd);
    }

    printf("Server shutdown complete.\n");
    exit(0);
}
