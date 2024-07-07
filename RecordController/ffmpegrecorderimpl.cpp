#include "ffmpegrecorderimpl.h"

#include <ctime>

RecorderImpl::RecorderImpl(RecordSetting settings)
{
    av_log_set_level(AV_LOG_ERROR);

    /*
     * TODO: read these values from input
     */
    this->settings = settings;
    m_inputFormat = "gdigrab";
    m_inputDevice = "desktop";
    createOutputFileName();

    if(initInCtx() && initOutCtx() && initOutAudioCtx() && initOutputFile()) {
        isRecorderReady = true;
    } else {
        isRecorderReady = false;
    }
}

RecorderImpl::~RecorderImpl() {
    // av_write_trailer(m_outFmtCtx);
    if(isRecording) {
        isRecording = false;
        if(captureAudioThread.joinable()) {
            captureVideoThread.join();
        }
        if(handleCapturedVideoDataThread.joinable()) {
            handleCapturedVideoDataThread.join();
        }
        if(captureAudioThread.joinable()) {
            captureAudioThread.join();
        }

        av_write_trailer(m_outFmtCtx);
        avio_close(m_outFmtCtx->pb);
    }

    avformat_close_input(&m_inFmtCtx);
    avformat_close_input(&audioFmtCtx);
    avformat_free_context(m_inFmtCtx);
}

#define START_YEAR 1900
#define TIME_ZONE 7
void RecorderImpl::createOutputFileName() {
    m_outFileName = "out.mp4";
    time_t now = time(0);
    tm *gmtm = gmtime(&now);
    std::string outfile = "ScreenRecorder-RecordFile_";
    std::string year = std::to_string(START_YEAR + gmtm->tm_year);
    std::string month =  gmtm->tm_mon >= 9 ? std::to_string(1 + gmtm->tm_mon) : std::string("0") + std::to_string(1 + gmtm->tm_mon);
    std::string day =  gmtm->tm_mday >= 10 ? std::to_string(gmtm->tm_mday) : std::string("0") + std::to_string(gmtm->tm_mday);
    std::string hour = TIME_ZONE + gmtm->tm_hour >= 10 ? std::to_string(TIME_ZONE + gmtm->tm_hour) : std::string("0") + std::to_string(TIME_ZONE + gmtm->tm_hour);
    std::string min = gmtm->tm_min >= 10 ?  std::to_string(gmtm->tm_min) : std::string("0") +  std::to_string(gmtm->tm_min);
    std::string sec = gmtm->tm_sec >= 10 ?  std::to_string(gmtm->tm_sec) : std::string("0") +  std::to_string(gmtm->tm_sec);
    m_outFileName = outfile + year + "-" + month + "-" + day + "_" + hour + "-" + min + "-" + sec + ".mp4";
    std::cout << "output file name: " << m_outFileName << std::endl;
}

bool RecorderImpl::initInCtx() {
    bool ret = false;
    // std::cout << "initInCtx" << std::endl;

    avdevice_register_all();
    const AVInputFormat *inputFmt = av_find_input_format(m_inputFormat.c_str());
    if(!inputFmt) {
        std::cout << "Cannot find input format: " << m_inputFormat;
    } else {
        av_dict_set(&m_inRecordOpt, "probesize", "30M", 0);
        av_dict_set(&m_inRecordOpt, "framerate", std::to_string(settings.fps).c_str(), 0);
        av_dict_set(&m_inRecordOpt, "offset_x", std::to_string(settings.off_x).c_str(), 0);
        av_dict_set(&m_inRecordOpt, "offset_y", std::to_string(settings.off_y).c_str(), 0);
        av_dict_set(&m_inRecordOpt, "video_size",
                    (std::to_string(settings.width) + "x" + std::to_string(settings.height)).c_str(), 0);


        m_inFmtCtx = avformat_alloc_context();
        if(!m_inFmtCtx) {
            std::cout << "Alloc m_inFmtCtx failed" << std::endl;
        } else {
            // std::cout << "Alloc m_inFmtCtx success" << std::endl;
            int ret_openInput = avformat_open_input(&m_inFmtCtx, m_inputDevice.c_str(), inputFmt, &m_inRecordOpt);
            if(ret_openInput < 0) {
                std::cout << "Open device " << m_inputDevice << " failed." << std::endl;
                av_strerror(ret_openInput, err_buff, ERR_BUFF_SIZE);
                std::cout << "Error: " << err_buff << std::endl;
            } else {
                // std::cout << "Open device " << m_inputDevice << " success." << std::endl;
                if(initIniVideoCtx() && initInAudioCtx()) {
                    ret = true;
                } else {
                    std::cout << "initInVideoCtx failed" << std::endl;
                }
            }
        }
    }

    return ret;
}

