// Created by Arif
// March 7, 2015

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <unistd.h>

#define PORT 8882

int server();
int client(char*);
void *send_message(void*);
void *recv_message(void*);


int main(int argc, char *argv[])
{
    if(argc > 2) 
    {
        printf("Invalid number of arguments\n");
        return 1;
    }
    else if(argc == 2)
        client(argv[1]);
    else
        server();

    return 0;
}

int server() 
{
    int err, socket_desc,new_socket, c;
    int option = 1;
    struct sockaddr_in server, client;
    pthread_t sender, receiver;

    // Creating socket
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if( socket_desc == -1 )
    {
        perror("Could not create socket: ");
        return 1;
    }
    setsockopt(socket_desc, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

    puts("Socket Created...");

    // Prepare the sockaddr_in structure
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);

    // Bind
    if(bind(socket_desc, (struct sockaddr *)&server, sizeof server) < 0)
    {
        perror("Error while binding: ");
        return 1;
    }
    puts("Binding Done...");

    //Listen
    listen(socket_desc , 0);

    puts("Awaiting Incoming Connections...");
    // Accept Connection
    c = sizeof (struct sockaddr_in);
    new_socket = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c);
    if(new_socket < 0)
    {
        perror("Accept Failed: ");
        return 1;
    }

    shutdown(socket_desc, 2);
    puts("Connection Accepted...\n");

    //Creating sender and receiver threads
    err = pthread_create(&sender, NULL, send_message, (void *)(intptr_t)new_socket );
    if (err != 0) 
        perror("Could not create sender thread");

    err = pthread_create(&receiver, NULL, recv_message, (void *)(intptr_t)new_socket );
    if (err != 0)
        perror("Could not create receiver thread");

    //Wait for threads
    pthread_join(sender, NULL);

    return 0;
}


int client(char *ip) {
    int err, socket_desc, c;
    struct sockaddr_in server;
    pthread_t sender, receiver;

    // Creating socket
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if( socket_desc == -1 )
    {
        perror("Could not create socket: ");
        return 1;
    }

    puts("Socket Created...");

    // Prepare the sockaddr_in structure
    server.sin_addr.s_addr = inet_addr(ip);
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);

    //Connect remote server
    if( err = connect(socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0 ) 
    {
        perror("");
        return 1;
    }
    
    puts("Connected...");

    //Creating Sender and Receiver Threads 
    err = pthread_create(&receiver, NULL, send_message, (void*)(intptr_t)socket_desc);
    if (err != 0)
        perror("Could not create thread");

    err = pthread_create(&sender, NULL, recv_message, (void*)(intptr_t)socket_desc);
    if (err != 0)
        perror("Could not create thread");


    //Wait for Threads
    pthread_join(sender, NULL);
    pthread_join(receiver, NULL);
    
    return 0;
}

void *send_message(void *ptr)
{
    char message[2000];
    int socket = (intptr_t)ptr;

    while(strcmp(message, "exit\n"))
    {
        fflush(stdin);
        memset(message, '\0', sizeof message);
        fgets(message, 2000, stdin);
        if(send(socket, message, strlen(message), 0) < 0)
        {
            perror("Send Failed: ");
            return;
        }
    }
}

void *recv_message(void *ptr)
{
    char message[2000];
    int socket = (intptr_t)ptr;
    while(1)
    {
        memset(message, '\0', sizeof message);
        recv(socket, message, 2000, 0);
        if(!strcmp(message, "exit\n"))
            break;
        printf(">>%s", message);
    }
}