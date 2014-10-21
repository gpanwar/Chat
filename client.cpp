#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <string>
#include <cstring>
#include "Message.h"
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sstream>
#include <signal.h>
#include <vector>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

#define SOCK_PORT 10914
#define MAX_CHAR  512


using namespace std;

struct sockaddr_in serv_addr;
int sock_stat;
int bind_stat;
int conn_stat;
char *in_usr_nm;
char *out_usr_nm;
MESSAGE buff_msg;

char* getUsrNm();
int getChoice();
int getActiveUsers();
int beginChat();
void* writeMsg();
void* recv_msg();
void exitApp1();
void saveToBuffer(MESSAGE msg_send);

void* exitApp(int sig){
	cout << "Chat session ended\n";
	//TODO send msg to server that the client has exited out of the chat room.
	MESSAGE msg_send;
	msg_send.msg_type = 6;
	//msg_send.payload = "END$" + in_usr_nm;
	strcpy(msg_send.payload,"END$");
	strcat(msg_send.payload,in_usr_nm);
	if(sendto(sock_stat, &msg_send, sizeof(msg_send), 0, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) == -1){
	//if(send(sock_stat, msg_send, sizeof(msg_send),0) == -1){
		cout << "Unable to send message to server.\n\n";
		exitApp1();
	}
	close(sock_stat);
	exit(0);
}

int main(int argc, char* argv[]){

	char *serv_ip_addr;
	
	pthread_t thr1, thr2;
	if (signal(SIGINT,(void *)exitApp)==0)
	//signal(SIGTSTP,(void *)exitApp);
	if(argc != 2){
		cout << "Pass Server IP address as an input argument\n";
		cout << "Ex: ./a.out <ip_addr>\n";
		return (0);
	}else{ 
		serv_ip_addr = argv[1];
	}
	cout << "Server IP adrress: " << serv_ip_addr << "\n";

	// Create socket
	sock_stat = socket(AF_INET, SOCK_STREAM, 0);

	if(sock_stat == -1){
		cout << "Error creating client socket\n";
		return(0);
	}else
		cout << "Socket created\n";

	serv_addr.sin_family 		= AF_INET;
	serv_addr.sin_port		= SOCK_PORT;
	serv_addr.sin_addr.s_addr	= inet_addr(serv_ip_addr);

	
	/*bind_stat = bind(sock_stat, (struct sockaddr *)&serv_addr, sizeof(serv_addr));	//bind IP and Port to Socket

	if(-1 == bind_stat){
		cout << "Error binding port to socket\n";
		return(0);
	}else
		cout << "Socket bound to Port: " << SOCK_PORT << "\n";*/
	
	conn_stat = connect(sock_stat, (struct sockaddr *) &serv_addr, sizeof(serv_addr));

	if(conn_stat == -1){
		cout << "Error connecting to Server\n";
		cout << "\tServer IP: " << serv_ip_addr << "\n";
		cout << "\tPort: " << SOCK_PORT << "\n";
		return(0);
	}else{
		cout << "Connected to Server:\n";
		cout << "\tServer IP: " << serv_ip_addr << "\n";
		cout << "\tPort: " << SOCK_PORT << "\n";
	}
	
	int bRcvd = 0;
	char rcvBuffer[100];
	if((bRcvd = recv(sock_stat, rcvBuffer, 99, 0)) == -1){
		cout << "Unable to receive message to server. Retry.\n\n";
	}
	cout << "Test message from server: " << rcvBuffer << "\n";
	in_usr_nm = getUsrNm();	// Read username
	if(!in_usr_nm){
		return (0);
	}
	cout << "Username \"" << in_usr_nm << "\" approved by server\n";

	//thread to send writeMsg
	pthread_create(&thr1, NULL, (void *)writeMsg, (void *)sock_stat);
	//thread to receive fetchMsg
	pthread_create(&thr2, NULL, (void *)recv_msg, (void *)sock_stat);
	
	cout << "Socket connection closed\n";
	close(sock_stat);
	return (0);
}

char* getUsrNm(){

	int bRcvd = -1;
	int bSent = 0;
	char rcvBuffer[100];
	int retryCount = 0;
	MESSAGE msg_send;
	MESSAGE msg_rcv;
	char* usr_nm;
	while(bRcvd < 0 && retryCount < 5){
		cout << "Enter username (limited to 10 characters):\n";
		cin >> msg_send.payload;
		//sock_stat
		msg_send.msg_type = 0;
		//strcpy(message.payload,usr_nm);
		saveToBuffer(msg_send);
		if(sendto(sock_stat, &msg_send, sizeof(msg_send), 0, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) == -1){
		//if(send(sock_stat, (void *)msg_send, sizeof(msg_send),0) == -1){
			retryCount++;
			cout << "Unable to send message to server. Retry.\n\n";
			continue;
		}
		if((bRcvd = recvfrom(sock_stat, &msg_rcv, sizeof(msg_rcv), 0, (struct sockaddr*) &serv_addr, sizeof(serv_addr))) == -1){
		//if((bRcvd = recv(sock_stat, msg_rcv, sizeof(msg_rcv), 0)) == -1){
			retryCount++;
			cout << "Unable to receive message to server. Retry.\n\n";
			continue;
		}
		if(msg_rcv.payload == "-1"){
			retryCount++;
			cout << "\n\tThe given username is already reserved by another user.\n";
			cout << "\tRetry with another username.\n\n";
			continue;
		}
	}
	strcpy(usr_nm, msg_rcv.payload);
	//free(msg_send);
	//free(msg_rcv);
	if(retryCount == 5){
		return NULL;
	}
	return usr_nm;
}

