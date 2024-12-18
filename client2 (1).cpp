#include <iostream> 
#include <string> 
#include <stdio.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <arpa/inet.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <netdb.h> 
#include <sys/uio.h> 
#include <sys/wait.h> 
#include <fcntl.h> 
#include <fstream> 
#include <regex>  
#include <algorithm> 
using namespace std; 
 
void TCPclient(char* serverIp, int port) { 
    //create a message buffer  
    char msg[1500]; 
    //setup a socket and connection tools  
    struct hostent* host = gethostbyname(serverIp); 
    sockaddr_in sendSockAddr; 
    bzero((char*)&sendSockAddr, sizeof(sendSockAddr)); 
    sendSockAddr.sin_family = AF_INET; 
    sendSockAddr.sin_addr.s_addr = 
        inet_addr(inet_ntoa(*(struct in_addr*)*host->h_addr_list)); 
    sendSockAddr.sin_port = htons(port); 
    int clientSd = socket(AF_INET, SOCK_STREAM, 0); 
    //try to connect... 
    int status = connect(clientSd, 
        (sockaddr*)&sendSockAddr, sizeof(sendSockAddr)); 
    if (status < 0) 
    { 
        cout << "Error connecting to socket!" << endl; 
        exit(1); 
    } 
    cout << "Connected to the server!" << endl; 
    while (1) 
    { 
        cout << "Print the expression you want to calculate" << endl; 
        cout << ">"; 
        string data; 
        getline(cin, data); 
        
        regex mask("\\(*\\s?-?\\d+(.\\d+)?\\s?\\)*\\s?[-+*/]\\s?\\(*\\s?-?\\d+(.\\d+)?\\s?\\)*\\s?[-+*/]\\s?\\(*\\s?-?\\d+(.\\d+)?\\s?\\)*"); 
        regex mask2("\\(*\\s?-?\\d+(.\\d+)?\\s?\\)*\\s?[-+*/]\\s?\\(*\\s?-?\\d+(.\\d+)?\\s?\\)*"); 
        while (data == "" || (data != "exit" && !(regex_match(data, mask) && regex_match(data, mask2)) && (count(data.begin(), data.end(), '(') != count(data.begin(), data.end(), ')')))) {     
            cout << "Wrong input, try again<" << data << ">\n"; 
            cout << ">"; 
            getline(cin, data); 
        } 
        
        if (data == "exit") 
        { 
            memset(&msg, 0, sizeof(msg));//clear the buffer 
            strcpy(msg, data.c_str()); 
            send(clientSd, (char*)&msg, strlen(msg), 0); 
            break; 
        } 
        
        memset(&msg, 0, sizeof(msg));//clear the buffer  
        strcpy(msg, data.c_str()); 
        send(clientSd, (char*)&msg, strlen(msg), 0); 
        memset(&msg, 0, sizeof(msg));//clear the buffer  
        recv(clientSd, (char*)&msg, sizeof(msg), 0); 
        cout << "Result: " << msg << endl; 
    } 
    close(clientSd); 
    cout << "Connection closed" << endl; 
} 
 
