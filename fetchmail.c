#include<stdio.h>
#include <ctype.h>
#include<stdlib.h>
#include<unistd.h>
#include<netdb.h>
#include<string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define NAME_LENGTH 100
#define BUFFER_SIZE 1024

void get_input_line(int argc, char* argv[], char *username, char *password, char *folder, int *messageNum, int *tls, char *command, char *serverName){
    for (int i = 1; i < argc; i+=2){
        // username
        if (strcmp(argv[i], "-u") == 0 && i + 1 < argc){
            strcpy(username, argv[i + 1]);
        }// password
        else if (strcmp(argv[i], "-p") == 0 && i + 1 < argc){
            strcpy(password, argv[i + 1]);
        }// folder
        else if(strcmp(argv[i], "-f") == 0 && i + 1 < argc){
            strcpy(folder, argv[i + 1]);
        }
        // messageNum
        else if(strcmp(argv[i], "-n") == 0 && i + 1 < argc){
            *messageNum = atoi(argv[i + 1]);
        }// command and server_name
        else if(strcmp(argv[i], "-t") == 0 && i + 1 < argc){
            *tls = 1;
        } strcpy(command, argv[argc - 2]);
        strcpy(serverName, argv[argc - 1]);
    }
}



int establish_connection(char* serverName, char* port){
    struct addrinfo hints, *res, *rp;
    int sockfd;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if(getaddrinfo(serverName, port, &hints, &res) != 0){
        perror("getaddrinfo");
        exit(EXIT_FAILURE);
    }
    for (rp = res; rp != NULL; rp = rp ->ai_next){
        sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if (sockfd == -1) {
            continue;
        }
        if (connect(sockfd, res->ai_addr, res->ai_addrlen) != -1) {
            break;
        }
        close(sockfd);
    }
    if (rp == NULL){
        fprintf(stderr, "failed to connect\n");
        exit(EXIT_FAILURE);
    }
    
    freeaddrinfo(res);

    return sockfd;
}

void send_command(int sockfd, const char *command) {
    if (send(sockfd, command, strlen(command), 0) == -1) {
        perror("send");
        exit(3);
    }
}

void receive_response(int sockfd, char *buffer, int size) {
    ssize_t bytes_received = recv(sockfd, buffer, size, 0);
    if (bytes_received == -1) {
        perror("recv");
        exit(3);
    }
    buffer[bytes_received] = '\0';
}

// Function to convert a string to lowercase
void to_lowercase(char *str) {
    while (*str) {
        if (*str == ':') {
            break;  // Stop conversion when encountering a colon
        }
        *str = tolower(*str);
        str++;
    }
}

void parse_email(int sockfd, int message_num) {
    char buffer[BUFFER_SIZE];
    char receive_buffer[BUFFER_SIZE];
    char lowercase_buffer[BUFFER_SIZE];
    char *from_start;
    char *end_of_line;
    char *from_field;
    if (message_num == -1){
        printf("Message not found\n");
        exit(3);
    }
    snprintf(buffer, BUFFER_SIZE, "A04 FETCH %d BODY.PEEK[HEADER.FIELDS (FROM)]\r\n", message_num);

    send_command(sockfd, buffer);

    receive_response(sockfd, receive_buffer, BUFFER_SIZE);
    strcpy(lowercase_buffer, receive_buffer);
    to_lowercase(lowercase_buffer);
    // Find the position of "From: " in the response
    from_start = strstr(lowercase_buffer, "from: ");
    if (from_start != NULL) {
        from_start += strlen("From: ");
        end_of_line = strchr(from_start, '\n');
        if (end_of_line != NULL) {
            int length = end_of_line - from_start;
            from_field = malloc(length + 1);
            if (from_field != NULL) {
                strncpy(from_field, from_start, length);
                from_field[length - 1] = '\0';  // Null-terminate the string
                printf("From: %s\n", from_field);
                free(from_field);
            } 
        } 
    } 

    snprintf(buffer, BUFFER_SIZE, "A05 FETCH %d BODY.PEEK[HEADER.FIELDS (TO)]\r\n", message_num);

    send_command(sockfd, buffer);

    receive_response(sockfd, receive_buffer, BUFFER_SIZE);
    strcpy(lowercase_buffer, receive_buffer);
    to_lowercase(lowercase_buffer);
    from_start = strstr(lowercase_buffer, "to: ");
    if (from_start == NULL) {
        from_start = strstr(receive_buffer, "T0: ");
    }
    if (from_start != NULL) {
        from_start += strlen("To: ");
        // Find the end of the line after "From: "
        end_of_line = strchr(from_start, '\n');
        if (end_of_line != NULL) {
            int length = end_of_line - from_start;
            from_field = malloc(length + 1);    
            if (from_field != NULL) {
                strncpy(from_field, from_start, length);
                from_field[length - 1] = '\0';  // Null-terminate the string
                printf("To: %s\n", from_field);
                free(from_field);
            }
        } 
    }else{
        printf("To:%s\n", "");
    }

    snprintf(buffer, BUFFER_SIZE, "A06 FETCH %d BODY.PEEK[HEADER.FIELDS (DATE)]\r\n", message_num);

    send_command(sockfd, buffer);

    receive_response(sockfd, receive_buffer, BUFFER_SIZE);
    strcpy(lowercase_buffer, receive_buffer);
    to_lowercase(lowercase_buffer);
    // Find the position of "From: " in the response
    from_start = strstr(lowercase_buffer, "date: ");
    if (from_start != NULL) {
        from_start += strlen("Date: ");
        end_of_line = strchr(from_start, '\n');
        if (end_of_line != NULL) {
            if (from_field != NULL) {
                int length = end_of_line - from_start;
                from_field = malloc(length + 1);
                if (from_field != NULL) {  
                    strncpy(from_field, from_start, length);
                    from_field[length - 1] = '\0';  // Null-terminate the string
                    printf("Date: %s\n", from_field);
                    free(from_field);
                }
            } 
        } 
    }
    snprintf(buffer, BUFFER_SIZE, "A07 FETCH %d BODY.PEEK[HEADER.FIELDS (SUBJECT)]\r\n", message_num);
    send_command(sockfd, buffer);
    receive_response(sockfd, receive_buffer, BUFFER_SIZE);
    strcpy(lowercase_buffer, receive_buffer);
    to_lowercase(lowercase_buffer);

    // Find the position of "Subject: " in the response
    from_start = strstr(lowercase_buffer, "subject: ");
    if (from_start != NULL) {
        from_start += strlen("Subject: ");
        end_of_line = strchr(from_start, '\n');
        if (end_of_line != NULL) {
            int length = end_of_line - from_start; 
            from_field = malloc(length + 1); 
            if (from_field != NULL){   
                strncpy(from_field, from_start, length);
                from_field[length - 1] = '\0'; 
                printf("Subject: %s\n", from_field);
            //    for (int i = 32; from_field[i] != '\0'; i++) {
            //         printf("%c", from_field[i]);
            //    }
             //   printf("|\n");
             //   printf("length=%d\n", length);
                free(from_field);
            }
        } 
    }else{
        printf("Subject: <No subject>\n");
    }
    // Exit with status 0
    exit(0);
}



