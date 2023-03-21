Project 1 - IPK calculator protocol
Author: Milan Takac - xtakac09
2BIT VUT FIT (BUT)

After compilation, an executable file is created. Using is described in README.

After executing the command, the code will:
- check set parameters
- translate the adress
- connect to server (TCP)
- read user's input from command line
- process the input (UDP): 
    - empty the buffer
    - set opcode to request
    - set payload length
    - add inserted message
    into the buffer
- send request
- empty the buffer
- receive response from server
- print the response
- read user's input from command line in case of repeated queries
- safely disconnect from server (TCP)

In case of caught interrupt signal (C-c), it terminates the communication (UDP). If you're 
communicating using TCP, there is a signal handler that sets variable keepRunning to 0, and
from waiting for user's input it escapes after the while loop, and safely disconnects from 
server.

Code is well documented, so there shouldn't be any problem with trying to understand it.

Different operating systems
There are different things to look for in code like this to be multiplatform. This code is 
written for UNIX systems. 
Few key differences are sockets. Each system uses different library for them. For example 
WINDOWS uses library winsock.h meanwile UNIX uses sys/socket.h. The process of creating them 
is also different. Another thing to take in mind are different byte orders. WINDOWS uses 
little-endian and UNIX uses big-endian. Unfortunately this code isn't implemented for using 
on WINDOWS.

