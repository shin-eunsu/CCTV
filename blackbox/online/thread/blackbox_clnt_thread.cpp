// simple_camera.cpp
// MIT License
// Copyright (c) 2019 JetsonHacks
// See LICENSE for OpenCV license and additional information
// Using a CSI camera (such as the Raspberry Pi Version 2) connected to a 
// NVIDIA Jetson Nano Developer Kit using OpenCV
// Drivers for the camera and OpenCV are included in the base image

#define DEBUG
#define DEBUG2

#define IP "192.168.1.10"
#define DISK_AVAILABLE_SIZE 1048576 * 10 //10GB

#include<iostream>
#include<opencv2/opencv.hpp>
#include<cstdio>
#include<ctime>
#include<cstring>
#include<cstdlib>

#include<stdio.h>
#include<sys/vfs.h>
#include<sys/stat.h>
#include<dirent.h>
#include<unistd.h>
#include<string.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<sys/wait.h>
#include<pthread.h>

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

pthread_mutex_t mutx;

typedef struct _DataSet
{
	//int file_len;
	int clnt_sock;
	char buf_snd[BUFSIZ];
	char file_name[100]; // 파일 이름 20191216_151625
	char save_path[BUFSIZ]; // 파일 저장 경로 및 이름
	//FILE* fd;
}DataSet;

//func
void* send_data(void* arg);

