/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.2.4
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QWidget *widget;
    QTabWidget *tabWidget;
    QWidget *SettingTab;
    QWidget *widget_2;
    QVBoxLayout *verticalLayout_2;
    QLabel *label;
    QWidget *widget_5;
    QGridLayout *gridLayout;
    QComboBox *listAudioDevice;
    QWidget *widget_3;
    QVBoxLayout *verticalLayout;
    QLabel *labelVideoFPS;
    QHBoxLayout *horizontalLayout_3;
    QRadioButton *radioButtonFPS24;
    QRadioButton *radioButtonFPS30;
    QRadioButton *radioButtonFPS60;
    QFrame *line;
    QWidget *RecordTab;
    QWidget *widget_6;
    QHBoxLayout *horizontalLayout;
    QPushButton *recordControlButton;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(445, 481);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        widget = new QWidget(centralwidget);
        widget->setObjectName(QString::fromUtf8("widget"));
        widget->setGeometry(QRect(10, 10, 421, 431));
        tabWidget = new QTabWidget(widget);
        tabWidget->setObjectName(QString::fromUtf8("tabWidget"));
        tabWidget->setGeometry(QRect(10, 10, 401, 411));
        SettingTab = new QWidget();
        SettingTab->setObjectName(QString::fromUtf8("SettingTab"));
        widget_2 = new QWidget(SettingTab);
        widget_2->setObjectName(QString::fromUtf8("widget_2"));
        widget_2->setGeometry(QRect(10, 30, 341, 80));
        verticalLayout_2 = new QVBoxLayout(widget_2);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        label = new QLabel(widget_2);
        label->setObjectName(QString::fromUtf8("label"));

        verticalLayout_2->addWidget(label);

        widget_5 = new QWidget(widget_2);
        widget_5->setObjectName(QString::fromUtf8("widget_5"));
        gridLayout = new QGridLayout(widget_5);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        listAudioDevice = new QComboBox(widget_5);
        listAudioDevice->setObjectName(QString::fromUtf8("listAudioDevice"));

        gridLayout->addWidget(listAudioDevice, 0, 0, 1, 1);


        verticalLayout_2->addWidget(widget_5);

        widget_3 = new QWidget(SettingTab);
        widget_3->setObjectName(QString::fromUtf8("widget_3"));
        widget_3->setGeometry(QRect(10, 220, 311, 78));
        verticalLayout = new QVBoxLayout(widget_3);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        labelVideoFPS = new QLabel(widget_3);
        labelVideoFPS->setObjectName(QString::fromUtf8("labelVideoFPS"));

        verticalLayout->addWidget(labelVideoFPS);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        radioButtonFPS24 = new QRadioButton(widget_3);
        radioButtonFPS24->setObjectName(QString::fromUtf8("radioButtonFPS24"));

        horizontalLayout_3->addWidget(radioButtonFPS24);

        radioButtonFPS30 = new QRadioButton(widget_3);
        radioButtonFPS30->setObjectName(QString::fromUtf8("radioButtonFPS30"));
        radioButtonFPS30->setChecked(true);

        horizontalLayout_3->addWidget(radioButtonFPS30);

        radioButtonFPS60 = new QRadioButton(widget_3);
        radioButtonFPS60->setObjectName(QString::fromUtf8("radioButtonFPS60"));

        horizontalLayout_3->addWidget(radioButtonFPS60);


        verticalLayout->addLayout(horizontalLayout_3);

        line = new QFrame(SettingTab);
        line->setObjectName(QString::fromUtf8("line"));
        line->setGeometry(QRect(10, 160, 381, 16));
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);
        tabWidget->addTab(SettingTab, QString());
        RecordTab = new QWidget();
        RecordTab->setObjectName(QString::fromUtf8("RecordTab"));
        widget_6 = new QWidget(RecordTab);
        widget_6->setObjectName(QString::fromUtf8("widget_6"));
        widget_6->setGeometry(QRect(10, 10, 184, 42));
        horizontalLayout = new QHBoxLayout(widget_6);
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        recordControlButton = new QPushButton(widget_6);
        recordControlButton->setObjectName(QString::fromUtf8("recordControlButton"));

        horizontalLayout->addWidget(recordControlButton);

        tabWidget->addTab(RecordTab, QString());
        MainWindow->setCentralWidget(centralwidget);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName(QString::fromUtf8("statusbar"));
        MainWindow->setStatusBar(statusbar);

        retranslateUi(MainWindow);

        tabWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "MainWindow", nullptr));
        label->setText(QCoreApplication::translate("MainWindow", "Input audio device", nullptr));
        labelVideoFPS->setText(QCoreApplication::translate("MainWindow", "Video FPS", nullptr));
        radioButtonFPS24->setText(QCoreApplication::translate("MainWindow", "24", nullptr));
        radioButtonFPS30->setText(QCoreApplication::translate("MainWindow", "30", nullptr));
        radioButtonFPS60->setText(QCoreApplication::translate("MainWindow", "60", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(SettingTab), QCoreApplication::translate("MainWindow", "Setting", nullptr));
        recordControlButton->setText(QCoreApplication::translate("MainWindow", "Start Recording", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(RecordTab), QCoreApplication::translate("MainWindow", "Screen Record", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
