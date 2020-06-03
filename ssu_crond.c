#include "./ssu_crond.h"
int main(void){
	readcycletime();
	return 0;
}
void readcycletime(){
	//ssu_crontab_file에서 주기가져옴  
	FILE *fp;
	char *cronfile="ssu_crontab_file";

	if((fp=fopen(cronfile,"r"))<0){
		fprintf(stderr,"fopen error for %s\n",cronfile);
		exit(1);
	}
	char cronfilebuf[BUFFER_SIZE];
	memset(cronfilebuf,0,BUFFER_SIZE);
	while(1){
		fgets(cronfilebuf,BUFFER_SIZE,fp);
		if(feof(fp))
			break;
		printf("%s",cronfilebuf);
//		memset(cronfilebuf,0,BUFFER_SIZE);
	}
	//sprintf(cronfilebuf,"%s %s %s %s %s %s",min,hour,day,month,weekday,syscmd);
	calcul_time(cronfilebuf);

	fclose(fp);

} 
void calcul_time(char *str){
	//trim "add" cmd part
	printf("str:%s\n",str);
	//실행주기 명령어 입력받는 코드
	//실행주기 입력: *'\0'*'\0'*'\0'*'\0'*'\0' 공백 다섯번째까지 
	char min[TIME_SIZE];
	char hour[TIME_SIZE];
	char day[TIME_SIZE];
	char month[TIME_SIZE];
	char weekday[TIME_SIZE];
	char syscmd[BUFFER_SIZE];
	memset(min,0,TIME_SIZE);
	memset(hour,0,TIME_SIZE);
	memset(day,0,TIME_SIZE);
	memset(month,0,TIME_SIZE);
	memset(weekday,0,TIME_SIZE);
	memset(syscmd,0,BUFFER_SIZE);
	int i=0;
	int len=strlen(str);
	//min
	int a=0;
	int j;
	for(j=0;j<len && str[j]!=' ';j++){
		min[a]=str[j];
		a++;
		i++;
	}
	printf("min:%s\n",min);
	//hour
	i=j;
	while(i<len && str[i]==' ')
		i++;
	a=0;
	for(j=i;j<len && str[j]!=' ';j++){
		hour[a]=str[j];
		a++;
		i++;
	}
	printf("hour:%s\n",hour);
	//day
	i=j;
	while(i<len && str[i]==' ')
		i++;
	a=0;
	for(j=i;j<len && str[j]!=' ';j++){
		day[a]=str[j];
		a++;
		i++;
	}
	printf("day:%s\n",day);
	//month
	i=j;
	while(i<len && str[i]==' ')
		i++;
	a=0;
	for(j=i;j<len && str[j]!=' ';j++){
		month[a]=str[j];
		a++;
		i++;
	}
	printf("month:%s\n",month);
	//weekday
	i=j;
	while(i<len && str[i]==' ')
		i++;
	a=0;
	for(j=i;j<len && str[j]!=' ';j++){
		weekday[a]=str[j];
		a++;
		i++;
	}
	printf("weekday:%s\n",weekday);
	//명령어 입력 '\n' 개행만날때까지 _명령어는 예외처리 안해도됨 
	i=j;
	while(i<len && str[i]==' ')
		i++;
	a=0;
	for(j=i;j<len && str[j]!='\n';j++){
		syscmd[a]=str[j];
		a++;
		i++;
	}
	printf("sys:%s\n",syscmd);


}
//디몬프로세스 시작
void startdaemon(){
	int fd, maxfd;
	pid_t pid;

	//fork()로 자식프로세스를 생성한다
	//이 함수는 한번호출되나 두개의 리턴값을 리턴하는 함수다
	//자식에게 리턴하는 값은 0, 부모에게는 새 자식프로세스의 ID
	if((pid=fork())<0){
		fprintf(stderr,"pid fork error\n");
		exit(1);
	}
	else if(pid!=0){
		//포그라운드프로그램 실행 
	}
	pid=getpid();
	//터미널종료시 signal의 영향을 받지 않는다
	signal(SIGHUP,SIG_IGN);
	close(0);//STDIN_FILENO
	close(1);//STDOUT_FILENO
	close(2);//STDERR_FILENO
	maxfd=getdtablesize();

	for(fd=0;fd<maxfd;fd++)
		close(fd);

	umask(0);

	//setsid로 새로운 세션만들고, 현재프로세스(자식)의 세션의 PID가 제어권을 가지도록한다
	if(setsid()==-1){
		fprintf(stderr,"set sid error\n");
		exit(0);
	}
	fd=open("/dev/null",O_RDWR);
	dup(0);
	dup(0);

	//모니터링작업 
	while(1){
		sleep(1);


	}
}


