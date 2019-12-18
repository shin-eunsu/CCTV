#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<dirent.h>

int main(int argc, char* argv[])
{
	int cli_sock;

	char buf_snd[BUFSIZ];
	char buf_rcv[BUFSIZ];

	struct sockaddr_in ser_addr;

	struct dirent** dir_list;
	int dir_cnt;

	char fileName[20];
	FILE* fd;
	int file_len;

	if(argc != 3)
	{
		printf("<IP> <Port>\n");
		exit(1);
	}
//	while(1)
//	{
		memset(&buf_rcv, '\0', BUFSIZ);
		cli_sock = socket(PF_INET, SOCK_STREAM, 0);
		if(cli_sock == -1)
		{
			printf("err sock()\n");
			exit(1);
		}
		
		memset(&ser_addr, 0, sizeof(ser_addr));
		ser_addr.sin_family = AF_INET;
		ser_addr.sin_addr.s_addr = inet_addr(argv[1]);
		ser_addr.sin_port = htons(atoi(argv[2]));

		if(connect(cli_sock, (struct sockaddr*)&ser_addr, sizeof(ser_addr)) == -1)
		{
			printf("err connect()\n");
			exit(1);
		}

		if((dir_cnt = scandir(".", &dir_list, NULL, alphasort)) == -1)
			perror("scandir err: ");

		strcpy(fileName, dir_list[2]->d_name);
		printf("fileName: %s\n", fileName);
		//send filename
		send(cli_sock, fileName, sizeof(fileName), 0);
		usleep(100);
		
		if((fd = fopen(fileName, "rb")) == NULL)
			perror("fopen err: ");

		while(1)
		{
			file_len = fread(buf_snd, 1, sizeof(buf_snd), fd);
//			printf("file_len: %d\n", file_len);
//			//send data
			send(cli_sock, buf_snd, file_len, 0);
			if(feof(fd))
				break;
		}

		close(cli_sock);
//	}
	return 0;
}
