#include "acceptClient.h"
#include <winsock2.h>
#include <process.h>
#include <iostream>
#include <QDebug>

acceptClient::acceptClient(QObject *parent,SOCKET* client_sock):
    QThread(parent), acceptedClient(false), client(NULL), client_socket(client_sock)
{
}

void acceptClient::run(){
//    *client = 0;
    client = emit startClientAccept(client_socket);
//    while(!client){};
//    qDebug() << client;
//    this->acceptedClient = true;




}
