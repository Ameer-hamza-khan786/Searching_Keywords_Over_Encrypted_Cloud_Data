#include <iostream>
#include <fstream>
#include <unordered_set>
#include <unordered_map>
#include <cstring>
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h> 
#include <sstream>
using namespace std;

unordered_map<string, unordered_set<string>> invIndex;
class FileReceiver {
private:
    int serverSocket;
    int port;
public:
    FileReceiver(int port) : port(port) {}

    bool receiveFiles() {
         // Create socket
        int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSocket == -1) {
            cerr << "Error creating socket." << endl;
            exit(0);
        }


        // Server address
        struct sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(port);
        serverAddr.sin_addr.s_addr = INADDR_ANY;

        // Bind socket
        if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
            cerr << "Error binding socket." << endl;
            close(serverSocket);
            exit(0);
        }

        // Listen
        if (listen(serverSocket, 5) == -1) {
            cerr << "Error listening on socket." << endl;
            close(serverSocket);
            exit(0);
        }

        cout << "Server listening on port " << port << endl;

        while (true) {
            // Accept incoming connection
            struct sockaddr_in clientAddr;
            socklen_t clientAddrLen = sizeof(clientAddr);
            int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);
            if (clientSocket == -1) {
                cerr << "Error accepting connection." << endl;
                close(serverSocket);
                return false;
            }

            cout << "Client connected: " << inet_ntoa(clientAddr.sin_addr) << endl;

            // Receive files
            while (true) {
                // Receive file name length
                uint32_t nameLen;
                int bytesReceived = recv(clientSocket, &nameLen, sizeof(nameLen), 0);
                if (bytesReceived <= 0) {
                    cout << "Client disconnected." << endl;
                    break;
                }
                nameLen = ntohl(nameLen);

                // Receive file name
                char fileName[nameLen + 1];
                bytesReceived = recv(clientSocket, fileName, nameLen, 0);
                if (bytesReceived <= 0) {
                    cout << "Client disconnected." << endl;
                    break;
                }
                fileName[nameLen] = '\0';

                // Receive file content length
                uint32_t fileLen;
                bytesReceived = recv(clientSocket, &fileLen, sizeof(fileLen), 0);
                if (bytesReceived <= 0) {
                    cout << "Client disconnected." << endl;
                    break;
                }
                fileLen = ntohl(fileLen);

                // Receive file content
                char fileContents[fileLen + 1];
                bytesReceived = recv(clientSocket, fileContents, fileLen, 0);
                if (bytesReceived <= 0) {
                    cout << "Client disconnected." << endl;
                    break;
                }
                fileContents[fileLen] = '\0';

                // Write received file content to file
                ofstream outFile(fileName, ios::binary);
                outFile.write(fileContents, fileLen);
                outFile.close();

                cout << "Received file: " << fileName << endl;
            }

            // Close client socket
            close(clientSocket);
            break;
        }

        // Close server socket
        close(serverSocket);
        return true;
    }

void receive_file(){
    // Start receiving files
    bool success = receiveFiles();
    if (!success) {
        cerr << "Error receiving files." << endl;
        exit(0);
    }
}


