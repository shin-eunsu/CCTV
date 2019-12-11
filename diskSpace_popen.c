#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>

typedef struct _mountinfo
{
	FILE* fp;
	char devname[80];
	char mountdir[80];
	char blocks[100];
	char used[100];
	char avail[100];
	char usedp[100];
}MOUNTP;

int main(int argc, char* argv[])
{
	if(argc != 2)
	{
		puts("<arg err: + mount info>");
		return 1;
	}
	MOUNTP* MP = (MOUNTP*)malloc(sizeof(MOUNTP));
	FILE* read_fp;
	char buf[BUFSIZ];
	memset(buf, '\0', sizeof(buf));
	read_fp = popen("df", "r");

	while(fgets(buf, 255, read_fp))
	{
		sscanf(buf, "%s%s%s%s%s%s", MP->devname, MP->blocks, MP->used,
				MP->avail, MP->usedp, MP->mountdir);
		if(strcmp(MP->mountdir, argv[1]) == 0)
		{
			printf("'%s' Diskinfo\n", argv[1]);
			printf("DiskSize:	%s MB\nUsedSize:	%s MB\nAvailable:	%s MB\n",
					MP->blocks, MP->used, MP->avail);
			return 0;
		}
	}
	printf("%s: no matching device.\n", argv[1]);
	return 0;
}
