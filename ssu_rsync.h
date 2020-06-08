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
#define FILE_SIZE 256
#define PATH_SIZE 528
#define OPT_SIZE 8 

#define REPLACE 123467890
#define JUSTCHECK 987654321
#define CHECK 87654321

struct timeval begin_t, end_t;

	char tmpdst[PATH_SIZE];
	char sigdst[PATH_SIZE];

//dst
typedef struct _dnode{
	char dstpath[FILE_SIZE];
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
	char logpath[FILE_SIZE];
	char subpath[FILE_SIZE];
	char sigsrc[FILE_SIZE];//SIGINT받는경우 백업tmpdst를 sigsrc로 살려줌 
	char tmpdst[BUFFER_SIZE];//SIGINT받는경우 (기존dst를 백업한곳) 
	char origindst[BUFFER_SIZE];//SIGINT받는경우 (기존dst)
	int replace;//1 if replace
	int mtime;
	long fsize;
	struct _snode *next;
}sNode;
sNode *shead;

//대체 백업 위한 노드
typedef struct _bnode{
	char newdst[FILE_SIZE];//SIGINT받는경우 백업tmpdst를 sigsrc로 살려줌 
	char tmpdst[BUFFER_SIZE];//SIGINT받는경우 (기존dst를 백업한곳) 
	char origindst[BUFFER_SIZE];//SIGINT받는경우 (기존dst)
	int replace;//1 if replace
	struct _bnode *next;
}bNode;
bNode *bhead;


int scan_dst(char *dststr, Node *srcnode);
int scan_src(char *srcstr, sNode *srcnode,int nosub,char *logpath);
void list_dstinsert(Node *newnode);
void list_srcinsert(sNode *newnode);
void list_backupinsert(bNode *newnode);
void list_dstprint();
void list_srcprint();
int list_samenamesearch(char *cmpfname,int opt,char *newdst);
int list_samefilesearch(char *cmpfname,int cmpmtime, long cmpfsize);
void parsechar(char *tmp,char *onlyfname,char *ch);
int rsync_copyF(char *src,char *onlysrcfname,char *dst,char *cmdstr);
int	rsync_copyD(char *src, char *dst,char *cmdstr,int ropt);
int rsync_replaceF(char *src, char *onlysrcfname,char *dst,char *cmdstr);
int rsync_replaceD(char *src,char *onlysrcdname,char *dst,char *cmdstr);
int mopt_samefilesearch(char *cmpfname,int cmpmtime, long cmpfsize,char *cmdstr);
void do_topt(char *src, char *onlysrcfname,char *dst, char *cmdstr,int isdir);
static void quit_rsync(int signo);
void get_time(char *str);
void write_rsynclog_timecmd(char *cmdstr);
void write_rsynclog_files(char *fname, long fsize);
int rmdirs(const char *path, int force);
void ssu_runtime(struct timeval *begin_t, struct timeval *end_t);
