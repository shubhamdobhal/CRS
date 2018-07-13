#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <pthread.h>
#include <ctype.h>
#include <signal.h>
#include<vector>
#include<iostream>
#include "testget1.h"

//for getting file size using stat()
#include<sys/stat.h>
//for sendfile()
#include<sys/sendfile.h>
//for O_RDONLY
#include<fcntl.h>

using namespace std;
char sendbuffer[500];
char recvbuffer[500];
char command[100];
char argument[100];
int connfd=0;
int crs_sockfd=0;
int sockfd = 0;
int size;

int control_status_client = 0;
int control_status_server = 7;

int port_number;

char add[]="127.0.0.1";

int other_server_port=0;

void *pauses(void *arg)
{
	char control;
	while(control_status_client!=2)
	{
		if(kbhit())
		{
			control=getchar();
			if(control=='p')
			{
				control_status_client = 1;
			}
			else if(control=='r')
			{
				control_status_client= 0;
			}
			
		}	
	}
}

void *server(void *arg)
{

	//printf("Listening on Port number %d\n",port_number);
	int listenfd=0;
	struct sockaddr_in serv_addr_own;

	int file_handler;
	struct stat obj;
	char *f;
	
	listenfd = socket(AF_INET,SOCK_STREAM,0);
	memset(&serv_addr_own,'0',sizeof(serv_addr_own));

	serv_addr_own.sin_family = AF_INET;
	serv_addr_own.sin_addr.s_addr=htonl(INADDR_ANY);
	serv_addr_own.sin_port = htons(port_number);

	bind(listenfd,(struct sockaddr*)&serv_addr_own,sizeof(serv_addr_own));
	
	listen(listenfd,10);

	while(1)
	{
		char file_name[100];
		connfd = accept(listenfd,(struct sockaddr*)NULL,NULL);
		control_status_server = 0;	
		
		recv(crs_sockfd,file_name,100,0);	

		//printf("\nNew Connection with client ID: %d\n",connfd);

		int size = 0;
		char file_char;
		file_handler = open(file_name, O_RDONLY);
		if(file_handler==-1)
		{
			size=0;
			send(connfd,&size,sizeof(int),0);
		}
		else
		{
			stat(file_name,&obj);
			size=obj.st_size;
			int send_bytes = 0;
			send(connfd, &size, sizeof(int),0);
			while(send_bytes<size)
			{
				read(file_handler,&file_char,sizeof(char));
				send(connfd,&file_char,sizeof(char),0);
				send_bytes++;
			}
			system("clear");
			printf("\nFile Request Processed successfully!!\n");
			cout<<"CRS Command $$ ";
		}
		close(connfd);
		control_status_server = 7;
	}
}

void client(){
	
	struct sockaddr_in serv_addr;
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    	{
        	printf("\n Error : Could not create socket \n");
        	return;
    	} 
	memset(&serv_addr,'0',sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(other_server_port);

	if(inet_pton(AF_INET, add, &serv_addr.sin_addr)<=0)
	{
		printf("\n inet_pton error occured\n");
		return;
	} 
	
	if( connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		printf("\n Server Not Found !! \n");
		return;
	}	
	printf("\nServer contacted successfully!!\n");

	int size = 0;
	char *f;
	control_status_client = 0;
	char file_char; 

	pthread_t pause_id;
	pthread_create(&pause_id,NULL,&pauses,(void*)NULL);
	
	recv(sockfd, &size, sizeof(int), 0);
	if(size!=0)
	{
		f=(char*)malloc(size);	
		
		int i=0;
		char hash[100];
		float per;
		int j = 0;
		
		while(i<size)
		{				
			if(control_status_client==1)
			{
				while(control_status_client!=0);
			}			

			recv(sockfd,&file_char,sizeof(char),0);
			f[i++] = file_char;

			if(i%100000==0)
			{
				per = (i*100.0)/size;
				system("clear");
				printf("\t\tSize of File is :: %.2f MB\n\n",(float)size/(1048576));
				printf("\t\t  Download Percent :: %.1f %\n",per);
				if((int)(per)%2==0){
					int res = per/2,k;
					for(k = 0;k<res;k++)
						hash[k]='#';
					hash[k]='\0';
				}					
				printf("\n%s :: %.1f%\n",hash,per);				
			}
						
		}
		system("clear");
		printf("Download Percent :: 100.0 %\n");
		int file_handler = open(argument, O_CREAT | O_EXCL | O_WRONLY, 0666);	
		write(file_handler, f, size);
		printf("\nFile successfully recieved with the name: %s\n",argument);
	}
	else
		printf("\nFile couldn't be located on Server!!\n");

	control_status_client = 2;
	pthread_join(pause_id,NULL);	
	close(sockfd);
}


int main(int argc, char *argv[])
{
	struct sockaddr_in serv_addr_other;	
	
	if(argc != 2)
    	{
        	printf("\n Usage: %s <ip of server> \n",argv[0]);
        	return 1;
    	} 
	
    	if((crs_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    	{
        	printf("\n Error : Could not create socket \n");
        	return 1;
    	} 
	
    	memset(&serv_addr_other, '0', sizeof(serv_addr_other)); 
	
	serv_addr_other.sin_family = AF_INET;
    	serv_addr_other.sin_port = htons(7777); 
	
	if(inet_pton(AF_INET, argv[1], &serv_addr_other.sin_addr)<=0)
	{
		printf("\n inet_pton error occured\n");
		return 1;
	} 
	
	if( connect(crs_sockfd, (struct sockaddr *)&serv_addr_other, sizeof(serv_addr_other)) < 0)
	{
		printf("\n Error : Connect Failed \n");
		return 1;
	}	

	recv(crs_sockfd,&port_number,sizeof(int),0);

        //connection of other client	
	
	pthread_t server_thread;
	pthread_create(&server_thread,NULL,&server,(void *)NULL);
	
	while(1)
	{	
		//while(control_status_server!=7);		

		printf("\nCRS Command $$ ");
		scanf("%s%s",command,argument);
		
		strcpy(sendbuffer,command);
		strcat(sendbuffer," ");
		strcat(sendbuffer,argument);

		write(crs_sockfd,sendbuffer,500);

		if(!strcmp(command,"search"))
		{
			int element;
			int size_list;

			recv(crs_sockfd,&size_list,sizeof(int),0);
			if(size_list==0){
				cout<<"sorry mirror found!!\n";
			}
			else{
				cout<<endl;
				for(int i=0;i<size_list;i++)
				{
					recv(crs_sockfd,&element,sizeof(int),0);
					cout<<i+1<<"---> Client ID : "<<element<<endl;			
				}
				
				cout<<"\nChoose your mirror :: ";
				cin>>element;

				send(crs_sockfd,&element,sizeof(int),0);
				recv(crs_sockfd,&other_server_port,sizeof(int),0);

				if(other_server_port==0)
					cout<<"Invalid Choice"<<endl;
				else
				{
					client();				
				}
			}
		}

		else if(!strcmp(command,"share"))
		{
			int ack=0;
			recv(crs_sockfd,&ack,sizeof(int),0);
			if(ack==7)
				cout<<"\nFile successfully shared"<<endl;
			else
				cout<<"\nSomething went wrong, try again"<<endl;
		}

		else if(!strcmp(command,"quit"))
		{
			close(crs_sockfd);
			exit(0);
		}
		
		else 
		{
			cout<<"\nInvalid CRS Command"<<endl;
		}

	}	
	
	pthread_join(server_thread,NULL);	

	return 0;
}
