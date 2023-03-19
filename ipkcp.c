#include <stdio.h>
#include <string.h>
#include <stdlib.h>         // atoi
#include <sys/socket.h>     // socket, sendto
#include <netdb.h>          // gethostbyname
#include <arpa/inet.h>      // htons
#include <unistd.h>         // close
#include <signal.h>

#define BUFFER_SIZE 512     // maximum length of payload - 256B + 2B (opcode, payload length)

// TODO
//  - check adresy v argumentu


// rozdil mezi win a unix bude mozna v adresach neceho, struktura sockaddr_in nebo tak

static volatile int keepRunning = 1;

void intHandler() {
    keepRunning = 0;
}

int main(int argc, char** argv){
    signal(SIGINT, intHandler);                                     // For C-c interrupt

// Parameter check 
    if(argc != 7){                                                  // Number of args
        fprintf(stderr, "ERROR: Wrong number of arguments. %d of 6 needed.\n", argc);
        return 1;
    }

    if(strcmp(argv[1], "-h") != 0 && strcmp(argv[3], "-p") != 0 && strcmp(argv[5], "-m") != 0){ 
        fprintf(stderr, "ERROR: Wrong arguments. Correct form is \"ipkcpc -h <host> -p <port> -m <mode>\"\n");
        return 1;
    }

    if(!(strcmp(argv[6], "tcp") == 0 || strcmp(argv[6], "udp") == 0)){
        fprintf(stderr, "ERROR: Wrong mode \n%s\n - choose \"tcp\" or \"udp\".\n", argv[6]);
        return 1;
    }

    if(atoi(argv[4]) < 0){                                          // Checks if port number isn't negative
        fprintf(stderr, "ERROR: Wrong port parameter: %s.\n", argv[4]);
        return 1;
    }

    char conType[4];                                                // Connection type (UDP/TCP)
    strcpy(conType, argv[6]);

    char host_address[16];               
    strcpy(host_address, argv[2]);

    int port_number = atoi(argv[4]);
    char buffer[BUFFER_SIZE];
    char input[BUFFER_SIZE];
    int family = AF_INET;                                                       // ipv4

    // Getting the message to send, getting server info 
    struct hostent *server = gethostbyname(host_address);                       // Gets info about server. Parameter takes either address or name
    if(server == NULL){                                                         // Invalid server name
        fprintf(stderr, "ERROR: no such host %s.\n", host_address);
        return 1;
    }
    printf("host address %s\n", host_address);

    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));                         // fills server address with zeros

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port_number);                               // sets port that the socket will use. htonl() translates an unsigned long integer into network byte order
    memcpy(&server_address.sin_addr.s_addr, server->h_addr, server->h_length); 

    printf("INFO: Server socket: %s : %d \n", 
            inet_ntoa(server_address.sin_addr),                                  // server's in address
            ntohs(server_address.sin_port));                                     // server's in port

// UDP ------------------------------------------------------------------------------------------
    if(strcmp(conType, "udp") == 0){        // UDP - no need to disconnect if an error occurs
        int type = SOCK_DGRAM;

    // SOCKET() - creating socket
        int client_socket = socket(family, type, 0);
        if(client_socket <= 0){     
            fprintf(stderr, "ERROR: socket.\n");
            return 1;
        }

    // BIND() - binds socket to port
        // if(bind(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1){
        //     fprintf(stderr, "ERROR: bind.\n");
        //     return 1;
        // }

    // SENDTO() - sending message to server 
        socklen_t server_size = sizeof(server_address);                         

    // MESSAGE
        while(keepRunning){
            memset(buffer, 0, BUFFER_SIZE);                                     // Fills buffer with 0
            printf("Napis message:");
            if(fgets(input, BUFFER_SIZE, stdin) == NULL){                       // Reads from input 
                fprintf(stderr, "ERROR occured while reading input.\n");
                return 1;
            }

            if(strcmp(input, "exit\n") == 0){
                printf("Exiting...\n");
                return 0;
            }

            int inputLength = (int) strlen(input) - 1;                          // - 1, because '\n' is not considered as part of the payload length
            buffer[0] = 0;                                                      // Opcode (0 = request)
            buffer[1] = inputLength;                                            // Payload Length

            for(int i  = 0; i < inputLength; i++){                              // Fills the payload data area with user input
                buffer[i + 2] = input[i];                                      
            }

            int bytes_tx = sendto(client_socket, buffer, (inputLength + 2), 0, (struct sockaddr *) &server_address, server_size);    // inputLength + 2, because opcode + payloadLength + input (not counting '\0')
            if(bytes_tx < 0){
                fprintf(stderr, "ERROR: sendto.\n");
            }

            memset(buffer, 0, sizeof(buffer));                                   // Fills buffer with 0
            
        // RECVFROM() - waiting for response
            
            int bytes_rx = recvfrom(client_socket, buffer, BUFFER_SIZE, 0, (struct sockaddr *) &server_address, &server_size);
            if(bytes_rx < 0){
                fprintf(stderr, "ERROR: recvfrom.\n");
                return 1;
            } 

            int responseStatus = buffer[1];
            int responseLength = buffer[3];
            if(responseStatus == 1){           // STATUS CODE: 0 = OK, 1 = ERR
                fprintf(stderr, "ERR:");
                for(int i = 3; i < responseLength + 3; i++){
                    printf("%c", buffer[i]);
                }
            } else {                            // Had to duplicate it because of the new line (ERR sends \n, OK does not)
                printf("OK:");
                for(int i = 3; i < responseLength + 3; i++){
                    printf("%c", buffer[i]);
                }
                printf("\n");
            }
        }

    }

// TCP ------------------------------------------------------------------------------------------
    if(strcmp(conType, "tcp") == 0){        // TCP
        while(keepRunning){
        
        }
        printf("zmacklo se C-c\n");
        // tady doplnit kod pro interrupt aby se to odpojilo, a asi i normalne
    }

    printf("koncim\n");
    return 0;
}
