#include "mainwindow.h"
#include "AppController/appcontroller.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    // Create App Controller
    // Appcontroller *appController = new Appcontroller();

    Appcontroller appController;
    appController.show();

    return a.exec();
}
