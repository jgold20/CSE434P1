// Name of Author(s): Joshua Gold, Sudeep Aravind
// Course Number and Name: CSE 434, Computer Networks
// Semester: Fall 2016
// Project Part: 1
// CSE434

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>

#define MAXCHAR 1000

// Global count
int active_miners = 0;

//Sign log_book file with an id
void sign_log (char username[],  char ip[], int client_port, int coins){
 FILE * fp;
 fp = fopen ("server_log.txt", "a");
 // Username
 fprintf(fp, username);
 fprintf(fp, " ");
 // IP
 fprintf(fp, ip);
 fprintf(fp, " ");
 // Port Number
 fprintf(fp, "%d", client_port);
 fprintf(fp, " ");
 fprintf(fp,"%d",  coins);
 fprintf(fp, "\n");
 fclose(fp);
}

void updateMinerCount(int n) {
 FILE *fp;
 fp = fopen  ("server_log.txt", "w");
 fprintf(fp, "%i\n", n); 
 fclose(fp);
}


// Error handlers, if an error is present the program will sign client out and exit
void write_error (int n, char client_id[]) {
    if (n < 0) {
        perror("ERROR writing to socket");
        exit(1);
    }
}

void read_error (int n, char client_id[]) {
    if (n < 0) {
        perror("ERROR reading from socket");
        exit(1);
    }
}


// Q
void query(int sock) {
	FILE * fp;
	fp = fopen ("server_log.txt", "r");
	if(fp == NULL) {
	 printf("NULL FILE");
	 return;
	}
	int n;
	char server_response[256];
	char str[MAXCHAR];
	bzero(server_response,256);
	int i = 0;	
	int c;
	int index = 0;
	if(fp) {
		while((c = getc(fp)) != EOF) {
	 		str[index] = (char) c;	
	 		index = index + 1;
		}
	}
	fclose(fp);
		
	strcpy(server_response, str);
	n = (int) write(sock, server_response, sizeof(server_response));		
		
	//strcpy(server_response, "Qeuery");
	//n = (int) write(sock, server_response, sizeof(server_response));
	return;
}


// Register user
int register_user() {
 int pass = 1;
 return 1;
}

// Check that user is not already connected to the server
int check_user (char client_id[]) {
    FILE *fp;
    int c;
    int count = 0;
    int index = 0;
    char current_id[256];

    bzero(current_id, 256);
    fp = fopen("/tmp/server_log.txt", "r");
    // If file is not NULL
    if (fp) {
        // Read until EOF
        while ((c = getc(fp)) != EOF) {
            current_id[index] = (char) c;
            index = index + 1;
            if (c == '\n') {
                strtok(current_id, "\n");
                if (strcmp(client_id, current_id) == 0) {
                    // Increment count when matching ID is found
                    count = count + 1;
                    index = 0;
                    strcpy(current_id, "");
                }
            }
        } /* end while loop */
        // Close the file
        fclose(fp);
        // If count is a multiple of two the user is not currently in the server and the client can proceed
        if (count % 2 == 0) { return 1 ; }
    } // return 0 if the client with a matching ID is currently connected
    return 1;
} // end check_user


// Function to run when a client connects
void client_routine (int sock, char client_ip[], int c_port) {
    int n;
    
    char username[256];
    char buffer[256];
    char server_response[256];
    char ip[256];
    char port[256];
    char coins[256];
    bzero(server_response, 256);
    bzero(buffer,256);
    bzero(username, 256);
    char minerInfo[256];
    bzero(minerInfo, 256);
    // Read from client
    n = (int) read(sock, username, 255);
    read_error(n, username);

    // If check_user came back ok
        // Tells client they are connected
        active_miners = active_miners + 1;
	updateMinerCount(active_miners);
        strcpy(server_response, "logged in");
        n = (int) write(sock, server_response, sizeof(server_response));
        write_error(n, username);
	
        //Client routine loop, will go until and error occurs or client exits
        while (1) {
            bzero(buffer, 256);
            n = (int) read(sock, buffer, 255);
            read_error(n, username);

            printf("%s:", username);
            printf(" %s\n", buffer);
            strtok(buffer, "\n");
	   // user options
            if (strcmp(buffer, "exit") == 0) {
		// Update miner
		active_miners = active_miners - 1;
		updateMinerCount(active_miners);
                strcpy(server_response, "Discoinnected...");
                n = (int) write(sock, server_response, sizeof(server_response));
                write_error(n, username);
                break;
		// Query
            } else if(strcmp(buffer, "query") == 0){ 
		query(sock);
		// Register User
	      } else if(strstr(buffer,"register") != NULL) {
			if(register_user() == 1) {
		//		printf("%s\n", buffer);
				
				// Get data from string 
				// username IP-AAdress port coins
				strcpy(minerInfo, &buffer[9]);
				printf(minerInfo);			
				
				sign_log(username, client_ip, c_port, 0);
				strcpy(server_response, "Success");
				n = (int) write(sock, server_response, sizeof(server_response));
			} else {
				strcpy(server_response, "Failure");
				n = (int) write(sock, server_response, sizeof(server_response));
			 } 
			
		  // Deregester
		} else if(strcmp(buffer, "deregister") == 0) {
			strcpy(server_response, "deregister a miner");
			n = (int) write(sock, server_response, sizeof(server_response));
		    // Save
		    } else if(strcmp(buffer, "save") == 0){
				strcpy(server_response, "File Saved!");
				n = (int) write(sock, server_response, sizeof(server_response));
			} else {
				strcpy(server_response, "Please enter a valid command...."); 
                		n = (int) write(sock, server_response, sizeof(server_response));
            	    	 }
         }
    
} // end client_routine


// Main function
int main(int argc, char *argv[]) {

    // Check for correct command line args
    if (argc != 2) {
        printf("Usage:\tserver <Server Port>\n");
        exit(0);
    }

    // FILE
    FILE *fp;
    fp = fopen("server_log.txt", "a");
    fclose(fp);

    // Sockets
    int client_socket, server_socket;
    int pid;
    int port_number;
    int client_port;
    // Set port number from command line arg
    port_number = atoi(argv[1]);

    // Char buffer for incoming data from client
    char buffer[256];
    socklen_t clilen;
    // define the server address
    struct sockaddr_in server_address, client_addr;
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons((uint16_t) port_number);

    // Bind the socket to our desired IP and port
    if (bind(server_socket, (struct sockaddr*) &server_address, sizeof(server_address)) < 0) {
        perror("ERROR on binding");
        exit(1);
    }

    listen(server_socket, 5);
    clilen = sizeof(client_addr);
    bzero(buffer, 256);
    char str[INET_ADDRSTRLEN];
    

    // Server loop, checks for connection starts a new process with fork
    while (1) {
        // Confirm to client that they are connected to the server
        client_socket = accept(server_socket, (struct sockaddr *) &client_addr, &clilen);
	inet_ntop(AF_INET, &client_addr, str, INET_ADDRSTRLEN);
	client_port = (int)client_addr.sin_port;
	// Get Client Port Number
	
	
	// Check for succesful connection
        if (client_socket < 0) {
            perror("ERROR on accept");
            exit(1);
        }

        // Create child process
        pid = fork();

        if (pid < 0) {
            perror("ERROR on fork");
            exit(1);
        }

        if (pid == 0) {
            close(server_socket);
	   
            client_routine(client_socket, str, client_port);
//	printf("CLIENT QUIT!");
            exit(0);
        } else {
            // Close client socket
            close(client_socket);
        }
    } // end while loop
} // end main
