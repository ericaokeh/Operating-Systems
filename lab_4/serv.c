#include <netinet/in.h> //structure for storing address information 
#include <stdio.h> 
#include <string.h>
#include <stdlib.h> 
#include <sys/socket.h> //for socket APIs 
#include <sys/types.h> 
#include "list.h"
#include <unistd.h>      // NEW: For close()
#include <signal.h>      // NEW: For signal handling

#define PORT 9001
#define ACK "ACK "

// NEW: mylist is now global, so the signal_handler can access it
list_t *mylist;

// NEW: This is the signal handler for Ctrl-C (SIGINT)
void signal_handler(int sig) {
    if (sig == SIGINT) {
        printf("\nCaught Ctrl-C (SIGINT), terminating gracefully...\n");
        if (mylist != NULL) {
            list_free(mylist); // Deallocate memory
        }
        exit(0); // Exit successfully
    }
}
  
int main(int argc, char const* argv[]) 
{ 
  
    int n, val, idx;
    int servSockD = socket(AF_INET, SOCK_STREAM, 0); 

    // NEW: Add setsockopt to reuse the port, preventing "address already in use" errors
    int opt = 1;
    setsockopt(servSockD, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
  
    char buf[1024];
    char sbuf[1024];
    char* token;

    struct sockaddr_in servAddr; 

    // NEW: Register the signal handler to catch Ctrl-C
    signal(SIGINT, signal_handler);

    servAddr.sin_family = AF_INET; 
    servAddr.sin_port = htons(PORT); 
    servAddr.sin_addr.s_addr = INADDR_ANY; 
  
    // NEW: Added error checking for bind
    if (bind(servSockD, (struct sockaddr*)&servAddr, sizeof(servAddr)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
  
    listen(servSockD, 1); 
    printf("Server listening on port %d...\n", PORT);
  
    // NEW: Added error checking for accept
    int clientSocket = accept(servSockD, NULL, NULL); 
    if (clientSocket < 0) {
        perror("accept failed");
        exit(EXIT_FAILURE);
    }
    printf("Client connected.\n");

    mylist = list_alloc();  // create the list

    while(1){
        // NEW: Clear both buffers at the start of the loop
        memset(buf, '\0', 1024);
        memset(sbuf, '\0', 1024);

        // recvs messages from client socket 
        n = recv(clientSocket, buf, sizeof(buf) - 1, 0); // Read n-1 bytes
        
        // NEW: Check if client disconnected
        if (n <= 0) {
            if (n == 0) {
                printf("Client disconnected.\n");
            } else {
                perror("recv error");
            }
            break; // Exit loop
        }

        buf[n] = '\0'; // Ensure null-termination

        token = strtok(buf, " ");
        
        if (token == NULL) {
            continue; // Skip if empty command
        }

        if(strcmp(token,"exit") == 0){
            printf("Exit command received. Shutting down.\n");
            list_free(mylist);
            sprintf(sbuf, "Server shutting down.");
            // NEW: Send final message using strlen
            send(clientSocket, sbuf, strlen(sbuf), 0);
            break; // Break from loop for graceful shutdown
        }
        else if(strcmp(token,"get_length") == 0){
            val = list_length(mylist);
            sprintf(sbuf,"%s%d", "Length = ", val);
        }
        else if(strcmp(token,"add_front") == 0){
            token = strtok(NULL, " ");
            if (token != NULL) {
                val = atoi(token);
                list_add_to_front(mylist,val);
                sprintf(sbuf,"%s%d", ACK, val);
            } else {
                sprintf(sbuf, "Error: Missing value");
            }
        }
        else if(strcmp(token,"remove_position") == 0){
            token = strtok(NULL, " ");
            if (token != NULL) {
                idx = atoi(token);
                val = list_remove_at_index(mylist,idx);
                sprintf(sbuf,"%s%d", ACK, val);
            } else {
                sprintf(sbuf, "Error: Missing index");
            }
        }
        else if(strcmp(token,"print") == 0){
            // NEW: Assuming listToString allocates memory, we must free it
            char* list_str = listToString(mylist);
            if (list_str != NULL) {
                sprintf(sbuf, "%s", list_str);
                free(list_str); // Free the string
            } else {
                sprintf(sbuf, "(empty list)");
            }
        }
        
        // --- NEW: ADDED THE OTHER OPERATIONS ---

        else if(strcmp(token,"add_back") == 0){
            token = strtok(NULL, " ");
            if (token != NULL) {
                val = atoi(token);
                list_add_to_back(mylist, val);
                sprintf(sbuf,"%s%d", ACK, val);
            } else {
                sprintf(sbuf, "Error: Missing value");
            }
        }
        else if(strcmp(token,"add_position") == 0){
            token = strtok(NULL, " "); // get index
            if (token != NULL) {
                idx = atoi(token);
                token = strtok(NULL, " "); // get value
                if (token != NULL) {
                    val = atoi(token);
                    list_add_at_index(mylist, val, idx);
                    sprintf(sbuf,"%s%d", ACK, val);
                } else {
                    sprintf(sbuf, "Error: Missing value");
                }
            } else {
                sprintf(sbuf, "Error: Missing index");
            }
        }
        else if(strcmp(token,"remove_back") == 0){
            val = list_remove_from_back(mylist); 
            sprintf(sbuf,"%s%d", ACK, val); 
        }
        else if(strcmp(token,"remove_front") == 0){
            val = list_remove_from_front(mylist);
            sprintf(sbuf,"%s%d", ACK, val);
        }
        else if(strcmp(token,"get") == 0){
            token = strtok(NULL, " "); // get index
            if (token != NULL) {
                idx = atoi(token);
                val = list_get_elem_at(mylist, idx); 
                // Assuming -1 or similar means error
                if (val == -1) { 
                    sprintf(sbuf, "Error: Invalid index %d", idx);
                } else {
                    sprintf(sbuf,"%d", val);
                }
            } else {
                sprintf(sbuf, "Error: Missing index");
            }
        }
        
        // --- END OF NEW OPERATIONS ---

        else {
            // NEW: Handle unknown commands
            sprintf(sbuf, "Error: Unknown command '%s'", token);
        }

        // NEW: Send using strlen, not sizeof, to avoid sending junk data
        send(clientSocket, sbuf, strlen(sbuf), 0);
    }
    
    // NEW: Clean up sockets before closing
    printf("Shutting down server.\n");
    close(clientSocket);
    close(servSockD);
  
    return 0; 
}