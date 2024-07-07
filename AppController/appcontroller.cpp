#include "appcontroller.h"

Appcontroller::Appcontroller(QWidget *parent) : QMainWindow(parent)
{
    initialize();
    showScreen();
}

Appcontroller::~Appcontroller() {
    if(m_recordScreen->isRecordState()) {
        std::cout << "Recording, wait for recorder to stop" << std::endl;
        this->stopRecord();
        std::cout << "Stopped recording" << std::endl;
    }
}

void Appcontroller::initialize() {
    m_appController = new Ui::MainWindow();
    // m_appController = new Ui::AppController();
    m_appController->setupUi(this);
    setFixedSize(this->size());
    connect(m_appController->tabWidget, SIGNAL(tabBarClicked(int)), this, SLOT(handleTabClicked(int)));
}

void Appcontroller::handleTabClicked(int tabIndex) {
    if(tabIndex < 0) {
        return;
    }
    if(tabIndex == m_appController->tabWidget->indexOf(m_appController->RecordTab)) {
        this->showRecordScreen();
    } else {
        if(tabIndex == m_appController->tabWidget->indexOf(m_appController->SettingTab)) {
            showSettingSCreen();
        }
    }
    if(tabIndex != m_appController->tabWidget->indexOf(m_appController->RecordTab)) {
        if(m_recordScreen != nullptr) {
            m_recordScreen->hide();
        }
    }
}

void Appcontroller::initListAudioDevices(std::vector<std::pair<QString, QString>> listDevices) {
    for(auto device : listDevices) {
        m_appController->listAudioDevice->addItem(device.second);
    }
}

void Appcontroller::showScreen() {
    setWindowFlags(windowFlags() | Qt::WindowMinimizeButtonHint);
    this->setWindowTitle("ScreenRecorder");
    this->show();
    if(m_appController->tabWidget->currentIndex() == m_appController->tabWidget->indexOf(m_appController->RecordTab)) {
        this->showRecordScreen();
    } else if(m_appController->tabWidget->currentIndex() == m_appController->tabWidget->indexOf(m_appController->SettingTab)) {
        this->showSettingSCreen();
    }
}

void Appcontroller::clearListAudioDevices() {
    m_appController->listAudioDevice->clear();
}

void Appcontroller::showSettingSCreen() {
    clearListAudioDevices();
    auto listOutAudioDevices = getOutputAudioDevices();
    initListAudioDevices(listOutAudioDevices);
}

void Appcontroller::showRecordScreen() {
    if(m_recordScreen == nullptr) {
        m_recordScreen = new RecordScreen();
        connect(this, SIGNAL(closeEvent()), m_recordScreen, SLOT(close()));
    }

    if(m_recordScreen != nullptr) {
        m_recordScreen->setVisible(true);
        if(m_recordScreen->isRecordState()) {
            m_appController->recordControlButton->setText("Stop Recording");
            connect(m_appController->recordControlButton, SIGNAL(clicked(bool)), this, SLOT(stopRecord()));
        } else {
            m_appController->recordControlButton->setText("Start Recording");
            QObject::connect(m_appController->recordControlButton, SIGNAL(clicked()),
                                this, SLOT(startRecord()));
        }
    }
}

void Appcontroller::getFPS(RecordSetting *setting) {
    setting->fps = 30;
    if(m_appController->radioButtonFPS24->isChecked()) {
        setting->fps = 24;
    } else if(m_appController->radioButtonFPS30->isChecked()) {
        setting->fps = 30;
    } else if(m_appController->radioButtonFPS60->isChecked()) {
        setting->fps = 60;
    }
}

void Appcontroller::getOutAudioName(RecordSetting *setting) {
    std::string selectedOutAudio = m_appController->listAudioDevice->currentText().toStdString();
    setting->outAudioName = selectedOutAudio;
}

void Appcontroller::getUserOption(RecordSetting *setting) {
    getFPS(setting);
    getOutAudioName(setting);
}

std::vector<std::pair<QString, QString>> Appcontroller::getOutputAudioDevices() {
    std::vector<std::pair<QString, QString>> listDevices;
    const QList<QAudioDevice> outaudioDevices = QMediaDevices::audioInputs();
    for (const QAudioDevice &device : outaudioDevices)
    {

        listDevices.push_back({device.id(), device.description()});
    }
    return listDevices;
}

void Appcontroller::getRecordScreenProperty(RecordSetting *setting) {
    auto selectedArea = m_recordScreen->getScreenSelector();
    setting->off_x = selectedArea.topLeft().x() + SELECTOR_BORDER_SIZE;
    setting->off_y = selectedArea.topLeft().y() + SELECTOR_BORDER_SIZE;
    setting->height = selectedArea.height() - 2*SELECTOR_BORDER_SIZE -1;
    setting->width = selectedArea.width() - 2*SELECTOR_BORDER_SIZE - 1;
}

void Appcontroller::getRecordSettings(RecordSetting *setting) {
    getRecordScreenProperty(setting);
    getUserOption(setting);
}

void Appcontroller::setSettingTabEnabled(bool enabled) {
    int settingIndex = m_appController->tabWidget->indexOf(m_appController->SettingTab);
    if(settingIndex >= 0) {
        m_appController->tabWidget->setTabEnabled(settingIndex, enabled);
    }
}

void Appcontroller::startRecord() {
    m_appController->recordControlButton->setText("Stop Recording");
    m_recordScreen->toRecordState();
    QObject::disconnect(m_appController->recordControlButton, SIGNAL(clicked()),
                        this, SLOT(startRecord()));
    RecordSetting currentSetting;
    getRecordSettings(&currentSetting);
    m_recoder = new RecorderImpl(currentSetting);
    if(m_recoder->startRecord()) {
        setSettingTabEnabled(false);
        QObject::connect(m_appController->recordControlButton, SIGNAL(clicked()),
                            this, SLOT(stopRecord()));
    } else {
        m_recordScreen->toNormalState();
        m_appController->recordControlButton->setText("Start Recording");
        std::cout << "Cannot start recording" << std::endl;
    }

}

void Appcontroller::stopRecord() {
    setSettingTabEnabled(true);
    QObject::disconnect(m_appController->recordControlButton, SIGNAL(clicked()),
                        this, SLOT(stopRecord()));
    QObject::connect(m_appController->recordControlButton, SIGNAL(clicked()),
                        this, SLOT(startRecord()));
    m_recoder->stopRecord();
    m_recordScreen->toNormalState();
    m_appController->recordControlButton->setText("Start Recording");
    delete m_recoder;
}
