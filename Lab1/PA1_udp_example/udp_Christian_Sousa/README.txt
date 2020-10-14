Author: Christian F. Sousa
UDP Server


Description:
    This is a simple UDP-server and client group that can do simple commands between one another. They can be run on the same machine or remotly as long as the machines 
    can communicate via internet.


Setting up:

    First decided what IP adress and port number you will use. You may choose any port number 5001-65535, just be sure to use the same port number for both the client and 
    the server. For the ip adress, if both programs are on the same machine you may use "localhost" instead of an ip. If the machines are remote the IP will be the adress 
    that the server program in running. You can find this you by typeing hostname -i into the terminal.

    To set up the client please navigate to the Client directory, then use the "make" command from the makefile. From there use the last command to run the program.
    >> cd ./Client
    >> make client
    >> ./client <IPADDR> <PORTNUM>


    To set up the server plase navigate to the Server directory, then use the make command from the makefile. From that point use the last command to run the program.
    >> cd ./Server
    >> make server
    >> ./server <PORTNUM>

    After use please be sure to navigate to both server and client directories and use the "make clean" command to remove the leftover executables.
    
the Client:
    Upon running the client will print out a menu that is shown below:
        - get [Filename] //gets file from server and puts in on the client
        - put [Filename] //take file from client directoy and puts in on server directory
        - delete [Filename] //removes a file from the server directory
        - ls // lists all files in the server directory
        - exit //exits the program on both clien and server ends.


