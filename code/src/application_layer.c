#include "application_layer.h"
#include "link_layer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Function to implement the application layer
void applicationLayer(const char *serialPort, const char *role, int baudrate, int numTries, int timeout, const char *filename) {
    LinkLayer connectionParameters;
    
    // Set the connection parameters
    strcpy(connectionParameters.serialPort, serialPort);
    connectionParameters.baudRate = baudrate;
    connectionParameters.nRetransmissions = numTries;
    connectionParameters.timeout = timeout;

    if (strcmp(role, "tx") == 0) {
        connectionParameters.role = LlTx;  // Transmitter
    } else {
        connectionParameters.role = LlRx;  // Receiver
    }

    // Open the link layer connection
    if (llopen(connectionParameters) < 0) {
        printf("Error opening the connection\n");
        exit(1);
    }

    // Transmitter: Send the file
    if (connectionParameters.role == LlTx) {
        printf("Transmitting file: %s\n", filename);
        // Open file and transmit its content
        FILE *file = fopen(filename, "rb");
        if (!file) {
            perror("Error opening file");
            exit(1);
        }

        unsigned char buffer[256];
        size_t bytesRead;
        while ((bytesRead = fread(buffer, 1, sizeof(buffer), file)) > 0) {
            if (llwrite(buffer, bytesRead) < 0) {
                printf("Error sending data\n");
                exit(1);
            }
        }

        fclose(file);
    } 
    // Receiver: Receive the file
    else if (connectionParameters.role == LlRx) {
        printf("Receiving file: %s\n", filename);
        // Open file for writing the received data
        FILE *file = fopen(filename, "wb");
        if (!file) {
            perror("Error opening file");
            exit(1);
        }

        unsigned char buffer[256];
        int bytesRead;
        while ((bytesRead = llread(buffer)) > 0) {
            fwrite(buffer, 1, bytesRead, file);
        }

        fclose(file);
    }

    // Close the link layer connection
    if (llclose(0) < 0) {
        printf("Error closing the connection\n");
        exit(1);
    }
}