bool RecorderImpl::initIniVideoCtx() {
    // std::cout << "initInVideoCtx starts" << std::endl;
    bool ret = false;

    if(m_inFmtCtx) {
        int ret_findStreamInfo = avformat_find_stream_info(m_inFmtCtx, &m_inRecordOpt);
        if(ret_findStreamInfo < 0) {
            std::cout << "Could not find stream info." << std::endl;
            av_strerror(ret_findStreamInfo, err_buff, ERR_BUFF_SIZE);
             std::cout << "Error: " << err_buff << std::endl;
        } else {
            // std::cout << "Find stream infor success." << std::endl;
        }

        for(int streamIdx = 0; streamIdx < (int) m_inFmtCtx->nb_streams; streamIdx ++) {
            if(m_inFmtCtx->streams[streamIdx]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
                // std::cout << "Found video stream at index " << streamIdx << std::endl;
                // std::cout << "Codec ID: " << m_inFmtCtx->streams[streamIdx]->codecpar->codec_id << std::endl;
                m_inVideostreamIindex = streamIdx;
                break;
            }
        }

        if(m_inVideostreamIindex < 0) {
            std::cout << "Cannot find VIDEO stream from input" << std::endl;
        } else {
            m_inVideoDecoder = avcodec_find_decoder(m_inFmtCtx->streams[m_inVideostreamIindex]->codecpar->codec_id);
            if(!m_inVideoDecoder) {
                // std::cout << "Cannot find decoder for codec id: " << m_inFmtCtx->streams[m_inVideostreamIindex]->codecpar->codec_id << std::endl;
            } else {
                m_inVideoDecocedctx = avcodec_alloc_context3(m_inVideoDecoder);
                if(m_inVideoDecocedctx) {
                    // std::cout << "Alloc m_inVideoDecocedctx success" << std::endl;
                    int ret_copyParams = avcodec_parameters_to_context(m_inVideoDecocedctx, m_inFmtCtx->streams[m_inVideostreamIindex]->codecpar);
                    if(ret_copyParams < 0) {
                        std::cout << "Cannot copy parameters to context" << std::endl;
                    } else {
                        // std::cout << "Copy parameters to context success" << std::endl;
                    }
                    int ret_openCodec = avcodec_open2(m_inVideoDecocedctx, m_inVideoDecoder, NULL);
                    if(ret_openCodec < 0) {
                        std::cout << "Open Decoder error" << std::endl;
                    } else {
                        // std::cout << "Open Decoder success" << std::endl;
                        ret = true;
                    }
                } else {
                    std::cout << "Alloc m_inVideoDecocedctx failed" << std::endl;
                }
            }
        }
    } else {
        std::cout << "Cannot initInVideoCtx: m_inFmtCtx = nullptr." << std::endl;
    }

    // std::cout << "initIniVideoCtx finishes" << std::endl;

    return ret;
}

bool RecorderImpl::startRecord() {
    bool ret = false;
    if(isRecorderReady) {
        // std::cout << "Start recording" << std::endl;
        isRecording = true;
        captureVideoThread = std::thread(&RecorderImpl::captureDesktop, this);
        handleCapturedVideoDataThread = std::thread(&RecorderImpl::handleCapturedVideoData, this);
        captureAudioThread = std::thread(&RecorderImpl::captureAudio, this);
        ret = true;
    } else {
        std::cout << "Cannot start recording. Please check the log file for more detail." << std::endl;
    }
    return ret;
}

