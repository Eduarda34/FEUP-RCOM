// Application layer protocol implementation
#include "application_layer.h"


int processReceivedPacket(unsigned char *buffer, int buffer_size, const char *file_path)
{
    static int dest_file_fd;
    switch (buffer[0])
    {
    // DATA PACKET
    case 1:
        if (buffer_size < 4) {
            printf("Invalid data packet size\n");
            return -1;
        }
        
        unsigned data_size = buffer[3] + (256 * buffer[2]);
        
        if (buffer_size < data_size + 4) {
            printf("Data packet size mismatch\n");
            return -1;
        }

        if (write(dest_file_fd, &buffer[4], data_size) < 0) {
            perror("Error writing to destination file");
            return -1;
        }
        return 0;

    // START CONTROL PACKET
    case 2:
        if ((dest_file_fd = open(file_path, O_WRONLY | O_CREAT | O_TRUNC, 0777)) < 0) {
            perror("Error opening destination file");
            return -1;
        }
        return 0;

    // END CONTROL PACKET
    case 3:
        if (close(dest_file_fd) < 0) {
            perror("Error closing destination file");
            return -1;
        }
        // RESET FILE DESCRIPTOR
        dest_file_fd = -1;  
        return 0;
    default:
        printf("Invalid packet received\n");
        return -1;
    }
}

unsigned char *createStartControlPacket(const char *filename, struct stat *file_stat, int l1, int l2) {
    int start_size = 5 + l1 + l2;
    unsigned char *packet = (unsigned char *)malloc(start_size);
    
    if (packet == NULL) {
        return NULL; 
    }

    // INIT WITH 0
    memset(packet, 0, start_size);
    
    // PACKET DETAILS
    packet[0] = 2;          // PACKET ID
    packet[1] = 0;          // CONTROL INFO
    packet[2] = l1;         // LEN FILE
    memcpy(&packet[3], &(file_stat->st_size), l1); 
    packet[3 + l1] = 1;     // FILE NAME
    packet[4 + l1] = l2;    // FILE LEN
    memcpy(&packet[5 + l1], filename, l2); 

    return packet; 
}

unsigned char *createEndControlPacket(const char *filename, struct stat *file_stat, int l1, int l2) {
    int start_size = 5 + l1 + l2;
    unsigned char *endpacket = (unsigned char *)malloc(start_size);
    
    if (endpacket == NULL) {
        return NULL; 
    }

    // PACKET DETAILS
    endpacket[0] = 3;           // ID PACKET
    endpacket[1] = 0;           // CONTROL INFO
    endpacket[2] = l1;          // LEN FILE
    memcpy(&endpacket[3], &(file_stat->st_size), l1); 
    endpacket[3 + l1] = 1;     // FILE NAME
    endpacket[4 + l1] = l2;    // FILE LEN
    memcpy(&endpacket[5 + l1], filename, l2); 

    return endpacket; 
}

unsigned char *createDataPayloadPacket(const unsigned char *msg, ssize_t bytes_read, unsigned packet_number) {
    int packet_size = bytes_read + 4;
    unsigned char *packet = (unsigned char *)malloc(packet_size);
    
    if (packet == NULL) {
        return NULL; 
    }

    packet[0] = 1;                          // DATA ID
    packet[1] = packet_number % 256;        // PACKET NUMBER
    packet[2] = (bytes_read >> 8) & 0xFF;   // LEN FILE HIGH BYTE
    packet[3] = bytes_read & 0xFF;          // LEN FILE LOW BYTE
    memcpy(&packet[4], msg, bytes_read);    

    return packet;
}