int getChoice(){
	int choice;
	cout << "\n";
	cout << "*************** Main Menu ***************";
	cout << "Enter an option:\n";
	cout << "\t1: List available users\n";
	cout << "\t2: Chat\n";
	cout << "\t3: Exit application\n";
	
	cin >> choice;
	return choice;
}

int getActiveUsers(){
	cout << "Retrieving list of active users...\n";
	MESSAGE msg_send;
	MESSAGE msg_rcv;
	//vector<string> usersList;
	
	msg_send.msg_type = 2;
	saveToBuffer(msg_send);
	if(sendto(sock_stat, &msg_send, sizeof(msg_send), 0, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) == -1){
	//if(send(sock_stat, msg_send, sizeof(msg_send),0) == -1){
		cout << "Unable to send message to server. Retry.\n\n";
		//free(msg_send);
		//free(msg_rcv);
		exitApp1();
	}
	if(recvfrom(sock_stat, &msg_rcv, sizeof(msg_rcv), 0, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) == -1){
		cout << "Unable to receive message to server. Retry.\n\n";
		//free(msg_send);
		//free(msg_rcv);		
		exitApp1();
	}
	if(!msg_rcv.payload){
		//cout << "No active users at this time."
		//free(msg_send);
		//free(msg_rcv);
		return 0;
	}else{
		//istringstream msg(msg_rcv.payload);
		//string s;
		cout << "\n********** List of Active Users **********\n";
		/*while(getline(msg,s,'$')){
			cout << "\t" << s;
			usersList.push_back(s);
		}*/
		cout << msg_rcv.payload;
	}
	cout << "\n";
	//free(msg_send);
	//free(msg_rcv);
	return 1;	
}

int beginChat(){
	int choice = 0;
	bool loopExit = false;
	//vector<string> usersList;
	MESSAGE msg_send;
	MESSAGE msg_rcv;
	while(!loopExit){
		//getActiveUsers();
		if(!getActiveUsers()){
			cout << "No active users available at this time. Returning to main menu.\n";
			return 0;
		}
		cout << "Enter choice:\n";
		cout << "\t1: Enter username to begin chat\n";
		cout << "\t2: Exit\n";
		cin  >> choice;
		switch(choice){
			case 1:	cout << "Enter username:\t";
				cin >> out_usr_nm;
				cout << "\n";
				loopExit = true;
				break;
			case 2:	return 0;
			default: cout << "Enter the right choice (1 or 2)\n";
				break;
		}
		/*if(!validateUserName(out_usr_nm, usersList))
			cout << "Invalid username entered. Please enter the valid username listed below:\n";
		else
			loopExit = true;*/
			
	}
	//msg_send.msg_type = 3;
	//strcpy(msg_send.payload, in_usr_nm + "$" + out_usr_nm);
	return 0;
}

/*int validateUserName(string usr_nm, vector<string> usersList){
	if(find(usersList.begin(),usersList.end(),usr_nm) != usersList.end())
		return 1;
	else
		return 0;
}*/


void* writeMsg(){
	MESSAGE msg_send;
	MESSAGE msg_rcv;
	//char buf_msg[MAX_CHAR];
	bool tempflag;
	while(1){
		tempflag = false;
		while(!tempflag){

			switch(getChoice()){
				case 1: getActiveUsers();
					cout << "\n\n";
					break;
				case 2: beginChat();
					tempflag = true;
					break;
				case 3: cout << "Thank You!\n";
					return;
					break;
				default: "Enter a valid option.\n";
				 	break;
			}
		}
		cout << "Sending request to user \"" << out_usr_nm <<"\"... \n" ;
		msg_send.msg_type = 3;
		strcpy(msg_send.payload, out_usr_nm);
		saveToBuffer(msg_send);
		if(sendto(sock_stat, &msg_send, sizeof(msg_send), 0, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) == -1){
		//if(send(sock_stat, msg_send, sizeof(msg_send),0) == -1){
			cout << "Unable to send message to server. Retry.\n\n";
			//free(msg_send);
			//free(msg_rcv);
			exitApp1();
		}
		if(recvfrom(sock_stat, &msg_rcv, sizeof(msg_rcv), 0, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) == -1){
			cout << "Unable to receive message to server. Retry.\n\n";
			//free(msg_send);
			//free(msg_rcv);		
			exitApp1();
		}
		if(!strcmp(msg_rcv.payload,"ACCEPT")){
			cout << "Request accepted...\n";
			while(1){
				cout << in_usr_nm << ":";
				msg_send.msg_type = 1;
				fgets(msg_send.payload, MAX_CHAR-1, stdin);
				cout << "\n";

				saveToBuffer(msg_send);
				sendto(sock_stat, &msg_send, sizeof(msg_send), 0, (struct sockaddr*) &serv_addr, sizeof(serv_addr));
				//send(sock_stat, msg_send, sizeof(msg_send), 0);
				bzero(msg_send.payload, MAX_CHAR);
			}
		}else if(!strcmp(msg_rcv.payload,"DENY")){
			int ip;
			cout << "Request denied by " << out_usr_nm << "\n";
			cout << "Enter \'1\' to return to main menu else any other number to exit.\n";
			cin >> ip;
			if(ip != 1)
				break;
			else
				continue;
		}else if(!strcmp(msg_rcv.payload,"ERROR")){
			cout << "Invalid username entered...\n";
			cout << "Returning to main menu...\n";
			break;
		}
	}
	pthread_exit((void* )0);
}

