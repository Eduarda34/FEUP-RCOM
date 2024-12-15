#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "getIP.h"

// Function to parse an FTP URL and extract user credentials, host, port, and path
int parseFtpUrl(const char *url_str, URL *url) {
    char temp[256];

    // Copy the input URL string into a temporary buffer to avoid modifying the original    
    strncpy(temp, url_str, sizeof(temp) - 1);
    temp[sizeof(temp) - 1] = '\0';

    // Basic validation: URL must start with ftp://
    if (strncmp(temp, "ftp://", 6) != 0) {
        fprintf(stderr, "Error: URL must start with ftp://\n");
        return -1;
    }

    // Skip the "ftp://" part of the URL and point to the user and host part
    char *user_info = temp + 6; 
    char *host_path = strchr(user_info, '@');
    char *path = strchr(user_info, '/');

    // Validate path existence; URL must include a path
    if (!path) {
        fprintf(stderr, "Error: Invalid URL, missing path\n");
        return -1;
    }


    strncpy(url->path, path + 1, sizeof(url->path) - 1);
    url->path[sizeof(url->path) - 1] = '\0';
    *path = '\0'; 

    // Extract user credentials if present, otherwise set default anonymous credentials
    if (host_path) {
        *host_path = '\0'; 
        if (sscanf(user_info, "%63[^:]:%63s", url->user, url->password) != 2) {
            fprintf(stderr, "Error: Invalid user credentials format\n");
            return -1;
        }
        user_info = host_path + 1;
    } else {
        // Default credentials
        strncpy(url->user, "anonymous", sizeof(url->user) - 1);
        strncpy(url->password, "guest", sizeof(url->password) - 1);
    }
    url->user[sizeof(url->user) - 1] = '\0';
    url->password[sizeof(url->password) - 1] = '\0';

    // Extract the host and port information from the URL (default port is 21)    char host[256] = {0};
    int port = 21; 
    if (sscanf(user_info, "%255[^:]:%d", host, &port) != 1 && sscanf(user_info, "%255s", host) != 1) {
        fprintf(stderr, "Error: Invalid host or port format\n");
        return -1;
    }

    // Store the host name and port in the URL structure
    strncpy(url->host, host, sizeof(url->host) - 1);
    url->host[sizeof(url->host) - 1] = '\0';
    url->port = port;

    // Resolve the host to an IP address using getaddrinfo    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET; 
    hints.ai_socktype = SOCK_STREAM; 

    // Get the address information for the host
    if (getaddrinfo(url->host, NULL, &hints, &res) != 0) {
        fprintf(stderr, "Error: Unable to resolve host '%s'\n", url->host);
        return -1;
    }

    // Convert the IP address from binary to string format
    struct sockaddr_in *addr = (struct sockaddr_in *)res->ai_addr;
    if (inet_ntop(AF_INET, &(addr->sin_addr), url->ip, sizeof(url->ip)) == NULL) {
        fprintf(stderr, "Error: Unable to convert IP address to string\n");
        freeaddrinfo(res);
        return -1;
    }

    freeaddrinfo(res);
    return 0;
}
