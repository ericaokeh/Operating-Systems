#include "server.h"

/*
 * Main thread for each client. Receives all messages
 * and processes commands. Receives a pointer to the
 * socket file descriptor.
 */

extern struct room *rooms_head;
extern struct dm   *dms_head;

char *trimwhitespace(char *str)
{
  char *end;

  while(isspace((unsigned char)*str)) str++;

  if(*str == 0)
    return str;

  end = str + strlen(str) - 1;
  while(end > str && isspace((unsigned char)*end)) end--;

  end[1] = '\0';

  return str;
}

void *client_receive(void *ptr) {
   int client = *(int *) ptr;  // socket
   free(ptr);

   int received, i;
   char buffer[MAXBUFF], sbuffer[MAXBUFF];
   char tmpbuf[MAXBUFF];
   char cmd[MAXBUFF], username[30];
   char *arguments[80];

   struct node *currentUser;
    
   send(client, server_MOTD, strlen(server_MOTD), 0);

   // Create guest user name
   sprintf(username, "guest%d", client);
   head = insertFirstU(head, client, username);

   // Put guest into default room
   currentUser = findBySocket(head, client);
   addUserToRoom(&rooms_head, currentUser, DEFAULT_ROOM);

   while (1) {
      if ((received = read(client, buffer, MAXBUFF)) > 0) {
            buffer[received] = '\0';
            strcpy(cmd, buffer);
            strcpy(sbuffer, buffer);

            // 1. Tokenize
            arguments[0] = strtok(cmd, delimiters);
            i = 0;
            while(arguments[i] != NULL) {
                arguments[++i] = strtok(NULL, delimiters);
                if (arguments[i-1])
                    arguments[i-1] = trimwhitespace(arguments[i-1]);
            }

            if (arguments[0] == NULL) {
                continue;
            }

            /////////////////////////////////////////////////////
            // 2. Execute command

            if(strcmp(arguments[0], "create") == 0 && arguments[1]) {
               printf("create room: %s\n", arguments[1]); 
               createRoom(&rooms_head, arguments[1]);
               sprintf(buffer, "Room %s created.\nchat>", arguments[1]);
               send(client , buffer , strlen(buffer) , 0 );
            }
            else if (strcmp(arguments[0], "join") == 0 && arguments[1]) {
               printf("join room: %s\n", arguments[1]);  
               currentUser = findBySocket(head, client);
               addUserToRoom(&rooms_head, currentUser, arguments[1]);
               sprintf(buffer, "Joined room %s\nchat>", arguments[1]);
               send(client , buffer , strlen(buffer) , 0 );
            }
            else if (strcmp(arguments[0], "leave") == 0 && arguments[1]) {
               printf("leave room: %s\n", arguments[1]); 
               currentUser = findBySocket(head, client);
               removeUserFromRoom(rooms_head, currentUser, arguments[1]);
               sprintf(buffer, "Left room %s\nchat>", arguments[1]);
               send(client , buffer , strlen(buffer) , 0 );
            } 
            else if (strcmp(arguments[0], "connect") == 0 && arguments[1]) {
               printf("connect to user: %s \n", arguments[1]);
               struct node *me  = findBySocket(head, client);
               struct node *you = findU(head, arguments[1]);
               if (you) {
                   connectUsers(&dms_head, me, you);
                   sprintf(buffer, "DM connected to %s\nchat>", you->username);
               } else {
                   sprintf(buffer, "No such user %s\nchat>", arguments[1]);
               }
               send(client , buffer , strlen(buffer) , 0 );
            }
            else if (strcmp(arguments[0], "disconnect") == 0 && arguments[1]) {
               printf("disconnect from user: %s\n", arguments[1]);
               struct node *me  = findBySocket(head, client);
               struct node *you = findU(head, arguments[1]);
               if (you) {
                   disconnectUsers(&dms_head, me, you);
                   sprintf(buffer, "DM disconnected from %s\nchat>", you->username);
               } else {
                   sprintf(buffer, "No such user %s\nchat>", arguments[1]);
               }
               send(client , buffer , strlen(buffer) , 0 );
            }                  
            else if (strcmp(arguments[0], "rooms") == 0) {
                printf("List all the rooms\n");
                buildRoomListBuffer(rooms_head, buffer, sizeof(buffer));
                strcat(buffer, "chat>");
                send(client , buffer , strlen(buffer) , 0 );
            }   
            else if (strcmp(arguments[0], "users") == 0) {
                printf("List all the users\n");
                buildUserListBuffer(head, buffer, sizeof(buffer));
                strcat(buffer, "chat>");
                send(client , buffer , strlen(buffer) , 0 );
            }                           
            else if (strcmp(arguments[0], "login") == 0 && arguments[1]) {
                // rename guestID to username
                currentUser = findBySocket(head, client);
                if (currentUser) {
                    strcpy(currentUser->username, arguments[1]);
                }
                sprintf(buffer, "Logged in as %s\nchat>", arguments[1]);
                send(client , buffer , strlen(buffer) , 0 );
            } 
            else if (strcmp(arguments[0], "help") == 0 ) {
                sprintf(buffer,
                        "login <username> - \"login with username\" \n"
                        "create <room> - \"create a room\" \n"
                        "join <room> - \"join a room\" \n"
                        "leave <room> - \"leave a room\" \n"
                        "users - \"list all users\" \n"
                        "rooms -  \"list all rooms\" \n"
                        "connect <user> - \"connect to user\" \n"
                        "disconnect <user> - \"disconnect from user\" \n"
                        "exit - \"exit chat\" \nchat>");
                send(client , buffer , strlen(buffer) , 0 );
            }
            else if (strcmp(arguments[0], "exit") == 0 || strcmp(arguments[0], "logout") == 0) {
                // remove user from all rooms/DMs and close socket
                printf("Client %d exiting\n", client);

                struct node *me = findBySocket(head, client);
                if (me) {
                    // remove from rooms
                    struct room *r = rooms_head;
                    while (r) {
                        removeUserFromRoom(r, me, r->name);
                        r = r->next;
                    }
                    // remove from DMs
                    struct dm *d = dms_head;
                    while (d) {
                        struct dm *next = d->next;
                        if (d->u1 == me || d->u2 == me)
                            disconnectUsers(&dms_head, d->u1, d->u2);
                        d = next;
                    }
                    head = deleteBySocket(head, client);
                }

                close(client);
                pthread_exit(NULL);
            }                         
            else { 
                 /////////////////////////////////////////////////////////////
                 // 3. sending a message (rooms + DMs)

                 struct node *from = findBySocket(head, client);
                 if (!from) continue;

                 sprintf(tmpbuf,"\n::%s> %s\nchat>", from->username, sbuffer);
                 strcpy(sbuffer, tmpbuf);

                 currentUser = head;
                 while(currentUser != NULL) {

                     if(client != currentUser->socket){  // dont send to yourself 
                         if (usersShareRoom(rooms_head, from, currentUser) ||
                             areDMConnected(dms_head, from, currentUser)) {
                             send(currentUser->socket , sbuffer , strlen(sbuffer) , 0 ); 
                         }
                     }
                     currentUser = currentUser->next;
                 }
            }

            memset(buffer, 0, sizeof(buffer));
      } else {
          // client disconnected
          struct node *me = findBySocket(head, client);
          if (me) {
              head = deleteBySocket(head, client);
          }
          close(client);
          pthread_exit(NULL);
      }
   }
   return NULL;
}
