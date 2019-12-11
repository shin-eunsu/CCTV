#include<stdio.h>
#include<sys/stat.h>
#include<sys/vfs.h>

int main(int argc, char* argv[])
{
	struct statfs sf;
	if(argc != 2)
	{
		printf("argc err + mount info\n");
		return 0;
	}
	if(statfs(argv[1], &sf) == 0)
	{
		printf("DiskSize: %ld KB\nAvailable: %ld KB\n", 
				sf.f_blocks * (sf.f_bsize/1024), sf.f_bavail * (sf.f_bsize/1024));
	}
	else
		printf("'%s' no matching deivece\n", argv[1]);

	return 0;
}
