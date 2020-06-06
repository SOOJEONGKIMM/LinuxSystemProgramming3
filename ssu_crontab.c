#include "./ssu_crontab.h"

int addOpt = 0;
int removeOpt = 0;
int exitOpt = 0;
int invalidOpt = 0;

int main(void) {
	ssu_crontab_prompt();
	return 0;
}

void ssu_crontab_prompt(void) {
	char cmdbuf[BUFFER_SIZE];
	int cmd;
	//먼저 ssu_crontab_file에 저장된 모든 명령어 출력 및 개행 
	FILE *fp;
	char *cronfile="ssu_crontab_file";
	int index=0;
	char cronbuf[BUFFER_SIZE];

	if(access(cronfile,F_OK)==0){
		if((fp=fopen(cronfile,"r+w"))<0){
			fprintf(stderr,"fopen error\n");
		}

		while(1){
			fgets(cronbuf,BUFFER_SIZE,fp);
			if(feof(fp))
				break;
			printf("%d. %s",index,cronbuf);
			index++;
			memset(cronbuf,0,BUFFER_SIZE);
		}
		fclose(fp);
	}

	while (1) {
		memset((char*)cmdbuf, 0, BUFFER_SIZE);

		printf("20162969>");
		fgets(cmdbuf, BUFFER_SIZE, stdin);

		if (cmdbuf[0] == '\n') {//엔터만 입력시 프롬프트 재출력
			continue;
		}
		cmdbuf[strlen(cmdbuf) - 1] = 0;//개행문자제거 
		check_opt(cmdbuf);
		if (addOpt & !removeOpt & !exitOpt & !invalidOpt) {
			addOpt = 0;
			do_addOpt(cmdbuf);
		}
		else if (!addOpt & removeOpt & !exitOpt & !invalidOpt) {
			removeOpt = 0;
			do_removeOpt(cmdbuf);
		}
		else if (!addOpt & !removeOpt & exitOpt & !invalidOpt) {
			printf("Program is ending, Bye.\n");
			exit(0);
		}
		else if (!addOpt & !removeOpt & !exitOpt &invalidOpt) {
			invalidOpt = 0;
			continue;
		}
	}
}

//프롬프트창 커멘드 재확인 
int check_opt(const char *str) {
	char trim[31];
	char cmdbuf[BUFFER_SIZE];
	memset((char*)trim, 0, sizeof((char*)trim));
	for (int i = 0; i < strlen(str) && str[i] != ' '; i++) {
		trim[i] = str[i];
	}
	while (1) {
		if (!strcmp(trim, "add")) {
			addOpt = 1;
			break;
		}
		if (!strcmp(trim, "remove")) {
			removeOpt = 1;
			break;
		}
		if (!strcmp(trim, "exit")) {
			exitOpt = 1;
			break;
		}
		else {
			invalidOpt = 1;
			return -1;
		}
	}
	return 1;

}
//실행시간확인 
void ssu_runtime(struct timeval *begin_t, struct timeval *end_t)
{
	end_t->tv_sec -= begin_t->tv_sec;

	if (end_t->tv_usec < begin_t->tv_usec) {
		end_t->tv_sec--;
		end_t->tv_usec += SECOND_TO_MICRO;
	}

	end_t->tv_usec -= begin_t->tv_usec;
	printf("Runtime: %ld:%06ld(sec:usec)\n", end_t->tv_sec, end_t->tv_usec);
}

