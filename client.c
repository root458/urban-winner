#include <sys/socket.h>
#include <sys/types.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdarg.h>

#include <errno.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <netdb.h>

#define SERVER_PORT 180
#define MAXLINE 4096
#define SA struct sockaddr

void err_n_die(const char *fmt, ...);
char *bin2hex(const unsigned char *input, size_t len);

int main(int argc, char **argv)
{
    int sockfd, n;
    int sendbytes;
    struct sockaddr_in servaddr;
    char sendline[MAXLINE];
    char recvline[MAXLINE];

    // Check if the program has been run right
    // Program is supposed to be run given an ip address
    // E.g ./client 172.217.170.206
    if (argc != 2)
        err_n_die("usage: %s<server address>", argv[0]);

    // Create a socket for the client
    // AF_INET: Address Family-Internet
    // SOCK_STREAM: Stream Socket
    // Two types of sockets: Stream sockets and Datagram sockets
    // With Datagram sockets, a single chunk of data is sent
    // With Stream socket, you can send a stream of data and het a stream back
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        err_n_die("Error while creating the socket!");

    bzero(&servaddr, sizeof(servaddr));
    // Specify address family
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    // Specify the port
    // htons: Host to network, short
    servaddr.sin_port = htons(SERVER_PORT);

    // Convert string representation of IP Address to a binary
    // representation of the address
    // Done by inet_pton() function
    if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0)
        err_n_die("inet_pton error for %s", argv[1]);

    // Connect to the server
    if ((connect(sockfd, (SA *)&servaddr, sizeof(servaddr))) < 0)
        err_n_die("connect failed");

    // Connected:
    // Create a line that is going to be sent
    sprintf(sendline, "GET / HTTP/1.1\r\n\r\n");
    sendbytes = strlen(sendline);

    // Write the line created above into the socket
    // This sends the characters over the network to the server
    if (write(sockfd, sendline, sendbytes) != sendbytes)
        err_n_die("write error");

    // Receive the response
    // Read response from the socket using the read() function
    memset(recvline, 0, MAXLINE);

    while ((n = read(sockfd, recvline, MAXLINE - 1)) > 0)
    {
        printf("%s", recvline);
    }
    // If read returned a negative then an error occurred
    if (n < 0)
        err_n_die("read error");
    // Exit
    exit(0);
}

void err_n_die(const char *fmt, ...)
{
    int errno_save;
    va_list ap;

    errno_save = errno;

    va_start(ap, fmt);
    vfprintf(stdout, fmt, ap);
    fprintf(stdout, "\n");
    fflush(stdout);

    if (errno_save != 0)
    {
        fprintf(stdout, "(errno = %d) : %s\n", errno_save,
                strerror(errno_save));
        fprintf(stdout, "\n");
        fflush(stdout);
    }

    va_end(ap);
    exit(1);
}
