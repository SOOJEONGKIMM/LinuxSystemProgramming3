#include "./ssu_crond.h"
int update=0;
int cntnum=0;

int main(void){
	if(pthread_mutex_init(&mutex,NULL)!=0)
		fprintf(stderr,"mutex init error\n");
	read_cronfile();

	while(1){
		sleep(2);
		/*check_cronfile();

		//ssu_crontab()에서 add나 remove된경우 
		if(update==1){
		read_cronfile();
		update=0;
		}*/
		list_print();
		pthread_cmd();
	}
	return 0;
}
void read_cronfile(){
	head=NULL;
	//ssu_crontab_file에서 주기가져옴  
	FILE *fp;
	char *cronfile="ssu_crontab_file";
	//변동있는 경우 업데이트되므로 초기화
	memset(readcrondfile,0,sizeof(readcrondfile));

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
		printf("readfile: %s\n",cronfilebuf);
		strcpy(readcrondfile[idx],cronfilebuf);
		idx++;
		printf("for comparefile:%s\n",readcrondfile[idx]);
		read_timecmd(cronfilebuf);
		memset(cronfilebuf,0,BUFFER_SIZE);
	}
	fclose(fp);
} 
//remove나 add로 cronfile 변동이 있는지 체크(무한반복)
void check_cronfile(){
	//ssu_crontab_file에서 주기가져옴  
	FILE *fp;
	char *cronfile="ssu_crontab_file";
	//한번 읽을때마다 초기화
	memset(checkcrondfile,0,sizeof(checkcrondfile));

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
		printf("readfile: %s\n",cronfilebuf);
		strcpy(checkcrondfile[idx],cronfilebuf);
		idx++;
		printf("for comparefile:%s\n",checkcrondfile[idx]);
		read_timecmd(cronfilebuf);
		memset(cronfilebuf,0,BUFFER_SIZE);
	}
	fclose(fp);
}
//read_cronfile과 check_cronfile 비교(변동이 있는지 판별) (무한반복)
void compare_cronfile(){
	int origincnt=0, checkcnt=0;
	while(checkcrondfile[checkcnt]!=NULL||readcrondfile[origincnt]!=NULL)
		checkcnt++;origincnt++;
	//remove된경우 
	if(checkcnt<origincnt){
		update=1;
		printf("removed..\n");
	}

	//add된경우 
	if(origincnt<checkcnt){
		update=1;
		printf("added...\n");
	}


}
//현재시간 계속 가져옴(무한반복)_주기계산의 시간과 일치확인용 
void get_localtime(char *timestr){
	time_t timer=time(NULL);
	struct tm *t=localtime(&timer);

	memset(timestr,0,TIME_SIZE);
	sprintf(timestr,"%02d-%02d-%02d-%02d-%02d\n",t->tm_min,t->tm_hour,t->tm_mday,t->tm_mon+1,t->tm_wday);
	printf("localtime: %s",timestr);
}
//compare localtime & cmdtime 
void pthread_cmd(){
	char timestr[TIME_SIZE];//localtime
	pid_t pid;
	int i;
	char localtime[TIME_SIZE];
	get_localtime(timestr);
	memset(localtime,0,TIME_SIZE);
	strcpy(localtime,timestr);

	Node *node;
	node=head;
	while(node){
		for(i=0;i<node->timeidx;i++){
			printf("l:%s",localtime);
			printf("c:%s",node->timebuf[i]);
			if(!strcmp(localtime,node->timebuf[i])){
				pthread_t t_id;//thread id
				printf("TIME IS NOW!!!!!!!!\n");
				node->t_id=t_id;
				if(pthread_create(&node->t_id,NULL,thread_handler,(void*)node)!=0)
					fprintf(stderr,"pthread_create() error\n");
				pthread_detach(node->t_id);

			}
		}
		node=node->next;
	}
	//pthread_exit();
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

}