void applicationLayer(const char *serialPort, const char *role, int baudRate,
                      int nTries, int timeout, const char *filename)
{
    LinkLayer connectionParameters;
    strncpy(connectionParameters.serialPort, serialPort, sizeof(connectionParameters.serialPort) - 1);
    connectionParameters.serialPort[sizeof(connectionParameters.serialPort) - 1] = '\0';

    printf("-----------------------------------\n");

    if (strcmp(role, "tx") == 0) {
        connectionParameters.role = LlTx;  // Transmitter
        printf("Role stored as: Transmitter (LlTx).\n");

    } else if (strcmp(role, "rx") == 0) {
        connectionParameters.role = LlRx;  // Receiver
    } else {
        perror("Error: Role not stored correctly.\n");
        exit(1);
    }

    connectionParameters.baudRate = baudRate;
    connectionParameters.nRetransmissions = nTries;
    connectionParameters.timeout = timeout;

    if (llopen(connectionParameters) < 0){
        perror("llopen failed.\n");
        exit(EXIT_FAILURE);
    }

    if (connectionParameters.role == LlTx) {
        int file_fd;
        struct stat file_stat;

        if (stat(filename, &file_stat) < 0) {
            perror("Error getting file information.\n");
            exit(EXIT_FAILURE);
        }

        if ((file_fd = open(filename, O_RDONLY)) < 0) {
            perror("Error opening file.\n");
            exit(EXIT_FAILURE);
        }

        int l1 = sizeof(file_stat.st_size);
        int l2 = strlen(filename);      
        unsigned char *packet = createStartControlPacket(filename, &file_stat, l1, l2);
        if (!packet) {
            fprintf(stderr, "Failed to create data packet.\n");
            close(file_fd);
            exit(EXIT_FAILURE);
        }
        
        if (llwrite(packet, 5 + l1 + l2) < 0) {
            perror("llwrite failed.\n");
            free(packet);
            close(file_fd);
            exit(EXIT_FAILURE);
        }

        free(packet);

        unsigned char msg[MSG_MAX_SIZE - 6]; 
        unsigned packet_number = 0;

        while (1) {
            ssize_t bytes_read = read(file_fd, msg, MSG_MAX_SIZE - 10);  
            if (bytes_read < 0) {
                perror("Error reading file\n");
                exit(EXIT_FAILURE);
            }

            // END FILE
            if (bytes_read == 0) {
                break; 
            }

            unsigned char *data_packet = createDataPayloadPacket(msg, bytes_read, packet_number);           
            if (!data_packet) {
                fprintf(stderr, "Failed to create data packet\n");
                close(file_fd);
                exit(EXIT_FAILURE);
            }

            if (llwrite(data_packet, bytes_read + 4) < 0) {
                perror("llwrite failed for data packet\n");
                free(data_packet);
                exit(EXIT_FAILURE);
            }

            printf("Sent packet %d\n", packet_number);
            packet_number++;
            free(data_packet);
        }

        // SEND END PACKET
        unsigned char *end_packet = createEndControlPacket(filename, &file_stat, l1, l2);       
        if (!end_packet) {
            fprintf(stderr, "Failed to create end packet\n");
            close(file_fd);
            exit(EXIT_FAILURE);
        }

        end_packet[0] = 3;
        if (llwrite(end_packet, 5 + l1 + l2) < 0) {
            fprintf(stderr, "llwrite failed for end packet\n");
        }

        free(end_packet);
        close(file_fd);
    }
    // Receiver: Receive the file
    else if (connectionParameters.role == LlRx) {
        unsigned char buf[MSG_MAX_SIZE - 6] = {0};

        while (1) {
            int bytesRead = llread(buf);

            if (bytesRead < 0) {
                fprintf(stderr, "llread failed\n");
                exit(EXIT_FAILURE); 
            }
            if (bytesRead == 0) {
                break; 
            }

            printf("llread: %d bytes read\n", bytesRead);
            if (processReceivedPacket(buf, bytesRead, filename) < 0) {
                fprintf(stderr, "Error parsing packet\n");
                exit(EXIT_FAILURE); 
            }

            // IF END PACKET BREAK 
            if (buf[0] == 3) { 
                break; // Exit on end packet
            }
        }
    }

    else {
        perror("Unidentified Role\n");
        exit(EXIT_FAILURE);
    }

    if (llclose(0) < 0) {
        perror("Error closing the connection\n");
        exit(EXIT_FAILURE);
    }
}