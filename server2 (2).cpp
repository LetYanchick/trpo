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
#include <stack>  
using namespace std; 
 
template<typename T> T eval(T a, T b, char op) { 
    switch (op) { 
    case '+': 
        return a + b; 
    case '-': 
        return a - b; 
    case '*': 
        return a * b; 
    case '/': 
        return a / b; 
    } 
} 
 
int precedence(char op) { 
    if (op == '*' || op == '/') 
        return 2; 
    if (op == '+' || op == '-') 
        return 1; 
    return 0; 
} 
 
void TCPserver(int port) { 
    //buffer to send and receive messages with 
    char msg[1500]; 
    //setup a socket and connection tools 
    sockaddr_in servAddr; 
    bzero((char*)&servAddr, sizeof(servAddr)); 
    servAddr.sin_family = AF_INET; 
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY); 
    servAddr.sin_port = htons(port); 
 
    //open stream oriented socket with internet address 
    //also keep track of the socket descriptor 
    int serverSd = socket(AF_INET, SOCK_STREAM, 0); 
    if (serverSd < 0) 
    { 
        cerr << "Error establishing the server socket" << endl; 
        exit(0); 
    } 
    //bind the socket to its local address 
    int bindStatus = bind(serverSd, (struct sockaddr*)&servAddr, 
        sizeof(servAddr)); 
    if (bindStatus < 0) 
    { 
        cerr << "Error binding socket to local address" << endl; 
        exit(0); 
    } 
    string fin = "No"; 
    while (fin != "Yes") { 
        cout << "Waiting for a client to connect..." << endl; 
        //listen for up to 5 requests at a time 
        listen(serverSd, 5); 
        //receive a request from client using accept 
        //we need a new address to connect with the client 
        sockaddr_in newSockAddr; 
        socklen_t newSockAddrSize = sizeof(newSockAddr); 
        //accept, create a new socket descriptor to  
        //handle the new connection with client 
        int newSd = accept(serverSd, (sockaddr*)&newSockAddr, &newSockAddrSize); 
        if (newSd < 0) 
        { 
            cerr << "Error accepting request from client!" << endl; 
            exit(1); 
        } 
        cout << "Connected with client!" << endl; 
        while (1) 
        { 
            //receive a message from the client (listen) 
            memset(&msg, 0, sizeof(msg));//clear the buffer 
            recv(newSd, (char*)&msg, sizeof(msg), 0); 
            if (!strcmp(msg, "exit")) 
            { 
                cout << "Client has quit the session" << endl; 
                close(newSd); 
                break; 
            } 
            cout << "Client: " << msg << endl; 
            string expr = string(msg); 
            int i; 
            bool divmistake = 0; 
            stack<char> opers; 
            string res; 
            if (expr.find('.') != expr.npos || expr.find('/') != expr.npos) { 
                stack<double> nums; 
                for (i = 0; i < expr.length(); i++) { 
                    if (divmistake) 
                        break; 
                    if (expr[i] == ' ') 
                        continue; 
 
                    else if (expr[i] == '(') 
                        opers.push('('); 
 
                    /*else if ((expr[i] == '-' && isdigit(expr[i + 1]) && (nums.empty() || !(isdigit(expr[i - 1]) && isdigit(expr[i - 2])))) || isdigit(expr[i])) { */
                    else if ((expr[i] == '-' && isdigit(expr[i + 1]) && (nums.empty() || ((!opers.empty() && opers.top() == '(') && !(isdigit(expr[i - 1]) || isdigit(expr[i - 2]))))) || isdigit(expr[i])) { 

                        string val = ""; 
                        if (expr[i] == '-') { 
                            val += expr[i]; 
                            i++; 
                        } 
                        while (i < expr.length() && (isdigit(expr[i]) || expr[i] == '.')) { 
                            val += expr[i]; 
                            i++; 
                        } 
                        nums.push(stod(val)); 
                        i--; 
                    } 
 
                    else if (expr[i] == ')') { 
                        while (opers.top() != '(') { 
                            double num2 = nums.top(); 
                            nums.pop(); 
                            double num1 = nums.top(); 
                            nums.pop(); 
                            char op = opers.top(); 
                            opers.pop(); 
                            if (op == '/' && num2 == 0) { 
                                divmistake = 1; 
                                break; 
                            } 
                            nums.push(eval(num1, num2, op)); 
                        } 
                        opers.pop(); 
                    } 
 
                    else { 
                        while (!opers.empty() && precedence(opers.top()) >= precedence(expr[i])) { 
                            double num2 = nums.top(); 
                            nums.pop(); 
                            double num1 = nums.top(); 
                            nums.pop(); 
                            char op = opers.top(); 
                            opers.pop(); 
                            if (op == '/' && num2 == 0) { 
                                divmistake = 1; 
                                break; 
                            } 
                            nums.push(eval(num1, num2, op)); 
                        } 
                        if (!divmistake) 
                            opers.push(expr[i]); 
                    } 
                } 
                if (!divmistake) { 
                    while (!opers.empty()) { 
                        double num2 = nums.top(); 
                        nums.pop(); 
                        double num1 = nums.top(); 
                        nums.pop(); 
                        char op = opers.top(); 
                        opers.pop(); 
                        if (op == '/' && num2 == 0) { 
                            divmistake = 1; 
                            break; 
                        } 
                        nums.push(eval(num1, num2, op)); 
                    } 
                    if (!divmistake) 
                        res = to_string(nums.top()); 
                } 
            } 
            else { 
                stack<int> nums; 
                for (i = 0; i < expr.length(); i++) { 
 
                    if (expr[i] == ' ') 
                        continue; 
 
                    else if (expr[i] == '(') 
                        opers.push('('); 
 
                    else if ((expr[i] == '-' && isdigit(expr[i + 1]) && (nums.empty() || ((!opers.empty() && opers.top() == '(') && !(isdigit(expr[i - 1]) || isdigit(expr[i - 2]))))) || isdigit(expr[i])) { 
                        string val = ""; 
                        if (expr[i] == '-') { 
                            val += expr[i]; 
                            i++; 
                        } 
                        while (i < expr.length() && (isdigit(expr[i]) || expr[i] == '.')) { 
                            val += expr[i]; 
                            i++; 
                        } 
                        nums.push(stoi(val)); 
                        i--; 
                    } 
 
                    else if (expr[i] == ')') { 
                        while (opers.top() != '(') { 
                            int num2 = nums.top(); 
                            nums.pop(); 
                            int num1 = nums.top(); 
                            nums.pop(); 
                            char op = opers.top(); 
                            opers.pop(); 
                            if (op == '/' && num2 == 0) { 
                                divmistake = 1; 
                                break; 
                            } 
                            nums.push(eval(num1, num2, op)); 
                        } 
                        if (!divmistake) 
                            opers.pop(); 
                    } 
 
                    else { 
                        while (!opers.empty() && precedence(opers.top()) >= precedence(expr[i])) { 
                            int num2 = nums.top(); 
                            nums.pop(); 
                            int num1 = nums.top(); 
                            nums.pop(); 
                            char op = opers.top(); 
                            opers.pop(); 
                            if (op == '/' && num2 == 0) { 
                                divmistake = 1; 
                                break; 
                            } 
                            nums.push(eval(num1, num2, op)); 
                        } 
                        opers.push(expr[i]); 
                    } 
                } 
                if (!divmistake) { 
                    while (!opers.empty()) { 
                        int num2 = nums.top(); 
                        nums.pop(); 
                        int num1 = nums.top(); 
                        nums.pop(); 
                        char op = opers.top(); 
                        opers.pop(); 
                        if (op == '/' && num2 == 0) { 
                            divmistake = 1; 
                            break; 
                        } 
                        nums.push(eval(num1, num2, op)); 
                    } 
                    if (!divmistake) 
                        res = to_string(nums.top()); 
                } 
            } 
            if (divmistake) { 
                string mistake = "Division by zero"; 
                cout << mistake << endl; 
                memset(&msg, 0, sizeof(msg)); //clear the buffer  
                strcpy(msg, mistake.c_str()); 
                send(newSd, (char*)&msg, strlen(msg), 0); 
            } 
            else { 
                cout << "result: " << res << endl; 
                memset(&msg, 0, sizeof(msg)); //clear the buffer  
                strcpy(msg, res.c_str()); 
                //send the message to client  
                send(newSd, (char*)&msg, strlen(msg), 0); 
            } 
        } 
        cout << "Do you want to close connection? Yes/No\n"; 
        getline(cin, fin); 
    } 
    //we need to close the socket descriptors after we're all done 
 
    close(serverSd); 
    cout << "Connection closed..." << endl; 
} 
 