int main(int argc, char* argv[])
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
	char make_dir[50];
	char remove_dir[400];
	char check_minute[3];
	char dir_path_init[25] = "/home/user1/blackbox/";
	char dir_path[40]; // 저장 폴더 경로 
	
	//time
	time_t UTCtime = time(0);
	struct tm* tm = localtime(&UTCtime);
	
	//diskinfo
	struct statfs sf;
	int diskSize;
	
	//scandir
	struct dirent** dir_list;
	int dir_cnt;

	//socket
	struct sockaddr_in serv_addr;

	//thread
	pthread_t p_thread;
	pthread_attr_t attr;
	int thread_id;
	DataSet* DS;
	DS = (DataSet*)malloc(sizeof(DataSet));
	
    std::string pipeline = gstreamer_pipeline(capture_width,
	capture_height,
	display_width,
	display_height,
	framerate,
	flip_method);
	std::cout << "Using pipeline: \n\t" << pipeline << "\n";

	cv::VideoCapture cap(pipeline, cv::CAP_GSTREAMER);
	int fourcc = VideoWriter::fourcc('D', 'I', 'V', 'X');
	
	if(!cap.isOpened())
	{
		std::cout<<"Failed to open camera."<<std::endl;
		return (-1);
	}
	cv::Mat img;
	
	while(1)
	{
		pthread_mutex_lock(&mutx);
		if((DS->clnt_sock = socket(PF_INET, SOCK_STREAM, 0)) == -1)
		{
			perror("socket err: ");
//			exit(1);
		}

		memset(&DS->buf_snd, 0, sizeof(DS->buf_snd));
		memset(&serv_addr, 0, sizeof(serv_addr));

		serv_addr.sin_family = AF_INET;
		serv_addr.sin_addr.s_addr = inet_addr(IP);
		serv_addr.sin_port = htons(atoi(argv[1]));

		if(connect(DS->clnt_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
		{
			perror("connect err: ");
//			exit(1);
		}
#ifdef DEBUG
		cout << "* connect socket: " << DS->clnt_sock << endl;
#endif
		//Disk Space Check
		statfs("/", &sf);
		diskSize = sf.f_bavail * (sf.f_bsize/1024);
		
		if(diskSize < DISK_AVAILABLE_SIZE) // 10GB
		{
#ifdef DEBUG
		cout << "* start rmdir " << endl;
#endif
			//scandir
			if((dir_cnt = scandir(dir_path_init, &dir_list, NULL, alphasort)) == -1)
				perror("scandir err: ");
#ifdef DEBUG
		cout << "* scan dir " << endl;
#endif
			//rm dir
			sprintf(remove_dir, "rm -rf %s%s", dir_path_init, dir_list[2]->d_name);
			if(system(remove_dir) == -1)
				perror("err rm dir: ");
			else
				cout << "@ remove " << dir_list[2]->d_name << " directory" << endl;
#ifdef DEBUG
		cout << "* finish rmdir " << endl;
#endif
			//scan dir 메모리 반환
			for(int i = 0; i < dir_cnt; i++)
				free(dir_list[i]);
			free(dir_list);
#ifdef DEBUG
		cout << "* free rmdir memory" << endl;
#endif
		}

		cnt = 0; //frame cnt set 0
		//get current time
		UTCtime = time(0);
		tm = localtime(&UTCtime);
		
		//file_name: 20191216_101531
		strftime(DS->file_name, sizeof(DS->file_name), "%Y%m%d_%H%M%S",tm);
		strftime(dir_path, sizeof(dir_path), "%Y%m%d_%H/", tm); // 20191216_10
		strftime(check_minute, sizeof(check_minute), "%M", tm);
		sprintf(make_dir, "%s%s", dir_path_init, dir_path);
		
		if(!strcmp(check_minute, "00") || !strcmp(check_minute, "01"))

				mkdir_check = 1;
		
		if(mkdir_check) //첫 시작시엔 무조건 한번 실행
		{
			if((dir_err = mkdir(make_dir, 0777)) == -1)
				perror("err mkdir: ");
			mkdir_check = 0;
		}
		
		sprintf(DS->save_path, "%s%s%s.avi", dir_path_init, dir_path, DS->file_name);
		pthread_mutex_unlock(&mutx);

	
#ifdef DEBUG
		cout<<"\n****************************************************************"<< endl;
		cout<<"* file_name: " << DS->file_name << endl;
		cout<<"* dir_path: " << dir_path << endl;
		cout<<"* M: " << check_minute << endl;
		cout<<"* make_dir: " << make_dir << endl;
		cout<<"* Availble Disk Size: ["<< diskSize << "]KB" << endl;
		cout<<"* filename: "<< DS->file_name << endl;
		cout<<"* save_path: "<< DS->save_path << endl;
#endif

		VideoWriter outputVideo(DS->save_path, fourcc, framerate, Size(capture_width, capture_height));
				
		while(true)
		{
			if (!cap.read(img))
			{
				std::cout<<"Capture read error"<<std::endl;
				exit(1);
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
			if(cnt > 1800)
				break;
		}//while(true)
#ifdef DEBUG
		cout<<"* cap finish\n"<<endl;
#endif
		//thread
		if(pthread_attr_init(&attr) != 0)
		{
			perror("attr err: ");
//			exit(1);
		}
//		if(pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE) != 0)
		if(pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED) != 0)
		{
			perror("thread setdetach err: ");
//			exit(1);
		}
		if((thread_id = pthread_create(&p_thread, &attr, send_data, (void*)DS)) < 0)
		{
			perror("thread err: ");
//			exit(1);
		}
		if(pthread_attr_destroy(&attr) != 0)
		{
			perror("thread_destroy err: ");
//			exit(1);
		}
	}

	cap.release();
	cv::destroyAllWindows();
	free(DS);
	return 0;
}

void* send_data(void* arg)
{
	DataSet* ds = (DataSet*)arg;
#ifdef DEBUG2
	cout<<"** set DataSet"<<endl;
#endif

	int file_len = 0;
	char buf_snd[BUFSIZ];
	bzero(buf_snd, sizeof(buf_snd));
	FILE* fd;
	long file_size = 0;
	char file_name2[100];
	char save_path2[BUFSIZ];
	int clnt_sock2 = ds->clnt_sock;
	strcpy(file_name2, ds->file_name);
	strcpy(save_path2, ds->save_path);

#ifdef DEBUG2
	cout<<"** start send_data thread"<<endl;
#endif

	send(clnt_sock2, file_name2, strlen(file_name2) + 1, 0);
#ifdef DEBUG2
	cout <<"** send file_name: " << file_name2 << endl;
	cout <<"** save_path: " << save_path2 << endl;
#endif
	
	if((fd = fopen(save_path2, "rb")) == NULL)
		perror("** fopen err: ");

#ifdef DEBUG2
	cout <<"** fopen: " << fd << endl;
#endif

	while(1)
	{
		file_len = fread(buf_snd, 1, sizeof(buf_snd), fd);
		file_size += file_len;
		//send data
		send(clnt_sock2, buf_snd, file_len, 0);
		if(feof(fd))
			break;
	}
	cout<<"\n** file_size: "<< file_size << endl;
	
	shutdown(clnt_sock2, SHUT_WR);
	fflush(fd);

	fclose(fd);
	close(clnt_sock2);
#ifdef DEBUG2
	cout<<"** send complete: "<< save_path2 << endl;
	cout<<"** close fd(" << fd <<") clnt_sock(" << clnt_sock2 << ")" << endl;
#endif
	return 0;
}

//void* record_video(void* arg)
//{

//}

//void* socket_connect(void* arg)
//{
//}