void RecorderImpl::captureDesktop() {
    // std::cout << "Capturing desktop" << std::endl;
    AVPacket *pkt;
    while(isRecording) {
        pkt = av_packet_alloc();
       if(readPktFromInputCtx(pkt)) {
           if(!savereadDesktopVideoPkt(pkt)) {
               std::cout << "Cannot save read pkt" << std::endl;
           }
       } else {
           std::cout << "Cannot read packet from input ctx" << std::endl;
       }
    }

    // std::cout << "Stop capturing desktop" << std::endl;
}

bool RecorderImpl::readPktFromInputCtx(AVPacket* pkt) {
    bool ret = false;
    int ret_readFrame = av_read_frame(m_inFmtCtx, pkt);
    if(ret_readFrame >= 0) {
        ret = true;
    } else {
        std::cout << "Read packet from input context error." << std::endl;
        av_strerror(ret_readFrame, err_buff, ERR_BUFF_SIZE);
        std::cout << "Error: " << err_buff << std::endl;
    }

    return ret;
}

bool RecorderImpl::savereadDesktopVideoPkt(AVPacket* pkt) {
    bool ret = false;
    if(pkt == nullptr) {
        std::cout << "savereadDesktopVideoPkt: pkt = nullptr" << std::endl;
    }

    pkt->stream_index = m_outVideoStreamIndex;
    std::lock_guard<std::mutex> lk(m_VideoQueueMutex);
    if(m_VideoPktQueue.size() < MAX_NUMBER_CACHE_PKT) {
        m_VideoPktQueue.push(pkt);
        ret = true;
    } else {
        std::cout << "Maximum number of cached pkt reached. Ignore this pkt" << std::endl;
    }

    return ret;
}

void RecorderImpl::handleCapturedVideoData() {
    AVPacket* pkt;
    AVPacket outPkt;
    AVFrame* outFrame;
    uint8_t *outBuf = nullptr;
    int outBufLen = av_image_get_buffer_size(m_inVideoDecocedctx->pix_fmt, m_inVideoDecocedctx->width, m_inVideoDecocedctx->height, 32);
    outBuf = (uint8_t*)av_malloc(outBufLen);
    outFrame = av_frame_alloc();
    av_image_fill_arrays(outFrame->data, outFrame->linesize, outBuf,
                         m_outVideoEncoderCtx->pix_fmt,
                         m_outVideoEncoderCtx->width,
                         m_outVideoEncoderCtx->height, 1);
    av_init_packet(&outPkt);
    outFrame->width = m_outVideoEncoderCtx->width;
    outFrame->height = m_outVideoEncoderCtx->height;
    outFrame->format = m_outVideoEncoderCtx->pix_fmt;

    int i = 1;
    int j = 1;

    AVFrame* decodedFrame;
    decodedFrame = av_frame_alloc();
    // av_image_fill_arrays()
    int k = 0;
    // while(k < MAX_TEST_FRAME) {
    while(isRecording) {
        if(readDesktopVideoPkt(&pkt)) {
            if(pkt == nullptr) {
                std::cout << "Error: pkt == nullptr" << std::endl;
                continue;
            }
            if(pkt->stream_index != m_outVideoStreamIndex) {
                std::cout << "Error: Wrong index packet" << std::endl;
                continue;
            }

            int av_err = avcodec_send_packet(m_inVideoDecocedctx, pkt);
            if(av_err >= 0) {
                // av_packet_unref(pkt);
                // av_packet_free(&pkt);
                int av_err = avcodec_receive_frame(m_inVideoDecocedctx, decodedFrame);
                if(av_err >= 0) {
                    // scale the decoded frame to match the out encoder
                    int ret_swsScale = sws_scale(swsCtx, decodedFrame->data, decodedFrame->linesize, 0, m_inVideoDecocedctx->height,
                              outFrame->data, outFrame->linesize);
                    if(ret_swsScale > 0) {
                        outFrame->pts = (int64_t)j * (int64_t)settings.fps * (int64_t)settings.fps * (int64_t)100 / (uint64_t)settings.fps;
                        j++;
                        av_err = avcodec_send_frame(m_outVideoEncoderCtx, outFrame);
                        if(av_err >= 0) {
                            av_err = avcodec_receive_packet(m_outVideoEncoderCtx, &outPkt);
                            if(av_err >= 0) {
                                outPkt.pts = (int64_t)i * (int64_t)30 * (int64_t)30 * (int64_t)100 / (uint64_t)30;
                                outPkt.dts = (int64_t)i * (int64_t)30 * (int64_t)30 * (int64_t)100 / (uint64_t)30;
                                i++;
                                writeVideoPktToOutputCtx(&outPkt);
                                k++;
                            } else {
                                std::cout << "Read packet from encoder error" << std::endl;
                                av_strerror(av_err, err_buff, ERR_BUFF_SIZE);
                                std::cout << "Error: " << err_buff << std::endl;
                            }
                        } else {
                            std::cout << "Send frame to encoder error" << std::endl;
                            av_strerror(av_err, err_buff, ERR_BUFF_SIZE);
                            std::cout << "Error: " << err_buff << std::endl;
                        }
                    } else {
                        std::cout << "Error: sws_scale returns: " << ret_swsScale << std::endl;
                    }
                } else {
                    std::cout << "Cannot read frame from decoder" << std::endl;
                    av_strerror(av_err, err_buff, ERR_BUFF_SIZE);
                    std::cout << "Error: " << err_buff << std::endl;
                }
            } else {
                std::cout << "Cannot provide raw pkt to decoder. error code: "<< av_err << std::endl;
                av_strerror(av_err, err_buff, ERR_BUFF_SIZE);
                std::cout << "Error: " << err_buff << std::endl;
            }
        }
    }
    isRecording = false;
    // std::cout << "Stop recording" << std::endl;
}

