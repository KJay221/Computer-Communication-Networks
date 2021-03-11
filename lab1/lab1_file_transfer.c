#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <errno.h>

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

void tcp_send(char *hostname,int port,char *input_filename);
void tcp_recv(char *hostname,int port);
void udp_send();
void udp_recv();

int main(int argc, char *argv[]){
    if(strcmp(argv[1],"tcp") == 0 && strcmp(argv[2],"send") == 0)
        tcp_send(argv[3],atoi(argv[4]),argv[5]);
    else if(strcmp(argv[1],"tcp") == 0 && strcmp(argv[2],"recv") == 0)
        tcp_recv(argv[3],atoi(argv[4]));
    else if(strcmp(argv[1],"udp") == 0 && strcmp(argv[2],"send") == 0)
        udp_send();
    else if(strcmp(argv[1],"udp") == 0 && strcmp(argv[2],"recv") == 0)
        udp_recv();
    return 0;
}

void tcp_send(char *hostname,int port,char *input_filename){
    printf("%s\n","tcp_send");
    //start tcp send
    int sockfd, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    //open socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");

    //get host    
    server = gethostbyname(hostname);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(port);
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR connecting");

    //read file
    FILE *read_ptr;
    read_ptr = fopen("test_input.txt","rb");
    unsigned char buffer[100];
    int read_size;
    while(!feof(read_ptr)){
        read_size = 0;
        bzero(buffer,100);
        read_size = fread(buffer,sizeof(char),10,read_ptr);
        if(read_size <= 0)
            break;
        n = write(sockfd,buffer,read_size);
        if (n < 0) 
            error("ERROR writing to socket");
    }

    //close
    close(sockfd);
    return;
}

void tcp_recv(char *hostname,int port){
    printf("%s\n","tcp_recv");
    //start tcp recv
    int sockfd, newsockfd;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;
    int n;

    //open socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");

    //bind
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(port);
    if (bind(sockfd, (struct sockaddr *) &serv_addr,
            sizeof(serv_addr)) < 0)
            error("ERROR on binding");
    
    //listen and accept
    listen(sockfd,5);
    clilen = sizeof(cli_addr);
    newsockfd = accept(sockfd,
                (struct sockaddr *) &cli_addr,
                &clilen);
    if (newsockfd < 0)
        error("ERROR on accept");
    
    //get data
    FILE *output_ptr;
    output_ptr = fopen("test_output.txt","wb");
    unsigned char buffer[100];
    while(1){
        bzero(buffer,100);
        n=0;
        n = read(newsockfd,buffer,100);
        if (n <= 0)
            break;
        fwrite(buffer,sizeof(char),n,output_ptr);
    }

    //close
    close(newsockfd);
    close(sockfd);
    return;
}

void udp_send(){
    printf("%s\n","udp_send");
    return;
}

void udp_recv(){
    printf("%s\n","udp_recv");
    return;
}

