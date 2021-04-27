#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
 
struct sockaddr_in localSock;
struct ip_mreq group;
 
int main(int argc, char *argv[])
{
	int sd;
	sd = socket(AF_INET, SOCK_DGRAM, 0);
	if(sd < 0){
		perror("Opening datagram socket error");
		exit(1);
	}
	else
	printf("Opening datagram socket....OK.\n");

	
	int reuse = 1;
	if(setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse)) < 0){
		perror("Setting SO_REUSEADDR error");
		close(sd);
		exit(1);
	}
	else
		printf("Setting SO_REUSEADDR...OK.\n");
	

	memset((char *) &localSock, 0, sizeof(localSock));
	localSock.sin_family = AF_INET;
	localSock.sin_port = htons(4321);
	localSock.sin_addr.s_addr = INADDR_ANY;
	if(bind(sd, (struct sockaddr*)&localSock, sizeof(localSock))){
		perror("Binding datagram socket error");
		close(sd);
		exit(1);
	}
	else
		printf("Binding datagram socket...OK.\n");
	 

	group.imr_multiaddr.s_addr = inet_addr("226.1.1.1"); 
	group.imr_interface.s_addr = inet_addr("127.0.0.1");  
	if(setsockopt(sd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&group, sizeof(group)) < 0)
	{
		perror("Adding multicast group error");
		close(sd);
		exit(1);
	}
	else
		printf("Adding multicast group...OK.\n");


	//read msg
	FILE *output_ptr;
	char* output_filename;
	output_filename = argc != 2?"text_output.txt":argv[1];
    output_ptr = fopen(output_filename,"wb");
	char databuf[1024];
	int n = -1,file_size = 0,msg_size = 0;
	while(n < file_size){
		bzero(databuf,1024);
		msg_size = recvfrom(sd, databuf, 1024, 0, NULL, NULL);
		if(msg_size < 0){
			perror("Reading datagram message error");
			close(sd);
			exit(1);
		}
		else{
			if(n == -1){
				n = 0;
				file_size = atoi(databuf);
			}
			else{
				fwrite(databuf,sizeof(char),msg_size,output_ptr);	
				n+=msg_size;
			}
		}
	}
	printf("Reading datagram message...OK.\n");
	printf("receive file size: %dkb\n",n/1000);
	fclose(output_ptr);
	

	return 0;
}
