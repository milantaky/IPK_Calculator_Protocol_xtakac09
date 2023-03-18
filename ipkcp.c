#include <stdio.h>
#include <string.h>
#include <stdlib.h>         // atoi
#include <sys/socket.h>     // socket, sendto
#include <netdb.h>          // gethostbyname
#include <arpa/inet.h>      // htons
#include <unistd.h>         // close

#define BUFFER_SIZE 258     // maximum length of payload - 256B + 2B (opcode, payload length)

// TODO
//  - check adresy v argumentu


// rozdil mezi win a unix bude mozna v adresach neceho, struktura sockaddr_in nebo tak

int main(int argc, char** argv){
/*
UDP
- vytvorim socket               DONE
- zjistim info o serveru        DONE
- ziskam zpravu k poslani
- upravim do formatu?
- poslu zpravu
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

    uint16_t port_number = atoi(argv[4]);
    char buffer[BUFFER_SIZE];
    char input[BUFFER_SIZE];

// UDP - no need to disconnect if an error occurs
    if(strcmp(conType, "udp") == 0){        
        int family = AF_INET;                                                       // ipv4
        int type = SOCK_DGRAM;
        int client_socket = socket(family, type, 0);

    // SOCKET() - creating socket
        if(client_socket <= 0){     
            fprintf(stderr, "ERROR: socket.\n");
            return 1;
        }
    
    // Getting the message to send, getting server info ---------------------------------
        struct hostent *server = gethostbyname(host_address);                       // Gets info about server. Parameter takes either address or name
        if(server == NULL){
            fprintf(stderr, "ERROR: no such host %s\n", host_address);
            return 1;
        }

        struct sockaddr_in server_address;
        memset(&server_address, 0, sizeof(server_address));                         // fills server address with zeros

        server_address.sin_family = AF_INET;
        server_address.sin_port = htons(port_number);                               // sets port that the socket will use. 
                                                                                    // htonl() translates an unsigned long integer into network byte order
        
        memcpy(&server_address.sin_addr.s_addr, server->h_name, server->h_length); 

        printf("INFO: Server socket: %s : %d \n", 
               inet_ntoa(server_address.sin_addr),                                  // server's in address
               ntohs(server_address.sin_port));                                     // server's in port

    // SENDTO() - sending message to server

        // REQUEST  

        // operator = "+" / "-" / "*" / "/"                 
        // expr = "(" operator 2*(SP expr) ")" / 1*DIGIT        --->  this means,  that it is possible to send (+ 1 2), or (+ (- 2 4) 4), or (+ (+ 1 2) (+ 1 2))
        //                                                            or that an expression can be 8 or (+ 3 3)
        // query = "(" operator 2*(SP expr) ")"

        // OPCODE: 0 = request, 1 = response

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

        struct sockaddr *address = (struct sockaddr *) &server_address;
        int address_size = sizeof(server_address);                         


        // Fill the buffer -> scan from input, prepare for request
        memset(buffer, 0, sizeof(buffer));                                  // Fills buffer with 0

        if(fgets(input, BUFFER_SIZE, stdin) == NULL){                       // Reads from input 
            fprintf(stderr, "ERROR occured while reading input.\n");
            return 1;
        }

        // I expect that the input is well-formatted -> if not, the server will reply with an error response
            // EXAMPLE:
            // input: (+ 1 2)
            // opcode = 0           (request)
            // payload length = 7    
            // payload data = "(+ 1 2)"

        int inputLength = (int) strlen(input) - 1;                           // - 1, because '\0' is not considered as part of the payload length
        buffer[0] = '0';                                                     // Opcode
        buffer[1] = inputLength + '0';                                       // Payload Length

        for(int i  = 0; i < inputLength; i++){                               // Fills the payload data area with user input
            buffer[i + 2] = input[i];                                      
        }

        // printf("Obsah bufferu - opcode: %c\n", buffer[0]);
        // printf("Obsah bufferu - payload length: %c\n", buffer[1]);
        printf("Obsah bufferu se zpravou: %s\n", buffer);

        int bytes_tx = sendto(client_socket, buffer, strlen(buffer) + 1, 0, address, address_size);    // strlen() + 1, because here we want the '\0' to be counted as well
        if(bytes_tx < 0){
            fprintf(stderr, "ERROR: sendto.\n");
        }
        
    // RECVFROM() - waiting for response
        // Status code: 0 = OK, 1 = ERROR

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

        int bytes_rx = recvfrom(client_socket, buffer, BUFFER_SIZE, 0, address, (socklen_t *) &address_size);
        if(bytes_rx < 0){
            fprintf(stderr, "ERROR: recvfrom.\n");
            return 1;
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
