#ifndef FFMPEGRECORDERIMPL_H
#define FFMPEGRECORDERIMPL_H

#include <iostream>
#include <queue>
#include <mutex>
#include <string>
#include <future>

#include "combaseapi.h"

#include "recorderiface.h"

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavdevice/avdevice.h"
#include "libavformat/avformat.h"
#include "libavformat/avio.h"
#include "libavutil/audio_fifo.h"
#include "libavutil/imgutils.h"
#include "libavutil/opt.h"
#include "libswresample/swresample.h"
#include "libswscale/swscale.h"
}

#define ERR_BUFF_SIZE           100
#define INPUT_FORMAT_GDIGRAB    "gdigrab"
#define DEVICE_NAME_DESKTOP     "desktop"
#define OUT_ENCODER             AV_CODEC_ID_MPEG4
#define MAX_NUMBER_CACHE_PKT    20

#define MAX_TEST_FRAME          2000

class RecorderImpl : public RecorderIface
{
public:
    RecorderImpl();
    ~RecorderImpl();
    bool startRecord();
    bool stopRecord();
    bool saveRecord();

    bool testRecord();

private:
    char err_buff[ERR_BUFF_SIZE];

    bool isRecorderReady = false;
    bool isRecording = false;

    // For input context
    std::string     m_inputFormat;
    std::string     m_inputDevice;
    AVDictionary    *m_inRecordOpt = nullptr;
    AVFormatContext *m_inFmtCtx = nullptr;
    AVCodecContext  *m_inVideoDecocedctx = nullptr;
    const AVCodec   *m_inVideoDecoder;
    int             m_inVideostreamIindex = -1;

    // For output context
    std::string         m_outFileName;
    // For encoding video stream
    AVFormatContext     *m_outFmtCtx = nullptr;
    AVCodecContext      *m_outVideoEncoderCtx = nullptr;
    const AVCodec       *m_outVideoEncoder;
    struct SwsContext   *swsCtx = nullptr;  // For sws scaling

    // Queue to save input packets
    std::queue<AVPacket*>    m_VideoPktQueue;
    std::mutex              m_VideoQueueMutex;

    bool initInCtx();
    bool initIniVideoCtx();
    bool readPktFromInputCtx(AVPacket* pkt);
    bool saveReadDesktopPkt(AVPacket* pkt);
    void captureDesktop();
    std::thread captureVideoThread;

    bool initOutCtx();
    bool initOutVideoCtx();
    bool readDesktopPkt(AVPacket** pkt);
    bool writePktToOutputCtx(AVPacket *pkt);
    void handleCapturedData();
    std::thread handleCapturedDataThread;
    AVPacket* savePkt;
};

#endif // FFMPEGRECORDERIMPL_H
