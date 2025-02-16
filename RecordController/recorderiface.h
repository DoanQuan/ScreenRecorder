#ifndef RECORDERIFACE_H
#define RECORDERIFACE_H

#include "AppController/RecordSetting.h"

class RecorderIface {
public:
    virtual bool startRecord() = 0;
    virtual bool stopRecord() = 0;
    virtual bool saveRecord() = 0;
};

#endif // RECORDERIFACE_H
