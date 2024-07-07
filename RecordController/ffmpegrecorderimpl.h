#ifndef FFMPEGRECORDERIMPL_H
#define FFMPEGRECORDERIMPL_H

#include <iostream>
#include <queue>
#include <mutex>
#include <string>
#include <future>

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
#define OUT_VIDEO_ENCODER       AV_CODEC_ID_MPEG4
#define MAX_NUMBER_CACHE_PKT    20

#define OUT_AUDIO_ENCODER       AV_CODEC_ID_AAC

#define MAX_TEST_FRAME          2000

class RecorderImpl : public RecorderIface
{
public:
    RecorderImpl(RecordSetting settings);
    ~RecorderImpl();
    bool startRecord();
    bool stopRecord();
    bool saveRecord();

    void applySetting(RecordSetting settings) { this->settings = settings;}

private:
    char err_buff[ERR_BUFF_SIZE];

    bool isRecorderReady = false;
    bool isRecording = false;
    RecordSetting settings;

    // For input context
    std::string     m_inputFormat;
    std::string     m_inputDevice;
    AVDictionary    *m_inRecordOpt = nullptr;
    AVFormatContext *m_inFmtCtx = nullptr;
    AVCodecContext  *m_inVideoDecocedctx = nullptr;
    const AVCodec   *m_inVideoDecoder;
    int             m_inVideostreamIindex = -1;
    int             m_outVideoStreamIndex = -1;

    // For output context
    std::string         m_outFileName;
    // For encoding video stream
    AVFormatContext     *m_outFmtCtx = nullptr;
    AVCodecContext      *m_outVideoEncoderCtx = nullptr;
    const AVCodec       *m_outVideoEncoder;
    struct SwsContext   *swsCtx = nullptr;  // For sws scaling
    std::mutex          m_writeOutputMutex;

    // Queue to save input packets
    std::queue<AVPacket*>    m_VideoPktQueue;
    std::mutex              m_VideoQueueMutex;

    void createOutputFileName();
    bool initInCtx();
    bool initIniVideoCtx();
    bool readPktFromInputCtx(AVPacket* pkt);
    bool savereadDesktopVideoPkt(AVPacket* pkt);
    void captureDesktop();
    std::thread captureVideoThread;

    bool initOutCtx();
    bool initOutVideoCtx();
    bool readDesktopVideoPkt(AVPacket** pkt);
    bool writeVideoPktToOutputCtx(AVPacket *pkt);
    void handleCapturedVideoData();
    std::thread handleCapturedVideoDataThread;
    AVPacket* savePkt;

    // For capturing audio
    AVFormatContext *audioFmtCtx = nullptr;
    AVAudioFifo *audioBuff;
    int m_inAudioStreamIndex = -1;
    int m_outAudioStreamIndex = -1;
    bool initInAudioCtx();
    bool initOutAudioCtx();
    void captureAudio();
    const AVCodec *m_inAudioDecoder;
    AVCodecContext *m_inAudioDecoderCtx;
    const AVCodec *m_outAudioEncoder;
    AVCodecContext *m_outAudioEncoderCtx;
    SwrContext *swrCtx = nullptr;
    std::thread captureAudioThread;

    bool initOutputFile();
};

#endif // FFMPEGRECORDERIMPL_H