void debug(){
	/*	for(i=0;i<=MIN_ITEM;i++)
		printf("min deliver 작업중....index%d에 %d저장\n",i,min_crond[i]);
		for(i=0;i<=HOUR_ITEM;i++)
		printf("hour deliver 작업중....index%d에 %d저장\n",i,hour_crond[i]);
		for(i=0;i<=DAY_ITEM;i++)
		printf("daydeliver 작업중....index%d에 %d저장\n",i,day_crond[i]);
		for(i=0;i<=MONTH_ITEM;i++)
		printf("month deliver 작업중....index%d에 %d저장\n",i,month_crond[i]);
		for(i=0;i<=WEEKDAY_ITEM;i++)
		printf("weekday deliver 작업중....index%d에 %d저장\n",i,weekday_crond[i]);
	 */
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
	printf("min:%s\n",min);
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
	printf("hour:%s\n",hour);
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
	printf("day:%s\n",day);
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
	printf("month:%s\n",month);
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
	printf("weekday:%s\n",weekday);
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
	printf("sys:%s\n",syscmd);
	make_systimebuf(syscmd);


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
	printf("처음 연산자 토큰%s %s 길이:%ld\n",start,end,strlen(start));

	//기호 없이 숫자만 존재 
	if(end==NULL&&(strstr(start,"*")==NULL)){
		printf("%s  ==  %s only numbers\n",start,end);//숫자만 존재 
		cntnum=0;
		int isol_num=atoi(start);
		cntslash[0]=isol_num;
	}
	else if(!strcmp(start,"*")&&strlen(start)==1){//독립적'*'
		printf("%s %s is '*'str:%s",start,end,str);
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
		/*	end=strpbrk(start,comma);//','기호로 나누기 
			if(start==end)//','기호가 없는 경우
			printf("%s %s no ',' exist.str:%s",start,end,str);*/
		//','주기기호가 존재한다면 
		if(strstr(start,",")!=NULL){
			//','기호 개수 세기 
			for(i=0;start[i];i++)
				if(start[i]==',')commacnt++;
			//printf("commacnt:%d\n",commacnt);
			//start:1-5/2,6-10/3    end:,6-10/3     
			//printf("start:%s  !=  %s ','exists.str:%s\n",start,end,str);//숫자만 존재 
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
				//printf("sta:%s  !=  %s ','exists.tok:%s\n",start,end,tokens[row]);//숫자만 존재 
				int k=0;
				//strcpy(tokens[row],end);
				/*if(tokens[row][0]==','){
				  for(k=0;tokens[row][k];k++){
				  tokens[row][k]=tokens[row][k+1];
				  }
				  strcpy(end,tokens[row]);
				  strcpy(start,tokens[row]);
				  if(commacnt!=1)
				  end=strpbrk(start,comma);//','기호로 나누기 
				  }*/
				printf("'/'주기기호 파싱전에 tokens[%d]:%s\n",row,tokens[row]);
				if(strstr(tokens[row],",")!=NULL){
					printf("파싱후 tok업데이트 tok:%s ,end:%s\n",tokens[row],end);
					char tokbackup[TOKEN_CNT];
					int toklen=strlen(tokens[row]);
					strcpy(tokbackup,tokens[row]);
					memset(tokens[row],0,TOKEN_CNT);
					strncpy(tokens[row],tokbackup,toklen-strlen(end));
				}//,작업 마지막 아이 

				printf("'/'주기기호 strstr전에 tokens[%d]:%s\n",row,tokens[row]);
				//'/'주기기호가 존재한다면 
				if(strstr(tokens[row],"/")!=NULL){
					printf("','기호와 '/'기호 모두 존재\n");
					char startbackup[BUFFER_SIZE];
					strcpy(startbackup,start);
					printf("'/'기호존재before parse calcul tok:%s start:%s end:%s cntslash:%d\n",tokens[row],start,end,cntslash[row]);
					//strcpy(tokens[row],end);
					if(commacnt!=commanum)
						cntnum++;
					parse_calcul(tokens,startbackup,end,itemcnt,cntslash);
					printf("cntnum:%d\n",cntnum);
					strcpy(end,end+1);
					printf("'/'기호존재after parse calcul tok:%s startbackup:%s end:%s cntslash:%d\n",tokens[row],startbackup,end,cntslash[row]);
					start++;
					strcpy(tokens[row],end);
				}
				else if(strstr(tokens[row],"/")==NULL&&strstr(tokens[row],"-")!=NULL){
					//숫자-숫자 
					printf("계산전 bar start:%s end:%s tok:%s\n",start,end,tokens[row]); 
					printf("숫자-숫자 계산중..\n");
					end=strpbrk(tokens[row],bar);
					//strcpy(tokens[row],start);
					printf("bar start:%s end:%s tok:%s\n",start,end,tokens[row]); 
					strncpy(tokens[row+3],tokens[row],strlen(tokens[row])-strlen(end));
					strcpy(end,end+1);//trim '-'
					strncpy(tokens[row+4],end,strlen(end));
					printf("tokens[row+3]:%s\n",tokens[row+3]);//1   1부터
					printf("tokens[row+4]:%s\n",tokens[row+4]);//5   5까지 

					if(commacnt!=commanum)
						cntnum++;
					count_withbar(tokens[row+3],tokens[row+4],cntslash);
					strcpy(tokens[row],end);
				}
				else {
					//숫자만 있는 경우 
					if(end[0]==',')
						strcpy(end,end+1);

					printf("','기호만 존재하고 '/'주기기호는 없음\n");
					char startbackup[BUFFER_SIZE];
					printf("숫자만before parse calcul tok:%s start:%s end:%s cntslash:%d\n",tokens[row],start,end,cntslash[row]);
					int endint=atoi(tokens[row]);
					if(commacnt!=commanum)
						cntnum++;
					cntslash[cntnum]=endint;
					printf("cntnum:%d\n",cntnum);
					printf("숫자만after parse calcul tok:%s start:%s end:%s cntslash:%d\n",tokens[row],start,end,cntslash[row]);
					for(int i=0;i<=cntnum;i++)
						printf("%d에 저장\n",cntslash[i]);
					//start++;end++;
					printf("숫자만작업완료after parse calcul tok:%s start:%s end:%s cntslash:%d\n",tokens[row],start,end,cntslash[row]);
					memset(tokens[row],0,sizeof(tokens[row]));
				if(strstr(end,",")==NULL){
					strcpy(tokens[row],end);
				}
					//strcpy(tokens[row],end);
					printf("숫자만작업완료after parse calcul tok:%s start:%s end:%s cntslash:%d\n",tokens[row],start,end,cntslash[row]);
				}

				commacnt--;
				//row++;
			}//','기호개수만큼 while루프돌고 끝 
		}//','기호 있는 경우끝
		else{//','기호 없는 경우 시작 
			//'/'주기기호가 존재한다면 
			if(strstr(start,"/")!=NULL){
				printf("','기호는 없지만 '/'기호 존재 hello");
				//'/'주기 기호로 나누기 
				//strcpy(tokens[row],start);
				end=strpbrk(start,slash);
				//strcpy(start,tokens[row]);
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
					strcpy(end,end+1);//trim'-'
					strncpy(tokens[row+4],end,strlen(end));
					printf("tokens[row+3]:%s\n",tokens[row+3]);//1   1부터
					printf("tokens[row+4]:%s\n",tokens[row+4]);//5   5까지 
					count_slash_withbar(tokens[row+3],tokens[row+4],tokens[row+2],cntslash);
				}
				else
					count_slash(tokens[row+1],tokens[row+2],cntslash,itemcnt);
			}
			else if(strstr(start,"/")==NULL&&strstr(start,"-")!=NULL){
				printf("계산전 bar start:%s end:%s tok:%s\n",start,end,tokens[row]); 
				printf("숫자-숫자 계산중..\n");
				end=strpbrk(start,bar);
				strcpy(tokens[row],start);
				printf("bar start:%s end:%s tok:%s\n",start,end,tokens[row]); 
				strncpy(tokens[row+3],start,strlen(start)-strlen(end));
				strcpy(end,end+1);//trim '-'
				strncpy(tokens[row+4],end,strlen(end));
				printf("tokens[row+3]:%s\n",tokens[row+3]);//1   1부터
				printf("tokens[row+4]:%s\n",tokens[row+4]);//5   5까지 

				count_withbar(tokens[row+3],tokens[row+4],cntslash);
			}

		}//','기호 없는 경우 끝 

	}//숫자only랑 독자적'*'제외 케이스들 끝 
	printf("====================================================\n");
	for(int i=0;i<=cntnum;i++)
		printf("모두완료.index%d에 %d저장\n",i,cntslash[i]);
	deliver_crondtime(cntslash,itemcnt,cntnum);
}
//구조체배열에 저장 
void deliver_crondtime(int *savebuf, int itemcnt,int cntnum){
	int i;

	printf("delivering..cntnum:%d\n",cntnum);

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
			printf("debug hour:%d\n",hour_crond[i]);
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

	//list_insert(node);
	//printf("delivering..buf:%ls %ls %ls %ls %ls\n",CT.min_crond,CT.hour_crond,CT.day_crond,CT.month_crond,CT.weekday_crond);




}
void make_systimebuf(char *syscmd){
	int i=0;
	Node *node=(Node*)malloc(sizeof(Node));
	memset(node,0,sizeof(node));
	printf("arrived here\n");
	char timestr[BUFFER_SIZE];
	memset(timestr,0,BUFFER_SIZE);
	for(int a=0;a<=mincnt;a++){
		for(int b=0;b<=hourcnt;b++){
			for(int c=0;c<=daycnt;c++){
				for(int d=0;d<=monthcnt;d++){
					for(int e=0;e<=weekdaycnt;e++){

						sprintf(node->timebuf[i],"%02d-%02d-%02d-%02d-%02d\n",min_crond[a],hour_crond[b],day_crond[c],month_crond[d],weekday_crond[e]);
						printf("made timestr:%s\n",node->timebuf[i]);
						i++;
					}
				}
			}
		}
	}
	printf("timeidx:%d\n",i);
	node->timeidx=i;
	strcpy(node->sysbuf,syscmd);
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
	printf("parse_calcul called\n");
	char *comma=",";
	char *slash="/";
	char *commaslash=",/";
	char *bar="-";
	int row=0;
	int i;
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
		count_slash_withbar(tokens[row+3],tokens[row+4],tokens[row+2],savebuf);
		printf("cntnum:%d\n",cntnum);
	}
	else
		count_slash(tokens[row+1],tokens[row+2],savebuf,itemcnt);
	printf("cntnum:%d\n",cntnum);
}
// 숫자-숫자/주기 계산
void count_slash_withbar(char *startcnt, char *endcnt, char *slash, int *savebuf){
	int startcntINT=atoi(startcnt);
	int endcntINT=atoi(endcnt);
	int slashINT=atoi(slash);
	startcntINT--;
	printf("숫자-숫자/주기 계산중..\n");
	while(endcntINT>=startcntINT){
		startcntINT+=slashINT;
		printf("st:%d\n",startcntINT);
		savebuf[cntnum]=startcntINT;
		printf("save[%d]:%d\n",cntnum,savebuf[cntnum]);
		if(endcntINT<=startcntINT)
			break;
		cntnum++;
	}
	printf("%s부터 %s까지 %s만큼 건너뛰어 \n",startcnt,endcnt,slash);
	//printf("%d부터 %d까지 %d만큼 건너뛰어 \n",startcntINT,endcntINT,slashINT);
	for(int i=0;i<=cntnum;i++)
		printf("%d에 저장\n",savebuf[i]);

}
//숫자-숫자 계산 
void count_withbar(char *startcnt, char *endcnt, int *savebuf){
	int startcntINT=atoi(startcnt);
	int endcntINT=atoi(endcnt);
	//startcntINT--;
	printf("숫자-숫자 계산중..\n");
	while(endcntINT>=startcntINT){
		printf("st:%d\n",startcntINT);
		savebuf[cntnum]=startcntINT;
		if(endcntINT<=startcntINT)
			break;
		startcntINT++;
		cntnum++;
	}
	printf("%s부터 %s까지 \n",startcnt,endcnt);
	for(int i=0;i<=cntnum;i++)
		printf("%d에 저장\n",savebuf[i]);
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

	if(strcmp(cnt,"*")){
		fprintf(stderr,"weird....not '*' for */주기\n");
		exit(1);
	}

	startcnt--;
	int i=0;
	while(Itemcnt){
		startcnt+=slashINT;
		printf("st:%d\n",startcnt);
		if(Itemcnt<startcnt)
			break;
		savebuf[cntnum]=startcnt;
		printf("save[%d]:%d\n",cntnum,savebuf[cntnum]);
		cntnum++;
	}
	printf("%s를  %s만큼 건너뛰어 \n",cnt,slash);
	for(i=0;i<cntnum;i++)
		printf("%d에 저장\n",savebuf[i]);

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
