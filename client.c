#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#define LENGTH 2048
#ifndef TRANSFER_FILE_TRANSFER_H
#define TRANSFER_FILE_TRANSFER_H
#define MAX_LINE 4096
#define LINSTENPORT 7788
#define SERVERPORT 8877
#define BUFFSIZE 4096
#endif //TRANSFER_FILE_TRANSFER_H

int flag1 = 0;
volatile sig_atomic_t flag = 0;
int sockfd = 0;
char name[32];
ssize_t total=0;
int f_flag = 0;
char filename[1000] = {0};

void str_overwrite_stdout();
void str_trim_lf (char* arr, int length);
void catch_ctrl_c_and_exit(int sig);
void send_msg_handler();
void recv_msg_handler();
void sendfile(FILE *fp, int sockfd);
void writefile(int sockfd, FILE *fp);

int main(int argc, char **argv){
	if(argc != 2){
		printf("Usage: %s <port>\n", argv[0]);
		return EXIT_FAILURE;
	}

	char *ip = "127.0.0.1";
	int port = atoi(argv[1]);

	signal(SIGINT, catch_ctrl_c_and_exit);

	printf("Please enter your name: ");
	fgets(name, 32, stdin);
	str_trim_lf(name, strlen(name));


	if (strlen(name) > 32 || strlen(name) < 2){
		printf("Name must be less than 30 and more than 2 characters.\n");
		return EXIT_FAILURE;
	}

	struct sockaddr_in server_addr;

	/* Socket settings */
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(ip);
	server_addr.sin_port = htons(port);


  // Connect to Server
	int err = connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
	if (err == -1) {
		printf("ERROR: connect\n");
		return EXIT_FAILURE;
	}

	// Send name
	send(sockfd, name, 32, 0);

	printf("------------------VCHAT WELCOMES YOU-------------\n");

	printf("Usage: 1 - 'Your text message to be sent here'\n");
	printf("       2 - 'Your path to the file to be sent'\n");
	printf("       Please make sure you enter the index\n");
	printf("       i.e. either 1 or 2 for text or file_transfer respectively\n");

	pthread_t send_msg_thread;
    if(pthread_create(&send_msg_thread, NULL, (void *) send_msg_handler, NULL) != 0){
		printf("ERROR: pthread\n");
    return EXIT_FAILURE;
	}

	pthread_t recv_msg_thread;
  if(pthread_create(&recv_msg_thread, NULL, (void *) recv_msg_handler, NULL) != 0){
		printf("ERROR: pthread\n");
		return EXIT_FAILURE;
	}

	while (1){
		if(flag){
			printf("\nBye\n");
			break;
    }
	}

	close(sockfd);

	return EXIT_SUCCESS;
}

