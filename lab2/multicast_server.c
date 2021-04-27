#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>


struct in_addr localInterface;
struct sockaddr_in groupSock;

int main (int argc, char *argv[ ])
{
	int sd;
	sd = socket(AF_INET, SOCK_DGRAM, 0);
	if(sd < 0){
	  perror("Opening datagram socket error");
	  exit(1);
	}
	else
	  printf("Opening the datagram socket...OK.\n");


	memset((char *) &groupSock, 0, sizeof(groupSock));
	groupSock.sin_family = AF_INET;
	groupSock.sin_addr.s_addr = inet_addr("226.1.1.1");
	groupSock.sin_port = htons(4321);


	localInterface.s_addr = inet_addr("127.0.0.1");
	

	if(setsockopt(sd, IPPROTO_IP, IP_MULTICAST_IF, (char *)&localInterface, sizeof(localInterface)) < 0){
	  perror("Setting local interface error");
	  exit(1);
	}
	else
	  printf("Setting the local interface...OK\n");


	//send msg
	//get filesize
	char *input_filename = argv[1];
	FILE *read_ptr;
    read_ptr = fopen(argv[2],"rb");
    //get file size
    struct stat st;
    stat(input_filename, &st);
	char databuf[100];
	int datalen = 0;
	datalen = sprintf(databuf,"%ld",st.st_size);
	if(sendto(sd, databuf, datalen, 0, (struct sockaddr*)&groupSock, sizeof(groupSock)) < 0){
		perror("Sending datagram message error");
	}
	else
	  printf("Sending datagram message...OK\n");


	return 0;
}
