// -rwxr-xr-x 1 eunsu eunsu 8592 12월 10 13:47 time

// - 파일유형( d 디렉토리, b 블록 디바이브, c 문자디바이스, l 링크)
// rwxr-xr-x 권한 
// // 1 링크 수
// eunsu 사용자 이름
// eunsu 그룹이름
// 8592 파일크기
// 12월 10 13:47 최종수정일
// time 파일이름

//======================================================================
//struct stat {
//    dev_t         st_dev;      /* device */
//    ino_t         st_ino;      /* inode */
//    mode_t        st_mode;     /* protection */
//    nlink_t       st_nlink;    /* number of hard links */
//    uid_t         st_uid;      /* user ID of owner */
//    gid_t         st_gid;      /* group ID of owner */
//    dev_t         st_rdev;     /* device type (if inode device) */
//    off_t         st_size;     /* total size, in bytes */
//    blksize_t     st_blksize;  /* blocksize for filesystem I/O */
//    blkcnt_t      st_blocks;   /* number of blocks allocated */
//    time_t        st_atime;    /* time of last access */
//    time_t        st_mtime;    /* time of last modification */
//    time_t        st_ctime;    /* time of last change */
//};
//======================================================================


//======================================================================
//struct statfs {
//   long    f_type;     /* 파일 시스템 타입(아래에서 보여준다) */
//   long    f_bsize;    /* 최적화된 전송 블럭 크기 */
//   long    f_blocks;   /* 파일 시스템내 총 데이터 블럭들 */
//   long    f_bfree;    /* 파일 시스템내 여유 블럭들 */
//   long    f_bavail;   /* 비-슈퍼 유저를 위한 여유 블럭들 */
//   long    f_files;    /* 파일 시스템내 총 파일 노드들 */
//   long    f_ffree;    /* 파일 시스템내 여유 파일 노드들 */
//   fsid_t  f_fsid;     /* 파일 시스템 ID */
//   long    f_namelen;  /* 파일 이름의 최대 길이 */
//   long    f_spare[6]; /* 나중을 위한 여유분 */
//};
//======================================================================

//======================================================================
//struct group {
//    char   *gr_name;       /* group name */
//    char   *gr_passwd;     /* group password */
//    gid_t   gr_gid;        /* group ID */
//    char  **gr_mem;        /* group members */
//};
//======================================================================

//======================================================================
//struct passwd
//{
//  char *pw_name;        /* Username.  */
//  char *pw_passwd;      /* Password.  */
//  __uid_t pw_uid;       /* User ID.  */
//  __gid_t pw_gid;       /* Group ID.  */
//  char *pw_gecos;       /* Real name.  */
//  char *pw_dir;         /* Home directory.  */
//  char *pw_shell;       /* Shell program.  */
//};
//======================================================================


#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<pwd.h>
#include<dirent.h>
#include<time.h>
#include<sys/time.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<sys/vfs.h>
#include<sys/types.h>

#define COLOR_RED		"\x1b[31m"
#define COLOR_GREEN		"\x1b[32m"
#define COLOR_YELLOW	"\x1b[33m"
#define COLOR_BLUE		"\x1b[34m"
#define COLOR_MAGENTA	"\x1b[35m"
#define COLOR_CYAN		"\x1b[36m"
#define COLOR_BRIGHT_RED		"\x1b[91m"
#define COLOR_BRIGHT_GREEN		"\x1b[92m"
#define COLOR_BRIGHT_YELLOW		"\x1b[93m"
#define COLOR_BRIGHT_BLUE		"\x1b[94m"
#define COLOR_BRIGHT_MAGENTA	"\x1b[95m"
#define COLOR_BRIGHT_CYAN		"\x1b[96m"
#define COLOR_RESET		"\x1b[0m"
//printf(COLOR_RED "text" COLOR_RESET "\n");

struct stat st;
struct statfs stf;
mode_t file_mode;

struct dirent* dir_entry;
DIR* dir_info;

struct passwd* user_pw;
struct passwd* group_pw;

char fileTypeUmask[11];


void printFileList();
int hideFileCheck(char* filename);
void getFileTypeUmask();
void printFileSet(char user_id, char group_id, int flag);

int main(int argc, char* argv[])
{
	dir_info = opendir(".");
	if(argc == 1) //ls
	{
		while(dir_entry = readdir(dir_info)) //dir search
		{
			if(-1 == stat(dir_entry->d_name, &st))
			{
				perror("Err: ");
				exit(0);
			}
			file_mode = st.st_mode;
			
			//(regular file | DIR | LNK) && hidefile remove
			if((dir_entry->d_type & (DT_REG | DT_DIR | DT_LNK) &&
						!hideFileCheck(dir_entry->d_name)))
			{
				printFileList("");
			}
		}
	}
	else if(argc == 2) //option
	{
		if(strcmp(argv[1], "-a") == 0)
		{
			while(dir_entry = readdir(dir_info)) //dir search
			{
				if(-1 == stat(dir_entry->d_name, &st))
				{
					perror("Err: ");
					exit(0);
				}
				file_mode = st.st_mode;

				if((dir_entry->d_type & (DT_REG | DT_DIR | DT_LNK)))
				{
					printFileList("-a");
				}
			}
		}
		else if(strcmp(argv[1], "-al") == 0)
		{
			while(dir_entry = readdir(dir_info)) //dir search
			{
				if(-1 == stat(dir_entry->d_name, &st))
				{
					perror("Err: ");
					exit(0);
				}
				file_mode = st.st_mode;

				if((dir_entry->d_type & (DT_REG | DT_DIR | DT_LNK)))
				{
					printFileList("-al");
				}//
			}
		}
	}//else if(argc == 2)
	else
	{
		perror("argc err:");
		return 1;
	}
}

