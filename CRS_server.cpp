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
#include <map>
#include <vector>
#include <iostream>
#define PORT_NO 7777
#define MAX 10

using namespace std;

struct entry
{
	vector<int> file_owner;
};

int connfd[MAX]={0};

int port_number_client=5004;
char buffer[500];
char command[100];
char argument[100];

map<int,int> client_info;

map<string,entry> file_info;

void* read(void *arg)
{
	int* con = (int*)arg;
	cout<<"\nRead thread for Client "<<*con<<" Executed!!"<<endl;
	while(1)
	{	
		read(*con,buffer,500);
		sscanf(buffer,"%s%s",command,argument);

		cout<<"\nClient ID "<<*con<<" requested :: "<<buffer<<endl;

		if(!strcmp(command,"search"))
		{
			vector<int> clients_list = file_info[string(argument)].file_owner;
			int size_list = clients_list.size();
			
			send(*con,&size_list,sizeof(int),0);
			if(size_list == 0)	continue;		
			
			int element;
			for(int i=0;i<clients_list.size();i++)
			{
				element = clients_list[i];			
				send(*con,&element,sizeof(int),0);
			}

			int choice;
			recv(*con,&choice,sizeof(int),0);
			
			int port_no = 0;
			
			if(!(choice<1 || choice>size_list))
			{
				int con_id = clients_list[choice-1];
				port_no = client_info[con_id];
			} 

			int con_id = clients_list[choice-1];
			send(con_id,argument,100,0);
			send(*con,&port_no,sizeof(int),0);
		}
	
		else if(!strcmp(command,"share"))
		{
			if(file_info.find(string(argument))!=file_info.end())
			{
				file_info[string(argument)].file_owner.push_back(*con);
			}
			else
			{
				entry temp;
				temp.file_owner.push_back(*con);
				file_info[string(argument)]=temp;
			}
			int ack=7;
			send(*con,&ack,sizeof(int),0);
		}
		if(!strcmp(command,"quit"))
		{
			close(*con);
			cout<<"\nClient ID "<<*con<<" left !!"<<endl;
			break;
		}
		else 
			cout<<"\nInvalid CRS Command"<<endl;	
	}
}


int main()
{

	int listenfd = 0;
	struct sockaddr_in serv_addr;
	listenfd = socket(AF_INET,SOCK_STREAM,0);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(PORT_NO);
	
	bind(listenfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr));

	listen(listenfd,10);
	pthread_t listen_client[MAX];
	while(1)
	{
		for(int i =0;i<MAX;i++)
		{
			connfd[i] = accept(listenfd,(struct sockaddr*)NULL,NULL);
			printf("\nNew Connection with ID: %d\n",connfd[i]);
			send(connfd[i],&port_number_client,sizeof(int),0);
			client_info[connfd[i]]=port_number_client;
			pthread_create(&listen_client[i],NULL,&read,&connfd[i]);
			port_number_client++;			
		}
		for(int i=0;i<MAX;i++)
			pthread_join(listen_client[i],NULL);
		
	}
	
	return 0;
}