void logging_on(int sockfd, char *username, char* password){
    char buffer[BUFFER_SIZE];
    char receive_buffer[BUFFER_SIZE];
    // Construct the login command
    snprintf(buffer, BUFFER_SIZE, "A01 LOGIN %s %s\r\n", username, password);

    // Send login command to server
    send_command(sockfd, buffer);

    // Receive response from server
    receive_response(sockfd, receive_buffer, BUFFER_SIZE);

    // Check if login was successful
    if (strstr(receive_buffer, "A01 OK") == NULL) {
        printf("Login failure\n");
        exit(3);
    }
}

void select_folder(int sockfd, const char *folder) {
    char buffer[BUFFER_SIZE];
    char receive_buffer[BUFFER_SIZE];

    // Send SELECT command to server
    snprintf(buffer, BUFFER_SIZE, "A02 SELECT \"%s\"\r\n", folder);
    send_command(sockfd, buffer);

    // Receive response from server
    receive_response(sockfd, receive_buffer, BUFFER_SIZE);
    // Check if folder selection was successful
    if (strstr(receive_buffer, "A02 OK") == NULL) {
        printf("Folder not found\n");
        exit(3);
    }
}

int get_size(int sockfd, char *receive_buffer){
    int size = -1;
    char *size_start = NULL;
    char *ptr = receive_buffer;
    int break_tool = 0;
    while(1){
        ssize_t bytes_received = recv(sockfd, ptr, 1, 0);
        if (bytes_received == -1){
            printf("recv");
            exit(1);
        }else if (bytes_received == 0){
            printf("Message not found\n");
            exit(3); // Exit with status 3
        }
        if (*ptr == '\n' && break_tool == 1){
            receive_buffer[bytes_received] = '\0';
            break;
        }else if(*ptr == '{' && size == -1){
            size_start = ptr;
        }else if(*ptr == '}' && size_start != NULL){
            // Stop recording the size when closing curly brace is encountered
            *ptr = '\0';  // Null-terminate the substring
            sscanf(size_start + 1, "%d", &size);  // Skip '{' and parse the number
            break_tool = 1;
        }
        ptr++;
    }return size;
}

void retrieve_email(int sockfd, int message_num) {
    char buffer[BUFFER_SIZE];
    int size = -1;
    if (message_num == -1){
        printf("Message not found\n");
        exit(3);
    }

    // Construct FETCH command to retrieve email content
    snprintf(buffer, BUFFER_SIZE, "A03 FETCH %d BODY.PEEK[]\r\n", message_num);
    // Send FETCH command to server
    send_command(sockfd, buffer);

    // get size
    size = get_size(sockfd, buffer);
    
    if(size == -1 || size == 0){
        printf("cannot get size\n");
        exit(3);
    }

    // Receive response from server
    char retrieve_buffer[size];
    receive_response(sockfd, retrieve_buffer, size);

    // Check if email retrieval was successful
    if (strstr(retrieve_buffer, "Message not found") != NULL) {
        fprintf(stderr, "Message not found\n");
        exit(3); // Exit with status 3
    }

    // Print the raw email content to stdout
    printf("%s", retrieve_buffer);

    // Exit with status 0
    exit(0);
}