void UDPclient(char* ip, int port) { 
    //create a message buffer  
    char msg[1500]; 
    //setup a socket and connection tools  
    sockaddr_in sendSockAddr; 
    bzero((char*)&sendSockAddr, sizeof(sendSockAddr)); 
    sendSockAddr.sin_family = AF_INET; 
    sendSockAddr.sin_addr.s_addr = htonl(INADDR_ANY); 
    sendSockAddr.sin_port = htons(port); 
 
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0); 
    if (sockfd < 0) 
    { 
        cerr << "Error establishing the client socket" << endl; 
        exit(0); 
    } 
    // non-blocking 
    fcntl(sockfd, F_SETFL, O_NONBLOCK); 
 
    socklen_t len; 
    
    // checking
    string check = "Client is ready";
    strcpy(msg, check.c_str());
    sendto(sockfd, (char*)&msg, strlen(msg), MSG_CONFIRM, (struct sockaddr*)&sendSockAddr, sizeof(sendSockAddr));
    memset(&msg, 0, sizeof(msg));//clear the buffer 
    while (recvfrom(sockfd, (char*)&msg, sizeof(msg), MSG_WAITALL, (struct sockaddr*)&sendSockAddr, &len) == -1)
        continue;
    cout << msg << endl;
    memset(&msg, 0, sizeof(msg));//clear the buffer    
    
    
    string acknow = "result is received by client";
    while (1) 
    { 
        cout << "Print the expression you want to calculate" << endl; 
        cout << ">"; 
        string data; 
        getline(cin, data); 
        
        regex mask("\\(*\\s?-?\\d+(.\\d+)?\\s?\\)*\\s?[-+*/]\\s?\\(*\\s?-?\\d+(.\\d+)?\\s?\\)*\\s?[-+*/]\\s?\\(*\\s?-?\\d+(.\\d+)?\\s?\\)*"); 
        regex mask2("\\(*\\s?-?\\d+(.\\d+)?\\s?\\)*\\s?[-+*/]\\s?\\(*\\s?-?\\d+(.\\d+)?\\s?\\)*"); 
        while (data == "" || (data != "exit" && !(regex_match(data, mask) && regex_match(data, mask2)) && (count(data.begin(), data.end(), '(') != count(data.begin(), data.end(), ')')))) { 
            cout << "Wrong input, try again\n"; 
            cout << ">"; 
            getline(cin, data); 
        }
        
        if (data == "exit") 
        { 
            memset(&msg, 0, sizeof(msg));//clear the buffer    
            strcpy(msg, data.c_str());
            sendto(sockfd, (char*)&msg, strlen(msg), MSG_CONFIRM, (struct sockaddr*)&sendSockAddr, sizeof(sendSockAddr));   
            bool success = 0;
            for (int i = 0; i < 3 && !success; i++){
                if (recvfrom(sockfd, (char*)&msg, sizeof(msg), MSG_WAITALL, (struct sockaddr*)&sendSockAddr, &len) == -1){
                   sleep(3);
                   if (recvfrom(sockfd, (char*)&msg, sizeof(msg), MSG_WAITALL, (struct sockaddr*)&sendSockAddr, &len) == -1){
                        sendto(sockfd, (char*)&msg, strlen(msg), MSG_CONFIRM, (struct sockaddr*)&sendSockAddr, sizeof(sendSockAddr)); 
                    }
                    else
                        success = 1;
                }
                else
                    success = 1;
            }
            if (success){
                cout << msg << endl;
                break;
            }
            else{
                cout << "Connection with server is lost" << endl;
                break;
            }
        } 
        
        memset(&msg, 0, sizeof(msg));//clear the buffer    
        strcpy(msg, data.c_str());
        sendto(sockfd, (char*)&msg, strlen(msg), MSG_CONFIRM, (struct sockaddr*)&sendSockAddr, sizeof(sendSockAddr));
        bool success = 0;
        for (int i = 0; i < 3 && !success; i++){
            if (recvfrom(sockfd, (char*)&msg, sizeof(msg), MSG_WAITALL, (struct sockaddr*)&sendSockAddr, &len) == -1){
                sleep(3);
                if (recvfrom(sockfd, (char*)&msg, sizeof(msg), MSG_WAITALL, (struct sockaddr*)&sendSockAddr, &len) == -1){
                    sendto(sockfd, (char*)&msg, strlen(msg), MSG_CONFIRM, (struct sockaddr*)&sendSockAddr, sizeof(sendSockAddr)); 
                }
                else 
                    success = 1;
            }
            else 
                success = 1;
        }
        if (success)
            cout << msg << endl;
        else{
            cout << "Connection with server is lost" << endl;
            break;
        }
        
        memset(&msg, 0, sizeof(msg));//clear the buffer  
        while (recvfrom(sockfd, (char*)&msg, sizeof(msg), MSG_WAITALL, (struct sockaddr*)&sendSockAddr, &len) == -1) 
            sleep(1);
        cout << "Result: " << msg << endl; 
        
        // acknowledgement for server
        memset(&msg, 0, sizeof(msg)); 
        strcpy(msg, acknow.c_str());
        sendto(sockfd, (char*)&msg, strlen(msg), MSG_CONFIRM, (struct sockaddr*)&sendSockAddr, sizeof(sendSockAddr));
    } 
    close(sockfd); 
    cout << "Connection closed" << endl; 
} 
 
//Client side 
int main(int argc, char* argv[]) 
{ 
    //we need 3 things: ip address, port number and mode, in that order 
    if (argc != 4) 
    { 
        cerr << "Usage: ip_address port mode" << endl; exit(0); 
    } //grab the IP address and port number  
    char* serverIp = argv[1]; int port = atoi(argv[2]); string mode = argv[3]; 
    if (mode == "t") 
        TCPclient(serverIp, port); 
    else if (mode == "u") 
        UDPclient(serverIp, port); 
    return 0; 
}
