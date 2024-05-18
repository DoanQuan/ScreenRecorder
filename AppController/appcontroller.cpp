// Main window
/*
 * TODO:
 * * Create a simple user interface that could show the record screen
 * * Create a simple screen recorder with ffmpeg
 */
#include "appcontroller.h"

Appcontroller::Appcontroller()
{
    initialize();
    showScreen();
}

void Appcontroller::initialize() {
    if(m_buttonShowRecordScreen == nullptr){
        m_buttonShowRecordScreen = new QPushButton("Record Screen");
        QObject::connect(m_buttonShowRecordScreen, SIGNAL(clicked()),
                         this, SLOT(showRecordScreen()));
    }
    if(m_buttonStartStopRecord == nullptr) {
        m_buttonStartStopRecord = new QPushButton("Start Recording");
        QObject::connect(m_buttonStartStopRecord, SIGNAL(clicked()),
                         this, SLOT(startRecord()));
    }

    m_recoder = new RecorderImpl();
}

void Appcontroller::showScreen() {
    if(m_buttonShowRecordScreen != nullptr) {
        m_buttonShowRecordScreen->show();
    }
}

void Appcontroller::showRecordScreen() {
    if(m_recordScreen == nullptr) {
        m_recordScreen = new RecordScreen();
    }

    if(m_recordScreen != nullptr) {
        m_recordScreen->show();
    }
    m_buttonStartStopRecord->show();
}

void Appcontroller::startRecord() {
    qCritical() << "Start Record button clicked";
    m_buttonStartStopRecord->setText("Stop Recording");
    QObject::disconnect(m_buttonStartStopRecord, SIGNAL(clicked()),
                        this, SLOT(startRecord()));
    QObject::connect(m_buttonStartStopRecord, SIGNAL(clicked()),
                        this, SLOT(stopRecord()));
    m_recoder->startRecord();
}

void Appcontroller::stopRecord() {
    qCritical() << "Stop Record button clicked";
    m_buttonStartStopRecord->setText("Start Recording");
    QObject::disconnect(m_buttonStartStopRecord, SIGNAL(clicked()),
                        this, SLOT(stopRecord()));
    QObject::connect(m_buttonStartStopRecord, SIGNAL(clicked()),
                        this, SLOT(startRecord()));
    m_recoder->stopRecord();
}
