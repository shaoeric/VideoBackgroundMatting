#pragma once
#include <net.h>
#include <opencv2/opencv.hpp>

class BackgroundMatting
{
public:
	BackgroundMatting();
	~BackgroundMatting();

	void load(int model_size=512);
	void process(cv::Mat bgr, cv::Mat &mask, cv::Mat &foreground);
	int draw(cv::Mat &img, cv::Mat &alpha);

private:
	int target_size;
	const float means[3] = { 0, 0, 0 };
	const float norms[3] = { 1 / 255.0, 1 / 255.0, 1 / 255.0 };
	const float mean_vals[3] = { 123.675f, 116.28f,  103.53f };
	const float norm_vals[3] = { 0.01712475f, 0.0175f, 0.01742919f };

	ncnn::Net net;
	ncnn::Mat r1i, r2i, r3i, r4i;
	ncnn::Mat r4o, r3o, r2o, r1o;
	bool is_first;
};

