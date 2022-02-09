#include "pch.h"
#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "BackgroundMatting.h"
#include <net.h>
#include "VideoWriter.h"
#include "VideoReader.h"
#include <sys/stat.h>
#include <ctime>
#include <filesystem> 

#pragma comment(lib, "./lib/avcodec.lib")
#pragma comment(lib, "./lib/avutil.lib")
#pragma comment(lib, "./lib/avformat.lib")
#pragma warning(disable: 4996);

using namespace std;
using namespace std::tr2::sys;

inline bool file_exist(const std::string& name) {
	struct stat buffer;
	return (stat(name.c_str(), &buffer) == 0);
}



void video_matting_ffmpeg_read_write(std::string in_path, std::string out_path)
{
	BackgroundMatting bgm;
	int device_count = bgm.get_device_count();

	cv::Mat frame;
	cv::Mat alpha;
	//cv::Mat show;
	//cv::Mat show_mat;

	VideoReader reader = VideoReader(in_path);
	int fps = reader.get_fps();
	int height = reader.get_height();
	int width = reader.get_width();
	int bit_rate = reader.get_bit_rate();
	

	VideoWriter writer = VideoWriter(width, height, out_path, fps, bit_rate, device_count);
	int i = 1;
	double total = 0.0;
	double start = clock();
	int ret = 1;
	while (true)
	{
		int ret = reader.read(frame);
		if (ret == 1)
		{
			/*cv::cvtColor(frame, show, cv::COLOR_BGR2RGB);
			cv::resize(show, show, cv::Size(720, 480));
			cv::imshow("", show);*/

			bgm.draw(frame, alpha);
			/*cv::cvtColor(frame, show_mat, cv::COLOR_BGR2RGB);
			cv::resize(show_mat, show_mat, cv::Size(720, 480));
			cv::imshow("mat", show_mat);*/
			writer.write(frame);
			//cv::waitKey(1);
			i++;
			if (i % 100 == 0)
			{
				double current = clock();
				printf("Processing %d frames costs %.2f ms, %.2f ms in average\n", i, current - start, (current - start) / i);
			}
		}
		else if (ret < 0)
		{
			break;
		}
		/*if (i >= 500) {
			break;
		}*/
	}
	double current = clock();
	printf("Totally %d frames, %.2f ms, %.2f ms in average\n", i, current - start, (current - start) / i);
	writer.flush();
	reader.flush();
}

int main(int argc, char *argv[])
{
	string in_path = argv[1];

	if (!file_exist(in_path))
	{
		printf("Video [in] %s not found !\n", in_path.c_str());
		return 0;
	}

	path p(in_path.c_str());
	path filename = p.stem();
	cout << "Reading video from: " << p.string() << endl;

	p.replace_filename(filename.string() + "_matting.mp4");
	string matting_path = p.string();
	
	cout << "Writing matting to: " << matting_path << endl;

	video_matting_ffmpeg_read_write(in_path, matting_path);
	return 0;
}