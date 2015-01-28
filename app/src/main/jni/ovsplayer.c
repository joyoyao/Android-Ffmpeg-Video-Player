#include <stdio.h>
#include <stdlib.h>
#include <android/log.h>
#include <android/bitmap.h>

#include "com_example_chengang_myapplication_MainActivity.h"
#include "libavutil/avutil.h"
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"

#define  LOG_TAG    "ovsplayer"
#define  LOG(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

static void fill_bitmap(AndroidBitmapInfo*  info, void *pixels, AVFrame *pFrame)
{
    uint8_t *frameLine;

    int  yy;
    for (yy = 0; yy < info->height; yy++) {
        uint8_t*  line = (uint8_t*)pixels;
        frameLine = (uint8_t *)pFrame->data[0] + (yy * pFrame->linesize[0]);

        int xx;
        for (xx = 0; xx < info->width; xx++) {
            int out_offset = xx * 4;
            int in_offset = xx * 3;

            line[out_offset] = frameLine[in_offset];
            line[out_offset+1] = frameLine[in_offset+1];
            line[out_offset+2] = frameLine[in_offset+2];
            line[out_offset+3] = 254;
        }
        pixels = (char*)pixels + info->stride;
    }
}

JNIEXPORT jstring JNICALL Java_com_example_chengang_myapplication_MainActivity_getStringFromNative
  (JNIEnv * env , jobject obj, jstring url, jstring bitmap)
  {
        char wd[512];
        LOG("[%s]", "hello terminal");

//        char filename[512];
//        sprintf(filename, "%s", "/sdcard/1.flv");

        const char * filename = (*env)->GetStringUTFChars(env, url, 0);

//        FILE * fd;
//        fd = fopen(filename, "r");
//        fseek(fd, 0L, 2);
//        long filesize = ftell(fd);

        // 打开媒体文件
        av_register_all();
        AVFormatContext *input_context = NULL;
        int ret = avformat_open_input(&input_context, filename, NULL, NULL);

        // 分别找出一路视频流和一路音频流
        ret = avformat_find_stream_info(input_context, NULL);
//        int streamNum = input_context->nb_streams;
        int videoStream = -1;
        int audioStream = -1;
        int i;
        for (i=0; i<input_context->nb_streams; i++) {
            if(input_context->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO) {
                videoStream = i;
            }
            else if(input_context->streams[i]->codec->codec_type==AVMEDIA_TYPE_AUDIO) {
                audioStream = i;
            }
        }

        // 初始化编码器
        AVCodecContext *pCodecCtx = input_context->streams[videoStream]->codec;
        AVCodec *pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
        avcodec_open2(pCodecCtx, pCodec, NULL);
        LOG("Video size is [%d x %d]", pCodecCtx->width, pCodecCtx->height);

        // 初始化一个YUV帧内存空间
        AVFrame * pFrame = av_frame_alloc();

        // 初始化一个RGB帧内存空间
        AVFrame * pFrameRGB = av_frame_alloc();
        int buffBytes = avpicture_get_size(AV_PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height);
        uint8_t * buffer = (uint8_t *) av_malloc( buffBytes * sizeof(uint8_t) );
        avpicture_fill( (AVPicture *)pFrameRGB, buffer, AV_PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height);

        // 拿到Java中Bitmap的引用
        AndroidBitmapInfo  bitMapInfo;
        AndroidBitmap_getInfo(env, bitmap, &bitMapInfo);

        // Lock Bitmap
        void * pixels_lock;
        AndroidBitmap_lockPixels(env, bitmap, &pixels_lock);

        // 把视频往前seek一点
        av_seek_frame(input_context, videoStream, 5000, 0);

        // 读取一个视频帧
        AVPacket packet;
        static struct SwsContext *img_convert_ctx;
        int readVideoFrameNum = 0;
        int isDecodeSucc = 0;

        while (readVideoFrameNum < 40)
        {
            av_read_frame(input_context, &packet);
            if(packet.stream_index==videoStream)
            {
                avcodec_decode_video2(pCodecCtx, pFrame, &isDecodeSucc, &packet);
                if (isDecodeSucc)
                {
                    int target_width = 640;
                    int target_height = 480;
                    img_convert_ctx = sws_getContext(
                        pCodecCtx->width,   pCodecCtx->height,  pCodecCtx->pix_fmt,
                        pCodecCtx->width,   pCodecCtx->height,      AV_PIX_FMT_RGB24,
//                        target_width,       target_height,      AV_PIX_FMT_RGB24,
                        SWS_BICUBIC, NULL, NULL, NULL);
                    sws_scale(img_convert_ctx, (const uint8_t* const*)pFrame->data, pFrame->linesize, 0, pCodecCtx->height, pFrameRGB->data, pFrameRGB->linesize);

                    fill_bitmap(&bitMapInfo, pixels_lock, pFrameRGB);
                    readVideoFrameNum++;
                }
            }
        }

        // Unlock Bitmap
        AndroidBitmap_unlockPixels(env, bitmap);

        sprintf(wd, "AVCODEC VERSION %u\n, videoStream[%d], audioStream[%d]" , avcodec_version(), videoStream, audioStream);
        avformat_close_input(&input_context);

        return (*env)->NewStringUTF(env, wd);
  }