bool RecorderImpl::readDesktopVideoPkt(AVPacket** pkt) {
    bool ret = false;
    std::lock_guard<std::mutex> lk(m_VideoQueueMutex);
    if(!m_VideoPktQueue.empty()) {
        *pkt = m_VideoPktQueue.front();
        m_VideoPktQueue.pop();

        ret = true;
    }

    return ret;
}

bool RecorderImpl::writeVideoPktToOutputCtx(AVPacket *pkt) {
    bool ret = true;
    std::lock_guard<std::mutex> lk(m_writeOutputMutex);
    int av_err = av_write_frame(m_outFmtCtx, pkt);
    if(av_err < 0) {
        std::cout << "Cannot write packet to output" << std::endl;
    } else {
        // std::cout << "Write video packet to output file success" << std::endl;
    }

    return ret;
}

bool RecorderImpl::stopRecord() {
    isRecording = false;
    if(captureAudioThread.joinable()) {
        captureVideoThread.join();
    }
    if(handleCapturedVideoDataThread.joinable()) {
        handleCapturedVideoDataThread.join();
    }
    if(captureAudioThread.joinable()) {
        captureAudioThread.join();
    }

    av_write_trailer(m_outFmtCtx);
    avio_close(m_outFmtCtx->pb);
    return true;
}

bool RecorderImpl::saveRecord() {
    return true;
}

bool RecorderImpl::initOutCtx() {
    bool ret = false;

    int ret_AllocOutCtx = avformat_alloc_output_context2(&m_outFmtCtx, NULL, NULL, m_outFileName.c_str());
    if(ret_AllocOutCtx < 0) {
        std::cout << "Alloc output context failed." << std::endl;
        av_strerror(ret_AllocOutCtx, err_buff, ERR_BUFF_SIZE);
        std::cout << "Error: " << err_buff << std::endl;
    } else {
        if(initOutVideoCtx()) {
            ret = true;
        } else {
            std::cout << "Cannot initOutCtx: initOutVideoCtx failed" << std::endl;
        }
    }

    return ret;
}