void* recv_msg(){
	MESSAGE msg_send;
	MESSAGE msg_rcv;
	int choice;
	while(1){
		if(recvfrom(sock_stat, &msg_rcv, sizeof(msg_rcv), 0, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) == -1){
		//if(recv(sock_stat, msg_rcv, sizeof(msg_rcv), 0) == -1){
			cout << "Unable to receive message to server. Retry.\n\n";
			//free(msg_send);
			//free(msg_rcv);		
			exitApp1();
		}
		switch(msg_rcv.msg_type){
			case 1:	cout << out_usr_nm << ": " << msg_rcv.payload << "\n";
				break;
			case 3: cout << msg_rcv.payload << " has requested to chat with you.\n";
				cout << "Select:\n";
				cout << "\t1: Accept\n";
				cout << "\t2: Return to main menu\n";
				cin  >> choice;
				switch(choice){
					case 1:	msg_send.msg_type = 3;
						strcpy(msg_send.payload,"YES$");
						strcat(msg_send.payload,msg_rcv.payload);
						//msg_send.payload = "YES$" + msg_rcv.payload;
						out_usr_nm = msg_rcv.payload;
						saveToBuffer(msg_send);
						if(sendto(sock_stat, &msg_send, sizeof(msg_send), 0, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) == -1){
						//if(send(sock_stat, msg_send, sizeof(msg_send),0) == -1){
							cout << "Unable to send message to server.\n\n";
							//free(msg_send);
							//free(msg_rcv);
							exitApp1();
						}
						//free(msg_send);
						break;
					case 2:	msg_send.msg_type = 3;
						strcpy(msg_send.payload,"NO$");
						strcat(msg_send.payload,msg_rcv.payload);
						//msg_send.payload = "NO$" + msg_rcv.payload;
						out_usr_nm = msg_rcv.payload;
						saveToBuffer(msg_send);
						if(sendto(sock_stat, &msg_send, sizeof(msg_send), 0, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) == -1){
						//if(send(sock_stat, msg_send, sizeof(msg_send),0) == -1){
							cout << "Unable to send message to server.\n\n";
							//free(msg_send);
							//free(msg_rcv);
							exitApp1();
						}
						cout << "Request denied\n";
						//free(msg_send);
						return;
					default: cout << "Returning to main menu";
						 pthread_exit((void* )0);
						 return;
				}
			case 4: cout << "Resending previous message\n";
				//if(NULL != buff_msg){
				if(sendto(sock_stat, &buff_msg, sizeof(buff_msg), 0, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) == -1){
				//if(send(sock_stat, buff_msg, sizeof(buff_msg),0) == -1){
					cout << "Unable to send message to server.\n\n";
					exitApp1();
				}
				//}
				break;
			case 5: cout << "User: " << out_usr_nm << " is currently not available.\n";
				cout << "Returning to main menu\n";
				pthread_exit((void* )0);
				return;
			case 6: cout << "User "<< out_usr_nm << "has closed the connection\n";
				cout << "Returning to main menu\n";
				pthread_exit((void* )0);
				return;
		}
	}
	pthread_exit((void* )0);
}

void exitApp1(){
	MESSAGE msg_send;
	msg_send.msg_type = 6;
	strcpy(msg_send.payload,"END$");
	strcat(msg_send.payload,in_usr_nm);
	//msg_send.payload = "END$" + in_usr_nm;
	if(sendto(sock_stat, &msg_send, sizeof(msg_send), 0, (struct sockaddr*) &serv_addr, sizeof(serv_addr)) == -1){
	//if(send(sock_stat, msg_send, sizeof(msg_send),0) == -1){
		cout << "Unable to send message to server.\n\n";
	}
	close(sock_stat);
	exit(1);
}

void saveToBuffer(MESSAGE msg_send){
	buff_msg.msg_type = msg_send.msg_type;
	strcpy(buff_msg.payload, msg_send.payload);
}


