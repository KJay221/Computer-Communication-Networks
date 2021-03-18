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
#include <time.h>

void error(const char *msg){
    perror(msg);
    exit(1);
}

struct UDP_Package{
    unsigned long int index;
    char buffer[1024];
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
    //get file size
    struct stat st;
    stat(input_filename, &st);
    write(sockfd,(char *)&st.st_size,8);
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
    //start count time
    clock_t start_time, end_time;
    start_time = clock();

    FILE *output_ptr;
    output_ptr = fopen("test_output.txt","wb");
    //get data size
    int data_size = 0;
    float read_size = 0;
    read(newsockfd,(char *)&data_size,8);
    unsigned char buffer[1024];
    int log[4] = {1,1,1,1};
    double total_time = 0;
    while(1){
        bzero(buffer,1024);
        n = 0;
        n = read(newsockfd,buffer,1024);
        if (n <= 0)
            break;
        fwrite(buffer,sizeof(char),n,output_ptr);

        //log
        read_size+=n;
        float percent = read_size/(float)data_size;
        if(percent >= 0.25 && log[0]){
            end_time = clock();
            printf("25%% %fs\n",((double)(end_time-start_time))/CLOCKS_PER_SEC);
            log[0] = 0;
        }
        else if(percent >= 0.50 && log[1]){
            end_time = clock();
            printf("50%% %fs\n",((double)(end_time-start_time))/CLOCKS_PER_SEC);
            log[1] = 0;
        }
        else if(percent >= 0.75 && log[2]){
            end_time = clock();
            printf("75%% %fs\n",((double)(end_time-start_time))/CLOCKS_PER_SEC);
            log[2] = 0;
        }
        else if(percent >= 1 && log[3]){
            end_time = clock();
            total_time = ((double)(end_time-start_time))/CLOCKS_PER_SEC;
            printf("100%% %fs\n",total_time);
            log[3] = 0;
        }

    }
    //print info
    printf("Total trans time: %fs\n",total_time);
    printf("file size : %fMB\n",(float)data_size/1000000);

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
    int read_size = 0;
    struct UDP_Package buffer;
    int get = 0;
    //get file_size and send to recv
    struct stat st;
    stat(input_filename, &st);
    buffer.index = st.st_size;
    for(int i=0;i<1;i++)
        sendto(sockfd, (char *)&buffer, sizeof(buffer), 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

    buffer.index=0;
    //timeout setting
    struct timeval tv;
	tv.tv_sec = 0; //timeout sec
	tv.tv_usec =  1;
	setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    while(1){
        read_size = 0;
        get = 0;
        bzero(buffer.buffer,1024);
        read_size = fread(buffer.buffer,sizeof(char),1024,read_ptr);
        if(read_size <= 0)
            break;
        for(int i=0;i<100;i++){
            sendto(sockfd, (char *)&buffer, sizeof(unsigned long int)+read_size, 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
            if(recvfrom(sockfd,(char *)&get, sizeof(int), 0,NULL,NULL)>0){
                break;
            }
        }
        buffer.index++;
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
    float recv_size = 0;
    int get = 1;
    struct UDP_Package buffer;
    struct sockaddr_in peeraddr;
    socklen_t peerlen;
    peerlen = sizeof(peeraddr);
    //check file_size
    float file_size = 0;
    recvfrom(sockfd,(char *)&buffer, sizeof(buffer), 0,(struct sockaddr *)&peeraddr, &peerlen);
    file_size = buffer.index;

    while(1){
        bzero(buffer.buffer,1024);
        buffer.index = 0;
        n = 0;
        n = recvfrom(sockfd,(char *)&buffer, sizeof(buffer), 0,NULL,NULL);
        if(buffer.index == 0){
            n-=sizeof(unsigned long int);
            if(n>0){
                fwrite(buffer.buffer,sizeof(char),n,output_ptr);
                recv_size+=n;
            }
            else
                error("fail to get package");
            break;
        }  
    }
    bzero(buffer.buffer,1024);
    n = 0;
    unsigned long int index = 1;
    while(recv_size < file_size){
        n = recvfrom(sockfd,(char *)&buffer, sizeof(buffer), 0,NULL,NULL);
        if(index == buffer.index){
            n-=sizeof(unsigned long int);
            if(n>0){
                fwrite(buffer.buffer,sizeof(char),n,output_ptr);
                recv_size+=n;
                index++;
                sendto(sockfd, (char *)&get, sizeof(int), 0, (struct sockaddr *)&peeraddr, peerlen);
            }
            else
                error("fail to get package");
        }
        bzero(buffer.buffer,1024);
        buffer.index = 0;
        n = 0;
    }

    //close
    close(sockfd);
    fclose(output_ptr);
    return;
}

