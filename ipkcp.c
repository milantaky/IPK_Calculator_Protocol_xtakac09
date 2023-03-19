#include <stdio.h>
#include <string.h>
#include <stdlib.h>         // atoi
#include <sys/socket.h>     // socket, sendto
#include <sys/types.h>
#include <netdb.h>          // gethostbyname
#include <arpa/inet.h>      // htons
#include <unistd.h>         // close
#include <signal.h>         // C-c interrupt signal

#define BUFFER_SIZE_UDP 512     
#define BUFFER_SIZE_TCP 1024     

// TODO
//  - check adresy v argumentu


// rozdil mezi win a unix bude mozna v adresach neceho, struktura sockaddr_in nebo tak

static volatile int keepRunning = 1;

// void intHandler() {
//     keepRunning = 0;
// }

void intHandler() {
    keepRunning = 0;
    printf("dostal jsem se sem\n");
    // if(strcmp(mode, "tcp")){
    //     char buff[4] = "BYE";
    //     int bytes_tx = send(socket, buff, strlen(buff), 0);
    //     if(bytes_tx < 0){
    //         fprintf(stderr, "ERROR: send.\n");
    //     }

    //     int bytes_rx = recv(socket, buff, 4, 0);
    //     if(bytes_rx < 0){
    //         fprintf(stderr, "ERROR: recv.\n");
    //     }

    //     printf("%s", buff);                                                   // Server's response

    //     shutdown(socket, SHUT_RD);
    //     shutdown(socket, SHUT_WR);
    //     shutdown(socket, SHUT_RDWR);

    //     close(socket);
    // }

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
    char buffer_udp[BUFFER_SIZE_UDP];
    char buffer_tcp[BUFFER_SIZE_TCP];
    char input[BUFFER_SIZE_UDP];
    char input_tcp[BUFFER_SIZE_TCP];
    int family = AF_INET;                                                       // ipv4

    // Getting the message to send, getting server info 
    struct hostent *server = gethostbyname(host_address);                       // Gets info about server. Parameter takes either address or name
    if(server == NULL){                                                         // Invalid server name
        fprintf(stderr, "ERROR: no such host %s.\n", host_address);
        return 1;
    }

    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));                         // fills server address with zeros

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port_number);                               // sets port that the socket will use. htonl() translates an unsigned long integer into network byte order
    memcpy(&server_address.sin_addr.s_addr, server->h_addr_list[0], server->h_length); 

    printf("INFO: Server socket: %s : %d \n", 
            inet_ntoa(server_address.sin_addr),                                  // server's in address
            ntohs(server_address.sin_port));                                     // server's in port

    socklen_t server_size = sizeof(server_address);    

// UDP ------------------------------------------------------------------------------------------
    if(strcmp(conType, "udp") == 0){        // UDP - no need to disconnect if an error occurs
        int type = SOCK_DGRAM;

    // SOCKET() - creating socket
        int client_socket = socket(family, type, 0);
        if(client_socket <= 0){     
            fprintf(stderr, "ERROR: socket.\n");
            return 1;
        }

        while(keepRunning){
        // MESSAGE
            memset(buffer_udp, 0, BUFFER_SIZE_UDP);                                 // Fills buffer with 0
            
            if(fgets(input, BUFFER_SIZE_UDP, stdin) == NULL){                       // Reads from input 
                fprintf(stderr, "ERROR occured while reading input.\n");
                return 1;
            }

            if(strcmp(input, "exit\n") == 0){
                printf("Exiting...\n");
                return 0;
            }

            int inputLength = (int) strlen(input) - 1;                          // - 1, because '\n' is not considered as part of the payload length
            buffer_udp[0] = 0;                                                      // Opcode (0 = request)
            buffer_udp[1] = inputLength;                                            // Payload Length

            for(int i  = 0; i < inputLength; i++){                              // Fills the payload data area with user input
                buffer_udp[i + 2] = input[i];                                      
            }

        // SENDTO() - sending message to server 
            int bytes_tx = sendto(client_socket, buffer_udp, (inputLength + 2), 0, (struct sockaddr *) &server_address, server_size);    // inputLength + 2, because opcode + payloadLength + input (not counting '\0')
            if(bytes_tx < 0){
                fprintf(stderr, "ERROR: sendto.\n");
            }

            memset(buffer_udp, 0, sizeof(buffer_udp));                                   // Fills buffer with 0
            
        // RECVFROM() - waiting for response
            
            int bytes_rx = recvfrom(client_socket, buffer_udp, BUFFER_SIZE_UDP, 0, (struct sockaddr *) &server_address, &server_size);
            if(bytes_rx < 0){
                fprintf(stderr, "ERROR: recvfrom.\n");
                return 1;
            } 

            int responseStatus = buffer_udp[1];
            int responseLength = buffer_udp[3];
            if(responseStatus == 1){           // STATUS CODE: 0 = OK, 1 = ERR
                fprintf(stderr, "ERR:");
                for(int i = 3; i < responseLength + 3; i++){
                    printf("%c", buffer_udp[i]);
                }
            } else {                            // Had to duplicate it because of the new line (ERR sends \n, OK does not)
                printf("OK:");
                for(int i = 3; i < responseLength + 3; i++){
                    printf("%c", buffer_udp[i]);
                }
                printf("\n");
            }
        }

    }

// TCP ------------------------------------------------------------------------------------------
    if(strcmp(conType, "tcp") == 0){      
    // SOCKET() - creating socket
        int type = SOCK_STREAM;
        int client_socket = socket(family, type, 0);
        if(client_socket <= 0){     
            fprintf(stderr, "ERROR: socket.\n");
            return 1;
        }

    // CONNECT
        if(connect(client_socket, (struct sockaddr *) &server_address, sizeof(server_address)) != 0){
            fprintf(stderr, "ERROR: connect.\n");
            return 1;
        }

        while(keepRunning){
        // MESSAGE
            memset(buffer_tcp, 0, BUFFER_SIZE_TCP);                                     // Fills buffer with 0
            
            if(fgets(input_tcp, BUFFER_SIZE_TCP, stdin) == NULL){                       // Reads from input 
                fprintf(stderr, "ERROR occured while reading input.\n");
                return 1;
            }

            strcpy(buffer_tcp, input_tcp);                                              // Copies input into buffer
    
     // SEND()
            int bytes_tx = send(client_socket, buffer_tcp, strlen(buffer_tcp), 0);
            if(bytes_tx < 0){
                fprintf(stderr, "ERROR: send.\n");
            }

            memset(buffer_tcp, 0, BUFFER_SIZE_TCP);                                     // Fills buffer with 0

    // RECV()
            int bytes_rx = recv(client_socket, buffer_tcp, BUFFER_SIZE_TCP, 0);
            if(bytes_rx < 0){
                fprintf(stderr, "ERROR: recv.\n");
            }

            printf("%s", buffer_tcp);                                                   // Server's response

            if(strcmp(buffer_tcp, "BYE\n") == 0) break;
        }

    // CLOSE()
        if(!keepRunning){ 
            printf("zmacklo se C-c, ukoncuju.\n");


        } else {
            printf("ukoncuju\n");
        }

        shutdown(client_socket, SHUT_RD);
        shutdown(client_socket, SHUT_WR);
        shutdown(client_socket, SHUT_RDWR);

        close(client_socket);
    }

    return 0;
}
