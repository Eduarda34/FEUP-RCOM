#ifndef GETIP_H
#define GETIP_H

/* Default FTP port */
#define FTP_PORT 21

/* Default login credentials for anonymous access */
#define DEFAULT_USER        "rcom"
#define DEFAULT_PASSWORD    "rcom"

/* Data structure to store parsed FTP URL details */
typedef struct {
    char host[256];    // Hostname of the FTP server
    char user[64];     // FTP username
    char password[64]; // FTP password
    int port;          // FTP port (default: 21)
    char path[256];    // Path to the resource on the server
    char ip[16];       // Resolved IP address of the FTP server
} URL;

// Parse an FTP URL and extract its components into a URL structure.
int parseFtpUrl(const char *url_str, URL *url);

#endif // GETIP_H
