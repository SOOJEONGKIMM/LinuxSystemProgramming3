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
#define TIME_SIZE 128
#define TOKEN_CNT 50
#define MINLEN 64
#define MAXTIME 256

#define MIN_ITEM 59
#define HOUR_ITEM 23
#define DAY_ITEM 31
#define MONTH_ITEM 12
#define WEEKDAY_ITEM 6
pthread_mutex_t mutex;

char readcrondfile[BUFFER_SIZE][BUFFER_SIZE];
char checkcrondfile[BUFFER_SIZE][BUFFER_SIZE];


typedef struct _crondtime{
	pthread_t t_id;
	int min_crond[MAXTIME];
	int hour_crond[MAXTIME];
	int day_crond[MAXTIME];
	int month_crond[MAXTIME];
	int weekday_crond[MAXTIME];
//	struct _crondtime *next;
}CT;
//Node *head;

struct timeval begin_t, end_t;

void read_cronfile();
void check_cronfile();
void compare_cronfile();
void read_timecmd(char *str);
int make_tokens(char *str, char tokens[TOKEN_CNT][MINLEN],int itemcnt);
void deliver_crondtime(int *savebuf, int itemcnt,int cntnum);
void count_slash_withbar(char *startcnt, char *endcnt, char *slash, int *savebuf);
void count_slash(char *cnt, char *slash, int *savebuf,int itemcnt);
void count_withbar(char *startcnt, char *endcnt, int *savebuf);
void parse_calcul(char tokens[TOKEN_CNT][MINLEN],char *start,char *end,int itemcnt,int *savebuf);
void clear_tokens(char tokens[TOKEN_CNT][MINLEN]);
void calcultime(char *timestr);
void startdaemon();
//void list_insert(Node *newNode);
