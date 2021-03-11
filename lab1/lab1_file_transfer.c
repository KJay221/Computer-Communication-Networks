#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/stat.h>

void error(const char *msg){
    perror(msg);
    exit(1);
}

struct UDP_Package{
    char buffer[1024];
    long long int index;
};


void tcp_send(char *hostname,int port,char *input_filename);
void tcp_recv(char *hostname,int port);
void udp_send(char *hostname,int port,char *input_filename);
void udp_recv(char *hostname,int port);

int main(int argc, char *argv[]){
    if(strcmp(argv[1],"tcp") == 0 && strcmp(argv[2],"send") == 0)
        tcp_send(argv[3],atoi(argv[4]),argv[5]);
    else if(strcmp(argv[1],"tcp") == 0 && strcmp(argv[2],"recv") == 0)
        tcp_recv(argv[3],atoi(argv[4]));
    else if(strcmp(argv[1],"udp") == 0 && strcmp(argv[2],"send") == 0)
        udp_send(argv[3],atoi(argv[4]),argv[5]);
    else if(strcmp(argv[1],"udp") == 0 && strcmp(argv[2],"recv") == 0)
        udp_recv(argv[3],atoi(argv[4]));
    return 0;
}

void tcp_send(char *hostname,int port,char *input_filename){
    printf("%s\n","tcp_send");
    //start tcp send
    int sockfd, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    //open socket
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0))< 0) 
        error("ERROR opening socket");

    //get host    
    server = gethostbyname(hostname);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }

    //connect
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(port);
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR connecting");

    //send file
    FILE *read_ptr;
    read_ptr = fopen(input_filename,"rb");
    unsigned char buffer[1024];
    int read_size;
    while(!feof(read_ptr)){
        read_size = 0;
        bzero(buffer,1024);
        read_size = fread(buffer,sizeof(char),1024,read_ptr);
        if(read_size <= 0)
            break;
        n = write(sockfd,buffer,read_size);
        if (n < 0) 
            error("ERROR writing to socket");
    }

    //close
    close(sockfd);
    fclose(read_ptr);
    return;
}

void tcp_recv(char *hostname,int port){
    printf("%s\n","tcp_recv");
    //start tcp recv
    int sockfd, newsockfd,n;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;

    //open socket
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0))< 0) 
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
    unsigned char buffer[1024];
    while(1){
        bzero(buffer,1024);
        n = 0;
        n = read(newsockfd,buffer,1024);
        if (n <= 0)
            break;
        fwrite(buffer,sizeof(char),n,output_ptr);
    }

    //close
    close(newsockfd);
    close(sockfd);
    fclose(output_ptr);
    return;
}

void udp_send(char *hostname,int port,char *input_filename){
    printf("%s\n","udp_send");
    //start udp_send
    int sockfd, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    //open socket
    if ((sockfd = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
        error("ERROR opening socket");

    //get host    
    server = gethostbyname(hostname);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }

    //connect
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(port);
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR connecting");

    //send data
    FILE *read_ptr;
    read_ptr = fopen(input_filename,"rb");
    //get file_size and send to recv
    struct stat st;
    stat(input_filename, &st);
    int file_size = st.st_size;
    for(int i=0;i<1000;i++)
        sendto(sockfd, (char *)&file_size, sizeof(file_size), 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    
    char buffer[1024] = {0};
    int read_size = 0;
    while(1){
        read_size = 0;
        bzero(buffer,1024);
        read_size = fread(buffer,sizeof(char),1024,read_ptr);
        if(read_size <= 0)
            break;
        sendto(sockfd, buffer, read_size, 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    }

    //close
    close(sockfd);
    fclose(read_ptr);
    return;
}

void udp_recv(char *hostname,int port){
    printf("%s\n","udp_recv");
    //start udp_recv
    int sockfd,n;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;

    //open socket
    if ((sockfd = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
        error("ERROR opening socket");

    //bind
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR on binding");
    
    //get data
    FILE *output_ptr;
    output_ptr = fopen("test_output.txt","wb");
    //check file_size
    int file_size = 0;
    for(int i=0;i<1000;i++)
        recvfrom(sockfd,(char *)&file_size, sizeof(file_size), 0,NULL,NULL);
    if(file_size <= 0)
        error("please resend again");

    int recv_size = 0;
    unsigned char buffer[1024];
    while(recv_size < file_size){
        bzero(buffer,1024);
        n = 0;
        n = recvfrom(sockfd, buffer, 1024, 0,NULL,NULL);
        if(n>0){
            fwrite(buffer,sizeof(char),n,output_ptr);
            recv_size+=n;
        }
        else
            error("fail to get package");
    }

    //close
    close(sockfd);
    fclose(output_ptr);
    return;
}

