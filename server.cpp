#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <string>
#include <cstring>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "Message.h"
#include <stdlib.h>



using namespace std;

struct clients{
    struct sockaddr_in client_addrs;    //client address
    char name[10];                      //client name
    int sock_filedesc;                   //client socketdesc
    int id,dest_id;                     //current id, dest_id
    struct clients *next;
};

struct comm_msg_buf {
    int msg_type;       //0 - Username auth //1 - Message //2 - Menu list of users  //3 - Auth user for chat // 4 - errpr resend // 5- dest mismatch
    char payload[512];
    int user_id1, user_id2;
    struct comm_msg_buf *next;
};

struct clients *firstclient = NULL;
struct clients *lastclient = NULL;
struct comm_msg_buf *first_msg=NULL;
struct comm_msg_buf *last_msg=NULL;
int current_id=0,current_msgid=0;
pthread_t client_thread[10];
int current_thread_id;

#define conn_port 10914  //listening port number for server to accept incoming connections on
#define max_conn 5      //maximum number of connections allowed by server to queue up when it is accepting connections

clients* addnewclient(sockaddr_in client_addrs, int sock_filedesc);
int client_talk(int clientSockFileDesc);

int main() {

    int sock_filedesc, incomingsock_filedesc;
    pthread_t thread_id;
    struct sockaddr_in server_addr, client_addr;
    cout << "Server Initializing, Please stand By. . . " << endl;
    cout << "Creating socket . . ." << endl;
    sock_filedesc = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_filedesc == -1)                        // error checking!
    {
        cout << "Error" << endl;
    }


    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); //set ip address for the server
    server_addr.sin_port = conn_port; // set port for listening
    cout << "Server is online!" << endl;
    cout << "IP ADDRESS: \t" << inet_ntoa(server_addr.sin_addr) << endl;  //display ip address to user for port number
    int sizeaddr_in = sizeof(struct sockaddr_in);
    if (bind(sock_filedesc, (struct sockaddr *) &server_addr, sizeof(struct sockaddr)) == -1)
    {
        cout << "Binding Failed, Server has crashed" << endl;
        return 0;
    }

    cout << "Bind successful" << endl;

    if (listen(sock_filedesc, max_conn) == -1)
    {
        cout << "Listening Failed, Server has crashed" << endl;
        return 0;
    }

    /*cout << "Listening for incoming connections now..." << endl;

    incomingsock_filedesc = accept(sock_filedesc, (struct sockaddr *)&client_addr, (socklen_t*)&sizeaddr_in);

    char msg[] = "Test"; 
    char buf[2000];
    int len, bytes_sent;
    len = strlen(msg);
    bytes_sent = send(incomingsock_filedesc, msg, len, 0);
    cout<<"Sent message"<<endl;
    */
    while (1)
    {
        cout << "Listening for incoming connections now..." << endl;

        incomingsock_filedesc = accept(sock_filedesc, (struct sockaddr *)&client_addr, (socklen_t*)&sizeaddr_in);

        struct clients* incoming_client = addnewclient(client_addr, incomingsock_filedesc);
        
        
        //new thread to this client ()
        pthread_create(&client_thread[current_thread_id], NULL, client_talk, (void *)sock_filedesc);
        current_thread_id++;


    }

    for (int i = 0; i <= current_thread_id; i++)
    {
        pthread_join(client_thread[i], NULL);
    }
    
    

    close(sock_filedesc);

    return 0;
}


clients* addnewclient(sockaddr_in client_addrs, int sock_filedesc)
{
    //setting up the client username


    struct clients *ptr = (struct clients*)malloc(sizeof(struct clients));
        if (ptr == NULL)
        {
        cout << "Problem in memory allocation" << endl;
        return NULL;
        }


    if (current_id == 0)                                    //if first client in list
    {
        current_id++;
        ptr->client_addrs = client_addrs;
        strcpy(ptr->name,"\0");
        ptr->sock_filedesc = sock_filedesc;
        ptr->id=current_id;
        ptr->dest_id=0;
        ptr->next=NULL;        
        firstclient = lastclient = ptr;
        current_id++;
        return ptr;
    }

        ptr->client_addrs = client_addrs;                   //if its second client or more
        strcpy(ptr->name,"\0");
        ptr->sock_filedesc = sock_filedesc;
        ptr->id=current_id;
        ptr->dest_id=0;   
        current_id++;
        lastclient->next = ptr;
        lastclient = ptr;
	
        return ptr;
        
}
struct comm_msg_buf* addcomm_msg_buf(int clientid1, int clientid2)
{
    //setting up the common msg buffer


