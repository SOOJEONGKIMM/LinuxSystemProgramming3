
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

typedef struct _node{
	char dstpath[PATH_SIZE];
	char onlydstfname[PATH_SIZE];
	int mtime;
	int fsize;
	struct _node *next;
}Node;
Node *head;


int scan_dst(char *dststr, Node *srcnode);
void list_insert(Node *newnode);
void list_print();
int list_samenamesearch(char *cmpfname);
int list_samefilesearch(char *cmpfname,int cmpmtime, int cmpfsize);
void parsechar(char *tmp,char *onlyfname,char *ch);
int rsync_copyF(char *src,char *onlysrcfname);
int	rsync_copyD(char *src,char *onlysrcfname);
int rsync_replaceF(char *src, char *onlysrcfname);
int rsync_replaceD(char *src, char *onlysrcfname);
