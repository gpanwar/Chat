#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <string>
#include <cstring>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

#define SOCK_PORT 10914
#define MAX_CHAR  512


using namespace std;

struct sockaddr_in serv_addr;
    int sock_stat;
    int bind_stat;
    int conn_stat;
char* getUsrNm();

int main(int argc, char* argv[]){

    char *serv_ip_addr;
    char *usr_nm;
    
    pthread_t t1,t2;

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

    serv_addr.sin_family        = AF_INET;
    serv_addr.sin_port      = SOCK_PORT;
    serv_addr.sin_addr.s_addr   = inet_addr(serv_ip_addr);

    
    /*bind_stat = bind(sock_stat, (struct sockaddr *)&serv_addr, sizeof(serv_addr));    //bind IP and Port to Socket

    if(-1 == bind_stat){
        cout << "Error binding port to socket\n";
        return(0);
    }else
        cout << "Socket bound to Port: " << SOCK_PORT << "\n";*/
    
    conn_stat = connect(sock_stat, (struct sockaddr *) &serv_addr, sizeof(serv_addr));

    if(conn_stat == -1){
        cout << "Error connecting to Server\n";
        cout << "\tServer IP: " << serv_ip_addr << "\n";
        cout << "\tPort: " << SOCK_PORT;
        return(0);
    }else{
        cout << "Connected to Server:\n";
        cout << "\tServer IP: " << serv_ip_addr << "\n";
        cout << "\tPort: " << SOCK_PORT << "\n";
    }
    /*usr_nm = getUsrNm();  // Read username
    if(!usr_nm){
        return (0);
    }
    cout << "Username " << usr_nm << " approved by server\n";*/
    int bRcvd = 0;
    char rcvBuffer[100];
    if((bRcvd = recv(sock_stat, rcvBuffer, 99, 0)) == -1){
            //retryCount++;
            cout << "Unable to receive message to server. Retry.\n\n";
            //continue;
        }
    cout << "Message from server: " << rcvBuffer << "\n";
    close(sock_stat);
    return (0);
}

char* getUsrNm(){

    char* usr_nm;
    int bRcvd = -1;
    int bSent = 0;
    char rcvBuffer[100];
    int retryCount = 0;
    while(bRcvd < 0 && retryCount < 5){
        cout << "Enter username (limited to 10 characters):\n";
        cin >> usr_nm;
        //sock_stat
        if(send(sock_stat, usr_nm, (strlen(usr_nm) - 1),0) == -1){
            retryCount++;
            cout << "Unable to send message to server. Retry.\n\n";
            continue;
        }
        if((bRcvd = recv(sock_stat, rcvBuffer, 99, 0)) == -1){
            retryCount++;
            cout << "Unable to receive message to server. Retry.\n\n";
            continue;
        }
        if(rcvBuffer == "-1"){
            retryCount++;
            cout << "\n\tThe given username is already reserved by another user.\n";
            cout << "\tRetry with another username.\n\n";
            continue;
        }
    }
    if(retryCount == 5){
        return NULL;
    }
    return usr_nm;
}
