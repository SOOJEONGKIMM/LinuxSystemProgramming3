#include "./ssu_crond.h"
int cntnum=0;
int main(void){
	if(pthread_mutex_init(&mutex,NULL)!=0)
		fprintf(stderr,"mutex init error\n");
	read_cronfile();
	startdaemon();

	return 0;
}
void read_cronfile(){
	head=NULL;
	if(pthread_mutex_init(&mutex,NULL)!=0)
		fprintf(stderr,"mutex init error\n");
	//ssu_crontab_file에서 주기가져옴  
	FILE *fp;
	char *cronfile="ssu_crontab_file";
	//변동있는 경우 업데이트되므로 초기화

	if((fp=fopen(cronfile,"r"))<0){
		fprintf(stderr,"fopen error for %s\n",cronfile);
		exit(1);
	}
	char cronfilebuf[BUFFER_SIZE];
	memset(cronfilebuf,0,BUFFER_SIZE);
	int idx=0;
	while(1){
		fgets(cronfilebuf,BUFFER_SIZE,fp);
		if(feof(fp))
			break;
		idx++;
		read_timecmd(cronfilebuf);
		memset(cronfilebuf,0,BUFFER_SIZE);
	}
	fclose(fp);
} 
//현재시간 계속 가져옴(무한반복)_주기계산의 시간과 일치확인용 
void get_localtime(char *timestr){
	time_t timer=time(NULL);
	struct tm *t=localtime(&timer);

	memset(timestr,0,TIME_SIZE);
	sprintf(timestr,"[%02d-%02d-%02d-%02d-%02d]",t->tm_min,t->tm_hour,t->tm_mday,t->tm_mon+1,t->tm_wday);
}
void get_logtime(char *str,char *checkstr){
	char timestr[TIME_SIZE];
	char timetmp[TIME_SIZE];//asctime마지막개행파싱위해 
	time_t ltime;
	struct tm *logtime;

	time(&ltime);
	logtime=localtime(&ltime);

	memset(timestr,0,TIME_SIZE);
	memset(str,0,TIME_SIZE);
	memset(timetmp,0,TIME_SIZE);
	strncpy(timetmp,asctime(logtime),strlen(asctime(logtime))-1);//마지막개행문자파싱 

	sprintf(timestr,"[%s]",timetmp);
	strcpy(str,timestr); 

	strncpy(checkstr,timestr,strlen(timestr)-9);
}
//compare localtime & cmdtime 
void pthread_cmd(){
	char timestr[TIME_SIZE];//localtime_crond명령어 비교를 위한
	char logtimestr[TIME_SIZE];//localtime_log기록을 위한 (timestr과 글자형식 다름)
	char checkstr[TIME_SIZE];//이미 실행항목인지 시간체크용(초 단위 파싱함) 
	pid_t pid;
	int i;
	char localtime[TIME_SIZE];
	get_localtime(timestr);
	memset(localtime,0,TIME_SIZE);
	strcpy(localtime,timestr);
	char checking[BUFFER_SIZE*2];

	FILE *fp;
	char *cronfile="ssu_crontab_log";
				memset(checking,0,BUFFER_SIZE);

	Node *node;
	node=head;
	while(node){
		for(i=0;i<node->timeidx;i++){
			if(!strcmp(localtime,node->timebuf[i])){
				pthread_t t_id;//thread id
				node->t_id=t_id;

				memset(checkstr,0,TIME_SIZE);
				get_logtime(logtimestr,checkstr);
				memset(checking,0,BUFFER_SIZE);
				sprintf(checking,"%s %s",checkstr,node->cmdline);

				if(check_donecmd(checking)){//완료된항목이라면 패스 
					node=node->next;
					continue;
				}
				if(pthread_create(&node->t_id,NULL,thread_handler,(void*)node)!=0)
					fprintf(stderr,"pthread_create() error\n");
				if((fp=fopen(cronfile,"a"))<0){
					fprintf(stderr,"fopen error for %s\n",cronfile);
					exit(1);
				}
				memset(logtimestr,0,TIME_SIZE);
				memset(checkstr,0,TIME_SIZE);
				get_logtime(logtimestr,checkstr);
				fprintf(fp,"%s run %s",logtimestr,node->cmdline);
				memset(checking,0,BUFFER_SIZE);
				sprintf(checking,"%s %s",checkstr,node->cmdline);
	CNode *done=(CNode*)malloc(sizeof(CNode));
	memset(done,0,sizeof(done));
	strcpy(done->checking,checking);
donelist_insert(done);
				fclose(fp);
				sleep(10);//해당 분 동안 sleep. (로그 무한출력 방지) 

				pthread_detach(node->t_id);

			}
		}
		node=node->next;
	}
	//pthread_exit();
}
int check_donecmd(char *pthreadbuf){
	CNode *done;
	done=chead;
	if(chead!=NULL){
	while(done){
		if(!strcmp(done->checking,pthreadbuf))
			return 1;
		done=done->next;
	}
	}
	return 0;
}
void* thread_handler(void *arg){
	char sys[BUFFER_SIZE];
	Node *node=(Node*)arg;
	pthread_t t_id=pthread_self();
	if(node->t_id!=t_id){
		fprintf(stderr,"error:thread id is different\n");
		node->t_id=t_id;
	}
	system(node->sysbuf);

	pthread_exit(0);

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
	cntnum=0;
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
	cntnum=0;
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
	cntnum=0;
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
	cntnum=0;
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
	cntnum=0;
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
	make_systimebuf(str,syscmd);


}
int make_tokens(char *str, char tokens[TOKEN_CNT][MINLEN],int itemcnt){
	char *start, *end;
	int cntslash[BUFFER_SIZE];
	memset(cntslash,0,BUFFER_SIZE);
	char *op="*/-,";
	char *comma=",";
	char *slash="/";
	char *bar="-";
	int row=0;
	int i;
	int commacnt=0;

	clear_tokens(tokens);
	start=end=NULL;
	start=str;
	end=strpbrk(start,op);

	//기호 없이 숫자만 존재 
	if(end==NULL&&(strstr(start,"*")==NULL)){
		cntnum=0;
		int isol_num=atoi(start);
		cntslash[0]=isol_num;
	}
	else if(!strcmp(start,"*")&&strlen(start)==1){//독립적'*'
		cntnum=itemcnt;
		if(itemcnt==MIN_ITEM||itemcnt==HOUR_ITEM||itemcnt==WEEKDAY_ITEM){
			for(i=0;i<=itemcnt;i++)
				cntslash[i]=i;
		}
		else if(itemcnt==DAY_ITEM||itemcnt==MONTH_ITEM){
			for(i=0;i<itemcnt;i++)
				cntslash[i]=i+1;
		}
	}
	else{
		//','주기기호가 존재한다면 
		if(strstr(start,",")!=NULL){
			//','기호 개수 세기 
			for(i=0;start[i];i++)
				if(start[i]==',')commacnt++;
			//','기호 토큰 나눠주기
			commacnt++;
			int commanum=commacnt;
			while(commacnt){
				if(strstr(start,comma)!=NULL){
					if(commacnt!=1)
						end=strpbrk(start,comma);
				}//','기호로 나누기 
				while(start<end){
					if(*start!=' ')
						strncat(tokens[row],start,1);
					start++;
				}
				//start:,6-10/3    end:,6-10/3     token:1-5/2
				int k=0;
				//'/'주기기호가 존재한다면 
				if(strstr(tokens[row],"/")!=NULL){
					char startbackup[BUFFER_SIZE];
					strcpy(startbackup,start);
					if(commacnt!=commanum)
						cntnum++;
					parse_calcul(tokens,startbackup,end,itemcnt,cntslash);
					strcpy(end,end+1);
					start++;
					strcpy(tokens[row],end);
				}
				else if(strstr(tokens[row],"/")==NULL&&strstr(tokens[row],"-")!=NULL){
					//숫자-숫자 
					end=strpbrk(tokens[row],bar);
					//strcpy(tokens[row],start);
					strncpy(tokens[row+3],tokens[row],strlen(tokens[row])-strlen(end));
					strcpy(end,end+1);//trim '-'
					strncpy(tokens[row+4],end,strlen(end));

					if(commacnt!=commanum)
						cntnum++;
					count_withbar(tokens[row+3],tokens[row+4],cntslash);
					strcpy(tokens[row],end);
				}
				else {
					//숫자만 있는 경우 
					if(end[0]==',')
						strcpy(end,end+1);

					char startbackup[BUFFER_SIZE];
					int endint=atoi(tokens[row]);
					if(commacnt!=commanum)
						cntnum++;
					cntslash[cntnum]=endint;
					memset(tokens[row],0,sizeof(tokens[row]));
					if(strstr(end,",")==NULL){
						strcpy(tokens[row],end);
					}
				}

				commacnt--;
				//row++;
			}//','기호개수만큼 while루프돌고 끝 
		}//','기호 있는 경우끝
		else{//','기호 없는 경우 시작 
			//'/'주기기호가 존재한다면 
			if(strstr(start,"/")!=NULL){
				//'/'주기 기호로 나누기 
				end=strpbrk(start,slash);
				strncpy(tokens[row+1],start,strlen(start)-strlen(end));
				strcpy(end,end+1);
				strncpy(tokens[row+2],end,strlen(end));
				//'-'주기기호가 존재한다면 
				if(strstr(tokens[row+1],"-")!=NULL){
					end=strpbrk(tokens[row+1],bar);
					strcpy(start,tokens[row+1]);
					strncpy(tokens[row+3],start,strlen(start)-strlen(end));
					strcpy(end,end+1);//trim'-'
					strncpy(tokens[row+4],end,strlen(end));
					count_slash_withbar(tokens[row+3],tokens[row+4],tokens[row+2],cntslash);
				}
				else
					count_slash(tokens[row+1],tokens[row+2],cntslash,itemcnt);
			}
			else if(strstr(start,"/")==NULL&&strstr(start,"-")!=NULL){
				end=strpbrk(start,bar);
				strcpy(tokens[row],start);
				strncpy(tokens[row+3],start,strlen(start)-strlen(end));
				strcpy(end,end+1);//trim '-'
				strncpy(tokens[row+4],end,strlen(end));

				count_withbar(tokens[row+3],tokens[row+4],cntslash);
			}

		}//','기호 없는 경우 끝 

	}//숫자only랑 독자적'*'제외 케이스들 끝 
	/*for(int i=0;i<=cntnum;i++)
		printf("모두완료.index%d에 %d저장\n",i,cntslash[i]);*/
	deliver_crondtime(cntslash,itemcnt,cntnum);
}
//구조체배열에 저장 
void deliver_crondtime(int *savebuf, int itemcnt,int cntnum){
	int i;


	if(itemcnt==MIN_ITEM){
		mincnt=cntnum;
		for(i=0;i<=cntnum;i++){
			min_crond[i]=savebuf[i];
		}
	}
	if(itemcnt==HOUR_ITEM){
		hourcnt=cntnum;
		for(i=0;i<=cntnum;i++){
			hour_crond[i]=savebuf[i];
		}
	}
	if(itemcnt==DAY_ITEM){
		for(i=0;i<=cntnum;i++)
			day_crond[i]=savebuf[i];
		daycnt=cntnum;
	}
	if(itemcnt==MONTH_ITEM){
		monthcnt=cntnum;
		for(i=0;i<=cntnum;i++)
			month_crond[i]=savebuf[i];
	}
	if(itemcnt==WEEKDAY_ITEM){
		weekdaycnt=cntnum;
		for(i=0;i<=cntnum;i++)
			weekday_crond[i]=savebuf[i];
	}
}
void make_systimebuf(char *cmdline,char *syscmd){
	int i=0;
	Node *node=(Node*)malloc(sizeof(Node));
	memset(node,0,sizeof(node));
	char timestr[BUFFER_SIZE];
	memset(timestr,0,BUFFER_SIZE);
	for(int a=0;a<=mincnt;a++){
		for(int b=0;b<=hourcnt;b++){
			for(int c=0;c<=daycnt;c++){
				for(int d=0;d<=monthcnt;d++){
					for(int e=0;e<=weekdaycnt;e++){

						sprintf(node->timebuf[i],"[%02d-%02d-%02d-%02d-%02d]",min_crond[a],hour_crond[b],day_crond[c],month_crond[d],weekday_crond[e]);
						i++;
					}
				}
			}
		}
	}
	node->timeidx=i;
	strcpy(node->sysbuf,syscmd);
	strcpy(node->cmdline,cmdline);
	list_insert(node);

	memset(min_crond,0,sizeof(min_crond));
	memset(hour_crond,0,sizeof(hour_crond));
	memset(day_crond,0,sizeof(day_crond));
	memset(month_crond,0,sizeof(month_crond));
	memset(weekday_crond,0,sizeof(weekday_crond));
	mincnt=0;
	hourcnt=0;
	daycnt=0;
	monthcnt=0;
	weekdaycnt=0;
}

