#ifndef CLIENTETCP_H
#define CLIENTETCP_H

#include "getIP.h"

// Function to create a TCP socket and connect to the specified IP and port
int createTcpSocketAndConnect(const char *ip, int port);

// Function to authenticate with the FTP server using the provided username and password
int authenticateWithFtpServer(int sockfd, const char *user, const char *password);

// Function to switch FTP connection to passive mode and retrieve IP and port
int enterPassiveModeAndGetIpPort(int sockfd, URL *url);

// Function to request the specified resource (file) from the FTP server
int requestFileFromFtpServer(int sockfd, const char *path);

// Function to download the file from the FTP server via the established data connection
int downloadFileFromFtpServer(int sockfd, const char *filename);

// Function to close the FTP control connection
void closeFtpConnection(int sockfd);

#endif // CLIENTETCP_H
