#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>

#define IP 2130706433  /* 127.0.0.1 */
#define BUFLEN 1024
#define RECEIVE_FILE_LEN 300
#define STRING_TIME_LENGTH 40

//Used to make sure all children processes end correctily

 void child_handler(int signum){
	 waitpid(-1,NULL, 0);
 }
int main(int argc, char* argv[]) {
	int port_number;
	char* logfile_name;
	FILE *read_file;
	FILE *log_file;
	char full_file_path[BUFLEN];
	char received_file[RECEIVE_FILE_LEN];
	pid_t pid;

	time_t current_time;
	struct tm *proper_time;
	time_t end_time;
	char string_current_time[STRING_TIME_LENGTH];
	char string_end_time[STRING_TIME_LENGTH];



	//Check for correct number of inputs
	if(argc != 4){
		printf("Invalid  Inputs\n");
		return 1;
	}


	 //Get the inputs
	 port_number = atoi(argv[1]);




	struct sockaddr_in si_server, si_client;
	int s, i, slen = sizeof(si_client);
	char buf[BUFLEN];

	//File path setting
	char file_path[30];

	strcpy(file_path, argv[2]);
	char abs_path[240];
	realpath(file_path, abs_path);

	//Log file setting
	char logFilePath[40];
	strcpy(logFilePath, argv[3]);
	char log_abs_path[240];
	realpath(logFilePath,log_abs_path);
	log_file = fopen(log_abs_path, "a+");
	if(log_file == NULL){
		printf("LOG file not found\n");
		return 1;
	}

	//Creating a socket	
	if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
		printf("Error in creating the socket");
		return 1;
	}

	//Setting the correct information for the socket
	memset((char *) &si_server, 0, sizeof(si_server));
	si_server.sin_family = AF_INET;
	si_server.sin_port = htons(port_number);
	si_server.sin_addr.s_addr = htonl(INADDR_ANY);

	//Binding the scoekt
	if (bind(s, (struct sockaddr*) &si_server, sizeof(si_server)) == -1) {
		printf("Error in binding the socket");
		return 1;
	}

	printf("\n\nServer listening to %s:%d\n\n", inet_ntoa(si_server.sin_addr),
			ntohs(si_server.sin_port));

	//Setting a signal handler to handle lost children
	signal(SIGCHLD, child_handler);

    //Damonizing the sever
	daemon(1,0);
	while (1) {
		
        printf("Waiting...\n");
		bzero(buf, BUFLEN);
		bzero(received_file, RECEIVE_FILE_LEN);
        //Waiting for the file name to be requested
		if (recvfrom(s, received_file, RECEIVE_FILE_LEN, 0, (struct sockaddr*) &si_client, &slen)
				== -1) {
			printf("receive error\n");
			return 1;
		}

        //Getting the current time
		current_time = time(NULL);
		proper_time = localtime(&current_time);
		strftime(string_current_time, STRING_TIME_LENGTH, "%A, %B %d %H:%M:%S %Y",proper_time);

        
		printf("Received %s\n", received_file);

        //Checking the file
		sprintf(full_file_path, "%s/%s", abs_path, received_file);

		read_file = fopen(full_file_path, "r");
		if (read_file == NULL) {
			fprintf(log_file, "<%s> <%d> <%s> <%s> <File not Found>\n",inet_ntoa(si_client.sin_addr), ntohs(si_client.sin_port),
									received_file, ctime(&current_time));
			perror("Error reading the file");
			continue;
		}
        //Forking if the file is good
		pid = fork();

		if (pid == -1) {
			printf("Fork fail\n");
			return 1;
		}

		//Child process
		if (pid == 0) {

			//while there are things to read
			while (fread(buf, 1, sizeof(buf), read_file) > 0) {
				//sends the chunk of the file
				if (sendto(s, buf, strlen(buf), 0,
						(struct sockaddr*) &si_client, sizeof(si_client))== -1) {
					fprintf(log_file, "<%s> <%d> <%s> <%s> <Transmission error>\n",inet_ntoa(si_client.sin_addr), ntohs(si_client.sin_port),
														received_file, ctime(&current_time));
					printf("send error\n");
					exit(1);
				}
			}

			//End time
			end_time = time(NULL);

			proper_time = localtime(&end_time);
			strftime(string_end_time, STRING_TIME_LENGTH, "%A, %B %d %H:%M:%S %Y",proper_time);

			//Sends a dollar sign
			buf[0] = '$';
			if (sendto(s, buf,1, 0, (struct sockaddr*) &si_client,
					sizeof(si_client)) == -1) {
				printf("send error\n");
				exit(1);
			}

			//Print to file
			fprintf(log_file, "<%s> <%d> <%s> <%s> <%s>\n",inet_ntoa(si_client.sin_addr), ntohs(si_client.sin_port),
						received_file, string_current_time,string_end_time);
			exit(0);
		}

	}
	close(s);
	return 0;
}

