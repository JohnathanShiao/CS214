#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main(int argc, char** argv)
{
    int net_sock;
    net_sock = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(9002);
    server.sin_addr.s_addr = INADDR_ANY;
    int status = connect(net_sock,(struct sockaddr*)&server,sizeof(server));
    if(status < 0)
        printf("There was an error making a connection to the remote socket\n\n");
    char response[256];
    recv(net_sock,&response, sizeof(response),0);
    printf("The server sent back:\n%s",response);
    close(net_sock);
    return 0;
}