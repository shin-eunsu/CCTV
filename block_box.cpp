// simple_camera.cpp
// MIT License
// Copyright (c) 2019 JetsonHacks
// See LICENSE for OpenCV license and additional information
// Using a CSI camera (such as the Raspberry Pi Version 2) connected to a 
// NVIDIA Jetson Nano Developer Kit using OpenCV
// Drivers for the camera and OpenCV are included in the base image

#define DEBUG

#include<iostream>
#include<opencv2/opencv.hpp>
#include<cstdio>
#include<ctime>
#include<cstring>
#include<cstdlib>

#include<sys/vfs.h>
#include<sys/stat.h>
#include<dirent.h>

// #include <opencv2/videoio.hpp>
// #include <opencv2/highgui.hpp>

using namespace cv;
using namespace std;

std::string gstreamer_pipeline (int capture_width, int capture_height, int display_width, int display_height, int framerate, int flip_method)
{
	return "nvarguscamerasrc ! video/x-raw(memory:NVMM), width=(int)" + 
		std::to_string(capture_width) + ", height=(int)" + std::to_string(capture_height) + 
		", format=(string)NV12, framerate=(fraction)" + std::to_string(framerate) +
		"/1 ! nvvidconv flip-method=" + std::to_string(flip_method) +
		" ! video/x-raw, width=(int)" + std::to_string(display_width) + 
		", height=(int)" + std::to_string(display_height) + 
		", format=(string)BGRx ! videoconvert ! video/x-raw, format=(string)BGR ! appsink";
}

int main()
{
    int capture_width = 640 ;
    int capture_height = 480 ;
    int display_width = 640 ;
    int display_height = 480 ;
    int framerate = 30.0;
    int flip_method = 0 ;
	int cnt = 0;

	int dir_err;

	int mkdir_check = 1;

	char make_dir[30];
	char remove_dir[30];
	char check_minute[3];
	char dir_path_init[30] = "/home/user1/blockbox/";
	char dir_path[30]; // 저장 폴더 경로 
	char file_name[30]; // 파일 이름 20191216_151625
	char save_path[100]; // 파일 저장 경로 및 이름
	time_t UTCtime = time(0);
	struct tm* tm = localtime(&UTCtime);

	//diskinfo
	struct statfs sf;
	int diskSize;

	//scandir
	struct dirent** dir_list;
	int dir_cnt;
	
    std::string pipeline = gstreamer_pipeline(capture_width,
	capture_height,
	display_width,
	display_height,
	framerate,
	flip_method);
	std::cout << "Using pipeline: \n\t" << pipeline << "\n";

	cv::VideoCapture cap(pipeline, cv::CAP_GSTREAMER);
	int fourcc = VideoWriter::fourcc('D', 'I', 'V', 'X');
//	int delay = cvRound(1000 / framerate);
	
	if(!cap.isOpened())
	{
		std::cout<<"Failed to open camera."<<std::endl;
		return (-1);
	}
	cv::Mat img;

	while(1)
	{
		//Disk Space Check
		statfs("/", &sf);
		diskSize = sf.f_bavail * (sf.f_bsize/1024);
		
		if(diskSize < 1048576 * 10) // diskSize < 10GB
		{
			//scandir
			if((dir_cnt = scandir(dir_path_init, &dir_list, NULL, alphasort)) == -1)
				perror("scandir err: ");
			//rm dir 맨앞
			strcpy(remove_dir, "rm -rf ");
			strcat(remove_dir, dir_list[0]->d_name);
			if((dir_cnt = system(remove_dir)) == -1)
				perror("err rm dir: ");
			else
				printf("Remove %s directory\n", dir_list[0]->d_name);
			//scan dir 메모리 반환
			for(int i = 0; i < dir_cnt; i++)
				free(dir_list[i]);
			free(dir_list);
		}
		
		memset(&save_path, 0, sizeof(save_path)); //init save_path
		strcpy(make_dir, "mkdir /home/user1/blockbox/"); //init make_dir

		cnt = 0; //frame cnt set 0
		//get current time
		UTCtime = time(0);
		tm = localtime(&UTCtime);
		strftime(file_name, 100, "%Y%m%d_%H%M%S", tm); // 파일명 (초 까지)
		strftime(dir_path, 20, "%Y%m%d_%H/", tm); // 폴더명 (시 까지)
		strcat(make_dir, dir_path);
		
		strftime(check_minute, 3, "%m", tm); //save파일 분단위 확인 (폴더 생성)
		if(!strcmp(check_minute, "00") || !strcmp(check_minute, "01"))
				mkdir_check = 1;
		
		if(mkdir_check) //첫 시작시엔 무조건 한번 실행
		{
			if((dir_err = system(make_dir)) == -1)
				perror("err mkdir: ");
			mkdir_check = 0;
		}
	
		strcpy(save_path, dir_path_init);
		strcat(save_path, dir_path); // /home/user1/blockbox/20191231_1550/ 폴더
		strcat(save_path, file_name); // + 파일명		
		strcat(save_path, ".avi"); // + avi 확장자


#ifdef DEBUG
		cout<<"Availble Disk Size: "<< diskSize << endl;
		cout<<"filename: "<< file_name << endl;
		cout<<"save_path: "<< save_path << endl;
#endif

		VideoWriter outputVideo(save_path, fourcc, framerate, Size(capture_width, capture_height));
		
		while(true)
		{
			if (!cap.read(img))
			{
				std::cout<<"Capture read error"<<std::endl;
				break;
			}
			outputVideo << img;
			cv::imshow("Live",img);

			if (waitKey(5) == 27)//esc
			{
				cap.release();
				cv::destroyAllWindows();
				return 0;
			}

			cnt++;
			if(cnt > 1790)
				break;
		}
	}
	cap.release();
	cv::destroyAllWindows();
	return 0;
}

