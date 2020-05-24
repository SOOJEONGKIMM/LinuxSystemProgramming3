#include "./ssu_crontab.h"

int addOpt = 0;
int removeOpt = 0;
int exitOpt = 0;
int invalidOpt = 0;

void ssu_crontab_prompt(void) {
	char cmdbuf[BUFFER_SIZE];
	int cmd;
	//���� ssu_crontab_file�� ����� ��� ��ɾ� ��� �� ���� 
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

		if (cmdbuf[0] == '\n') {//���͸� �Է½� ������Ʈ �����
			continue;
		}
		cmdbuf[strlen(cmdbuf) - 1] = 0;//���๮������ 
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

//������Ʈâ Ŀ��� ��Ȯ�� 
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
//����ð�Ȯ�� 
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



	//trim "add" cmd part
	int i;
	int len=strlen(str);
	for(i=0;strlen(str);i++){
		if(str[i]==' ')
			break;
	}
	while(i<len && str[i]==' ')
		i++;
	//�����ֱ� ��ɾ� �Է¹޴� �ڵ�
	//�����ֱ� �Է�: *'\0'*'\0'*'\0'*'\0'*'\0' ���� �ټ���°���� 
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
	printf("min:%s\n",min);
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

	//�����ֱ� ����ó��:*-,/�̿��� ��ȣ�� ���, �׸���5���ƴѰ��,���׸��� ������ ��� ���,  �ֱ�'/'���� ���°� �ƴ� ��� 
	//��ɾ� �Է� '\n' ���ุ�������� _��ɾ�� ����ó�� ���ص��� 
	i=j;
	while(i<len && str[i]==' ')
		i++;
	a=0;
	for(j=i;j<len && str[j]!='\n';j++){
		syscmd[a]=str[j];
		a++;
		i++;
	}
	printf("syscmd:%s\n",syscmd);
	//ssu_crontab_file�� ��������� 
	FILE *fp;
	char *cronfile="ssu_crontab_file";

	if((fp=fopen(cronfile,"a+"))<0){
		fprintf(stderr,"fopen error for %s\n",cronfile);
		exit(1);
	}
	fprintf(fp,"%s\n",str);
	fclose(fp);

	//�߰��� ��ɾ���� ssu_crontab_file ���� ǥ����� 
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
	printf("\n");


	gettimeofday(&end_t, NULL);
	ssu_runtime(&begin_t, &end_t);

	return 0;
}
int do_removeOpt(char *str) {
	gettimeofday(&begin_t, NULL);

	printf("remove opt\n");

	gettimeofday(&end_t, NULL);
	ssu_runtime(&begin_t, &end_t);

	return 0;
}
