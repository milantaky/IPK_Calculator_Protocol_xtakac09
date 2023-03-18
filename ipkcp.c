#include <stdio.h>
#include <string.h>
#include <stdlib.h>         // atoi
#include <sys/socket.h>     // socket, sendto
#include <netdb.h>          // gethostbyname
#include <arpa/inet.h>      // htons
#include <unistd.h>         // close

#define BUFFER_SIZE 512     // maximum length of payload - 256B + 2B (opcode, payload length)

// TODO
//  - check adresy v argumentu


// rozdil mezi win a unix bude mozna v adresach neceho, struktura sockaddr_in nebo tak

int main(int argc, char** argv){
/*
UDP
- vytvorim socket               DONE
- bind
- zjistim info o serveru        DONE
- ziskam zpravu k poslani       DONE   
- upravim do formatu?           DONE
- poslu zpravu                  DONE
- cekam na odpoved
- zavru socket                  DONE


*/





// Parameter check -------------------------------------------------------------------
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

    char conType[4];                                                                //connection type (UDP/TCP)
    strcpy(conType, argv[6]);

    char host_address[16];               
    strcpy(host_address, argv[2]);

    int port_number = atoi(argv[4]);
    char buffer[BUFFER_SIZE];
    char input[BUFFER_SIZE];

// UDP - no need to disconnect if an error occurs
    if(strcmp(conType, "udp") == 0){        
        int family = AF_INET;                                                       // ipv4
        int type = SOCK_DGRAM;

    // SOCKET() - creating socket
        int client_socket = socket(family, type, 0);
        if(client_socket <= 0){     
            fprintf(stderr, "ERROR: socket.\n");
            return 1;
        }
    
    // Getting the message to send, getting server info ---------------------------------
        struct hostent *server = gethostbyname(host_address);                       // Gets info about server. Parameter takes either address or name
        if(server == NULL){                                                         // Invalid server name
            fprintf(stderr, "ERROR: no such host %s.\n", host_address);
            return 1;
        }

        struct sockaddr_in server_address;
        memset(&server_address, 0, sizeof(server_address));                         // fills server address with zeros

        server_address.sin_family = AF_INET;
        server_address.sin_port = htons(port_number);                               // sets port that the socket will use. htonl() translates an unsigned long integer into network byte order
        memcpy(&server_address.sin_addr.s_addr, server->h_name, server->h_length);  

        printf("INFO: Server socket: %s : %d \n", 
               inet_ntoa(server_address.sin_addr),                                  // server's in address
               ntohs(server_address.sin_port));                                     // server's in port

    // BIND() - binds socket to port
        // if(bind(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1){
        //     fprintf(stderr, "ERROR: bind.\n");
        //     return 1;
        // }

    // SENDTO() - sending message to server 
        // OPCODE: 0 = request
        //         1 = response
        //
        //         0               1              2               3
        // 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
        // +---------------+---------------+-------------------------------+    OPCODE         --->     + - / *
        // |     Opcode    |Payload Length |          Payload Data         |    PAYLOAD LENGTH --->     length of sent message
        // |      (8)      |      (8)      |                               |    PAYLOAD DATA   --->     message itself
        // +---------------+---------------+ - - - - - - - - - - - - - - - +
        // :                     Payload Data continued ...                :
        // + - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
        // |                     Payload Data continued ...                |
        // +---------------------------------------------------------------+

        //struct sockaddr *address = (struct sockaddr *) &server_address;
        socklen_t server_size = sizeof(server_address);                         

        memset(buffer, 0, BUFFER_SIZE);                                  // Fills buffer with 0

    // MESSAGE
        if(fgets(input, BUFFER_SIZE, stdin) == NULL){                       // Reads from input 
            fprintf(stderr, "ERROR occured while reading input.\n");
            return 1;
        }

        int inputLength = (int) strlen(input) - 1;                          // - 1, because '\0' is not considered as part of the payload length
        buffer[0] = 0;                                                      // Opcode
        buffer[1] = inputLength;                                            // Payload Length

        for(int i  = 0; i < inputLength; i++){                              // Fills the payload data area with user input
            buffer[i + 2] = input[i];                                      
        }
        //close(client_socket);
        //int bytes_tx = sendto(client_socket, buffer, (inputLength + 2), 0, address, server_size);    // inputLength + 3, because opcode + payloadLength + input (not counting '\0')
        int bytes_tx = sendto(client_socket, buffer, (inputLength + 2), 0, (struct sockaddr *) &server_address, server_size);    // inputLength + 3, because opcode + payloadLength + input (not counting '\0')
        if(bytes_tx < 0){
            fprintf(stderr, "ERROR: sendto.\n");
        }
        printf("Bytes sent %d\n", bytes_tx);
        
    // RECVFROM() - waiting for response
        // STATUS CODE: 0 = OK
        //              1 = ERROR
        //
        // 0                   1                   2                   3
        // 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
        // +---------------+---------------+---------------+---------------+
        // |     Opcode    |  Status Code  |Payload Length | Payload Data  |
        // |      (8)      |      (8)      |      (8)      |               |
        // +---------------+---------------+---------------+ - - - - - - - +
        // :                     Payload Data continued ...                :
        // + - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
        // |                     Payload Data continued ...                |
        // +---------------------------------------------------------------+

        memset(buffer, 0, sizeof(buffer));                                   // Fills buffer with 0
        
        int bytes_rx = recvfrom(client_socket, buffer, BUFFER_SIZE, 0, (struct sockaddr *) &server_address, &server_size);
        printf("doslo\n");
        if(bytes_rx < 0){
            fprintf(stderr, "ERROR: recvfrom.\n");
            return 1;
        } else {
            printf("GOT A MESSAGE\n");
        }

        
        if(buffer[0] == '1'){
            printf("responded\n");
        }

        if(buffer[1] == '1'){
            fprintf(stderr, "ERROR: Invalid expression.\n");
        }

        int responseLength = buffer[3];
        printf("OK:");
        for(int i = 3; i < responseLength + 3; i++){
            printf("%c", buffer[i]);
        }
        printf("\n");

        close(client_socket);
    }

    if(strcmp(conType, "tcp") == 0){        // TCP
    }

    printf("kokot\n");
    return 0;
}
