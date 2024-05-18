/*
 * This class works as Main Window
 */
#ifndef APPCONTROLLER_H
#define APPCONTROLLER_H

#include <QPushButton>
#include <QWidget>
#include <QLabel>

#include "recordscreen.h"
#include "RecordController/recorderiface.h"
#include "RecordController/ffmpegrecorderimpl.h"

class Appcontroller : public QWidget
{
Q_OBJECT

public:
    Appcontroller();

private:
    void initialize();
    void showScreen();

private slots:
    void showRecordScreen();
    void startRecord();
    void stopRecord();

private:
    QPushButton     *m_buttonShowRecordScreen = nullptr;
    QPushButton     *m_buttonStartStopRecord = nullptr;
    RecordScreen    *m_recordScreen = nullptr;
    RecorderImpl    *m_recoder;
};

#endif // APPCONTROLLER_H