void UDPserver(int port) { 
    //buffer to send and receive messages with 
    char msg[1500]; 
    //setup a socket and connection tools 
    sockaddr_in servAddr, cliaAddr; 
    bzero((char*)&cliaAddr, sizeof(cliaAddr)); 
    bzero((char*)&servAddr, sizeof(servAddr)); 
    servAddr.sin_family = AF_INET; 
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY); 
    servAddr.sin_port = htons(port); 
    //creating socket file descriptor 
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0); 
    if (sockfd < 0) 
    { 
        cerr << "Error establishing the server socket" << endl; 
        exit(0); 
    } 
    // non-blocking 
    fcntl(sockfd, F_SETFL, O_NONBLOCK); 
    //bind the socket to its local address 
    int bindStatus = bind(sockfd, (struct sockaddr*)&servAddr, 
        sizeof(servAddr)); 
    if (bindStatus < 0) 
    { 
        cerr << "Error binding socket to local address" << endl; 
        exit(0); 
    } 
    socklen_t len = sizeof(cliaAddr); 
    
    // checking
    //sleep(5);
    memset(&msg, 0, sizeof(msg));//clear the buffer    
    while (recvfrom(sockfd, (char*)&msg, sizeof(msg), MSG_WAITALL, (struct sockaddr*)&cliaAddr, &len) == -1)
        continue;
    cout << msg << endl;
    memset(&msg, 0, sizeof(msg));//clear the buffer    
    string check = "Server is ready";
    strcpy(msg, check.c_str());
    sendto(sockfd, (char*)&msg, strlen(msg), MSG_CONFIRM, (struct sockaddr*)&cliaAddr, sizeof(cliaAddr));
    
    string fin = "No"; 
    string acknow1 = "expression is received by server";
    string acknow2 = "exit message is received by server";
    while (fin != "Yes") { 
        while (1) 
        { 
            //receive a message from the client  
            memset(&msg, 0, sizeof(msg));//clear the buffer 
            while (recvfrom(sockfd, (char*)&msg, sizeof(msg), MSG_WAITALL, (struct sockaddr*)&cliaAddr, &len) == -1)
                sleep(2);
            
            if (!strcmp(msg, "exit")) 
            { 
                //acknowledgement for client
                memset(&msg, 0, sizeof(msg)); 
                strcpy(msg, acknow2.c_str());
                sendto(sockfd, (char*)&msg, strlen(msg), MSG_CONFIRM, (struct sockaddr*)&cliaAddr, sizeof(cliaAddr));
                
                cout << "Client has quit the session" << endl; 
                break; 
            } 
            cout << "Client: " << msg << endl; 
            string expr = string(msg); 
            
            //acknowledgement for client
            memset(&msg, 0, sizeof(msg)); 
            strcpy(msg, acknow1.c_str());
            sendto(sockfd, (char*)&msg, strlen(msg), MSG_CONFIRM, (struct sockaddr*)&cliaAddr, sizeof(cliaAddr));
            
            int i; 
            bool divmistake = 0; 
            stack<char> opers; 
            string res; 
            if (expr.find('.') != expr.npos || expr.find('/') != expr.npos) { 
                stack<double> nums; 
                for (i = 0; i < expr.length(); i++) { 
                    if (divmistake) 
                        break; 
                    if (expr[i] == ' ') 
                        continue;
                    else if (expr[i] == '(') 
                        opers.push('('); 
 
                    else if ((expr[i] == '-' && isdigit(expr[i + 1]) && (nums.empty() || ((!opers.empty() && opers.top() == '(') && !(isdigit(expr[i - 1]) || isdigit(expr[i - 2]))))) || isdigit(expr[i])) { 
                        string val = ""; 
                        if (expr[i] == '-') { 
                            val += expr[i]; 
                            i++; 
                        } 
                        while (i < expr.length() && (isdigit(expr[i]) || expr[i] == '.')) { 
                            val += expr[i]; 
                            i++; 
                        } 
                        nums.push(stod(val)); 
                        i--; 
                    } 
 
                    else if (expr[i] == ')') { 
                        while (opers.top() != '(') { 
                            double num2 = nums.top(); 
                            nums.pop(); 
                            double num1 = nums.top(); 
                            nums.pop(); 
                            char op = opers.top(); 
                            opers.pop(); 
                            if (op == '/' && num2 == 0) { 
                                divmistake = 1; 
                                break; 
                            } 
                            nums.push(eval(num1, num2, op)); 
                        } 
                        opers.pop(); 
                    } 
 
                    else { 
                        while (!opers.empty() && precedence(opers.top()) >= precedence(expr[i])) { 
                            double num2 = nums.top(); 
                            nums.pop(); 
                            double num1 = nums.top(); 
                            nums.pop(); 
                            char op = opers.top(); 
                            opers.pop(); 
                            if (op == '/' && num2 == 0) { 
                                divmistake = 1; 
                                break; 
                            } 
                            nums.push(eval(num1, num2, op)); 
                        } 
                        if (!divmistake) 
                            opers.push(expr[i]); 
                    } 
                } 
                if (!divmistake) { 
                    while (!opers.empty()) { 
                        double num2 = nums.top(); 
                        nums.pop(); 
                        double num1 = nums.top(); 
                        nums.pop(); 
                        char op = opers.top(); 
                        opers.pop(); 
                        if (op == '/' && num2 == 0) { 
                            divmistake = 1; 
                            break; 
                        } 
                        nums.push(eval(num1, num2, op)); 
                    } 
                    if (!divmistake) 
                        res = to_string(nums.top()); 
                } 
            } 
            else { 
                stack<int> nums; 
                for (i = 0; i < expr.length(); i++) { 
 
                    if (expr[i] == ' ') 
                        continue; 
 
                    else if (expr[i] == '(') 
                        opers.push('('); 
 
                    else if ((expr[i] == '-' && isdigit(expr[i + 1]) && (nums.empty() || ((!opers.empty() && opers.top() == '(') && !(isdigit(expr[i - 1]) || isdigit(expr[i - 2]))))) || isdigit(expr[i])) { 
                        string val = ""; 
                        if (expr[i] == '-') { 
                            val += expr[i]; 
                            i++; 
                        } 
                        while (i < expr.length() && (isdigit(expr[i]) || expr[i] == '.')) { 
                            val += expr[i]; 
                            i++; 
                        } 
                        nums.push(stoi(val)); 
                        i--; 
                    } 
 
                    else if (expr[i] == ')') { 
                        while (opers.top() != '(') { 
                            int num2 = nums.top();
                            nums.pop(); 
                            int num1 = nums.top(); 
                            nums.pop(); 
                            char op = opers.top(); 
                            opers.pop(); 
                            if (op == '/' && num2 == 0) { 
                                divmistake = 1; 
                                break; 
                            } 
                            nums.push(eval(num1, num2, op)); 
                        } 
                        if (!divmistake) 
                            opers.pop(); 
                    } 
 
                    else { 
                        while (!opers.empty() && precedence(opers.top()) >= precedence(expr[i])) { 
                            int num2 = nums.top(); 
                            nums.pop(); 
                            int num1 = nums.top(); 
                            nums.pop(); 
                            char op = opers.top(); 
                            opers.pop(); 
                            if (op == '/' && num2 == 0) { 
                                divmistake = 1; 
                                break; 
                            } 
                            nums.push(eval(num1, num2, op)); 
                        } 
                        opers.push(expr[i]); 
                    } 
                } 
                if (!divmistake) { 
                    while (!opers.empty()) { 
                        int num2 = nums.top(); 
                        nums.pop(); 
                        int num1 = nums.top(); 
                        nums.pop(); 
                        char op = opers.top(); 
                        opers.pop(); 
                        if (op == '/' && num2 == 0) { 
                            divmistake = 1; 
                            break; 
                        } 
                        nums.push(eval(num1, num2, op)); 
                    } 
                    if (!divmistake) 
                        res = to_string(nums.top()); 
                } 
            } 
            if (divmistake) { 
                string mistake = "Division by zero"; 
                cout << mistake << endl; 
                
                memset(&msg, 0, sizeof(msg));//clear the buffer     
                strcpy(msg, mistake.c_str());
                sendto(sockfd, (char*)&msg, strlen(msg), MSG_CONFIRM, (struct sockaddr*)&cliaAddr, sizeof(cliaAddr));    
                bool success = 0;
                for (int i = 0; i < 3 && !success; i++){
                    if (recvfrom(sockfd, (char*)&msg, sizeof(msg), MSG_WAITALL, (struct sockaddr*)&cliaAddr, &len) == -1){
                        sleep(3);
                        if (recvfrom(sockfd, (char*)&msg, sizeof(msg), MSG_WAITALL, (struct sockaddr*)&cliaAddr, &len) == -1){
                            sendto(sockfd, (char*)&msg, strlen(msg), MSG_CONFIRM, (struct sockaddr*)&cliaAddr, sizeof(cliaAddr));
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
                    cout << "Connection with client is lost" << endl;
                    break;
                }
            } 
            else { 
                cout << "result: " << res << endl; 
                memset(&msg, 0, sizeof(msg));//clear the buffer     
                strcpy(msg, res.c_str());
                sendto(sockfd, (char*)&msg, strlen(msg), MSG_CONFIRM, (struct sockaddr*)&cliaAddr, sizeof(cliaAddr));  
                bool success = 0;
                for (int i = 0; i < 3 && !success; i++){
                    if (recvfrom(sockfd, (char*)&msg, sizeof(msg), MSG_WAITALL, (struct sockaddr*)&cliaAddr, &len) == -1){
                        sleep(3);
                        if (recvfrom(sockfd, (char*)&msg, sizeof(msg), MSG_WAITALL, (struct sockaddr*)&cliaAddr, &len) == -1){
                            sendto(sockfd, (char*)&msg, strlen(msg), MSG_CONFIRM, (struct sockaddr*)&cliaAddr, sizeof(cliaAddr));  
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
                    cout << "Connection with client is lost" << endl;
                    break;
                }
            } 
        } 
        cout << "Do you want to close connection? Yes/No\n"; 
        getline(cin, fin); 
        if (fin != "Yes"){
            // checking
            //sleep(5);
            memset(&msg, 0, sizeof(msg));//clear the buffer    
            while (recvfrom(sockfd, (char*)&msg, sizeof(msg), MSG_WAITALL, (struct sockaddr*)&cliaAddr, &len) == -1)
                continue;
            cout << msg << endl;
            memset(&msg, 0, sizeof(msg));//clear the buffer  
            sleep(2);
            string check = "Server is ready";
            strcpy(msg, check.c_str());
            sendto(sockfd, (char*)&msg, strlen(msg), MSG_CONFIRM, (struct sockaddr*)&cliaAddr, sizeof(cliaAddr));
        }
    } 
    close(sockfd); 
    cout << "Connection closed..." << endl; 
} 
 
//Server side 
int main(int argc, char* argv[]) 
{ 
    //for the server, we need to specify a port number and mode(udp or tcp) 
    if (argc != 3) 
    { 
        cerr << "Usage: port and mode" << endl; 
        exit(0); 
    } 
    //grab the port number 
    int port = atoi(argv[1]); 
    string mode = argv[2]; 
    if (mode == "t") 
        TCPserver(port); 
    else if (mode == "u") 
        UDPserver(port); 
    return 0; 
}