int do_addOpt(char *str) {
	gettimeofday(&begin_t, NULL);
	char *op=",-*/";
	char slash='/';
	char bar='-';



	//trim "add" cmd part
	int i;
	int len=strlen(str);
	for(i=0;strlen(str);i++){
		if(str[i]==' ')
			break;
	}
	while(i<len && str[i]==' ')
		i++;
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

	//min
	int a=0;
	int j;
	for(j=i;j<len && str[j]!=' ';j++){
		min[a]=str[j];
		a++;
		i++;
	}
	//	printf("min:%s\n",min);
	if(strlen(min)==0){
		printf("no run cycle input\n");
		gettimeofday(&end_t, NULL);
		ssu_runtime(&begin_t, &end_t);
		return 0;
	}
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
	//	printf("hour:%s\n",hour);
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
	//	printf("day:%s\n",day);
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
	//	printf("month:%s\n",month);
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
	//	printf("weekday:%s\n",weekday);

	//실행주기 예외처리:*-,/이외의 기호인 경우, 항목이5개아닌경우,각항목이 범위를 벗어난 경우,  주기'/'숫자 형태가 아닌 경우 

	int goodinput=0;
	if(strlen(min)==0||strlen(hour)==0||strlen(day)==0||strlen(month)==0||strlen(weekday)==0){
		printf("run cycle number is not five\n");
		gettimeofday(&end_t, NULL);
		ssu_runtime(&begin_t, &end_t);
		printf("\n");
		return 0;
	}
	//min 예외처리 
	int z=0;
	for(z=0;z<strlen(min);z++){
		if(((0x30<=min[z])&&(min[z]<=0x39))||min[z]==0x2A||min[z]==0x2D||min[z]==0x2C||min[z]==0x2F)
			goodinput=1;
		else{
			printf("run cycle input is not num,*-/\n");
			gettimeofday(&end_t, NULL);
			ssu_runtime(&begin_t, &end_t);
			printf("\n");
			return 0;
		}
	}
	//주기'/'숫자 형태가 아닌 경우 
	for(z=0;z<strlen(min);z++){
		if(min[z]==0x2F){//'/'주기기호 다음에 
			if((0x30>min[z+1])||(min[z+1]>0x39)){//0~9숫자가 아니라면 
				printf("run cycle input is not /num\n");
				gettimeofday(&end_t, NULL);
				ssu_runtime(&begin_t, &end_t);
				printf("\n");
				return 0;
			}
		}
	}
	//범위를 벗어난 경우
	int minint=atoi(min);
	if(minint<0||minint>59){
		printf("runcycle range is wrong\n");
		gettimeofday(&end_t, NULL);
		ssu_runtime(&begin_t, &end_t);
		printf("\n");
		return 0;
	}
	//주기와 같이 쓰였을때 파싱해서 범위벗어난 경우 예외처리 
	char *parser=NULL;
	char *parser1=NULL;
	char *parser2=NULL;
	char opbuf[TMP_SIZE];
	memset(opbuf,0,TMP_SIZE);
	printf("min:%s\n",min);
	int p=0;
	parser1=min;
	parser=strpbrk(min,op);
	opbuf[p]=parser[0];
	if(opbuf[p]==slash){
					printf("runcycle range is wrong(not 'range/number')\n");
					gettimeofday(&end_t, NULL);
					ssu_runtime(&begin_t, &end_t);
					printf("\n");
					return 0;
				}

	p++;
	int q=0;
	while(min!=NULL){
		sleep(1);
		parser1=parser;
		parser=strpbrk(parser1,op);
		if(parser==NULL)
			break;
		parser++;
		printf("parser1:%s parser:%s min:%s\n",parser1,parser,min);
		parser2=strpbrk(parser,op);
		printf("parser1:%s parser2:%s min:%s\n",parser1,parser2,min);
		char bone[TIME_SIZE][TIME_SIZE];
		//memset(bone,0,TIME_SIZE);
		if(parser2!=NULL){
			strncpy(bone[q],parser,strlen(parser)-strlen(parser2));
			opbuf[p]=parser2[0];
			printf("opbuf[%d]:%c\n",p,opbuf[p]);
			if(parser2[0]=='/'){
				printf("slash...\n");
				printf("before slash opbuf[%d]:%c\n",p-1,opbuf[p-1]);
				if(opbuf[p-1]!=slash&&opbuf[p-1]!=bar){
					printf("runcycle range is wrong(not 'range/number')\n");
					gettimeofday(&end_t, NULL);
					ssu_runtime(&begin_t, &end_t);
					printf("\n");
					return 0;
				}
			}
		}
		else
			strcpy(bone[q],parser);
		printf("bone[%d]:%s\n",q,bone[q]);
		int boneint=atoi(bone[q]);
		printf("boneint:%d\n",boneint);
		if(boneint<0||boneint>59){
			printf("runcycle range is wrong\n");
			gettimeofday(&end_t, NULL);
			ssu_runtime(&begin_t, &end_t);
			printf("\n");
			return 0;
		}
		if(strlen(parser)!=strlen(bone[q]))
			parser+=strlen(bone[q]);
		q++;p++;
	}
	printf("min after exceptionhandling:%s\n",min);
	//free(parser);
	//free(parser1);


	//hour 예외처리 
	for(z=0;z<strlen(hour);z++){
		if(((0x30<=hour[z])&&(hour[z]<=0x39))||hour[z]==0x2A||hour[z]==0x2D||hour[z]==0x2C||hour[z]==0x2F)
			goodinput=1;
		else{
			printf("run cycle input is wrong\n");
			gettimeofday(&end_t, NULL);
			ssu_runtime(&begin_t, &end_t);
			printf("\n");
			return 0;
		}
	}
	//주기'/'숫자 형태가 아닌 경우 
	for(z=0;z<strlen(hour);z++){
		if(hour[z]==0x2F){
			if((0x30>hour[z+1])||(hour[z+1]>0x39)){
				printf("run cycle input is wrong\n");
				gettimeofday(&end_t, NULL);
				ssu_runtime(&begin_t, &end_t);
				printf("\n");
				return 0;
			}
		}
	}
	//범위를 벗어난 경우
	int hourint=atoi(hour);
	if(hourint<0||hourint>23){
		printf("runcycle range is wrong\n");
		gettimeofday(&end_t, NULL);
		ssu_runtime(&begin_t, &end_t);
		printf("\n");
		return 0;
	}
	//주기와 같이 쓰였을때 파싱해서 범위벗어난 경우 예외처리 
	parser=NULL;
	parser1=NULL;
	parser2=NULL;
	printf("hour:%s\n",hour);
	parser=strpbrk(hour,op);
	while(hour!=NULL){
		sleep(1);
		parser1=parser;
		parser=strpbrk(parser1,op);
		if(parser==NULL)
			break;
		parser++;
		printf("parser1:%s parser:%s hour:%s\n",parser1,parser,hour);
		parser2=strpbrk(parser,op);
		printf("parser1:%s parser2:%s hour:%s\n",parser1,parser2,hour);
		char bone[TIME_SIZE];
		memset(bone,0,TIME_SIZE);
		if(parser2!=NULL)
			strncpy(bone,parser,strlen(parser)-strlen(parser2));
		else
			strcpy(bone,parser);
		printf("bone:%s\n",bone);
		int boneint=atoi(bone);
		if(boneint<0||boneint>23){
			printf("runcycle range is wrong\n");
			gettimeofday(&end_t, NULL);
			ssu_runtime(&begin_t, &end_t);
			printf("\n");
			return 0;
		}
		if(strlen(parser)!=strlen(bone))
			parser+=strlen(bone);
	}
	printf("hour after exceptionhandling:%s\n",hour);
	//free(parser);
	//free(parser1);


	//hour 예외처리 
	for(z=0;z<strlen(hour);z++){
		if(((0x30<=hour[z])&&(hour[z]<=0x39))||hour[z]==0x2A||hour[z]==0x2D||hour[z]==0x2C||hour[z]==0x2F)
			goodinput=1;
		else{
			printf("run cycle input is wrong\n");
			gettimeofday(&end_t, NULL);
			ssu_runtime(&begin_t, &end_t);
			printf("\n");
			return 0;
		}
	}
	//day 예외처리 
	for(z=0;z<strlen(day);z++){
		if(((0x30<=day[z])&&(day[z]<=0x39))||day[z]==0x2A||day[z]==0x2D||day[z]==0x2C||day[z]==0x2F)
			goodinput=1;
		else{
			printf("run cycle input is wrong\n");
			gettimeofday(&end_t, NULL);
			ssu_runtime(&begin_t, &end_t);
			printf("\n");
			return 0;
		}
	}
	//주기'/'숫자 형태가 아닌 경우 
	for(z=0;z<strlen(day);z++){
		if(day[z]==0x2F){
			if((0x30>day[z+1])||(day[z+1]>0x39)){
				printf("run cycle input is wrong\n");
				gettimeofday(&end_t, NULL);
				ssu_runtime(&begin_t, &end_t);
				printf("\n");
				return 0;
			}
		}
	}
	//범위를 벗어난 경우
	int dayint=atoi(day);
	if(dayint<0||dayint>31){
		printf("runcycle range is wrong\n");
		gettimeofday(&end_t, NULL);
		ssu_runtime(&begin_t, &end_t);
		printf("\n");
		return 0;
	}
	//주기와 같이 쓰였을때 파싱해서 범위벗어난 경우 예외처리 
	parser=NULL;
	parser1=NULL;
	parser2=NULL;
	printf("day:%s\n",day);
	parser=strpbrk(day,op);
	while(day!=NULL){
		sleep(1);
		parser1=parser;
		parser=strpbrk(parser1,op);
		if(parser==NULL)
			break;
		parser++;
		printf("parser1:%s parser:%s day:%s\n",parser1,parser,day);
		parser2=strpbrk(parser,op);
		printf("parser1:%s parser2:%s day:%s\n",parser1,parser2,day);
		char bone[TIME_SIZE];
		memset(bone,0,TIME_SIZE);
		if(parser2!=NULL)
			strncpy(bone,parser,strlen(parser)-strlen(parser2));
		else
			strcpy(bone,parser);
		printf("bone:%s\n",bone);
		int boneint=atoi(bone);
		if(boneint<0||boneint>31){
			printf("runcycle range is wrong\n");
			gettimeofday(&end_t, NULL);
			ssu_runtime(&begin_t, &end_t);
			printf("\n");
			return 0;
		}
		if(strlen(parser)!=strlen(bone))
			parser+=strlen(bone);
	}
	printf("day after exceptionhandling:%s\n",day);
	//month 예외처리 
	for(z=0;z<strlen(month);z++){
		if(((0x30<=month[z])&&(month[z]<=0x39))||month[z]==0x2A||month[z]==0x2D||month[z]==0x2C||month[z]==0x2F)
			goodinput=1;
		else{
			printf("run cycle input is wrong\n");
			gettimeofday(&end_t, NULL);
			ssu_runtime(&begin_t, &end_t);
			printf("\n");
			return 0;
		}
	}
	//주기'/'숫자 형태가 아닌 경우 
	for(z=0;z<strlen(month);z++){
		if(month[z]==0x2F){
			if((0x30>month[z+1])||(month[z+1]>0x39)){
				printf("run cycle input is wrong\n");
				gettimeofday(&end_t, NULL);
				ssu_runtime(&begin_t, &end_t);
				printf("\n");
				return 0;
			}
		}
	}
	//범위를 벗어난 경우
	int monthint=atoi(month);
	if(monthint<0||monthint>12){
		printf("runcycle range is wrong\n");
		gettimeofday(&end_t, NULL);
		ssu_runtime(&begin_t, &end_t);
		printf("\n");
		return 0;
	}
	//주기와 같이 쓰였을때 파싱해서 범위벗어난 경우 예외처리 
	parser=NULL;
	parser1=NULL;
	parser2=NULL;
	printf("month:%s\n",month);
	parser=strpbrk(month,op);
	while(month!=NULL){
		sleep(1);
		parser1=parser;
		parser=strpbrk(parser1,op);
		if(parser==NULL)
			break;
		parser++;
		printf("parser1:%s parser:%s month:%s\n",parser1,parser,month);
		parser2=strpbrk(parser,op);
		printf("parser1:%s parser2:%s month:%s\n",parser1,parser2,month);
		char bone[TIME_SIZE];
		memset(bone,0,TIME_SIZE);
		if(parser2!=NULL)
			strncpy(bone,parser,strlen(parser)-strlen(parser2));
		else
			strcpy(bone,parser);
		printf("bone:%s\n",bone);
		int boneint=atoi(bone);
		if(boneint<0||boneint>12){
			printf("runcycle range is wrong\n");
			gettimeofday(&end_t, NULL);
			ssu_runtime(&begin_t, &end_t);
			printf("\n");
			return 0;
		}
		if(strlen(parser)!=strlen(bone))
			parser+=strlen(bone);
	}
	printf("month after exceptionhandling:%s\n",month);
	//weekday 예외처리 
	for(z=0;z<strlen(weekday);z++){
		if(((0x30<=weekday[z])&&(weekday[z]<=0x39))||weekday[z]==0x2A||weekday[z]==0x2D||weekday[z]==0x2C||weekday[z]==0x2F)
			goodinput=1;
		else{
			printf("run cycle input is wrong\n");
			gettimeofday(&end_t, NULL);
			ssu_runtime(&begin_t, &end_t);
			printf("\n");
			return 0;
		}
	}
	//주기'/'숫자 형태가 아닌 경우 
	for(z=0;z<strlen(weekday);z++){
		if(weekday[z]==0x2F){
			if((0x30>weekday[z+1])||(weekday[z+1]>0x39)){
				printf("run cycle input is wrong\n");
				gettimeofday(&end_t, NULL);
				ssu_runtime(&begin_t, &end_t);
				printf("\n");
				return 0;
			}
		}
	}
	//범위를 벗어난 경우
	int weekint=atoi(weekday);
	if(weekint<0||weekint>6){
		printf("runcycle range is wrong\n");
		gettimeofday(&end_t, NULL);
		ssu_runtime(&begin_t, &end_t);
		printf("\n");
		return 0;
	}
	//주기와 같이 쓰였을때 파싱해서 범위벗어난 경우 예외처리 
	parser=NULL;
	parser1=NULL;
	parser2=NULL;
	printf("weekday:%s\n",weekday);
	parser=strpbrk(weekday,op);
	while(weekday!=NULL){
		sleep(1);
		parser1=parser;
		parser=strpbrk(parser1,op);
		if(parser==NULL)
			break;
		parser++;
		printf("parser1:%s parser:%s weekday:%s\n",parser1,parser,weekday);
		parser2=strpbrk(parser,op);
		printf("parser1:%s parser2:%s weekday:%s\n",parser1,parser2,weekday);
		char bone[TIME_SIZE];
		memset(bone,0,TIME_SIZE);
		if(parser2!=NULL)
			strncpy(bone,parser,strlen(parser)-strlen(parser2));
		else
			strcpy(bone,parser);
		printf("bone:%s\n",bone);
		int boneint=atoi(bone);
		if(boneint<0||boneint>6){
			printf("runcycle range is wrong\n");
			gettimeofday(&end_t, NULL);
			ssu_runtime(&begin_t, &end_t);
			printf("\n");
			return 0;
		}
		if(strlen(parser)!=strlen(bone))
			parser+=strlen(bone);
	}
	printf("weekday after exceptionhandling:%s\n",weekday);


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
	if(strlen(syscmd)==0){
		printf("no syscmd input\n");
		gettimeofday(&end_t, NULL);
		ssu_runtime(&begin_t, &end_t);
		printf("\n");
		return 0;
	}

	//	printf("syscmd:%s\n",syscmd);
	//ssu_crontab_file에 파일입출력 
	FILE *fp;
	char *cronfile="ssu_crontab_file";

	if((fp=fopen(cronfile,"a+"))<0){
		fprintf(stderr,"fopen error for %s\n",cronfile);
		exit(1);
	}
	char cronfilebuf[BUFFER_SIZE];
	memset(cronfilebuf,0,BUFFER_SIZE);
	sprintf(cronfilebuf,"%s %s %s %s %s %s",min,hour,day,month,weekday,syscmd);

	fprintf(fp,"%s\n",cronfilebuf);
	fclose(fp);

	//추가된 명령어까지 ssu_crontab_file 내용 표준출력 
	int index=0;
	char cronbuf[BUFFER_SIZE];

	if(access(cronfile,F_OK)==0){
		if((fp=fopen(cronfile,"r+w"))<0){
			fprintf(stderr,"fopen error\n");
		}

		while(1){
			fgets(cronbuf,BUFFER_SIZE,fp);
			if(feof(fp))
				break;
			printf("%d. %s",index,cronbuf);
			index++;
			memset(cronbuf,0,BUFFER_SIZE);
		}
		fclose(fp);
	}

	gettimeofday(&end_t, NULL);
	ssu_runtime(&begin_t, &end_t);
	printf("\n");

	return 0;
}
int do_removeOpt(char *str) {
	gettimeofday(&begin_t, NULL);
	//trim "add" cmd part
	int i;
	int len=strlen(str);
	for(i=0;strlen(str);i++){
		if(str[i]==' ')
			break;
	}
	while(i<len && str[i]==' ')
		i++;

	//COMMAND_NUMBER 입력 
	char cmdnum[TIME_SIZE];
	memset(cmdnum,0,TIME_SIZE);
	int a=0;
	int j;
	for(j=i;j<len && str[j]!=' ';j++){
		cmdnum[a]=str[j];
		a++;
		i++;
	}
	//COMMAND_NUMBER 입력하지않은 경우 예외처리 후 프롬프트로 제어가 넘어감 
	if(strlen(cmdnum)==0){
		printf("no COMMAND_NUMBER input\n");
		gettimeofday(&end_t, NULL);
		ssu_runtime(&begin_t, &end_t);
		return 0;
	}
	int cmdnumint=atoi(cmdnum);

	int index=0;
	char cronbuf[BUFFER_SIZE];
	long seek, start;
	FILE *fp;
	char *cronfile="ssu_crontab_file";
	int removed=0;

	if(access(cronfile,F_OK)==0){
		if((fp=fopen(cronfile,"rt+"))<0){
			fprintf(stderr,"fopen error\n");
		}
		//printf("before remove:");
		while(fgets(cronbuf,BUFFER_SIZE,fp)!=NULL);//printf("%s",cronbuf);
		long allbyte=ftell(fp);

		rewind(fp);
		while(1){
			seek=ftell(fp);
			if(fgets(cronbuf,BUFFER_SIZE,fp)==NULL) break;

			if(index==cmdnumint){
				start=seek;
				long len=allbyte-start;
				char *tmp=(char*)malloc(len);
				len=fread(tmp,1,len,fp);

				fseek(fp,start,SEEK_SET);
				fwrite(tmp,1,len,fp);
				fflush(fp);
				free(tmp);
				truncate(cronfile,ftell(fp));
				removed=1;
				break;
			}
			index++;
		}
		if(!removed){
			printf("COMMAND_NUMBER doesn't exist\n");
			gettimeofday(&end_t, NULL);
			ssu_runtime(&begin_t, &end_t);
			return 0;
		}
		memset(cronbuf,0,BUFFER_SIZE);
		rewind(fp);
		index=0;
		while(1){
			fgets(cronbuf,BUFFER_SIZE,fp);
			if(feof(fp))
				break;
			printf("%d. %s",index,cronbuf);
			index++;
			memset(cronbuf,0,BUFFER_SIZE);
		}
		fclose(fp);
	}

	gettimeofday(&end_t, NULL);
	ssu_runtime(&begin_t, &end_t);
	printf("\n");

	return 0;
}
