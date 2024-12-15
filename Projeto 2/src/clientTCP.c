#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "clientTCP.h"
#include "getIP.h"

// Create a TCP socket and connect to the specified IP and port
int createTcpSocketAndConnect(const char *ip, int port) {
    // Create a socket using IPv4 and TCP protocol
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Error creating socket");
        return -1;
    }

    // Set up the server address structure with the given IP and port
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip, &server_addr.sin_addr) <= 0) {
        perror("Error with inet_pton");
        close(sockfd);
        return -1;
    }

    // Try to connect to the server
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Error connecting to server");
        close(sockfd);
        return -1;
    }

    // Return socket file descriptor
    return sockfd;
}

// Authenticate with the FTP server using the provided username and password
int authenticateWithFtpServer(int sockfd, const char *user, const char *password) {
    char buffer[512];

    // Send USER command to the FTP server
    snprintf(buffer, sizeof(buffer), "USER %s\r\n", user);
    if (write(sockfd, buffer, strlen(buffer)) < 0) {
        perror("Error sending USER command");
        return -1; 
    }

    // Read the server's response to the USER command
    ssize_t bytes_read = read(sockfd, buffer, sizeof(buffer));
    if (bytes_read < 0) return -1;
    buffer[bytes_read] = '\0'; // Null-terminate buffer
    printf("Server response to USER command: %s\n", buffer); 

    // Check if authentication is successful
    if (strstr(buffer, "331") == NULL) {
        fprintf(stderr, "Authentication failed for user: %s\n", user);
        return -1;
    }

    // Send PASS command with password
    snprintf(buffer, sizeof(buffer), "PASS %s\r\n", password);
    if (write(sockfd, buffer, strlen(buffer)) < 0) {
        perror("Error sending PASS command");
        return -1;  
    }

    // Read the server's response to the PASS command
    bytes_read = read(sockfd, buffer, sizeof(buffer));
    if (bytes_read < 0) {
        perror("Error reading server response after PASS command");
        return -1;  
    }

    buffer[bytes_read] = '\0'; 
    printf("Server response to PASS command: %s\n", buffer); 

    // Check if password authentication is successful
    if (strstr(buffer, "230") == NULL) {
        fprintf(stderr, "Password authentication failed\n");
        return -1;
    }

    return 0;
}

// Switch FTP connection to passive mode and retrieve IP and port
int enterPassiveModeAndGetIpPort(int sockfd, URL *url) {
    char buffer[512];

    // Send PASV command to the FTP server to enter passive mode
    if (write(sockfd, "PASV\r\n", 6) < 0) {
        perror("Error sending PASV command");
        return -1;
    }

    // Read the server's response to the PASV command
    ssize_t bytes_read = read(sockfd, buffer, sizeof(buffer) - 1);
    if (bytes_read < 0) {
        perror("Error reading server response after PASV command");
        return -1;
    }

    buffer[bytes_read] = '\0';

    // Check if the server successfully entered passive mode
    if (strstr(buffer, "227") == NULL) {
        fprintf(stderr, "Failed to enter passive mode: %s\n", buffer);
        return -1;
    }

    // Parse the response to extract IP and port for data connection
    int ip1, ip2, ip3, ip4, port1, port2;
    char *start = strchr(buffer, '(');
    char *end = strchr(buffer, ')');
    if (!start || !end || sscanf(start + 1, "%d,%d,%d,%d,%d,%d", &ip1, &ip2, &ip3, &ip4, &port1, &port2) != 6) {
        fprintf(stderr, "Error parsing PASV response: %s\n", buffer);
        return -1;
    }

    // Store IP and port in the URL structure
    snprintf(url->ip, sizeof(url->ip), "%d.%d.%d.%d", ip1, ip2, ip3, ip4);
    // Compute port number
    url->port = port1 * 256 + port2;

    return 0;
}

// Request the specified resource (file) from the FTP server
int requestFileFromFtpServer(int sockfd, const char *path) {
    char buffer[512];

    // Send RETR command to the FTP server to request a file
    snprintf(buffer, sizeof(buffer), "RETR %s\r\n", path);
    if (write(sockfd, buffer, strlen(buffer)) < 0) {
        perror("Error sending RETR command");
        return -1;
    }

    // Read the server's response to the RETR command
    ssize_t bytes_read = read(sockfd, buffer, sizeof(buffer));
    if (bytes_read < 0) {
        perror("Error reading server response after RETR command");
        return -1;
    }

    // Check if the file transfer is starting (150 status code)
    if (strstr(buffer, "150") == NULL) {
        fprintf(stderr, "Failed to start file transfer\n");
        return -1;
    }

    return 0;
}

// Download the file from the FTP server via the established data connection
int downloadFileFromFtpServer(int sockfd, const char *filename) {
    // Open the local file for writing the received data
    FILE *file = fopen(filename, "wb");
    if (!file) {
        perror("Error opening file for writing");
        return -1;
    }

    char buffer[1024];
    ssize_t bytes_read;
    // Read data from server and write it to the file    
    while ((bytes_read = read(sockfd, buffer, sizeof(buffer))) > 0) {
        fwrite(buffer, 1, bytes_read, file);
    }

    if (bytes_read < 0) {
        perror("Error reading data from server during file transfer");
        fclose(file);
        return -1;
    }

    fclose(file);
    return 0;
}

// Close the FTP control connection
void closeFtpConnection(int sockfd) {
    if (close(sockfd) < 0) {
        perror("Error closing socket");
    } else {
        printf("FTP control connection closed successfully\n");
    }
}