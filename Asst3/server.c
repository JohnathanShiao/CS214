#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main(int argc, char** argv)
{
    char message[256] = "You have reached the server!";
    int serv_sock;
    serv_sock = socket(AF_INET,SOCK_STREAM,0);
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(9002);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    bind(serv_sock,(struct sockaddr*)&server_addr,sizeof(server_addr));
    listen(serv_sock,1);
    int client_sock;
    client_sock = accept(serv_sock,NULL,NULL);
    send(client_sock, message,sizeof(message),0);
    close(serv_sock);
    return 0;
}