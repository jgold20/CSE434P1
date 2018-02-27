// Name of Author(s): Joshua Gold, Sudeep Aravind
// Course Number and Name: CSE 434, Computer Networks
// Semester: Fall 2016
// Project Part: 1
// Time Spent: 6 hours

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netdb.h>

// Error handlers
void write_error(int n) {
    if (n < 0) {
        perror("ERROR writing to socket");
        exit(1);
    }
}

void read_error(int n) {
    if (n < 0) {
        perror("ERROR reading from socket");
        exit(1);
    }
}


// Main function
int main (int argc, char *argv[]) {
    // Vars
    struct hostent *server;
    char *client_number;
    char buffer[256], server_response[4096];
    int port_number, client_socket;
    int n;

    // Check that correct command line args where entered
    if (argc != 4) {
        if (argc == 3) {
            printf("Error: please provide a port number\n");
            exit(0);
        } else if (argc == 2) {
            printf("Error: please provide client number and port number\n");
            exit(0);
        } else {
            printf("Error: please provide a hostname, client number and port number\n");\
            exit(0);
        }
    }

    //Connect to an address
    struct sockaddr_in serv_addr;
    client_number = argv[2];
    port_number = atoi(argv[3]);
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) {
        perror("ERROR opening socket");
        exit(0);
    }

    // Server name is a command line arg
    server = gethostbyname(argv[1]);

    // If host name is NULL and the host does not exist
    if (server == NULL) {
        perror("ERROR, no such host");
        exit(0);
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy(server->h_addr, (char *)&serv_addr.sin_addr.s_addr, (size_t) server->h_length);
    serv_addr.sin_port = htons((uint16_t) port_number);

    // Check if connection is successful
    if (connect(client_socket,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) {
        perror("ERROR connecting");
        exit(0);
    }


    bzero(buffer, 256);

    // If connected to server send client number and check that no other clients with that ID are connected
    n = (int) send(client_socket, client_number, strlen(client_number), 0);
    write_error(n);

    bzero(buffer, 256);

    // Server response
    n = (int) recv(client_socket, &server_response, sizeof(server_response), 0);
    read_error(n);

    // Check server response
    strtok(server_response, "\n");
    if (strcmp(server_response, "logged in") == 0) {
        printf("recv from server:pass the client number check\n");
        printf("Type 'exit' to quit session\n");

        // Start client loop, once connected to the server
        while (1) {
            bzero(buffer, 256);
            printf("Please input message: ");
            fgets(buffer, 256, stdin);
            strtok(buffer, "\n");

            // Send message to server
            n = (int) send(client_socket, buffer, strlen(buffer), 0);
            write_error(n);

            // Receive response from server
            n = (int) recv(client_socket, &server_response, sizeof(server_response), 0);
            read_error(n);

            printf("server: %s\n", server_response);

            // on disconnect
            if (strcmp(server_response, "Disconnected...") == 0) { break; }

            strcpy(buffer, "");
        } // end while loop
    } else { printf("duplicate client number... rejected by server\n"); }

    // Close the socket
    close(client_socket);
    return 0;
}
