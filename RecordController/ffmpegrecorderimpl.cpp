#include "ffmpegrecorderimpl.h"

RecorderImpl::RecorderImpl()
{
    av_log_set_level(AV_LOG_ERROR);

    /*
     * TODO: read these values from input
     */
    m_inputFormat = "gdigrab";
    m_inputDevice = "desktop";
    m_outFileName = "out.mp4";

    if(initInCtx() && initOutCtx()) {
        isRecorderReady = true;
    } else {
        isRecorderReady = false;
    }
}

RecorderImpl::~RecorderImpl() {
    // av_write_trailer(m_outFmtCtx);
    // avformat_close_input(&m_inFmtCtx);
    // avformat_free_context(m_inFmtCtx);
}

bool RecorderImpl::initInCtx() {
    bool ret = false;
    std::cout << "initInCtx" << std::endl;

    avdevice_register_all();
    std::cout << "Registered all devices" << std::endl;
    std::cout << "Opening input format " << m_inputFormat << std::endl;
    const AVInputFormat *inputFmt = av_find_input_format(m_inputFormat.c_str());
    if(!inputFmt) {
        std::cout << "Cannot find input format: " << m_inputFormat;
    } else {
        std::cout << "Open input format " << m_inputFormat << " success" << std::endl;
        /*
         * TODO: Read these values from user input
         */
        av_dict_set(&m_inRecordOpt, "probesize", "30M", 0);
        // av_dict_set(&m_inRecordOpt, "video_size", "1920x1080", 0);
        av_dict_set(&m_inRecordOpt, "framerate", "30", 0);

        m_inFmtCtx = avformat_alloc_context();
        if(!m_inFmtCtx) {
            std::cout << "Alloc m_inFmtCtx failed" << std::endl;
        } else {
            std::cout << "Alloc m_inFmtCtx success" << std::endl;
            int ret_openInput = avformat_open_input(&m_inFmtCtx, m_inputDevice.c_str(), inputFmt, &m_inRecordOpt);
            if(ret_openInput < 0) {
                std::cout << "Open device " << m_inputDevice << " failed." << std::endl;
                av_strerror(ret_openInput, err_buff, ERR_BUFF_SIZE);
                std::cout << "Error: " << err_buff << std::endl;
            } else {
                std::cout << "Open device " << m_inputDevice << " success." << std::endl;
                if(initIniVideoCtx()) {
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
    std::cout << "initInVideoCtx starts" << std::endl;
    bool ret = false;

    if(m_inFmtCtx) {
        int ret_findStreamInfo = avformat_find_stream_info(m_inFmtCtx, &m_inRecordOpt);
        if(ret_findStreamInfo < 0) {
            std::cout << "Could not find stream info." << std::endl;
            av_strerror(ret_findStreamInfo, err_buff, ERR_BUFF_SIZE);
             std::cout << "Error: " << err_buff << std::endl;
        } else {
            std::cout << "Find stream infor success." << std::endl;
        }

        for(int streamIdx = 0; streamIdx < (int) m_inFmtCtx->nb_streams; streamIdx ++) {
            if(m_inFmtCtx->streams[streamIdx]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
                std::cout << "Found video stream at index " << streamIdx << std::endl;
                std::cout << "Codec ID: " << m_inFmtCtx->streams[streamIdx]->codecpar->codec_id << std::endl;
                m_inVideostreamIindex = streamIdx;
                break;
            }
        }

        if(m_inVideostreamIindex < 0) {
            std::cout << "Cannot find VIDEO stream from input" << std::endl;
        } else {
            m_inVideoDecoder = avcodec_find_decoder(m_inFmtCtx->streams[m_inVideostreamIindex]->codecpar->codec_id);
            if(!m_inVideoDecoder) {
                std::cout << "Cannot find decoder for codec id: " << m_inFmtCtx->streams[m_inVideostreamIindex]->codecpar->codec_id << std::endl;
            } else {
                m_inVideoDecocedctx = avcodec_alloc_context3(m_inVideoDecoder);
                if(m_inVideoDecocedctx) {
                    std::cout << "Alloc m_inVideoDecocedctx success" << std::endl;
                    int ret_copyParams = avcodec_parameters_to_context(m_inVideoDecocedctx, m_inFmtCtx->streams[m_inVideostreamIindex]->codecpar);
                    if(ret_copyParams < 0) {
                        std::cout << "Cannot copy parameters to context" << std::endl;
                    } else {
                        std::cout << "Copy parameters to context success" << std::endl;
                    }
                    int ret_openCodec = avcodec_open2(m_inVideoDecocedctx, m_inVideoDecoder, NULL);
                    if(ret_openCodec < 0) {
                        std::cout << "Open Decoder error" << std::endl;
                    } else {
                        std::cout << "Open Decoder success" << std::endl;
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

    std::cout << "initIniVideoCtx finishes" << std::endl;

    return ret;
}

bool RecorderImpl::startRecord() {
    bool ret = false;
    if(isRecorderReady) {
        std::cout << "Start recording" << std::endl;
        isRecording = true;
        captureVideoThread = std::thread(&RecorderImpl::captureDesktop, this);
        handleCapturedDataThread = std::thread(&RecorderImpl::handleCapturedData, this);
        // captureVideoThread.detach();
        // handleCapturedDataThread.detach();
    } else {
        std::cout << "Cannot start recording. Please check the log file for more detail." << std::endl;
    }
    return ret;
}

void RecorderImpl::captureDesktop() {
    std::cout << "Capturing desktop" << std::endl;
    AVPacket *pkt;
    while(isRecording) {
        pkt = av_packet_alloc();
       if(readPktFromInputCtx(pkt)) {
           if(!saveReadDesktopPkt(pkt)) {
               std::cout << "Cannot save read pkt" << std::endl;
           }
       } else {
           std::cout << "Cannot read packet from input ctx" << std::endl;
       }
    }

    std::cout << "Stop capturing desktop" << std::endl;
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

bool RecorderImpl::saveReadDesktopPkt(AVPacket* pkt) {
    bool ret = false;
    if(pkt == nullptr) {
        std::cout << "saveReadDesktopPkt: pkt = nullptr" << std::endl;
    }

    std::lock_guard<std::mutex> lk(m_VideoQueueMutex);
    if(m_VideoPktQueue.size() < MAX_NUMBER_CACHE_PKT) {
        m_VideoPktQueue.push(pkt);
        ret = true;
    } else {
        std::cout << "Maximum number of cached pkt reached. Ignore this pkt" << std::endl;
    }

    return ret;
}

void RecorderImpl::handleCapturedData() {
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
        if(readDesktopPkt(&pkt)) {
            if(pkt == nullptr) {
                std::cout << "Error: pkt == nullptr" << std::endl;
                continue;
            }
            if(pkt->stream_index != m_inVideostreamIindex) {
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
                        outFrame->pts = (int64_t)j * (int64_t)30 * (int64_t)30 * (int64_t)100 / (uint64_t)30;
                        j++;
                        av_err = avcodec_send_frame(m_outVideoEncoderCtx, outFrame);
                        if(av_err >= 0) {
                            av_err = avcodec_receive_packet(m_outVideoEncoderCtx, &outPkt);
                            if(av_err >= 0) {
                                outPkt.pts = (int64_t)i * (int64_t)30 * (int64_t)30 * (int64_t)100 / (uint64_t)30;
                                outPkt.dts = (int64_t)i * (int64_t)30 * (int64_t)30 * (int64_t)100 / (uint64_t)30;
                                i++;
                                writePktToOutputCtx(&outPkt);
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
    std::cout << "Stop recording" << std::endl;
}

bool RecorderImpl::readDesktopPkt(AVPacket** pkt) {
    bool ret = false;
    std::lock_guard<std::mutex> lk(m_VideoQueueMutex);
    if(!m_VideoPktQueue.empty()) {
        *pkt = m_VideoPktQueue.front();
        m_VideoPktQueue.pop();

        ret = true;
    }

    return ret;
}

bool RecorderImpl::writePktToOutputCtx(AVPacket *pkt) {
    bool ret = true;
    int av_err = av_write_frame(m_outFmtCtx, pkt);
    if(av_err < 0) {
        std::cout << "Cannot write packet to output" << std::endl;
    }

    return ret;
}

bool RecorderImpl::stopRecord() {
    isRecording = false;
    captureVideoThread.join();
    handleCapturedDataThread.join();

    av_write_trailer(m_outFmtCtx);
    // avformat_close_input(&m_inFmtCtx);
    // avformat_free_context(m_inFmtCtx);
    return true;
}

bool RecorderImpl::saveRecord() {
    return true;
}

bool RecorderImpl::initOutCtx() {
    std::cout << "initOutCtx" << std::endl;

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
    std::cout << "initOutVideoCtx starts" << std::endl;
    bool ret = false;

    if(!m_outFmtCtx) {
        std::cout << "initOutVideoCtx failed: m_outFmtCtx = nullptr" << std::endl;
    } else {
        m_outVideoEncoder = avcodec_find_encoder(OUT_ENCODER);
        if(!m_outVideoEncoder) {
            std::cout << "Cannot find encoder: " << OUT_ENCODER << std::endl;
        } else {
            m_outVideoEncoderCtx = avcodec_alloc_context3(m_outVideoEncoder);
            if(!m_outVideoEncoderCtx) {
                std::cout << "Alloc m_outVideoEncoderCtx failed" << std::endl;
            } else {
                std::cout << "Alloc m_outVideoEncoderCtx success" << std::endl;

                // TODO: Some configuration fileds must be filled with user input
                m_outVideoEncoderCtx->codec_id = OUT_ENCODER;
                m_outVideoEncoderCtx->codec_type = AVMEDIA_TYPE_VIDEO;
                m_outVideoEncoderCtx->width = 1920;
                m_outVideoEncoderCtx->height = 1080;
                m_outVideoEncoderCtx->pix_fmt = AV_PIX_FMT_YUV420P;
                m_outVideoEncoderCtx->time_base = {1, 25};
                m_outVideoEncoderCtx->qmin = 1;
                m_outVideoEncoderCtx->qmax = 51;

                // Create out video stream
                AVStream *outVideoStream = avformat_new_stream(m_outFmtCtx, m_outVideoEncoder);
                if(!outVideoStream) {
                    std::cout << "Cannot alloc new video stream" << std::endl;
                } else {
                    std::cout << "Create new video stream success" << std::endl;
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
                                std::cout << "sws get content success" << std::endl;
                                // Open output file
                                int ret_avioOpen = avio_open(&m_outFmtCtx->pb,m_outFileName.c_str(), AVIO_FLAG_WRITE);
                                if(ret_avioOpen < 0) {
                                    std::cout << "Cannot oen output file" << std::endl;
                                    av_strerror(ret_avioOpen, err_buff, ERR_BUFF_SIZE);
                                    std::cout << "Error: " << err_buff << std::endl;
                                } else {
                                    std::cout <<"Open oputput file success" << std::endl;
                                    int ret_writeHeader = avformat_write_header(m_outFmtCtx, NULL);
                                    if(ret_writeHeader < 0) {
                                        std::cout << "Cannot write header to output file" << std::endl;
                                    } else {
                                        std::cout << "Write header to output file success" << std::endl;
                                        ret = true;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    return ret;
}
