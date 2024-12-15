#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "clientTCP.h"

// Helper function to extract the filename from the URL path
void extractFilename(const char *path, char *filename, size_t size) {
    // Find the last '/' character in the path
    const char *slash = strrchr(path, '/');
    
    // Extract the filename
    if (slash && *(slash + 1) != '\0') {
        strncpy(filename, slash + 1, size - 1);
    } else if (slash) {
        // If there's no filename. Set a default.
        strncpy(filename, "index.html", size - 1);
    } else {
        // If there's no slash, the entire path is the filename
        strncpy(filename, path, size - 1);
    }
    // Ensure null-termination
    filename[size - 1] = '\0'; 
}

int main(int argc, char *argv[]) {
    // Check if the correct number of arguments are provided    
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <ftp://[user[:password]@]host[:port]/path>\n", argv[0]);
        return 1;
    }

    // Initialize and parse the URL into a structured format
    URL url;
    memset(&url, 0, sizeof(URL)); 
    if (parseFtpUrl(argv[1], &url) != 0) {
        fprintf(stderr, "Error: Failed to parse URL: %s\n", argv[1]);
        return 1;
    }

    // Extract the filename from the path
    char filename[256];
    extractFilename(url.path, filename, sizeof(filename));

    // Create a TCP socket and connect to the FTP server
    int socket_fd = createTcpSocketAndConnect(url.ip, url.port);
    if (socket_fd < 0) {
        fprintf(stderr, "Error: Failed to connect to %s:%d\n", url.ip, url.port);
        return 1;
    }
    printf("Connected to FTP server %s:%d\n", url.ip, url.port);

    // Authenticate with the FTP server using the provided user credentials    if (authenticateWithFtpServer(socket_fd, url.user, url.password) != 0) {
        fprintf(stderr, "Error: Authentication failed for user '%s'\n", url.user);
        closeFtpConnection(socket_fd);
        return 1;
    }
    printf("Authentication successful for user '%s'\n", url.user);

    // Switch FTP connection to passive mode and retrieve IP and port for data transfer    if (enterPassiveModeAndGetIpPort(socket_fd, &url) != 0) {
        fprintf(stderr, "Error: Failed to enter passive mode.\n");
        closeFtpConnection(socket_fd);
        return 1;
    }
    printf("Entered passive mode: Data connection at %s:%d\n", url.ip, url.port);

    // Request the specified file from the FTP server (trigger the file transfer)    if (requestFileFromFtpServer(socket_fd, url.path) != 0) {
        fprintf(stderr, "Error: Failed to request file: %s\n", url.path);
        closeFtpConnection(socket_fd);
        return 1;
    }
    printf("Requested file: %s\n", url.path);

    // Create a new socket for the data transfer (for downloading the file)    int data_socket_fd = createTcpSocketAndConnect(url.ip, url.port);
    if (data_socket_fd < 0) {
        fprintf(stderr, "Error: Failed to establish data connection to %s:%d\n", url.ip, url.port);
        closeFtpConnection(socket_fd);
        return 1;
    }
    printf("Data connection established to %s:%d\n", url.ip, url.port);

    // Download the file from the FTP server via the established data connection    if (downloadFileFromFtpServer(data_socket_fd, filename) != 0) {
        fprintf(stderr, "Error: Failed to download file: %s\n", filename);
        close(data_socket_fd);
        closeFtpConnection(socket_fd);
        return 1;
    }
    printf("File downloaded successfully: %s\n", filename);

    // Close both the data connection and the FTP control connection    close(data_socket_fd);
    closeFtpConnection(socket_fd);

    return 0;
}
