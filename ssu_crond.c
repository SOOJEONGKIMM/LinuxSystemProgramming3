#include "./ssu_crond.h"
int main(void){
	read_cronfile();
	return 0;
}
void read_cronfile(){
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
		read_timecmd(cronfilebuf);
		memset(cronfilebuf,0,BUFFER_SIZE);
	}

	fclose(fp);

} 
void read_timecmd(char *str){
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
	char tokens[TOKEN_CNT][MINLEN];

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
	make_tokens(min,tokens,MIN_ITEM);
	//calcultime(min);
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
	make_tokens(hour,tokens,HOUR_ITEM);
	//calcultime(hour);
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
	make_tokens(day,tokens,DAY_ITEM);
	//calcultime(day);
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
	make_tokens(month,tokens,MONTH_ITEM);
	//calcultime(month);
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
	make_tokens(weekday,tokens,WEEKDAY_ITEM);
	//calcultime(weekday);
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
int make_tokens(char *str, char tokens[TOKEN_CNT][MINLEN],int itemcnt){
	char *start, *end;
	char tmp[BUFFER_SIZE];
	int cntslash[BUFFER_SIZE];
	char *op="*/-,";
	char *comma=",";
	char *slash="/";
	char *bar="-";
	int row=0;
	int i;
	int commacnt=0;

	clear_tokens(tokens);
	start=str;
	end=strpbrk(start,op);

	//기호 없이 숫자만 존재 or 독립적'*'
	if(start==end){
		if(strlen(start)==1&&!strcmp(start,"*"))//독립적'*'
			printf("%s %s is '*'str:%s",start,end,str);
		else
			printf("%s  ==  %s only numbers\n",start,end);//숫자만 존재 
	}
	else{
		end=strpbrk(start,comma);//','기호로 나누기 
		if(start==end)//','기호가 없는 경우
			printf("%s %s no ',' exist.str:%s",start,end,str);
		else{
			//','기호 개수 세기 
			for(i=0;start[i];i++)
				if(start[i]==',')commacnt++;
			printf("commacnt:%d\n",commacnt);
			//start:1-5/2,6-10/3    end:,6-10/3     
			printf("start:%s  !=  %s ','exists.str:%s\n",start,end,str);//숫자만 존재 
			//','기호 토큰 나눠주기
			while(commacnt){
				while(start<end){
					printf("POOOOOINTER: *st:%d %s\n",*start,start);
					printf("PLAY start:%s  !=  %s ','exists.tok:%s\n",start,end,tokens[row]);//숫자만 존재 
					if(*start!=' ')
						strncat(tokens[row],start,1);
					start++;
				}
				//start:,6-10/3    end:,6-10/3     token:1-5/2
				printf("sta:%s  !=  %s ','exists.tok:%s\n",start,end,tokens[row]);//숫자만 존재 
				//'/'주기기호가 존재한다면 
				if(strstr(tokens[row],"/")!=NULL){
					//'/'주기 기호로 나누기 
					end=strpbrk(tokens[row],slash);
					strcpy(start,tokens[row]);
					//start:1-5/2 end:/2  tok:1-5/2
					printf("slash start:%s end:%s tok:%s\n",start,end,tokens[row]);
					strncpy(tokens[row+1],start,strlen(start)-strlen(end));
					strcpy(end,end+1);
					printf("end:%s\n",end);
					strncpy(tokens[row+2],end,strlen(end));
					//tokens[row+1]:1-5    tokens[row+2]:2
					printf("tokens[row+1]:%s\n",tokens[row+1]);
					printf("tokens[row+2]:%s\n",tokens[row+2]);
					//'-'주기기호가 존재한다면 
					if(strstr(tokens[row+1],"-")!=NULL){
						end=strpbrk(tokens[row+1],bar);
						strcpy(start,tokens[row+1]);
						printf("bar start:%s end:%s tok:%s\n",start,end,tokens[row+1]); 
						strncpy(tokens[row+3],start,strlen(start)-strlen(end));
						strcpy(end,end+1);
						strncpy(tokens[row+4],end,strlen(end));
						printf("tokens[row+3]:%s\n",tokens[row+3]);//1   1부터
						printf("tokens[row+4]:%s\n",tokens[row+4]);//5   5까지 
						count_slash_withbar(tokens[row+3],tokens[row+4],tokens[row+2],cntslash);
					}
					else
						count_slash(tokens[row+1],tokens[row+2],cntslash,itemcnt);
				}
				commacnt--;
			}//','기호개수만큼 while루프돌고 끝 
		}

	}
}
// 숫자-숫자/주기 계산
void count_slash_withbar(char *startcnt, char *endcnt, char *slash, int *savebuf){
	int startcntINT=atoi(startcnt);
	int endcntINT=atoi(endcnt);
	int slashINT=atoi(slash);
	int i=0;
	//int itemcnt=item;
	startcntINT--;
	while(endcntINT>=startcntINT){
		startcntINT+=slashINT;
		printf("st:%d\n",startcntINT);
		if(endcntINT<=startcntINT)
			break;
		savebuf[i]=startcntINT;
		printf("save[%d]:%d\n",i,savebuf[i]);
		i+=2;
	}


	printf("%s부터 %s까지 %s만큼 건너뛰어 %ls에 저장\n",startcnt,endcnt,slash,savebuf);
	printf("%d부터 %d까지 %d만큼 건너뛰어 %ls에 저장\n",startcntINT,endcntINT,slashINT,savebuf);

}
//  */주기 계산
void count_slash(char *cnt, char *slash, int *savebuf,int itemcnt){
	int Itemcnt=itemcnt;
	int startcnt;
	int slashINT=atoi(slash);

	if(itemcnt==MIN_ITEM||itemcnt==HOUR_ITEM||itemcnt==WEEKDAY_ITEM)
		startcnt=0;//0-59 0-23 0-6
	else if(itemcnt==DAY_ITEM||itemcnt==MONTH_ITEM)
		startcnt=1;//1-31 1-12 

	if(!strcmp(cnt,"*")){
		fprintf(stderr,"weird....not '*' for */주기\n");
		exit(1);
	}

	startcnt--;
	int i=0;
	while(Itemcnt){
		startcnt+=slashINT;
		printf("st:%d\n",startcnt);
		if(Itemcnt<=startcnt)
			break;
		savebuf[i]=startcnt;
		printf("save[%d]:%d\n",i,savebuf[i]);
		i+=2;
	}
	printf("%s를  %s만큼 건너뛰어 %ls에 저장\n",cnt,slash,savebuf);

}
int is_number(char c){
	return (c>='0'&&c<='9');
}
void clear_tokens(char tokens[TOKEN_CNT][MINLEN]){
	int i;
	for(i=0;i<TOKEN_CNT;i++)
		memset(tokens[i],0,sizeof(tokens[i]));
}
void calcultime(char *timestr){
	printf("time:%s\n",timestr);
	char first[TIME_SIZE];
	memset(first,0,TIME_SIZE);
	char second[TIME_SIZE];
	memset(second,0,TIME_SIZE);
	int a=0;
	int j,i;
	int len=strlen(timestr);
	// 1-5/2,6-10/3->2,4,8분마다 실행 
	for(j=0;j<len && timestr[j]!=',';j++){
		first[a]=timestr[j];
		a++;
		i++;
	}
	printf("first:%s\n",first);

	// 1-5/2 -> 2,4분마다 실행 
	char slash1[TIME_SIZE];
	memset(slash1,0,TIME_SIZE);
	char slash2[TIME_SIZE];
	memset(slash2,0,TIME_SIZE);
	int ii=0;int aa=0; int jj=0;
	for(ii=0;ii<strlen(first) && first[ii]!='/';ii++){
		slash1[aa]=first[jj];
		jj++;
		aa++;
	}
	printf("slash1:%s\n",slash1);//1-5
	ii=jj;
	while(ii<len && first[ii]=='/')
		ii++;
	aa=0;
	for(jj=ii;jj<strlen(first) && first[j]!='/';jj++){
		slash2[aa]=first[jj];
		aa++;
		ii++;
	}
	//1-5에서 1 5
	char startt[TIME_SIZE];
	char endtt[TIME_SIZE];
	memset(startt,0,TIME_SIZE);
	memset(endtt,0,TIME_SIZE);
	int ix=0;int ax=0; int jx=0;
	//1부터
	for(ix=0;ix<strlen(slash1) && slash1[ix]!='-';ix++){
		startt[ax]=slash1[jx];
		jx++;
		ax++;
	}
	printf("slash1:%s\n",slash1);//1-5
	ix=jx;
	while(ix<strlen(slash1) && slash1[ix]=='-')
		ix++;
	ax=0;
	//5까지
	for(jx=ix;jx<strlen(slash1) && slash1[jx]!='-';jx++){
		endtt[ax]=slash1[jx];
		ax++;
		ix++;
	}
	printf("start:%s\n",startt);
	printf("end:%s\n",endtt);

	int jmp=atoi(slash2);
	printf("slash2:%s\n",slash2);//2만큼씩 건너뜀
	int startT=0;
	startT=atoi(startt);
	int endT=0;
	endT=atoi(endtt);
	printf("st:%d en:%d\n",startT,endT);
	int cnt1=startT+jmp;
	int cnt2=cnt1+jmp;
	printf("cnt1:%d\n",cnt1);
	printf("cnt2:%d\n",cnt2);



	i=j;
	while(i<len && timestr[i]==',')
		i++;
	a=0;
	for(j=i;j<len && timestr[j]!=',';j++){
		second[a]=timestr[j];
		a++;
		i++;
	}
	printf("second:%s\n",second);



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