void printFileList(char* opt)
{
	char user_id[50] ={0};
	char group_id[50] = {0};
	char set_time[BUFSIZ];
	struct tm* tm = localtime(&st.st_mtime);

	//id
	user_pw = getpwuid(st.st_uid);
	group_pw = getpwuid(st.st_uid);
	strcpy(user_id, user_pw->pw_name);
	strcpy(group_id, group_pw->pw_name);

	//time
	strftime(set_time, sizeof(set_time), "%m월 %d %H:%M", tm);

	getFileTypeUmask();

	if(!strcmp("-a", opt) || !strcmp("", opt))
	{
		if(S_ISLNK(file_mode)) //symbolic link
			printf(COLOR_BRIGHT_CYAN "%s@" COLOR_RESET "\n", dir_entry->d_name);
		else if(S_ISDIR(file_mode)) //dir
			printf(COLOR_BRIGHT_BLUE "%s" COLOR_RESET "\n", dir_entry->d_name);
		else if(S_ISREG(file_mode)) //nomal file
		{
			if(st.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH))
				printf(COLOR_BRIGHT_GREEN "%s\n" COLOR_RESET, dir_entry->d_name);
			else
				printf("%s\n", dir_entry->d_name);
		}
	}
	else if(!strcmp("-al", opt))
	{
		if(S_ISLNK(file_mode)) //symbolic link
			printf(COLOR_BRIGHT_CYAN "%s %ld %s %s %lld	%s	%s" COLOR_RESET "\n",
					fileTypeUmask, st.st_nlink, user_id, group_id, (long long)st.st_size,
					set_time, dir_entry->d_name);
		else if(S_ISDIR(file_mode)) //dir
			printf(COLOR_BRIGHT_BLUE "%s %ld %s %s %lld	%s	%s" COLOR_RESET "\n",
					fileTypeUmask, st.st_nlink, user_id, group_id, (long long)st.st_size,
					set_time, dir_entry->d_name);
		if(S_ISREG(file_mode)) //nomal file
		{
			if(st.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH))
				printf("%s %ld %s %s %lld	%s" COLOR_BRIGHT_GREEN "	%s" COLOR_RESET"\n",
					fileTypeUmask, st.st_nlink, user_id, group_id, (long long)st.st_size,
					set_time, dir_entry->d_name);
			else
				printf("%s %ld %s %s %lld	%s	%s\n",
					fileTypeUmask, st.st_nlink, user_id, group_id, (long long)st.st_size,
					set_time, dir_entry->d_name);
		}
	}
}

int hideFileCheck(char* filename)
{
	char tmp[1];
	strncpy(tmp, filename, 1);
	if(!strcmp(tmp, "."))
		return 1; //hide
	else
		return 0; //no
}

void getFileTypeUmask()
{
	memset(&fileTypeUmask, 0, sizeof(fileTypeUmask));

	switch(st.st_mode & S_IFMT)
	{
		case S_IFREG:	strcat(fileTypeUmask, "-");	break; 
		case S_IFDIR:	strcat(fileTypeUmask, "d");	break;
		case S_IFCHR:	strcat(fileTypeUmask, "c");	break;
		case S_IFBLK:	strcat(fileTypeUmask, "b");	break;
		case S_IFSOCK:	strcat(fileTypeUmask, "s");	break;
		case S_IFLNK:	strcat(fileTypeUmask, "l");	break;
	}

	for(int i = 1; i < 10; i++)
	{
		
	}

	if(S_IRUSR & st.st_mode)
		strcat(fileTypeUmask, "r");
	else
		strcat(fileTypeUmask, "-");

	if(S_IWUSR & st.st_mode)
		strcat(fileTypeUmask, "w");
	else
		strcat(fileTypeUmask, "-");
	
	if(S_IXUSR & st.st_mode)
		strcat(fileTypeUmask, "x");
	else
		strcat(fileTypeUmask, "-");
	
	if(S_IRGRP & st.st_mode)
		strcat(fileTypeUmask, "r");
	else
		strcat(fileTypeUmask, "-");
	
	if(S_IWGRP & st.st_mode)
		strcat(fileTypeUmask, "w");
	else
		strcat(fileTypeUmask, "-");
	
	if(S_IXGRP & st.st_mode)
		strcat(fileTypeUmask, "x");
	else
		strcat(fileTypeUmask, "-");
	
	if(S_IROTH & st.st_mode)
		strcat(fileTypeUmask, "r");
	else
		strcat(fileTypeUmask, "-");
	
	if(S_IWOTH & st.st_mode)
		strcat(fileTypeUmask, "w");
	else
		strcat(fileTypeUmask, "-");
	
	if(S_IXOTH & st.st_mode)
		strcat(fileTypeUmask, "x");
	else
		strcat(fileTypeUmask, "-");
}

void checkFilesize()
{
}

//void get fileLNK()
//{

//}