bool RecorderImpl::initOutVideoCtx() {
    bool ret = false;

    if(!m_outFmtCtx) {
        std::cout << "initOutVideoCtx failed: m_outFmtCtx = nullptr" << std::endl;
    } else {
        m_outVideoEncoder = avcodec_find_encoder(OUT_VIDEO_ENCODER);
        if(!m_outVideoEncoder) {
            std::cout << "Cannot find encoder: " << OUT_VIDEO_ENCODER << std::endl;
        } else {
            m_outVideoEncoderCtx = avcodec_alloc_context3(m_outVideoEncoder);
            if(!m_outVideoEncoderCtx) {
                std::cout << "Alloc m_outVideoEncoderCtx failed" << std::endl;
            } else {
                // std::cout << "Alloc m_outVideoEncoderCtx success" << std::endl;
                m_outVideoEncoderCtx->codec_id = OUT_VIDEO_ENCODER;
                m_outVideoEncoderCtx->codec_type = AVMEDIA_TYPE_VIDEO;
                m_outVideoEncoderCtx->width = settings.width;
                m_outVideoEncoderCtx->height = settings.height;
                m_outVideoEncoderCtx->pix_fmt = AV_PIX_FMT_YUV420P;
                m_outVideoEncoderCtx->time_base = {1, 25};
                m_outVideoEncoderCtx->qmin = 1;
                m_outVideoEncoderCtx->qmax = 51;

                // Create out video stream
                AVStream *outVideoStream = avformat_new_stream(m_outFmtCtx, m_outVideoEncoder);
                if(!outVideoStream) {
                    std::cout << "Cannot alloc new video stream" << std::endl;
                } else {
                    // std::cout << "Create new video stream success" << std::endl;
                    int ret_paramCopy = avcodec_parameters_from_context(outVideoStream->codecpar, m_outVideoEncoderCtx);
                    if(ret_paramCopy < 0) {
                        std::cout << "Cannot copy parameters" << std::endl;
                    } else {
                        int ret_openCodec = avcodec_open2(m_outVideoEncoderCtx, m_outVideoEncoder, NULL);
                        if(ret_openCodec < 0) {
                            std::cout << "Cannot open decoder" << std::endl;
                        } else {
                            // for scaling
                            swsCtx = sws_getContext(m_inVideoDecocedctx->width,
                                                    m_inVideoDecocedctx->height,
                                                    m_inVideoDecocedctx->pix_fmt,
                                                    m_outVideoEncoderCtx->width,
                                                    m_outVideoEncoderCtx->height,
                                                    m_outVideoEncoderCtx->pix_fmt,
                                                    SWS_FAST_BILINEAR,
                                                    NULL,
                                                    NULL,
                                                    NULL);
                            if(!swsCtx) {
                                std::cout << "Cannot get sws content" << std::endl;
                            } else {
                                int ret_avioOpen = avio_open(&m_outFmtCtx->pb,m_outFileName.c_str(), AVIO_FLAG_WRITE);
                                if(ret_avioOpen < 0) {
                                    std::cout << "Cannot open output file" << std::endl;
                                    av_strerror(ret_avioOpen, err_buff, ERR_BUFF_SIZE);
                                    std::cout << "Error: " << err_buff << std::endl;
                                } else {
                                    ret = true;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    if(ret) {
        for(int i = 0; i < m_outFmtCtx->nb_streams; i++) {
            if(m_outFmtCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
                m_outVideoStreamIndex = i;
                break;
            }
        }

        if(m_outVideoStreamIndex < 0) {
            std::cout << "Could not find video stream in outfmtctx" << std::endl;
            ret = false;
        }
    }

    return ret;
}

bool RecorderImpl::initInAudioCtx() {
    std::cout << "init in audio ctx" << std::endl;
    bool ret = false;
    audioFmtCtx = avformat_alloc_context();
    if(audioFmtCtx) {
        AVDictionary *audioOpt = nullptr;
        av_dict_set(&audioOpt, "sample_rate", "44100", 0);
        av_dict_set(&audioOpt, "async", "25", 0);
        std::string audioDevice = "audio=" + settings.outAudioName;
        std::cout << "audio device: " << audioDevice << std::endl;
        const AVInputFormat *audioInputFmt = av_find_input_format("dshow");
        if(audioInputFmt) {
            int value = avformat_open_input(&audioFmtCtx, audioDevice.c_str(), audioInputFmt, &audioOpt);
            if(value >= 0) {
                int find_stream_info = avformat_find_stream_info(audioFmtCtx, NULL);
                if(find_stream_info < 0) {
                    std::cout << "Could not find stream info from audioFmtCtx" << std::endl;
                    av_strerror(find_stream_info, err_buff, ERR_BUFF_SIZE);
                    std::cout << "Error: " << err_buff << std::endl;
                }
                for(int i = 0 ; i < audioFmtCtx->nb_streams; i++) {
                    if(audioFmtCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
                        m_inAudioStreamIndex = i;
                        break;
                    }
                }
                if(m_inAudioStreamIndex < 0) {
                    std::cout << "Could not find audio stream from input" << std::endl;
                }
            } else {
                std::cout << "could not open audio" << std::endl;
                av_strerror(value, err_buff, ERR_BUFF_SIZE);
                std::cout << "Error: " << err_buff << std::endl;
            }
        } else {
            std::cout << "could not find input format" << std::endl;
        }
    } else {
        std::cout << "Cout not alloc audio format context" << std::endl;
    }

    if(m_inAudioStreamIndex >= 0) {
        AVCodecParameters *params = audioFmtCtx->streams[m_inAudioStreamIndex]->codecpar;
        m_inAudioDecoder = avcodec_find_decoder(params->codec_id);
        if(m_inAudioDecoder) {
            m_inAudioDecoderCtx = avcodec_alloc_context3(m_inAudioDecoder);
            if(m_inAudioDecoderCtx) {
                int param_to_context = avcodec_parameters_to_context(m_inAudioDecoderCtx, params);
                if(param_to_context < 0) {
                    std::cout << "avcodec_parameters_to_context failed." << std::endl;
                    av_strerror(param_to_context, err_buff, ERR_BUFF_SIZE);
                    std::cout << "Error: " << err_buff << std::endl;
                } else {
                    int ret_codecOpen = avcodec_open2(m_inAudioDecoderCtx, m_inAudioDecoder, NULL);
                    if(ret_codecOpen < 0) {
                        std::cout << "avcodec open m_inAudioDecoderCtx failed." << std::endl;
                        av_strerror(ret_codecOpen, err_buff, ERR_BUFF_SIZE);
                        std::cout << "Error: " << err_buff << std::endl;
                    } else {
                        ret = true;
                    }
                }
            } else {
                std::cout << "Could not alloc audio decoder ctx" << std::endl;
            }
        } else {
            std::cout << "Could not find audio decoder for codec ic: " << params->codec_id << std::endl;
        }
    }

    return ret;
}

bool RecorderImpl::initOutAudioCtx() {
    bool ret = false;
    if(!m_outFmtCtx || !audioFmtCtx) {
        std::cout << "cout not init out audio ctx: m_outFmtCtx = nullptr" << std::endl;
    } else {
        // init encoder first.
        m_outAudioEncoder = avcodec_find_encoder(OUT_AUDIO_ENCODER);
        if(m_outAudioEncoder) {
            // Define params for encoder
            m_outAudioEncoderCtx = avcodec_alloc_context3(m_outAudioEncoder);
            if(m_outAudioEncoderCtx) {
                m_outAudioEncoderCtx->codec_id = OUT_AUDIO_ENCODER;
                m_outAudioEncoderCtx->codec_type = AVMEDIA_TYPE_AUDIO;
                m_outAudioEncoderCtx->bit_rate = 128000;
                m_outAudioEncoderCtx->sample_rate = m_inAudioDecoderCtx->sample_rate;
                av_channel_layout_default(&(m_outAudioEncoderCtx->ch_layout), 2);
                m_outAudioEncoderCtx->sample_fmt = m_outAudioEncoder->sample_fmts ?
                            m_outAudioEncoder->sample_fmts[0] : AV_SAMPLE_FMT_FLTP;
                m_outAudioEncoderCtx->time_base = {1, m_inAudioDecoderCtx->sample_rate};
                m_outAudioEncoderCtx->strict_std_compliance = FF_COMPLIANCE_EXPERIMENTAL;
                if (m_outFmtCtx->oformat->flags & AVFMT_GLOBALHEADER)
                        m_outAudioEncoderCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
                int ret_codecOpen = avcodec_open2(m_outAudioEncoderCtx, m_outAudioEncoder, NULL);
                if(ret_codecOpen >= 0) {
                    AVStream *outAudioStream = avformat_new_stream(m_outFmtCtx, m_outAudioEncoder);
                    if(!outAudioStream) {
                        std::cout << "Cannot create new audio stream" << std::endl;
                    } else {
                        if(avcodec_parameters_from_context(outAudioStream->codecpar, m_outAudioEncoderCtx) < 0) {
                            std::cout << "Cannot convert parameter to audio codecpar" << std::endl;
                        } else {
                            int swr_alloc = swr_alloc_set_opts2(&swrCtx,
                                            &(m_outAudioEncoderCtx->ch_layout), m_outAudioEncoderCtx->sample_fmt, m_outAudioEncoderCtx->sample_rate,
                                            &(m_inAudioDecoderCtx->ch_layout), m_inAudioDecoderCtx->sample_fmt, m_inAudioDecoderCtx->sample_rate,
                                            0, nullptr);
                            if(swr_alloc < 0 || !swrCtx) {
                                std::cout << "swr set opt failed." << std::endl;
                                av_strerror(swr_alloc, err_buff, ERR_BUFF_SIZE);
                                std::cout << "Error: " << err_buff << std::endl;
                            } else {
                                int ret_swr_init = swr_init(swrCtx);
                                if(ret_swr_init < 0) {
                                    std::cout << "swr_nit returned: " << ret_swr_init << std::endl;
                                } else {
                                    ret = true;
                                }
                            }
                        }
                    }
                } else {
                    std::cout << "avcodec open m_outAudioEncoderCtx failed." << std::endl;
                    av_strerror(ret_codecOpen, err_buff, ERR_BUFF_SIZE);
                    std::cout << "Error: " << err_buff << std::endl;
                }
            } else {
                std::cout <<"Could not alloc out audio encoder ctx" << std::endl;
            }
        } else {
            std::cout << "could not find encoder for id " << OUT_AUDIO_ENCODER << std::endl;
        }
    }

    if(ret) {
        for(int i = 0; i < m_outFmtCtx->nb_streams; i++) {
            if(m_outFmtCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
                m_outAudioStreamIndex = i;
                break;
            }
        }

        if(m_outAudioStreamIndex < 0) {
            std::cout << "No audio stream in outfmtctx" << std::endl;
            ret = false;
        } else {
            audioBuff = av_audio_fifo_alloc(m_outAudioEncoderCtx->sample_fmt,
                                            m_outAudioEncoderCtx->ch_layout.nb_channels,
                                            1);
            if(!audioBuff) {
                std::cout << "Cannot allocate buffer for audio." << std::endl;
                ret = false;
            }
        }
    }

    return ret;
}

void RecorderImpl::captureAudio() {
    AVPacket *inPkt = nullptr, *outPkt = nullptr;
    AVFrame *decodedFrame = nullptr, *outputFrame = nullptr;
    uint8_t **resampledData;
    inPkt = (AVPacket*) av_malloc(sizeof(AVPacket));
    av_init_packet(inPkt);
    outPkt = (AVPacket*) av_malloc(sizeof(AVPacket));
    av_init_packet(outPkt);
    decodedFrame = av_frame_alloc();
    outputFrame = av_frame_alloc();
    int ret;
    int pts = 0;
    while(isRecording) {
        // std::cout << "Capture new audio packet" << std::endl;
        int ret = av_read_frame(audioFmtCtx, inPkt);
        if(ret >= 0) {
            if(inPkt->stream_index == m_inAudioStreamIndex) {
                int codec_ret = avcodec_send_packet(m_inAudioDecoderCtx, inPkt);
                if(codec_ret < 0) {
                    std::cout << "Send packet to decoder returned: " << codec_ret << std::endl;
                }
                while(codec_ret >= 0) {
                    codec_ret = avcodec_receive_frame(m_inAudioDecoderCtx, decodedFrame);
                    if(codec_ret >= 0) {
                        if(m_outFmtCtx->streams[m_outAudioStreamIndex]->start_time <= 0) {
                            m_outFmtCtx->streams[m_outAudioStreamIndex]->start_time = decodedFrame->pts;
                        }

                            // Allocate memory for sample
                            resampledData = (uint8_t **) calloc(m_outAudioEncoderCtx->ch_layout.nb_channels, sizeof(resampledData));
                            if(!resampledData) {
                                std::cout << "cannot initialize input samples" << std::endl;
                                continue;
                            }
                            ret = av_samples_alloc(resampledData, nullptr,
                                            m_outAudioEncoderCtx->ch_layout.nb_channels,
                                             decodedFrame->nb_samples,
                                             m_outAudioEncoderCtx->sample_fmt, 0);
                            if(ret >= 0) {
                                ret = swr_convert(swrCtx,
                                                  resampledData, decodedFrame->nb_samples,
                                                  (const uint8_t **)decodedFrame->extended_data, decodedFrame->nb_samples);
                                if(ret >= 0) {
                                    if(av_audio_fifo_realloc(audioBuff, av_audio_fifo_size(audioBuff) + decodedFrame->nb_samples) < 0) {
                                        std::cout << "Cannot realloc audio buf" << std::endl;
                                    } else {
                                        if(av_audio_fifo_write(audioBuff, (void**) resampledData, decodedFrame->nb_samples) < 0) {
                                            std::cout << "cannot write audio to audio buff" << std::endl;
                                        }
                                    }
                                    ;
                                    av_init_packet(outPkt);
                                    outPkt->data = nullptr;
                                    outPkt->size = 0;
                                    outputFrame = av_frame_alloc();
                                    outputFrame->nb_samples = m_outAudioEncoderCtx->frame_size;
                                    outputFrame->ch_layout = m_outAudioEncoderCtx->ch_layout;
                                    outputFrame->format = m_outAudioEncoderCtx->sample_fmt;
                                    outputFrame->sample_rate = m_outAudioEncoderCtx->sample_rate;
                                    av_frame_get_buffer(outputFrame, 0);
                                    while(av_audio_fifo_size(audioBuff) >= m_outAudioEncoderCtx->frame_size) {
                                        ret = av_audio_fifo_read(audioBuff, (void **)outputFrame->data, m_outAudioEncoderCtx->frame_size);
                                        if(ret > 0) {
                                            outputFrame->pts = pts;
                                            pts = pts + outputFrame->nb_samples;
                                            if(avcodec_send_frame(m_outAudioEncoderCtx, outputFrame) >= 0) {
                                                if(avcodec_receive_packet(m_outAudioEncoderCtx, outPkt) < 0) {
                                                    std::cout << "Coud not receive audio packet" << std::endl;
                                                    continue;
                                                }
                                                av_packet_rescale_ts(outPkt, m_outAudioEncoderCtx->time_base, m_outFmtCtx->streams[m_outAudioStreamIndex]->time_base);
                                                {
                                                    // outPkt->stream_index = m_outAudioStreamIndex;
                                                    outPkt->stream_index = m_outAudioStreamIndex;
                                                    std::lock_guard<std::mutex> lk(m_writeOutputMutex);
                                                    ret = av_write_frame(m_outFmtCtx, outPkt);
                                                    if(ret < 0) {
                                                        std::cout << "Write audio to output context error." << std::endl;
                                                    }
                                                }
                                            } else {
                                                std::cout << "Send audio frame to encoder error" << std::endl;
                                            }
                                        } else {
                                            std::cout << "audio fifo read error" << std::endl;
                                        }
                                    }

                                    av_frame_free(&outputFrame);
                                    av_packet_unref(outPkt);
                                } else {
                                    std::cout << "swr_convert returned: " << ret << std::endl;
                                }
                            } else {
                                std::cout << "av_samples_alloc returned: " << ret << std::endl;
                            }
                    } else {
                        std::cout << "Could not receive audio frame from decoder. Error code: " << codec_ret << std::endl;
                        av_strerror(codec_ret, err_buff, ERR_BUFF_SIZE);
                        std::cout << "Error: " << err_buff << std::endl;
                        if (codec_ret == AVERROR(EAGAIN) || codec_ret == AVERROR_EOF) {
                            break;
                        }
                    }
                }
            } else {
                std::cout << "Wrong audio stream index" << std::endl;
            }
        } else {
            std::cout << "Could not read frame from input audio" << std::endl;
        }
    }

    std::cout << "End capturing audio" << std::endl;
}

bool RecorderImpl::initOutputFile() {
    bool ret = false;
    if(m_outFmtCtx->nb_streams <= 0) {
        std::cout << "output file does not contain any stream" << std::endl;
    } else {
        int ret_writeHeader = avformat_write_header(m_outFmtCtx, NULL);
        if(ret_writeHeader < 0) {
            std::cout << "Cannot write header to output file" << std::endl;
        } else {
            ret = true;
        }
    }
    return ret;
}
