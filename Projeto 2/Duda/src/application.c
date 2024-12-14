/*compilar e executar
gcc -o download application.c
./download ftp://ftp.netlab.fe.up.pt/pub/filename */

#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <libgen.h>

#define FTP_PORT 21
#define MAX_RESPONSE_SIZE 1024
#define MAX_CMD_SIZE 1024

typedef struct {
    char* user;
    char* password;
    char* host;
    char* path;
} URL;

// Function prototypes
int parseURL(char* input, URL* url);
int connectToServer(char* ip, int port);
int sendCommand(int sockfd, char* command, char* response);
int getResponseCode(char* response);
void parsePassiveResponse(char* response, char* ip, int* port);

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s ftp://[<user>:<password>@]<host>/<url-path>\n", argv[0]);
        exit(-1);
    }

    URL url = {.user = "anonymous", .password = "anonymous@", .host = NULL, .path = NULL};
    if (parseURL(argv[1], &url) < 0) {
        fprintf(stderr, "Invalid URL format\n");
        exit(-1);
    }

    // Get IP from hostname
    struct hostent *h;
    if ((h = gethostbyname(url.host)) == NULL) {
        herror("gethostbyname()");
        exit(-1);
    }
    char* ip = inet_ntoa(*((struct in_addr *) h->h_addr));

    // Connect to control socket
    int controlSocket = connectToServer(ip, FTP_PORT);
    char response[MAX_RESPONSE_SIZE];
    
    // Read welcome message
    if (read(controlSocket, response, MAX_RESPONSE_SIZE) < 0) {
        perror("read()");
        exit(-1);
    }

    // Login sequence
    char command[MAX_CMD_SIZE];
    sprintf(command, "USER %s\r\n", url.user);
    if (sendCommand(controlSocket, command, response) < 0 || 
        getResponseCode(response) != 331) {
        fprintf(stderr, "USER command failed\n");
        exit(-1);
    }

    sprintf(command, "PASS %s\r\n", url.password);
    if (sendCommand(controlSocket, command, response) < 0 || 
        getResponseCode(response) != 230) {
        fprintf(stderr, "PASS command failed\n");
        exit(-1);
    }

    // Set binary mode
    if (sendCommand(controlSocket, "TYPE I\r\n", response) < 0 || 
        getResponseCode(response) != 200) {
        fprintf(stderr, "TYPE I command failed\n");
        exit(-1);
    }

    // Enter passive mode
    if (sendCommand(controlSocket, "PASV\r\n", response) < 0 || 
        getResponseCode(response) != 227) {
        fprintf(stderr, "PASV command failed\n");
        exit(-1);
    }

    // Parse passive mode response
    char dataIP[16];
    int dataPort;
    parsePassiveResponse(response, dataIP, &dataPort);

    // Connect to data socket
    int dataSocket = connectToServer(dataIP, dataPort);

    // Request file
    sprintf(command, "RETR %s\r\n", url.path);
    if (sendCommand(controlSocket, command, response) < 0 || 
        getResponseCode(response) != 150) {
        fprintf(stderr, "RETR command failed\n");
        exit(-1);
    }

    // Save file
    char* filename = basename(url.path);
    FILE* file = fopen(filename, "wb");
    if (file == NULL) {
        perror("fopen()");
        exit(-1);
    }

    char buffer[1024];
    int bytes;
    while ((bytes = read(dataSocket, buffer, sizeof(buffer))) > 0) {
        if (fwrite(buffer, 1, bytes, file) < bytes) {
            perror("fwrite()");
            exit(-1);
        }
    }

    fclose(file);
    close(dataSocket);

    // Read transfer complete message
    if (read(controlSocket, response, MAX_RESPONSE_SIZE) < 0 || getResponseCode(response) != 226) {
        fprintf(stderr, "File transfer failed\n");
        exit(-1);
    }

    // Quit
    if (sendCommand(controlSocket, "QUIT\r\n", response) < 0 || getResponseCode(response) != 221) {
        fprintf(stderr, "QUIT command failed\n");
        exit(-1);
    }

    close(controlSocket);
    return 0;
}

// Parse URL of format: ftp://[<user>:<password>@]<host>/<url-path>
int parseURL(char* input, URL* url) {
    if (strncmp(input, "ftp://", 6) != 0) return -1;
    
    char* cursor = input + 6;
    char* at = strchr(cursor, '@');
    
    if (at != NULL) {
        // Parse user:password
        char* colon = strchr(cursor, ':');
        if (colon == NULL) return -1;
        
        *colon = '\0';
        url->user = cursor;
        *at = '\0';
        url->password = colon + 1;
        cursor = at + 1;
    }
    
    // Parse host/path
    char* slash = strchr(cursor, '/');
    if (slash == NULL) return -1;
    
    *slash = '\0';
    url->host = cursor;
    url->path = slash + 1;
    
    return 0;
}

// Create and connect a socket to the specified server
int connectToServer(char* ip, int port) {
    int sockfd;
    struct sockaddr_in server_addr;
    
    // Create socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket()");
        exit(-1);
    }
    
    // Configure server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ip);
    server_addr.sin_port = htons(port);
    
    // Connect
    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect()");
        exit(-1);
    }
    
    return sockfd;
}

// Send command and receive response
int sendCommand(int sockfd, char* command, char* response) {
    if (write(sockfd, command, strlen(command)) < 0) {
        perror("write()");
        return -1;
    }
    
    if (read(sockfd, response, MAX_RESPONSE_SIZE) < 0) {
        perror("read()");
        return -1;
    }
    
    return 0;
}

// Extract response code from server response
int getResponseCode(char* response) {
    int code;
    sscanf(response, "%d", &code);
    return code;
}

// Parse passive mode response to get IP and port for data connection
void parsePassiveResponse(char* response, char* ip, int* port) {
    int ip1, ip2, ip3, ip4, port1, port2;
    sscanf(response, "227 Entering Passive Mode (%d,%d,%d,%d,%d,%d)",
           &ip1, &ip2, &ip3, &ip4, &port1, &port2);
    
    sprintf(ip, "%d.%d.%d.%d", ip1, ip2, ip3, ip4);
    *port = port1 * 256 + port2;
}