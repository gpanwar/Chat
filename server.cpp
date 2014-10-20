#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <string>
#include <cstring>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "Message.h"



using namespace std;

struct clients{
    struct sockaddr_in client_addrs;    //client address
    char name[10];                      //client name
    int sockFileDesc;                   //client socketdesc
    int id,dest_id;                     //current id, dest_id
    struct clients *next;
};
struct clients *firstclient = NULL;
struct clients *lastclient = NULL;
int current_id=0;

#define conn_port 10914  //listening port number for server to accept incoming connections on
#define max_conn 5      //maximum number of connections allowed by server to queue up when it is accepting connections

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

        incomingsock_filedesc = accept(sock_filedesc, (struct sockaddr *)&client_addr, &sizeaddr_in);

        struct client* incoming_client = addnewclient(client_addr, incomingsock_filedesc);
        
        
        //new thread to this client ()
        pthread_create(&client_thread[current_thread_id], NULL, client_talk, (void *)firstclient);
        current_thread_id++;


    }

    for (int i = 0; i <= current_thread_id; i++)
    {
        pthread_join(client_thread[i], NULL);
    }
    
    

    close(sock_filedesc);

    return 0;
}


struct client* addnewclient(struct sockaddr_in client_addrs, int sockFileDesc)
{
    //setting up the client username


    struct client *ptr = (struct client*)malloc(sizeof(struct client))
        if (ptr == NULL)
        {
        cout << "Problem in memory allocation" << endl;
        return NULL;
        }


    if (current_id == 0)
    {
        current_id++;
        ptr->client_addrs = client_addrs;
        ptr->name = NULL;
        ptr->sockFileDesc = sockFileDesc;
        ptr->id=current_id;
        ptr->dest_id=0;        
        firstclient = lastclient = ptr;

        return ptr;
    }

    //when username accepted
        ptr->client_addrs = client_addrs;
        ptr->name = NULL;
        ptr->sockFileDesc = sockFileDesc;
        ptr->next = NULL;

        lastclient->next = ptr;
        lastclient = ptr;
        
}


int client_talk(int clientSockFileDesc)
{
    //please choose a name
    int nameflag = 0,m;
    struct MESSAGE send_msg_buf,recv_msg_buf; 
    char name[10];
    struct clients *tempclient, *current_client;
    //check name in list
    current_client = firstclient;
    while(current_client->sockFileDesc != clientSockFileDesc)
            current_client=current_client->next;

while(1)
{   

recvmsg:
    m=recv(recv_msg_buf)                                                                                 //========================================================
    if(m==0)
        {//user has closed connection. so tell other user that client lost 
        }

    else if(m==-1)
        { //there was an error in the recieve request so transmit again
          //send user retransmit message
            continue;
        }
    else if(m>0) //succesfully read some bits into buffer, now start sending to other user.
        {   
            switch(recv_msg_buf->msg_type)
            {case 0: //username auth

                    if(current_client->name!=NULL)
                        {//send a message that username already selected
                        send_msg_buf.payload="Username already exists. Your username is ";
                        send_msg_buf.payload.append(current_client->name);
                        send(send_msg_buf);   
                        goto recvmsg;                                                                      //
                        }
                    
                    tempclient = firstclient;
                    strcpy(name,recv_msg_buf);
                    
                    
                    do
                    {
                    if (!strcmp(name, tempclient->name))
                    nameflag = 1;
                    tempclient = tempclient->next;
                    } while (nameflag == 0);
                    
                    if (nameflag == 1)
                    {
                        send_msg_buf.payload="Username already exists. Please select another username";
                        send(send_msg_buf);                                
                        //error please change name, name taken
                        goto recvmsg;
                    }
                          
                    else    //set name as available
                    {
                      tempclient = firstclient;
                      while (tempclient->sockFileDesc != clientSockFileDesc)
                      tempclient = tempclient->next;
                      strcpy(tempclient->name, name);         //assign name chose as the name for client in the list
                      send()                                    //congrats username accepted
                      goto recvmsg;
                    }
                    break;

             case 1: //Message to be sent
                    {

                    }


                    break;

             case 2:

                    break;
             case 3:






                    break;
             case 4: //error resend
                     send(send_msg_buf);
             default: //error in message bit payload, resend 
                        send_msg_buf.msg_type=4;
                        send_msg_buf.payload=NULL;
                        send(send_msg_buf);             
                        break;

            }










        }










}





}