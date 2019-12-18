#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<sys/socket.h>

int main(int argc, char* argv[])
{
	int ser_sock;
	int cli_sock;
	int cli_addr_size;

	struct sockaddr_in ser_addr;
	struct sockaddr_in cli_addr;	
	char buf_rcv[BUFSIZ];
	
	char file_name[20];
	FILE* fd;
	int file_len;
	char file_path[BUFSIZ] = "./";
	int data_len;

	if(argc != 2)
	{
		printf("<port>\n");
		exit(1);
	}

	ser_sock = socket(PF_INET, SOCK_STREAM, 0);
	if(ser_sock == -1)
	{
		printf("err sock()\n");
		exit(1);
	}
	memset(&ser_addr, 0, sizeof(ser_addr));
	ser_addr.sin_family = AF_INET;
	ser_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	ser_addr.sin_port = htons(atoi(argv[1]));

	if(bind(ser_sock, (struct sockaddr*)&ser_addr, sizeof(ser_addr)) == -1)
	{
		printf("err bind()\n");
		exit(1);
	}

	if(listen(ser_sock, 5) == -1)
	{
		printf("err listen()\n");
		exit(1);
	}

	while(1)
	{
		memset(&buf_rcv, '\0', BUFSIZ);
		cli_addr_size = sizeof(cli_addr);
		cli_sock = accept(ser_sock, (struct sockaddr*)&cli_addr, &cli_addr_size);
		if(cli_sock == -1)
		{
			printf("err accept()\n");
			exit(1);
		}
		//recv file_name
		file_len = recv(cli_sock, file_name, sizeof(file_name), 0);
		printf("fileName: %s\n", file_name);
		strcat(file_path, file_name);
		
		fd = fopen(file_path, "wb");
		fflush(NULL);
		usleep(100);

		//recv data
		while((data_len = recv(cli_sock, buf_rcv, BUFSIZ, 0)) != 0)
		{
			printf("data_len: %d\n", data_len);
			fwrite(buf_rcv, 1, data_len, fd);
		}
		printf("* recv data complete\n");

		fclose(fd);
		close(cli_sock);
	}
}
