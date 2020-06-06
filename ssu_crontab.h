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
#define TIME_SIZE 256
#define TMP_SIZE 528

struct timeval begin_t, end_t;


void ssu_crontab_prompt(void);
int check_opt(const char *str);
void ssu_runtime(struct timeval *begin_t, struct timeval *end_t);
int do_addOpt(char *str);
int do_removeOpt(char *str);
