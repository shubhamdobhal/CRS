# CRS
Central Repository Server

This project will only run on linux based operating system.
commands:
  1: to compile server : g++ -pthread CRS_serve -o server
  2: to compile client : g++ -pthread CRS_client -o client
 
After making the executables for server and client :
  steps to run:
    1. put client at different locations.
    2. run server with ./server command.
    3. run client with ./client 127.0.0.1 command.
    4. To share file "share" command is there.
        e.g. share abc.txt
    5. to search file "search" command is there.
        e.g. search abc.txt.
       Then choose the mirror to download the file.
    