    struct comm_msg_buf *ptr = (struct comm_msg_buf*)malloc(sizeof(struct comm_msg_buf));
        if (ptr == NULL)
        {
        cout << "Problem in memory allocation of common memory buffer" << endl;
        return NULL;
        }
        
        if (current_msgid == 0)                             //if its first msg buffer
        {
        current_msgid++;
        ptr->user_id2=clientid2;
        ptr->user_id1=clientid1; 
        ptr->msg_type=0;
        strcpy(ptr->payload,"\0");
       
        first_msg = last_msg = ptr;
        current_msgid++;
        return ptr;
        }

        ptr->msg_type=0;
        strcpy(ptr->payload,"\0");
        ptr->user_id2=clientid2;
        ptr->user_id1=clientid1;
        ptr->next=last_msg;
        last_msg=ptr;
        return ptr;        
}
void copy_into_comm_buff(struct comm_msg_buf *comm_buf,struct MESSAGE send_msg_buf)
{       
        strcpy(comm_buf->payload,send_msg_buf.payload);
        comm_buf->msg_type=send_msg_buf.msg_type;

}

int client_talk(int clientSockFileDesc)
{
    //please choose a name
    int nameflag = 0,m,i,flag;
    string authentication;
    struct MESSAGE send_msg_buf,recv_msg_buf; 
    char name[10];
    struct clients *tempclient, *current_client;
    struct comm_msg_buf *current_msg_buf;
    //check name in list
    current_client = firstclient;
    current_msg_buf = first_msg;
    tempclient = firstclient;
    while(current_client->sock_filedesc != clientSockFileDesc)
            current_client=current_client->next;

    while(1)
    {   

        recvmsg:
	m = recvfrom(clientSockFileDesc, &recv_msg_buf, sizeof(recv_msg_buf), 0, (struct sockaddr*) &current_client->client_addrs, (socklen_t*)sizeof(current_client->client_addrs));
        //m=recv(recv_msg_buf)                                                                                 //========================================================
        if(m==0)
            {//user has closed connection. so tell other user that client lost 
            if((current_msg_buf->user_id1 == current_client->id && current_msg_buf->user_id2 == current_client->dest_id) || (current_msg_buf->user_id2 == current_client->id && current_msg_buf->user_id1 == current_client->dest_id))
                    {
                        strcpy(send_msg_buf.payload,"Other user has closed connection");
                        send_msg_buf.msg_type=6;
                        tempclient=firstclient;
                        while(tempclient->id!=current_client->dest_id)                          
                            tempclient=tempclient->next;
                        //send(send_msg_buf,tempclient->sock_filedesc);                                                      // _______________________________________OMMMMMMMMMMMMM________________________________________             
			sendto(tempclient->sock_filedesc, &send_msg_buf, sizeof(send_msg_buf), 0, (struct sockaddr*) &tempclient->client_addrs, sizeof(tempclient->client_addrs));
                       //free the current_client from linked list 
			if(current_client==firstclient)
                             firstclient=current_client->next;

                        else 
                        {
                            tempclient=firstclient;    
                            while(tempclient->next!=current_client)
                                tempclient=tempclient->next;
                            tempclient->next=current_client->next;
                            while(tempclient->next!=NULL)
                                tempclient=tempclient->next;
                            lastclient=tempclient;
                        }                                                                                             
			free(current_client);
                    }
            }

        else if(m==-1)
            { //there was an error in the recieve request so transmit again
              //send user retransmit message
                send_msg_buf.msg_type=4;
                strcpy(send_msg_buf.payload,"\0");
                //send(send_msg_buf); 
		sendto(clientSockFileDesc, &send_msg_buf, sizeof(send_msg_buf), 0, (struct sockaddr*) &current_client->client_addrs, sizeof(current_client->client_addrs));
                goto recvmsg;
            }
        else if(m>0) //succesfully read some bits into buffer, now start sending to other user.
            {   
                switch(recv_msg_buf.msg_type)
                { 
                case 0: //username auth
                            if(current_client->name!=NULL)
                            {//send a message that username already selected
                            strcpy(send_msg_buf.payload,"Username already exists. Your username is ");
                            strcat(send_msg_buf.payload,current_client->name);
			sendto(clientSockFileDesc, &send_msg_buf, sizeof(send_msg_buf), 0, (struct sockaddr*) &current_client->client_addrs, sizeof(current_client->client_addrs));
                            //send(send_msg_buf);   
                            goto recvmsg;                                                                      //
                            }
                        
                            tempclient = firstclient;
                            strcpy(name,recv_msg_buf.payload);
                        
                            
                            do
                            {
                            if (!strcmp(name, tempclient->name))
                            nameflag = 1;
                            tempclient = tempclient->next;
                            } while (nameflag == 0);
                         
                            if (nameflag == 1)
                            {
                            strcpy(send_msg_buf.payload,"Username already exists. Please select another username");
			sendto(clientSockFileDesc, &send_msg_buf, sizeof(send_msg_buf), 0, (struct sockaddr*) &current_client->client_addrs, sizeof(current_client->client_addrs));
                            //send(send_msg_buf);                                
                            //error please change name, name taken
                            goto recvmsg;
                            }
                              
                            else    //set name as available
                            {
                            tempclient = firstclient;
                            while (tempclient->sock_filedesc != clientSockFileDesc)
                                tempclient = tempclient->next;
                            strcpy(tempclient->name, name);         //assign name chose as the name for client in the list
                            current_client=tempclient;
                            strcpy(send_msg_buf.payload,"Username accepted");
                            send_msg_buf.msg_type=0;
                            //send(current_client->sock_filedesc,send_msg_buf)  ;                                  //congrats username accepted
				sendto(current_client->sock_filedesc, &send_msg_buf, sizeof(send_msg_buf), 0, (struct sockaddr*) &current_client->client_addrs, sizeof(current_client->client_addrs));
                            goto recvmsg;
                            }
                            break;

                 case 1: //Message to be sent
                            current_msg_buf = first_msg;
                            while(!(current_msg_buf->user_id1 == current_client->id && current_msg_buf->user_id2 == current_client->dest_id) || (current_msg_buf->user_id2 == current_client->id && current_msg_buf->user_id1 == current_client->dest_id))                                       //buffer exists
                                {
                                    if(current_msg_buf->next==NULL)
                                       {strcpy(send_msg_buf.payload,"Please select a partner first\n");
                                        //send(send_msg_buf); 
					sendto(clientSockFileDesc, &send_msg_buf, sizeof(send_msg_buf), 0, (struct sockaddr*) &current_client->client_addrs, sizeof(current_client->client_addrs));
                                        goto recvmsg;
                                        }
                                    current_msg_buf=current_msg_buf->next;
                                }         


                            if(current_client->dest_id==0)
                            {
                                send_msg_buf.msg_type=5;
                                strcpy(send_msg_buf.payload,"");
                                //send(send_msg_buf); 
				sendto(clientSockFileDesc, &send_msg_buf, sizeof(send_msg_buf), 0, (struct sockaddr*) &current_client->client_addrs, sizeof(current_client->client_addrs));
                                goto recvmsg;
                            }

                            else if(current_client->dest_id >= current_id)
                            {
                                send_msg_buf.msg_type=5;
                                strcpy(send_msg_buf.payload,"Your chat partner has already left the server\n");
                                //send(send_msg_buf); 
				sendto(clientSockFileDesc, &send_msg_buf, sizeof(send_msg_buf), 0, (struct sockaddr*) &current_client->client_addrs, sizeof(current_client->client_addrs));
                                goto recvmsg;
                            }

                            else
                            {   if(tempclient->id != current_client->dest_id)
                                {   tempclient=firstclient;
                                    while(tempclient->id != current_client->dest_id)
                                        tempclient=tempclient->next;
                                }
                                send_msg_buf=recv_msg_buf;
                                copy_into_comm_buff(current_msg_buf,send_msg_buf);
                                //send(tempclient->sock_FileDesc,send_msg_buf);    
				sendto(clientSockFileDesc, &send_msg_buf, sizeof(send_msg_buf), 0, (struct sockaddr*) &current_client->client_addrs, sizeof(current_client->client_addrs));
                            }
                            break;

                 case 2: //list of users
                            {
                                current_client=firstclient;
                                strcpy(send_msg_buf.payload,"");
                                //send_msg_buf.payload.append(current_client->name);
                                //send_msg_buf.payload.append("\n");
				strcat(send_msg_buf.payload,current_client->name);
				strcat(send_msg_buf.payload,"\n");
                                while(current_client->next)
                                    {
                                        current_client=current_client->next;
                                        //send_msg_buf.payload.append(current_client->name);
                                        //send_msg_buf.payload.append("\n");
					strcat(send_msg_buf.payload,current_client->name);
					strcat(send_msg_buf.payload,"\n");
                                    }
                               // send(send_msg_buf); //sending list of users to client
				sendto(clientSockFileDesc, &send_msg_buf, sizeof(send_msg_buf), 0, (struct sockaddr*) &current_client->client_addrs, sizeof(current_client->client_addrs));
                                goto recvmsg;
                            }

                            break;
                 case 3:       i=0;
                                while(recv_msg_buf.payload[i]!='$')
                                    {
                                        i++;

                                    }
                                if(strlen(recv_msg_buf.payload)==i)             //new request for connection
                                    {
                                        tempclient=firstclient;
                                        while(tempclient->name!=recv_msg_buf.payload)
                                            tempclient=tempclient->next;
                                        
                                        if(tempclient->name!=recv_msg_buf.payload)//error in selection of username
                                            {
                                              send_msg_buf.msg_type=3;
                                              strcpy(send_msg_buf.payload,"ERROR");
                                              //send(send_msg_buf);
						sendto(clientSockFileDesc, &send_msg_buf, sizeof(send_msg_buf), 0, (struct sockaddr*) &current_client->client_addrs, sizeof(current_client->client_addrs));
                                            }

                                        else
                                            {
                                                send_msg_buf=recv_msg_buf;
                                                current_client->dest_id=tempclient->id;
                                                //send(tempclient->sock_filedesc,send_msg_buf);
						sendto(tempclient->sock_filedesc, &send_msg_buf, sizeof(send_msg_buf), 0, (struct sockaddr*) &tempclient->client_addrs, sizeof(tempclient->client_addrs));
                                            }      
                                    }
                                else if(recv_msg_buf.payload[i]=='$')            //reply for authentication
                                    {
                                        strcpy(name,"\0");
                                        i--;
					for(int j=0; j < i; j++){
						name[j] = recv_msg_buf.payload[j];
					}
                                        //name.append(recv_msg_buf.payload,0,i);
                                        i+=2;
                                        if(!strcmp(name,"YES"))
                                            {   strcpy(name,"\0");
						for(int j=0;recv_msg_buf.payload[i]!='\0';i++,j++)
								name[j]=recv_msg_buf.payload[i];
                                                //name.append(recv_msg_buf.payload,i,strlen(recv_msg_buf.payload)+1-i);
                                                tempclient=firstclient;
                                                while(tempclient->name!=name)
                                                    tempclient=tempclient->next;

                                                if(tempclient->dest_id==current_client->id)                //connection established for both
                                                    {current_client->dest_id=tempclient->id;
                                                     strcpy(send_msg_buf.payload,"ACCEPT");
                                                     //send(tempclient->sock_filedesc,send_msg_buf);
							sendto(tempclient->sock_filedesc, &send_msg_buf, sizeof(send_msg_buf), 0, (struct sockaddr*) &tempclient->client_addrs, sizeof(tempclient->client_addrs));
                                                     struct comm_msg_buf *ptr= addcomm_msg_buf(current_client->id,tempclient->id);
                                                    }
                                                else //wrong accept/deny msg recieved
                                                    {
                                                        strcpy(send_msg_buf.payload,"ERROR");
                                                    }


                                            }
                                        if(!strcmp(name,"NO"))
                                            {   strcpy(name,"\0");
							for(int j=0;recv_msg_buf.payload[i]!='\0';i++,j++)
								name[j]=recv_msg_buf.payload[i];
                                                //name.append(recv_msg_buf.payload,i,strlen(recv_msg_buf.payload)+1-i);
                                                tempclient=firstclient;
                                                while(tempclient->name!=name)
                                                    tempclient=tempclient->next;

                                                if(tempclient->dest_id==current_client->id)                //connection cancelled for both
                                                    {
                                                     current_client->dest_id=0;
                                                     tempclient->dest_id=0;
                                                     strcpy(send_msg_buf.payload,"DENY");
                                                     //send(tempclient->sock_filedesc,send_msg_buf);
							sendto(tempclient->sock_filedesc, &send_msg_buf, sizeof(send_msg_buf), 0, (struct sockaddr*) &tempclient->client_addrs, sizeof(tempclient->client_addrs));
                                                    }

                                                else //wrong accept/deny msg recieved
                                                   {
                                                     strcpy(send_msg_buf.payload,"ERROR");
                                                   }

                                            }
                                     else
                                        {   send_msg_buf.msg_type=4;                                        //----------------------------------------------------------------------------
                                            strcpy(send_msg_buf.payload,"");
                                           // send(send_msg_buf); 
						sendto(tempclient->sock_filedesc, &send_msg_buf, sizeof(send_msg_buf), 0, (struct sockaddr*) &tempclient->client_addrs, sizeof(tempclient->client_addrs));
                                            goto recvmsg;
                                        }                   
                                    }
                            break;
                 case 4: //error resend
                            if(current_msg_buf!=NULL && (current_msg_buf->user_id1 == current_client->id && current_msg_buf->user_id2 == current_client->dest_id) || (current_msg_buf->user_id2 == current_client->id && current_msg_buf->user_id1 == current_client->dest_id))
                                {
                                    strcpy(send_msg_buf.payload,current_msg_buf->payload);
                                    send_msg_buf.msg_type = current_msg_buf->msg_type;
                                }
                           //send(send_msg_buf);
				sendto(tempclient->sock_filedesc, &send_msg_buf, sizeof(send_msg_buf), 0, (struct sockaddr*) &tempclient->client_addrs, sizeof(tempclient->client_addrs));
                            goto recvmsg;
                            break;
                 case 5:    send_msg_buf.msg_type=5;
                            strcpy(send_msg_buf.payload,"");
                            //send(send_msg_buf); 
				sendto(tempclient->sock_filedesc, &send_msg_buf, sizeof(send_msg_buf), 0, (struct sockaddr*) &tempclient->client_addrs, sizeof(tempclient->client_addrs));
                            goto recvmsg;
                            break;
                 case 6:    //connection  to client terminated
                            if((current_msg_buf->user_id1 == current_client->id && current_msg_buf->user_id2 == current_client->dest_id) || (current_msg_buf->user_id2 == current_client->id && current_msg_buf->user_id1 == current_client->dest_id))
                            {
                                strcpy(send_msg_buf.payload,"Other user has closed connection");
                                send_msg_buf.msg_type=6;
                                tempclient=firstclient;
                                while(tempclient->id!=current_client->dest_id)                          
                                tempclient=tempclient->next;
                                //send(send_msg_buf,tempclient->sock_filedesc);                                                      // _______________________________________OMMMMMMMMMMMMM________________________________________             
				sendto(tempclient->sock_filedesc, &send_msg_buf, sizeof(send_msg_buf), 0, (struct sockaddr*) &tempclient->client_addrs, sizeof(tempclient->client_addrs));
                            }
                                //free the current_client from linked list
				if(current_client==firstclient)
                             firstclient=current_client->next;

                            else 
                            {
                                tempclient=firstclient;    
                                while(tempclient->next!=current_client)
                                    tempclient=tempclient->next;
                                tempclient->next=current_client->next;
                                while(tempclient->next!=NULL)
                                    tempclient=tempclient->next;
                                lastclient=tempclient;
                            }
				free(current_client);
                            break;
                 default: //error in message bit payload, resend 
                            send_msg_buf.msg_type=4;
                            strcpy(send_msg_buf.payload,"");
                            //send(send_msg_buf); 
				sendto(tempclient->sock_filedesc, &send_msg_buf, sizeof(send_msg_buf), 0, (struct sockaddr*) &tempclient->client_addrs, sizeof(tempclient->client_addrs));
                            goto recvmsg;            
                            break;

                }//end switch

                //}
            }//end else if
    }//end infinite while
}//end client_talk              