void parse_calcul(char tokens[TOKEN_CNT][MINLEN],char *start,char *end,int itemcnt,int *savebuf){
	char *comma=",";
	char *slash="/";
	char *commaslash=",/";
	char *bar="-";
	int row=0;
	int i;
	//'/'주기 기호로 나누기 
	end=strpbrk(tokens[row],slash);
	strcpy(start,tokens[row]);
	strncpy(tokens[row+1],start,strlen(start)-strlen(end));
	strcpy(end,end+1);
	strncpy(tokens[row+2],end,strlen(end));
	//'-'주기기호가 존재한다면 
	if(strstr(tokens[row+1],"-")!=NULL){
		end=strpbrk(tokens[row+1],bar);
		strcpy(start,tokens[row+1]);
		strncpy(tokens[row+3],start,strlen(start)-strlen(end));
		strcpy(end,end+1);
		strncpy(tokens[row+4],end,strlen(end));
		count_slash_withbar(tokens[row+3],tokens[row+4],tokens[row+2],savebuf);
	}
	else
		count_slash(tokens[row+1],tokens[row+2],savebuf,itemcnt);
}
// 숫자-숫자/주기 계산
void count_slash_withbar(char *startcnt, char *endcnt, char *slash, int *savebuf){
	int startcntINT=atoi(startcnt);
	int endcntINT=atoi(endcnt);
	int slashINT=atoi(slash);
	startcntINT--;
	while(endcntINT>=startcntINT){
		startcntINT+=slashINT;
		savebuf[cntnum]=startcntINT;
		if(endcntINT<=startcntINT)
			break;
		cntnum++;
	}
	//printf("%d부터 %d까지 %d만큼 건너뛰어 \n",startcntINT,endcntINT,slashINT);
	/*for(int i=0;i<=cntnum;i++)
		printf("%d에 저장\n",savebuf[i]);*/

}
//숫자-숫자 계산 
void count_withbar(char *startcnt, char *endcnt, int *savebuf){
	int startcntINT=atoi(startcnt);
	int endcntINT=atoi(endcnt);
	//startcntINT--;
	while(endcntINT>=startcntINT){
		printf("st:%d\n",startcntINT);
		savebuf[cntnum]=startcntINT;
		if(endcntINT<=startcntINT)
			break;
		startcntINT++;
		cntnum++;
	}
	/*for(int i=0;i<=cntnum;i++)
		printf("%d에 저장\n",savebuf[i]);*/
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

	startcnt--;
	int i=0;
	while(Itemcnt){
		startcnt+=slashINT;
		if(Itemcnt<startcnt)
			break;
		savebuf[cntnum]=startcnt;
		cntnum++;
	}
	/*for(i=0;i<cntnum;i++)
		printf("%d에 저장\n",savebuf[i]);*/

}
int is_number(char c){
	return (c>='0'&&c<='9');
}
void clear_tokens(char tokens[TOKEN_CNT][MINLEN]){
	int i;
	for(i=0;i<TOKEN_CNT;i++)
		memset(tokens[i],0,sizeof(tokens[i]));
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
	/*else if(pid!=0){
		//포그라운드프로그램 실행 
	}*/
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
		sleep(30);//30초마다 add나 remove 생겼는지 다시 검사 
		read_cronfile();

	//	list_print();//for debug
		pthread_cmd();
	}
}

void list_insert(Node *newnode){
	newnode->next=NULL;
	if(head==NULL)
		head=newnode;
	else{
		Node *search;
		search=head;
		while(search->next!=NULL)
			search=search->next;
		search->next=newnode;
	}
}
//작업수행완료한 항목들 리스트 
void donelist_insert(CNode *newnode){
	newnode->next=NULL;
	if(chead==NULL)
		chead=newnode;
	else{
		CNode *search;
		search=chead;
		while(search->next!=NULL)
			search=search->next;
		search->next=newnode;
	}
}
void list_print(){
	Node *node;
	node=head;
	int i=0;
	while(node){
		for(i=0;i<node->timeidx;i++){
			printf("delivering..timebuf:%s %s\n",node->timebuf[i],node->sysbuf);
		}
		node=node->next;
	}
}
