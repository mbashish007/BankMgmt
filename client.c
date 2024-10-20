#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 512

int main() {
    struct sockaddr_in serv;
    int sd;
    char buf[BUFFER_SIZE];
    char wbuf[BUFFER_SIZE];

    // Create socket
    sd = socket(AF_INET, SOCK_STREAM, 0);
    if (sd == -1) {
        perror("Socket creation failed\n");
        exit(1);
    }

    // Set up the server address
    serv.sin_family = AF_INET;
    serv.sin_addr.s_addr = INADDR_ANY;
    serv.sin_port = htons(6049);

    // Connect to the server
    if (connect(sd, (struct sockaddr*)(&serv), sizeof(serv)) == -1) {
        perror("connect\n");
        exit(1);
    }

    // Infinite loop to communicate with the server
    while (1) {
        // First, read message from server
        ssize_t bytesRead = read(sd, buf, BUFFER_SIZE - 1);
        if (bytesRead > 0) {
            buf[bytesRead] = '\0';  // Null-terminate the server message
            printf("Message from server:\n%s\n", buf);
        } else if (bytesRead == 0) {
            // Server has closed the connection
            printf("Server disconnected.\n");
            break;
        } else {
            // An error occurred during read
            perror("Read error\n");
            break;
        }

        // Now, take user input and send it to the server
        // printf("Enter Data: ");
        if (fgets(wbuf, sizeof(wbuf), stdin) == NULL) {
            perror("fgets error\n");
            break;
        }

        // Send the user input to the server
        ssize_t bytesWritten = write(sd, wbuf, strlen(wbuf));
        if (bytesWritten == -1) {
            perror("Write error\n");
            break;
        }
    }

    // Close the socket before exiting
    close(sd);
    return 0;
}
