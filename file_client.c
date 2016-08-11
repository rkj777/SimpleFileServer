#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>
#define BACKIP "127.0.0.1"
#define BUFLEN 1024

//Handles time out
 void alarm_handler(int signum){
	 printf("Aborting...Server unresponsive for more than 5 seconds\n");
	 exit(1);
 }

int main(int argc, char* argv[]){
	char* IP; 
	int port_number;
	char* fileName;

	struct sockaddr_in server;
	int sid ,server_len = sizeof(server);
	char buffer[BUFLEN];
	char input[BUFLEN];

    //Check correct values
	if(argc != 4){
		printf("Invalid  Inputs\n");
		return 1;	
	}
	
    //Get the values
	IP  = argv[1];
	port_number = atoi(argv[2]);
	fileName = argv[3];

    //Making the socket
	if ((sid= socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1){
		printf("Error in creating the socket");
		return 1;
	}
	
	memset((char*) &server, 0, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_port = htons(port_number);
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	
	if(inet_aton(IP, &server.sin_addr) == 0){
		printf("IP could not be converted\n");
		return 1;
	} 

	//Setting up the signal handler
	signal(SIGALRM, alarm_handler);

    //Sending the file name to the server
	bzero(buffer,BUFLEN);
	if(sendto(sid, fileName, strlen(fileName), 0, (struct sockaddr *) &server,server_len) == -1){
		printf("send error\n");
		return 1;
	}
	bzero(input, sizeof(input));
    //Getting data from the sever
	
    while(1){
		bzero(buffer, BUFLEN);

        //Setting a wait of 5 seconds
		alarm(5);
		if(recvfrom(sid,buffer,BUFLEN,0, (struct sockaddr *) &server, &server_len) == -1){
			printf("receive error\n");
			return 1;
		}
		alarm(0);

		printf("%s\n", buffer);
		//Checking for the last sent value 
        if(buffer[0] == '$' && strlen(buffer) == 1){
			break;
		}
	}

	close(sid);
	return 0;
}	
