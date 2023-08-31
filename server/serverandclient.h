#ifndef SERVERANDCLIENT_H
#define SERVERANDCLIENT_H

#include <QThread>
#include <QString>
#include <winsock2.h>

class serverAndClient: public QThread
{
    Q_OBJECT

public:
    explicit serverAndClient(QObject *parent = 0, const int idNumber = 0, const int clientIdIn = 0, bool stopThread = false);
    ~serverAndClient();
    void run();
    bool Stop;
    int clientId;
    int idNum;
    std::string needToSendAgain;
    std::string newCommand;
    bool needToChangeAccountNumber;
    bool oldNeedToChangeAccountNumber;
    std::string accountNumberToChange;
    int iResult;
    bool canContinue;
private:
    std::string convertToString(char* a, int size);
signals:
    // To communicate with Gui Thread
    // we need to emit a signal
    void changeCountOfClientsOnServer(int);
    void appendConsoleLogClientServer(std::string,std::string);

    void closeClientSocket(int clientId);
    void recieveBufferMessageFromClient(int clientId);
    void sendMessageToClient(int clientId, char*);
    void lastCommandChangeValue(std::string);
    std::string lastCommandGetValue();
public slots:

};

#endif // SERVERANDCLIENT_H
