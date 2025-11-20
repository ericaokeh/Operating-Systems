#include <netinet/in.h>
#include <stdio.h> 
#include <string.h>
#include <stdlib.h> 
#include <sys/socket.h>
#include <sys/types.h> 
#include "list.h"
#include <unistd.h>
#include <signal.h>
#include <ctype.h>

#define PORT 9001
#define ACK "ACK "
#define BUFFER_SIZE 1024

// mylist is global, so the signal_handler can access it
list_t *mylist;

// This is the signal handler for Ctrl-C (SIGINT)
void signal_handler(int sig) {
    if (sig == SIGINT) {
        printf("\nCaught Ctrl-C (SIGINT), terminating gracefully...\n");
        if (mylist != NULL) {
            list_free(mylist); // Deallocate memory
        }
        exit(0);
    }
}

// Helper function to trim whitespace from string
char* trim_whitespace(char *str) {
    if (str == NULL) return NULL;
    
    char *end;
    
    // Trim leading space
    while(isspace((unsigned char)*str)) str++;
    
    if(*str == 0) return str;
    
    // Trim trailing space
    end = str + strlen(str) - 1;
    while(end > str && isspace((unsigned char)*end)) end--;
    
    end[1] = '\0';
    return str;
}

// Helper function to check if string is empty or whitespace only
int is_empty_string(const char *str) {
    if (str == NULL) return 1;
    
    while(*str) {
        if (!isspace((unsigned char)*str)) return 0;
        str++;
    }
    return 1;
}
  
