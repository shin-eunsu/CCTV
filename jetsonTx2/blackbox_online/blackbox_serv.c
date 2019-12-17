#define DEBUG
#define DISK_AVAILABLE_SPACE 1047576 * 10 //(10GB)
#define SAVE_DISK "/home"

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<sys/vfs.h>
#include<dirent.h>

int main(int argc, char* argv[])
{
	int serv_sock;
	int clnt_sock;
	int clnt_addr_size;

	struct sockaddr_in serv_addr;
	struct sockaddr_in clnt_addr;	
	char buf_rcv[BUFSIZ];
	
	char check_minute[3];
	char file_name[20];
	char dir_name[20];
	char remove_dir[400];
	char dir_name_init[BUFSIZ] = "/home/eunsu/blackbox/";
	char save_path[BUFSIZ];
	
	FILE* fd;
	int file_len;
	int data_len;

	int mkdir_check = 1;
	char make_dir[50];

	//disk info
	struct statfs sf;
	int diskSize;

	//scandir
	struct dirent** dir_list;
	int dir_cnt;

	if(argc != 2)
		perror("argc err: ");

	if(mkdir(dir_name_init, 0777) == -1)
		printf("%s is Exist\n", dir_name_init);

	if((serv_sock = socket(PF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("socket err: ");
		exit(1);
	}
	
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(atoi(argv[1]));

	if(bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
	{
		perror("bind err: ");
		exit(1);
	}

	if(listen(serv_sock, 5) == -1)
	{
		perror("listen err: ");
		exit(1);
	}
	while(1)
	{
		//get DiskSpace(KB)
		statfs(SAVE_DISK, &sf);
		diskSize = sf.f_bavail * (sf.f_bsize/1024);

		bzero(dir_name, sizeof(dir_name));
		bzero(make_dir, sizeof(make_dir));
		bzero(save_path, sizeof(save_path));

		if(diskSize < DISK_AVAILABLE_SPACE) //10GB
		{
			if((dir_cnt = scandir(dir_name_init, &dir_list, NULL, alphasort)) == -1)
				perror("scandir err: ");
			sprintf(remove_dir, "rm -rf %s%s", dir_name_init, dir_list[2]->d_name);
			if(system(remove_dir) == -1)
				perror("err rmdir: ");
			else
				printf("remove %s directory\n", dir_list[2]->d_name);

			for(int i = 0; i < dir_cnt; i++)
				free(dir_list[i]);
			free(dir_list);
		}

		memset(&buf_rcv, '\0', BUFSIZ);
		clnt_addr_size = sizeof(clnt_addr);
		clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_size);
		if(clnt_sock == -1)
			perror("clnt_sock err: ");

		file_len = recv(clnt_sock, file_name, sizeof(file_name), 0); //get filename
		fflush(NULL);


		strncpy(dir_name, file_name, strlen(file_name) - 8); //20191216_10
		sprintf(dir_name, "%s/", dir_name);
		strncpy(check_minute, file_name + 11, 2); //get minute
		sprintf(save_path, "%s%s%s", dir_name_init, dir_name, file_name);
		sprintf(make_dir, "%s%s", dir_name_init, dir_name);

#ifdef DEBUG
		printf("*********************************************************************\n");
		printf("* dir_name: %s\n", dir_name);
		printf("* file_name: %s\n", file_name);
		printf("* save_path: %s\n", save_path);
		printf("* make_dir: %s\n", make_dir);
		printf("*********************************************************************\n");
#endif
		if((fd = fopen(save_path, "wb")) == NULL)
			perror("fopen err: ");
		usleep(100);

		if(!strcmp(check_minute, "00") || !strcmp(check_minute, "01"))
				mkdir_check = 1;
		if(mkdir_check)
		{
			if(mkdir(make_dir, 0777) == -1)
			{
				perror("mkdir err: ");
			}
			else
				printf("make %s directory\n", make_dir);
			mkdir_check = 0;
		}

		while((data_len = recv(clnt_sock, buf_rcv, BUFSIZ, 0)) != 0)
		{
			fwrite(buf_rcv, 1, data_len, fd);
		}

		fclose(fd);
		close(clnt_sock);
	}
}
