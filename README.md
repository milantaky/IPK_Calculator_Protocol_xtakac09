Project 1 - IPK calculator protocol
Author: Milan Takac - xtakac09
2BIT VUT FIT (BUT)

The goal of this project was to code a client, that will be able to send UDP/TCP request to server 
with Arithmetics-as-a-Service (Aaas) with simple equation (+ - * /), receive server's response with 
result and print it to terminal.

The client is started using: command make to compile the source code and get an executable file, then
using command "ipkcpc -h < host > -p < port > -m < mode >" where host is the IPv4 address of the server, 
port the server port, and mode either tcp or udp (e.g., ipkcpc -h 1.2.3.4 -p 2023 -m udp), then you see
SERVER INFO with ip address and port you are sending your requests to. To send a request, click on new 
line under SERVER INFO, insert your message and press ENTER. 

For UDP request send message such as: 
(+ 1 2)
(* 3 (- 4 2))
etc.
To end the programme, write "exit".

For TCP request you first have to send message "HELLO" to establish connection. then you can send messages
such as:
SOLVE (+ 1 2)
SOLVE (* 3 (- 4 2))
etc.
To end the programme, write "BYE".

If you terminate the process by pressing Ctrl + c, you will disconnect from the server, and with mode TCP, 
the connection will end as if you wrote "BYE".

File called ipkcp.c is kept for commit history, because in the process of development this was the 
file I was working on before renaming it to name specified by the assignment.