int main(int argc, char const* argv[]) 
{ 
    int n, val, idx;
    int servSockD = socket(AF_INET, SOCK_STREAM, 0); 

    // Error checking for socket creation
    if (servSockD < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Set socket option to reuse address
    int opt = 1;
    if (setsockopt(servSockD, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt failed");
        close(servSockD);
        exit(EXIT_FAILURE);
    }
  
    char buf[BUFFER_SIZE];
    char sbuf[BUFFER_SIZE];
    char* token;
    char* command;
    int send_response = 1;

    struct sockaddr_in servAddr; 

    // Register the signal handler to catch Ctrl-C
    signal(SIGINT, signal_handler);

    servAddr.sin_family = AF_INET; 
    servAddr.sin_port = htons(PORT); 
    servAddr.sin_addr.s_addr = INADDR_ANY; 
  
    // Bind socket
    if (bind(servSockD, (struct sockaddr*)&servAddr, sizeof(servAddr)) < 0) {
        perror("bind failed");
        close(servSockD);
        exit(EXIT_FAILURE);
    }
  
    if (listen(servSockD, 1) < 0) {
        perror("listen failed");
        close(servSockD);
        exit(EXIT_FAILURE);
    }
    printf("Server listening on port %d...\n", PORT);
  
    // Accept connection
    int clientSocket = accept(servSockD, NULL, NULL); 
    if (clientSocket < 0) {
        perror("accept failed");
        close(servSockD);
        exit(EXIT_FAILURE);
    }
    printf("Client connected.\n");

    mylist = list_alloc();  // create the list

    while(1){
        // Reset flags and buffers
        send_response = 1;
        memset(buf, '\0', BUFFER_SIZE);
        memset(sbuf, '\0', BUFFER_SIZE);

        // Receive message from client
        n = recv(clientSocket, buf, sizeof(buf) - 1, 0);
        
        // Check connection status
        if (n <= 0) {
            if (n == 0) {
                printf("Client disconnected.\n");
            } else {
                perror("recv error");
            }
            break;
        }

        buf[n] = '\0'; // Ensure null-termination
        
        // Clean the input - trim whitespace and handle edge cases
        char *cleaned_input = trim_whitespace(buf);
        
        // Skip empty commands
        if (is_empty_string(cleaned_input)) {
            printf("DEBUG: Skipping empty command\n");
            continue;
        }
        
        printf("DEBUG: Received raw: '%s' (length: %zu)\n", cleaned_input, strlen(cleaned_input));

        // Make a copy for command processing since strtok modifies the string
        char input_copy[BUFFER_SIZE];
        strncpy(input_copy, cleaned_input, sizeof(input_copy) - 1);
        input_copy[sizeof(input_copy) - 1] = '\0';
        
        command = strtok(input_copy, " ");
        
        if (command == NULL) {
            printf("DEBUG: No command found after parsing\n");
            continue;
        }
        
        printf("DEBUG: Processing command: '%s'\n", command);

        // Command processing
        if(strcmp(command, "exit") == 0){
            printf("Exit command received. Shutting down.\n");
            list_free(mylist);
            sprintf(sbuf, "Server shutting down.");
            send(clientSocket, sbuf, strlen(sbuf), 0);
            break;
        }
        else if(strcmp(command, "menu") == 0){
            printf("DEBUG: Menu command - no response needed\n");
            send_response = 0;
        }
        else if(strcmp(command, "get_length") == 0){
            val = list_length(mylist);
            sprintf(sbuf, "Length = %d", val);
        }
        else if(strcmp(command, "add_front") == 0){
            token = strtok(NULL, " ");
            if (token != NULL) {
                val = atoi(token);
                list_add_to_front(mylist, val);
                sprintf(sbuf, "%s%d", ACK, val);
            } else {
                sprintf(sbuf, "Error: Missing value for add_front");
            }
        }
        else if(strcmp(command, "remove_position") == 0){
            token = strtok(NULL, " ");
            if (token != NULL) {
                idx = atoi(token);
                val = list_remove_at_index(mylist, idx);
                if (val != -1) { // Assuming -1 indicates error
                    sprintf(sbuf, "%s%d", ACK, val);
                } else {
                    sprintf(sbuf, "Error: Invalid index %d", idx);
                }
            } else {
                sprintf(sbuf, "Error: Missing index for remove_position");
            }
        }
        else if(strcmp(command, "print") == 0){
            char* list_str = listToString(mylist);
            if (list_str != NULL) {
                sprintf(sbuf, "%s", list_str);
                free(list_str);
            } else {
                sprintf(sbuf, "(empty list)");
            }
        }
        else if(strcmp(command, "add_back") == 0){
            token = strtok(NULL, " ");
            if (token != NULL) {
                val = atoi(token);
                list_add_to_back(mylist, val);
                sprintf(sbuf, "%s%d", ACK, val);
            } else {
                sprintf(sbuf, "Error: Missing value for add_back");
            }
        }
        else if(strcmp(command, "add_position") == 0){
            token = strtok(NULL, " "); // get index
            if (token != NULL) {
                idx = atoi(token);
                token = strtok(NULL, " "); // get value
                if (token != NULL) {
                    val = atoi(token);
                    if (list_add_at_index(mylist, val, idx) == 0) { // Assuming 0 means success
                        sprintf(sbuf, "%s%d", ACK, val);
                    } else {
                        sprintf(sbuf, "Error: Failed to add at index %d", idx);
                    }
                } else {
                    sprintf(sbuf, "Error: Missing value for add_position");
                }
            } else {
                sprintf(sbuf, "Error: Missing index for add_position");
            }
        }
        else if(strcmp(command, "remove_back") == 0){
            val = list_remove_from_back(mylist);
            if (val != -1) {
                sprintf(sbuf, "%s%d", ACK, val);
            } else {
                sprintf(sbuf, "Error: Cannot remove from empty list");
            }
        }
        else if(strcmp(command, "remove_front") == 0){
            val = list_remove_from_front(mylist);
            if (val != -1) {
                sprintf(sbuf, "%s%d", ACK, val);
            } else {
                sprintf(sbuf, "Error: Cannot remove from empty list");
            }
        }
        else if(strcmp(command, "get") == 0){
            token = strtok(NULL, " ");
            if (token != NULL) {
                idx = atoi(token);
                val = list_get_elem_at(mylist, idx);
                if (val != -1) {
                    sprintf(sbuf, "%d", val);
                } else {
                    sprintf(sbuf, "Error: Invalid index %d", idx);
                }
            } else {
                sprintf(sbuf, "Error: Missing index for get");
            }
        }
        else {
            sprintf(sbuf, "Error: Unknown command '%s'", command);
            printf("DEBUG: Unknown command received: '%s'\n", command);
        }

        // Send response if needed
        if (send_response) {
            printf("DEBUG: Sending response: '%s'\n", sbuf);
            if (send(clientSocket, sbuf, strlen(sbuf), 0) < 0) {
                perror("send failed");
                break;
            }
        }
    }
    
    // Cleanup
    printf("Shutting down server.\n");
    if (mylist != NULL) {
        list_free(mylist);
    }
    close(clientSocket);
    close(servSockD);
  
    return 0; 
}