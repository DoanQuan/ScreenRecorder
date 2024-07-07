/*
 * This class works as Main Window
 */
#ifndef APPCONTROLLER_H
#define APPCONTROLLER_H

#include <QPushButton>
#include <QWidget>
#include <QLabel>
#include <QDialog>
#include <QAudioDevice>
#include <QMediaDevices>
#include <QMainWindow>

#include "recordscreen.h"
#include "RecordController/recorderiface.h"
#include "RecordController/ffmpegrecorderimpl.h"

#include "ui_mainwindow.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class Appcontroller : public QMainWindow
{
Q_OBJECT

public:
    Appcontroller(QWidget *parent = nullptr);
    ~Appcontroller();

private:
    void initialize();
    void showScreen();

private slots:
    void handleTabClicked(int tabIndex);
    void startRecord();
    void stopRecord();

private:
    QPushButton     *m_buttonShowRecordScreen = nullptr;
    QPushButton     *m_buttonStartStopRecord = nullptr;
    RecordScreen    *m_recordScreen = nullptr;
    RecorderImpl    *m_recoder;
    // Ui::AppController   *m_appController;
    Ui::MainWindow  *m_appController;

private:
    void showRecordScreen();
    void showSettingSCreen();
    void clearListAudioDevices();
    void setSettingTabEnabled(bool enabled);
    void getRecordSettings(RecordSetting* setting);
    void getRecordScreenProperty(RecordSetting* setting);
    void getUserOption(RecordSetting* setting);
    void getFPS(RecordSetting *setting);
    void getOutAudioName(RecordSetting* setting);
    std::vector<std::pair<QString, QString>> getOutputAudioDevices();
    void initListAudioDevices(std::vector<std::pair<QString, QString>> listDevices);
};

#endif // APPCONTROLLER_H
