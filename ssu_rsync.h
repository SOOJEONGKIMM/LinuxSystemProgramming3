
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>//getcwd()
#include<fcntl.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<sys/time.h>
#include<pthread.h>
#include<string.h>
#include<dirent.h>//scandir()
#include<time.h>
#include<errno.h>
#include<signal.h>
#include<sys/wait.h>

#define SECOND_TO_MICRO 1000000
#define BUFFER_SIZE 1024
#define TIME_SIZE 64
#define FILE_SIZE 128
#define PATH_SIZE 256
#define OPT_SIZE 8 

struct timeval begin_t, end_t;

//dst
typedef struct _dnode{
	char dstpath[PATH_SIZE];
	char onlydstfname[PATH_SIZE];
	int mtime;
	long fsize;
	struct _dnode *next;
}Node;
Node *head;

//src
typedef struct _snode{
	char srcpath[PATH_SIZE];
	char onlysrcfname[FILE_SIZE];
	char primfname[FILE_SIZE];//서브디렉토리 제외 파일들만 
	char logpath[PATH_SIZE];
	int mtime;
	long fsize;
	struct _snode *next;
}sNode;
sNode *shead;


int scan_dst(char *dststr, Node *srcnode);
int scan_src(char *srcstr, sNode *srcnode,int nosub,char *logpath);
void list_dstinsert(Node *newnode);
void list_srcinsert(sNode *newnode);
void list_dstprint();
void list_srcprint();
int list_samenamesearch(char *cmpfname);
int list_samefilesearch(char *cmpfname,int cmpmtime, long cmpfsize);
void parsechar(char *tmp,char *onlyfname,char *ch);
int rsync_copyF(char *src,char *onlysrcfname,char *dst,char *cmdstr);
int	rsync_copyD(char *src, char *dst,char *cmdstr);
int rsync_replaceF(char *src, char *onlysrcfname,char *dst,char *cmdstr);
int rsync_replaceD(char *src,char *onlysrcdname,char *dst,char *cmdstr);
int mopt_samefilesearch(char *cmpfname,int cmpmtime, long cmpfsize,char *cmdstr);
static void quit_rsync(int signo);
void get_time(char *str);
void write_rsynclog_timecmd(char *cmdstr);
void write_rsynclog_files(char *fname, long fsize);
