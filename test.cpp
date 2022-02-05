#include  "pch.h"

extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
}

#include <iostream>
#include <opencv2/opencv.hpp>

#define INBUF_SIZE 1920*1080
#define WIDTH 1440
#define HEIGHT 1080

cv::Mat AVFrame2Img(AVFrame *pFrame)
{
	int frameHeight = pFrame->height;
	int frameWidth = pFrame->width;
	
	//创建保存yuv数据的buffer
	uchar* pDecodedBuffer = (uchar*)malloc(frameHeight*frameWidth * sizeof(uchar)* 3 / 2);

	//从AVFrame中获取yuv420p数据，并保存到buffer
	int i, j, k;
	//拷贝y分量
	for (i = 0; i < frameHeight; i++)
	{
		memcpy(pDecodedBuffer + frameWidth * i,
			pFrame->data[0] + pFrame->linesize[0] * i,
			frameWidth);
	}
	//拷贝u分量
	for (j = 0; j < frameHeight / 2; j++)
	{
		memcpy(pDecodedBuffer + frameWidth * i + frameWidth / 2 * j,
			pFrame->data[1] + pFrame->linesize[1] * j,
			frameWidth / 2);
	}
	//拷贝v分量
	for (k = 0; k < frameHeight / 2; k++)
	{
		memcpy(pDecodedBuffer + frameWidth * i + frameWidth / 2 * j + frameWidth / 2 * k,
			pFrame->data[2] + pFrame->linesize[2] * k,
			frameWidth / 2);
	}
	//输出图像分配内存
	cv::Mat img = cv::Mat(frameHeight * 3 / 2, frameWidth, CV_8UC1, pDecodedBuffer);
	cv::Mat bgr;
	cv::cvtColor(img, bgr, cv::COLOR_YUV420p2RGB);
	//释放buffer
	free(pDecodedBuffer);
	return bgr;
}


int decode_video2(std::string input_filename)
{
	int ret = 0;

	AVFormatContext* ifmt_ctx = NULL, *ofmt_ctx = NULL;
	const AVCodec* codec = NULL;
	AVCodecContext* codec_ctx = NULL;

	AVPacket* pkt = NULL;
	AVFrame* frame = NULL;


	pkt = av_packet_alloc();

	frame = av_frame_alloc();

	// 1、打开多媒体文件
	ret = avformat_open_input(&ifmt_ctx, input_filename.data(), NULL, NULL);
	if (ret < 0)
	{
		std::cout << "avformat_open_input error,ret=" << ret << std::endl;
		return -4;
	}

	ret = avformat_find_stream_info(ifmt_ctx, NULL);
	if (ret < 0)
	{
		std::cout << "avformat_find_stream_info error,ret=" << ret << std::endl;
		return -5;
	}
	av_dump_format(ifmt_ctx, 0, input_filename.data(), 0);

	int video_stream_idx = -1;
	for (size_t i = 0; i < ifmt_ctx->nb_streams; i++)
	{
		if (ifmt_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			//avcodec_parameters_to_context(codec_ctx, ifmt_ctx->streams[i]->codecpar);
			video_stream_idx = i;
			break;
		}
	}
	if (video_stream_idx == -1) {
		exit(1);
	}
	// 2、find解码器
	
	codec = (AVCodec*)avcodec_find_decoder(ifmt_ctx->streams[video_stream_idx]->codecpar->codec_id);
	if (codec == NULL) {
		exit(1);
	}

	// 3、分配解码器上下文
	codec_ctx = avcodec_alloc_context3(codec);
	if (codec_ctx == NULL)
	{
		std::cout << "avcodec_alloc_context3 error" << std::endl;
	}

	// 4、获取解码参数，支持mp4/H264文件,这几行代码非常重要，否则不能解码mp4到yuv
	//设置加速解码
	codec_ctx->lowres = codec->max_lowres;
	codec_ctx->flags2 |= AV_CODEC_FLAG2_FAST;
	avcodec_parameters_to_context(codec_ctx, ifmt_ctx->streams[video_stream_idx]->codecpar);
		
	// 5、open解码器
	ret = avcodec_open2(codec_ctx, codec, NULL);
	if (ret < 0)
	{
		std::cout << "avcodec_open2 error, ret=" << ret << std::endl;
		exit(1);
	}
	

	while (av_read_frame(ifmt_ctx, pkt) >= 0)
	{
		// 如果是视频
		if (ifmt_ctx->streams[pkt->stream_index]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			if (pkt->size)
			{
				ret = avcodec_send_packet(codec_ctx, pkt);
				if (ret < 0 || ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
				{
					std::cout << "avcodec_send_packet: " << ret << std::endl;
					continue;
				}
				ret = avcodec_receive_frame(codec_ctx, frame);
				if (ret < 0 || ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
				{
					std::cout << "avcodec_receive_packet: " << ret << std::endl;
					continue;
				}
				cv::Mat img = AVFrame2Img(frame);
				cv::imshow("", img);
				cv::waitKey(1);
			}
			av_packet_unref(pkt);
			if (ret < 0)
			{
				break;
			}
		}
	}

	avcodec_free_context(&codec_ctx);
	avformat_free_context(ifmt_ctx);

	av_packet_free(&pkt);
	av_frame_free(&frame);

	return 0;
}

int main() {
	decode_video2("test.flv");
	return 0;
}