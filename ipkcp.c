#include <stdio.h>
#include <string.h>
#include <stdlib.h>         // atoi
#include <sys/socket.h>     // socket
#include <netdb.h>          // gethostbyname
#include <arpa/inet.h>      // htons

// TODO
//  - check adresy v argumentu


// rozdil mezi win a unix bude mozna v adresach neceho, struktura sockaddr_in nebo tak

int main(int argc, char** argv){
    if(argc != 7){                          // Number of args
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

    if(atoi(argv[4]) < 0){                  // checks if port number isn't negative
        fprintf(stderr, "ERROR: Wrong port parameter: %s.\n", argv[4]);
        return 1;
    }

    char conType[4];                        //connection type (UDP/TCP)
    strcpy(conType, argv[6]);

    char host_address[16];               
    strcpy(host_address, argv[2]);

    uint16_t port_number = atoi(argv[4]);

    // UDP - no need to disconnect if an error occurs
    if(strcmp(conType, "udp") == 0){        
        int family = AF_INET;               // ipv4
        int type = SOCK_DGRAM;
        int client_socket = socket(family, type, 0);

        // SOCKET() - creating socket
        if(client_socket <= 0){     
            fprintf(stderr, "ERROR occured while creating socket.\n");
            return 1;
        }
    
        // Getting the message to send, getting server info
        struct hostent *server = gethostbyname(host_address);         // Gets info about server. Parameter takes either address or name
        if(server == NULL){
            fprintf(stderr, "ERROR no such host %s\n", host_address);
            return 1;
        }

        struct sockaddr_in server_address;
        memset(&server_address, 0, sizeof(server_address));          // fills server address with zeros

        server_address.sin_family = AF_INET;
        server_address.sin_port = htons(port_number);               // sets port that the socket will use. 
                                                                    // htonl() translates an unsigned long integer into network byte order
        
        memcpy(&server_address.sin_addr.s_addr, server->h_name, server->h_length); 

        printf("INFO: Server socket: %s : %d \n", 
               inet_ntoa(server_address.sin_addr),                  // server's in address
               ntohs(server_address.sin_port));                     // server's in port

        // SENDTO() - sending message to server
        // struct sockaddr *address = (struct sockaddr *) &server_address;
        // int address_size = sizeof(server_address);
        // int flags = 0;
    }

    if(strcmp(conType, "tcp") == 0){        // TCP
    }

    printf("kokot\n");
    
    return 0;
}
