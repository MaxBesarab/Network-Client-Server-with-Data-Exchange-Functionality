#include "server.h"

#include <QApplication>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    server serverUI;
    a.setApplicationDisplayName("Max");
    serverUI.show();

    return a.exec();
}
