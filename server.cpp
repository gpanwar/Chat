#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <string>
#include <cstring>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>




using namespace std;

struct clients{
    struct sockaddr_in client_addrs;
    char name[10];
    int sockFileDesc;
    struct clients *next;
};
struct clients *firstclient = NULL;
struct clients *lastclient = NULL;
struct clients *tempclient = NULL;

#define conn_port 10914  //listening port number for server to accept incoming connections on
#define max_conn 5      //maximum number of connections allowed by server to queue up when it is accepting connections

int main() {

    int sock_filedesc, incomingsock_filedesc,client_list_flag=0;
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

   // while (1)
    //{
        cout << "Listening for incoming connections now..." << endl;

        incomingsock_filedesc = accept(sock_filedesc, (struct sockaddr *)&client_addr, (socklen_t*)&sizeaddr_in);

        //struct client* incoming_client = addnewclient(client_addr, incomingsock_filedesc);
        /*
        
        //new thread to this client ()


    }

    */
    char msg[] = "Test"; 
    char buf[2000];
    int len, bytes_sent;
    len = strlen(msg);
    bytes_sent = send(incomingsock_filedesc, msg, len, 0);
    cout<<"Sent message"<<endl;









    return 0;
}