void receive_query(){

     // Create socket
        int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSocket == -1) {
            cerr << "Error creating socket." << endl;
            exit(0);
        }

        // Server address
        struct sockaddr_in serverAddr;
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(port);
        serverAddr.sin_addr.s_addr = INADDR_ANY;

        // Bind socket
        if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
            cerr << "Error binding socket." << endl;
            close(serverSocket);
            exit(0);
        }
        // Listen
        if (listen(serverSocket, 5) == -1) {
            cerr << "Error listening on socket." << endl;
            close(serverSocket);
            exit(0);
        }

    cout << "Server listening for queries on port " << port << endl;


    while (true) {
        // Accept incoming connection
        struct sockaddr_in clientAddr;
        socklen_t clientAddrLen = sizeof(clientAddr);
        int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);
        if (clientSocket == -1) {
            cerr << "Error accepting connection." << endl;
            close(serverSocket);
            return;
        }

        cout << "Client connected: " << inet_ntoa(clientAddr.sin_addr) << endl;

        
        // Receive the size of query
        uint32_t querysize;
        int bytesReceived = recv(clientSocket, &querysize, sizeof(querysize), 0);
        if (bytesReceived <= 0) {
            cout << "Client disconnected." << endl;
            break;
        }
        querysize = ntohl(querysize);
        unordered_set<string> file_names; 
        
       // Receive query's
            while (querysize--) {
            uint32_t queryLen;
            int bytesReceived = recv(clientSocket, &queryLen, sizeof(queryLen), 0);
            if (bytesReceived <= 0) {
                cout << "Client disconnected." << endl;
                break;
            }
            queryLen = ntohl(queryLen);

            // Receive query
            char queryBuffer[queryLen + 1];
            bytesReceived = recv(clientSocket, queryBuffer, queryLen, 0);
            if (bytesReceived <= 0) {
                cout << "Client disconnected." << endl;
                break;
            }
            queryBuffer[queryLen] = '\0';

            string query(queryBuffer);

            // Get response for query
            string result;
            if(invIndex.find(query)!=invIndex.end()){
            for (const auto& response : invIndex[query]) {
                result = response;
                file_names.insert(result);
            }
            }
        
            }
            // code here
            send_files(clientSocket, file_names);
            
            // Close client socket
            close(clientSocket);
}
}
void send_files(int clientSocket, const  unordered_set<string>& file_names) {
    uint32_t files_size = file_names.size();
    files_size = htonl(files_size);
    send(clientSocket, &files_size, sizeof(files_size), 0);

    for (const string& fileName : file_names) {
        ifstream file(fileName+".dat", ios::binary);
        if (!file.is_open()) {
            cerr << "Error opening file: " << fileName << endl;
            continue;
        }

        // Send file name length and file name
        uint32_t nameLen = fileName.length();
        nameLen = htonl(nameLen);
        send(clientSocket, &nameLen, sizeof(nameLen), 0);
        send(clientSocket, fileName.c_str(), fileName.length(), 0);

        // Send file contents
        stringstream buffer;
        buffer << file.rdbuf();
        string fileContents = buffer.str();
        uint32_t fileLen = fileContents.length();
        fileLen = htonl(fileLen);
        send(clientSocket, &fileLen, sizeof(fileLen), 0);
        send(clientSocket, fileContents.c_str(), fileContents.length(), 0);

        file.close();
    }
}
};




// Load inverted index
void load_inverted_index(const string& filename,unordered_map<string, unordered_set<string>>& invIndex) {
    ifstream inFile(filename);
    if (!inFile.is_open()) {
        cerr << "Error opening file for deserialization: " << filename << endl;
        return;
    }
    string word;
    string docIdStr;
    while (inFile >> word) {
        // Read document IDs until the end of line
        getline(inFile, docIdStr);
        istringstream iss(docIdStr);
        while (iss >> docIdStr) {
            invIndex[word].insert(docIdStr);
        }
    }
    inFile.close();
}

    void printInvertedIndex() {
        for (const auto& word : invIndex) {
            cout << word.first << " --> ";
            for (const string& docId : word.second) {
                cout << docId << " ";
            }
            cout << endl;
        }
    }
int main() {
    
    // Port on which the server listens for incoming file transfers
    int serverPort = 12345; // Change to your desired port

    // Initialize file receiver
    FileReceiver fileReceiver(serverPort);
    int token;
    cout<<"ENTER OPTION OF SERVER \n";
    cout<<"PRESS \n 1 --> FOR RECEIVING DATA\n 2 --> FOR QUERY SEARCH\n 3--> FOR LOADING OF INVERTED INDEX\n";
    while(true){
    cin>>token;
    switch(token){
        case 1:
        cout<<"RECEIVE FILES... \n";
        fileReceiver.receive_file();
        break;
        case 2:
        cout<<"RECEIVING QUERY... \n";
        fileReceiver.receive_query();
        cout<<"RESPONSE SENT\n";
        break;
        case 3:
        cout<<"LOADING INDEX... \n";
        load_inverted_index("encrypted_inverted_index.txt", invIndex);
        printInvertedIndex();
        cout<<"INDEX LOADED\n";
        break;
        default :return 0;
    }
    }
    return 0;
}
    