void str_overwrite_stdout() {
  printf("%s", "> ");
  fflush(stdout);
}
void str_trim_lf (char* arr, int length) {
  int i;
  for (i = 0; i < length; i++) { // trim \n
    if (arr[i] == '\n') {
      arr[i] = '\0';
      break;
    }
  }
}
void catch_ctrl_c_and_exit(int sig) {
    flag = 1;
}
void send_msg_handler() {
  char message[LENGTH] = {};
	char buffer[LENGTH + 32] = {};

  while(1) {
  	str_overwrite_stdout();
    fgets(message, LENGTH, stdin);
    str_trim_lf(message, LENGTH);

    if (strcmp(message, "exit") == 0) {
			break;
    } 
    else if(message[0] == '1'){
		sprintf(buffer, "%s: %s\n", name, message+4);
		send(sockfd, buffer, strlen(buffer), 0);
    }
    else if(message[0] == '2'){
    	// char filename[strlen(message)-4];
    	// for(int i=0;i<strlen(message)-4;i++){
    	// 	filename[i] = message[i+4];
    	// }
        strcpy(buffer,"$$$");
        send(sockfd, buffer, strlen(buffer), 0);
    	// printf("message - %s\n",message);
    	// printf("message+4 - %s\n",message+4);
    	strcpy(filename,message);
    	
    	FILE *fp = fopen(message+4, "rb");
	    if (fp == NULL) 
	    {
	        perror("Can't open file");
	        exit(1);
	    }
	    sendfile(fp, sockfd);
        strcpy(buffer,"###");
        send(sockfd, buffer, strlen(buffer), 0);
        printf("Send Success, NumBytes = %ld\n", total);
    }
    else{
    	printf("Please check the below usage\n");
		printf("Usage: 1 - <Your text message to be sent here>\n");
		printf("       2 - <Your path to the file to be sent>\n");
		printf("       Please make sure you enter the index\n");
		printf("       i.e. either 1 or 2 for text or file_transfer respectively\n");
		strcpy(message,"");
    }
	bzero(message, LENGTH);
    bzero(buffer, LENGTH + 32);
  }
  catch_ctrl_c_and_exit(2);
}
void recv_msg_handler() {
	char message[LENGTH] = {};
  while (1) {
		int receive = recv(sockfd, message, LENGTH, 0);
        str_trim_lf(message,LENGTH);
		//printf("message - %s\n",message);
    if (receive > 0) {
        if(strcmp(message,"$$$")==0){
            flag1 = 1;
            char filename[BUFFSIZE]; 
            // if (recv(connfd, filename, BUFFSIZE, 0) == -1) 
            // {
            //     perror("Can't receive filename");
            //     exit(1);
            // }
            strcpy(filename,"Copy1");
            FILE *fp = fopen(filename, "wb");
            if (fp == NULL) 
            {
                perror("Can't open file");
                exit(1);
            }
            
            // char addr[INET_ADDRSTRLEN];
            // printf("Start receive file: %s from %s\n", filename, inet_ntop(AF_INET, &clientaddr.sin_addr, addr, INET_ADDRSTRLEN));
            writefile(sockfd, fp);
            printf("Receive Success, NumBytes = %ld\n", total);
            fclose(fp);
        }
        else if(flag1 == 0){
            printf("%s\n", message);
            str_overwrite_stdout();
        }
    } 
    else if (receive == 0) {
			break;
    } 
    else {
			// -1
		}
		memset(message, 0, sizeof(message));
  }
}
void sendfile(FILE *fp, int sockfd) 
{
    int n; 
    char sendline[MAX_LINE] = {0}; 
    while ((n = fread(sendline, sizeof(char), MAX_LINE, fp)) > 0) 
    {
	    total+=n;
        if (n != MAX_LINE && ferror(fp))
        {
            perror("Read File Error");
            exit(1);
        }
        // char s1[1000];
        // s1[0] = '2';
        // int q;
        // for(q=0;q<strlen(filename);q++){
        //     s1[q+1] = sendline[q];
        // }
        // s1[q+1] = '\0';
        if (send(sockfd, sendline, strlen(sendline), 0) == -1)
        {
            perror("Can't send file");
            exit(1);
        }
        memset(sendline, 0, MAX_LINE);
    }
}
void writefile(int sockfd, FILE *fp)
{
    ssize_t n;
    char buff[MAX_LINE] = {0};
    while (flag1 == 1 && (n = recv(sockfd, buff, MAX_LINE, 0)) > 0) 
    {
        // printf("IDK why!\n");
        // printf("n- %ld, buff -  %s\n",n,buff);
        if(buff[strlen(buff)-3]=='#' && buff[strlen(buff)-2]=='#' && buff[strlen(buff)-1]=='#'){
            flag1 = 0;
        }
	    total+=n;
        if (n == -1)
        {
            perror("Receive File Error");
            exit(1);
        }
        
        if (fwrite(buff, sizeof(char), n, fp) == n)
        {
            // printf("I guess, succeeded\n");
            perror("Write File Status - ");
            //exit(1);
        }
        memset(buff, 0, MAX_LINE);
        // printf("IDK, why - 2\n");
    }
    // printf("even IDK Why\n");
}