Author: Christian F. Sousa
Network systems PA2


Fuctionaliy:
    This progam is simple web server that is multithreaded such that it can service multiple users at the same time. Please not that this server only supports HTTP connections and the following 
    file extension:
    - HTML
    - TXT
    - PNG
    - gif
    - jpg
    - CSS
    - js


Building and using the server:

    This server can be build with the included make file by simply cding to the directory and typing "make".
    by using the command "make run" the server will then be started on port 8888.
    if you would like to use a different port you can use ./a.out <port> after building the file.
    afther this go to your web broswer and visit ther server at http://localhost:<port>
    