void mime(int sockfd, int message_num){
    char *head;
    char* tail;
    char* boundary;
    char buffer[BUFFER_SIZE];
    int size = -1;
    int length;
    char* line;
    int check = 0;
    if (message_num == -1){
        printf("Message not found\n");
        exit(3);
    }
    snprintf(buffer, BUFFER_SIZE, "A08 FETCH %d BODY.PEEK[]\r\n", message_num);
    send_command(sockfd, buffer);
    size = get_size(sockfd, buffer);
    
    if(size == -1 || size == 0){
        printf("cannot get size\n");
        exit(3);
    }

    // Receive response from server
    char receive_buffer[size];
    receive_response(sockfd, receive_buffer, size);
    head = strstr(receive_buffer, " boundary=");
    if (head != NULL) {
        head += strlen(" boundary=");
        tail = strchr(head, '\n');
        if (tail != NULL) {
            length = tail - head; 
            boundary = malloc(length); 
            if (boundary != NULL){   
                if (head[0] == '"'){
                    head++;
                    length--;
                    while (head[length - 1] != '"'){
                        length--;
                    }length--;
                }
                
                strncpy(boundary, head, length);
                boundary[length - 1] = '\0';
                if (boundary != NULL){   
                    strncpy(boundary, head, length);
                    boundary[length] = '\0';

                    // Prepare new boundary string
                    char *new_boundary = malloc(length + 2);
                    if (new_boundary != NULL) {
                        strcpy(new_boundary, "--");
                        strcat(new_boundary, boundary);
                        free(boundary); // Free the old boundary string
                        boundary = new_boundary; // Update boundary pointer to point to the new string
                    //    printf("boundary: %s\n", boundary);
                    //    for (int i = 0; boundary[i] != '\0'; i++) {
                    //        printf("%c\n", boundary[i]);
                    //    }
                    //    printf("\n");
                    } else {
                        // Handle memory allocation error for new_boundary
                        free(boundary); // Free the old boundary string
                        exit(3);
                    }
                }

            }
        } 
    }
    head = strstr(receive_buffer, boundary);// where the first boundary is
    // Calculate the offset to the boundary line
    size_t offset = head - receive_buffer;
    // Shift the contents of the receive_buffer to remove everything before the boundary line
    memmove(receive_buffer, head, size - offset);
    size -= offset;

    int con_print = 1;
    head = strstr(receive_buffer, boundary);
    tail = strstr(receive_buffer, boundary);
    for (int i = 0; i < 10; i++){
        head = tail;
        tail = strchr(head, '\n');
        if (tail != NULL) {
            length = tail - head; 
            line = malloc(length); 
            char* tool_line = malloc(length);
            strcpy(tool_line, line);
            to_lowercase(tool_line);
            printf("line = %s\n", line);
            if (line != NULL){   
                strncpy(line, head, length);
                line[length - 1] = '\0';
            }
            if(strstr(line, boundary) != NULL && check == 0){
                //start
                check = 1;
                continue;
            }else if(strstr(line, boundary) != NULL && check == 1){
                //finish
                con_print = 0;
            }else if(strstr(tool_line, "content-type") != NULL || strstr(tool_line, " charset=") != NULL || strstr(tool_line, "content-transfer-encoding") != NULL){
                //ignore Content-Type, charset, Content-Transfer-Encoding
                continue;
            }else{
                printf("%s\n", line);
            }free(line);
            free(tool_line);
        }
    }
    free(boundary);
}

int main(int argc, char* argv[]){
    char username[NAME_LENGTH];
    char password[NAME_LENGTH];
    char folder[NAME_LENGTH] = "INBOX"; // Default folder is INBOX
    int messageNum = -1;
    int tls = 0; // Flag to indicate whether TLS is requested
    char command[NAME_LENGTH];
    char serverName[NAME_LENGTH];

    get_input_line(argc, argv, username, password, folder, &messageNum, &tls, command, serverName);

    char *port = tls ? "993" : "143";
    int sockfd = establish_connection(serverName, port);
    char receive_buffer[BUFFER_SIZE];
    receive_response(sockfd, receive_buffer, BUFFER_SIZE);
 
    logging_on(sockfd, username, password);
    select_folder(sockfd, folder);
    if (strcmp(command, "retrieve") == 0){
        retrieve_email(sockfd, messageNum);
    }else if(strcmp(command, "parse") == 0){
        parse_email(sockfd, messageNum);
    }else if(strcmp(command, "mime") == 0){
        mime(sockfd, messageNum);
    }
    
    close(sockfd);
}