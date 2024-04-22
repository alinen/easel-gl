// Copyright (c) Aline Normoyle 2023
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
 
extern "C" // needed to use ffmpeg API in C++ (fixes linking errors)
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

#include "agl/image.h"

bool LoadVideo(const std::string& filename, std::vector<agl::Image>& frames) {

    // Open the input file
    AVFormatContext *formatContext = avformat_alloc_context();
    if (avformat_open_input(&formatContext, 
      filename.c_str(), nullptr, nullptr) != 0) {
        fprintf(stderr, "Error: Couldn't open the input file\n");
        return false;
    }

    // Retrieve stream information
    if (avformat_find_stream_info(formatContext, nullptr) < 0) {
        fprintf(stderr, "Error: Couldn't find stream information\n");
        avformat_close_input(&formatContext);
        return false;
    }

    // Find the video stream
    int videoStreamIndex = -1;
    for (unsigned int i = 0; i < formatContext->nb_streams; i++) {
        if (formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoStreamIndex = i;
            break;
        }
    }

    if (videoStreamIndex == -1) {
        fprintf(stderr, "Error: Couldn't find a video stream\n");
        avformat_close_input(&formatContext);
        return false;
    }

    // Get a pointer to the codec context for the video stream
    AVCodecParameters *codecParameters = formatContext->streams[videoStreamIndex]->codecpar;
    AVCodec *codec = avcodec_find_decoder(codecParameters->codec_id);
    if (codec == nullptr) {
        fprintf(stderr, "Error: Couldn't find a suitable video decoder\n");
        avformat_close_input(&formatContext);
        return false;
    }

    AVCodecContext *codecContext = avcodec_alloc_context3(codec);
    if (avcodec_parameters_to_context(codecContext, codecParameters) < 0) {
        fprintf(stderr, "Error: Couldn't copy codec parameters to codec context\n");
        avcodec_free_context(&codecContext);
        avformat_close_input(&formatContext);
        return false;
    }

    // Open the codec
    if (avcodec_open2(codecContext, codec, nullptr) < 0) {
        fprintf(stderr, "Error: Couldn't open the video codec\n");
        avcodec_free_context(&codecContext);
        avformat_close_input(&formatContext);
        return false;
    }

    // Allocate video frame
    AVFrame *rawframe = av_frame_alloc();

    // Allocate an AVPacket
    AVPacket *packet = av_packet_alloc();

    // Read frames and decode
    char buf[1024];
    while (av_read_frame(formatContext, packet) >= 0) {
        if (packet->stream_index == videoStreamIndex) {
            // Decode video frame
            int response = avcodec_send_packet(codecContext, packet);
            if (response < 0) {
                av_strerror(response, buf, 1024);
                fprintf(stderr, 
                  "Error while sending a packet to the decoder: %s\n", buf);
                break;
            }

            while (response >= 0) {
               response = avcodec_receive_frame(codecContext, rawframe);
               if (response == AVERROR(EAGAIN) || response == AVERROR_EOF) {
                    break;
               } 
               else if (response < 0) {
                    av_strerror(response, buf, 1024);
                    fprintf(stderr, 
                      "Error while receiving a frame from the decoder: %s\n", buf);
                    break;
               }

              AVFrame *dstFrame = av_frame_alloc();
              int dstWidth = rawframe->width;
              int dstHeight = rawframe->height;
              AVPixelFormat dstPixelFormat = AV_PIX_FMT_RGBA;  

              // Set up the sws context for pixel format conversion
              SwsContext *swsContext = sws_getContext(
                  rawframe->width, rawframe->height, 
                  static_cast<AVPixelFormat>(rawframe->format),
                  dstWidth, dstHeight, dstPixelFormat,
                  SWS_BICUBIC, nullptr, nullptr, nullptr
              );

              av_image_alloc(dstFrame->data, dstFrame->linesize, 
                dstWidth, dstHeight, dstPixelFormat, 1);

              // Perform the pixel format conversion
              sws_scale(
                  swsContext,
                  rawframe->data, rawframe->linesize, 0, rawframe->height,
                  dstFrame->data, dstFrame->linesize
              );

              agl::Image img;
              frames.push_back(img);
              frames.back().set(dstWidth, dstHeight, (unsigned char*) dstFrame->data[0]);
              
              av_freep(dstFrame->data);
              av_frame_free(&dstFrame);
              sws_freeContext(swsContext);
            }
        }

        av_packet_unref(packet);
    }

    // Free resources
    av_packet_free(&packet);
    av_frame_free(&rawframe);
    avcodec_free_context(&codecContext);
    avformat_close_input(&formatContext);

    return true;
}
 