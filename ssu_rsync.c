#include "./ssu_rsync.h"

int main(int argc, char *argv[]){
	char option[OPT_SIZE];
	char src[FILE_SIZE];
	char dst[FILE_SIZE];
	memset(option,0,OPT_SIZE);
	memset(src,0,FILE_SIZE);
	memset(dst,0,FILE_SIZE);
	if(argc<3){
		fprintf(stderr,"Usage: %s [option] <src> <dst>\n",argv[0]);
		exit(1);
	}
	//[OPTION]유무 판별
	if(!strcmp(argv[1],"-r")||!strcmp(argv[1],"-t")||!strcmp(argv[1],"-m")){
		if(argc!=4){
			fprintf(stderr,"Usage: %s [option] <src> <dst>\n",argv[0]);
			exit(1);
		}
		strcpy(option,argv[1]);
		strcpy(src,argv[2]);
		strcpy(dst,argv[3]);
	}
	else{
		if(argc!=3){
			fprintf(stderr,"Usage: %s [option] <src> <dst>\n",argv[0]);
			exit(1);
		}
		strcpy(src,argv[1]);
		strcpy(dst,argv[2]);
	}
	//src 접근권한 확인 
	struct stat srcbuf;
	lstat(src,&srcbuf);
	if(access(src,F_OK)!=0){
		printf("no access..\n");
		fprintf(stderr,"Usage: %s [option] <src> <dst>\n",argv[0]);
		exit(1);
	}
	//src 파일/디렉토리 판별 
	int file_src=0;
	int dir_src=0;
	if(S_ISREG(srcbuf.st_mode))
		file_src=1;
	else if(S_ISDIR(srcbuf.st_mode))
		dir_src=1;
	else{
		fprintf(stderr,"Usage: %s [option] <src> <dst>\n",argv[0]);
		exit(1);
	}
	printf("file:%d/dir:%d\n",file_src,dir_src);
	if(lstat(src,&srcbuf)<0){
		fprintf(stderr,"lstat error for %s\n",src);
		exit(1);
	}
	char onlysrcfname[PATH_SIZE];
	memset(onlysrcfname,0,PATH_SIZE);
	char *tmp=strrchr(src,'/');
	parsechar(tmp,onlysrcfname,"/");

	//src mtime, size 
	int srcmtime=0;
	long srcfsize=0;
	srcmtime=srcbuf.st_mtime;
	srcfsize=srcbuf.st_size;
	printf("src mtime:%d size:%ld\n",srcmtime,srcfsize);

	//dst 디렉토리 판별
	struct stat dstbuf;
	lstat(dst,&dstbuf);
	if(!S_ISDIR(dstbuf.st_mode)){
		printf("not dir..\n");
		fprintf(stderr,"Usage: %s [option] <src> <dst>\n",argv[0]);
		exit(1);
	}
	//dst 접근권한 확인 
	if(access(dst,F_OK)!=0){
		printf("no access..\n");
		fprintf(stderr,"Usage: %s [option] <src> <dst>\n",argv[0]);
		exit(1);
	}
	char dstscan[PATH_SIZE];//스캔하면서 dst배열포인터가 변형되므로 임시변수생성 
	memset(dstscan,0,PATH_SIZE);
	strcpy(dstscan,dst);
	//dst 디렉토리 스캔 
	Node *newnode;
	newnode=(Node*)malloc(sizeof(Node));
	scan_dst(dstscan, newnode);

	int samefile=0;
	int samename=0;
	int nofile=0;
	if(!list_samefilesearch(onlysrcfname,srcmtime, srcfsize)&&!list_samenamesearch(onlysrcfname))
		nofile=1;
	if(list_samefilesearch(onlysrcfname,srcmtime, srcfsize)){
		printf("same file. rsync job canceled.\n");
		samefile=1;
		exit(0);
	}
	if(!list_samefilesearch(onlysrcfname,srcmtime, srcfsize)&&list_samenamesearch(onlysrcfname))
		samename=1;
	printf("no:%d same file:%d, name:%d",nofile,samefile,samename);
	if(samefile){
		printf("Same file. Rsync job cancelled..\n");
		exit(0);
	}

	//src디렉토리 스캔 
	sNode *srcnode;
	srcnode=(sNode*)malloc(sizeof(sNode));
	char srcscan[PATH_SIZE];//스캔하면서 src배열포인터가 변형되므로 임시변수생성 
	memset(srcscan,0,PATH_SIZE);
	strcpy(srcscan,src);
	//[OPTION]없는 경우 
	if(strlen(option)==0){
		if(nofile){
			if(file_src)
				rsync_copyF(src,onlysrcfname,dst);
			else if(dir_src){//r옵션 없는 src디렉토리 경우, 서브디렉토리 제외 파일들만 복사 
				scan_src(srcscan, srcnode,1);
				rsync_copyD(src,dst);
			}
		}
		else if(samename){
			if(file_src)
				rsync_replaceF(src, onlysrcfname,dst);
			else if(dir_src){
				scan_src(srcscan, srcnode,1);
				rsync_replaceD(src, onlysrcfname,dst);
			}
		}
	}

	//-r option
	if(!strcmp(option,"-r")){
		printf("-r opt\n");
	}
	//-t option
	if(!strcmp(option,"-t")){
		printf("-t opt\n");
	}
	//-m option
	if(!strcmp(option,"-m")){
		printf("-m opt\n");
	}


	printf("option:%s src:%s dst:%s\n",option,src,dst);
	//list_print();
	return 0;
}
//src파일이 dst디렉토리 내 동일파일 존재하지않으면
//src파일을 dst 디렉토니 내에 복사함 
int rsync_copyF(char *src,char *onlysrcfname,char *dst){
	char newdst[PATH_SIZE];
	FILE *srcfpr;
	FILE *srcfpw;
	signal(SIGINT, (void*)quit_rsync);

	//src파일 복사(후에 srcfpr은 dst디렉토리로 옮겨짐)
	if((srcfpr=fopen(src,"r"))<0){
		fprintf(stderr,"fopen error for src_read:%s\n",src);
		exit(1);
	}
	struct stat srcstat;
	lstat(src,&srcstat);
	char tmpname[PATH_SIZE];
	memset(tmpname,0,PATH_SIZE);
	sprintf(tmpname,"%stmp",src);
	if((srcfpw=fopen(tmpname,"w"))<0){
		fprintf(stderr,"fopen error for src_write:%s\n",src);
		exit(1);
	}

	char buf[BUFFER_SIZE];
	memset(buf,0,BUFFER_SIZE);
	while(!feof(srcfpr)){
		fgets(buf,sizeof(buf),srcfpr);
		fputs(buf,srcfpw);
		fseek(srcfpw,0,SEEK_END);
		sprintf(buf,"\n");
	}
	printf("only:%s\n",onlysrcfname);
	fclose(srcfpr);
	fclose(srcfpw);

	//src파일을 dst디렉토리 내로 옮김 
	memset(newdst,0,PATH_SIZE);
	printf("dst:%s\n",dst);
	sprintf(newdst,"%s/%s",dst,onlysrcfname);
	if(rename(src,newdst)<0){
		fprintf(stderr,"rename error for %s to %s\n",src,newdst);
		exit(1);
	}
	//tmp를 src이름으로 변경 
	if(rename(tmpname,src)<0){
		fprintf(stderr,"rename error for %s to %s\n",tmpname,src);
		exit(1);
	}
	//복사src를 기존src접근권한으로 변경 
	if(chmod(src,srcstat.st_mode)<0){
		fprintf(stderr,"chmod error for newsrc:%s\n",src);
		exit(1);
	}

	printf("rsyn_copy file\n");
	return 0;
}
//서브디렉토리를 제외한 디렉토리 내 파일들을 dst디렉토리로 복사함 (no ropt)
int	rsync_copyD(char *src,char *dst){
	char newdst[PATH_SIZE];
	FILE *srcfpr;
	FILE *srcfpw;
	signal(SIGINT, (void*)quit_rsync);
	//src디렉토리 바로 아래 파일 복사(후에 srcfpr은 dst디렉토리로 옮겨짐)
	list_srcprint();
	sNode *srcd;
	srcd=shead;
	while(srcd->next!=NULL){
		printf("src:%s\n",srcd->onlysrcfname);
		if(strlen(srcd->primfname)==0){
			srcd=srcd->next;
			continue;
		}
		if((srcfpr=fopen(srcd->srcpath,"r"))<0){
			fprintf(stderr,"fopen error for src_read:%s\n",src);
			exit(1);
		}
		printf("srcfile:%s\n",srcd->onlysrcfname);
		struct stat srcstat;
		lstat(srcd->srcpath,&srcstat);
		char tmpname[PATH_SIZE];
		memset(tmpname,0,PATH_SIZE);
		sprintf(tmpname,"%stmp",srcd->onlysrcfname);
		if((srcfpw=fopen(tmpname,"w"))<0){
			fprintf(stderr,"fopen error for src_write:%s\n",src);
			exit(1);
		}
		char buf[BUFFER_SIZE];
		memset(buf,0,BUFFER_SIZE);
		while(!feof(srcfpr)){
			fgets(buf,sizeof(buf),srcfpr);
			fputs(buf,srcfpw);
			fseek(srcfpw,0,SEEK_END);
			sprintf(buf,"\n");
		}
		printf("copyD start\n");
		fclose(srcfpr);
		fclose(srcfpw);
		printf("here.........\n");

		//src파일을 dst디렉토리 내로 옮김 
		memset(newdst,0,PATH_SIZE);
		printf("prim:%s\n",srcd->primfname);
		sprintf(newdst,"%s/%s",dst,srcd->onlysrcfname);
		if(rename(srcd->srcpath,newdst)<0){
			fprintf(stderr,"rename error for %s to %s\n",srcd->srcpath,newdst);
			exit(1);
		}
		//tmp를 src이름으로 변경 
		if(rename(tmpname,srcd->srcpath)<0){
			fprintf(stderr,"rename error for %s to %s\n",tmpname,src);
			exit(1);
		}
		//복사src를 기존src접근권한으로 변경 
		if(chmod(srcd->srcpath,srcstat.st_mode)<0){
			fprintf(stderr,"chmod error for newsrc:%s\n",src);
			exit(1);
		}
		srcd=srcd->next;
	}


	list_dstprint();
	//	while(1){
	//sleep(3);
	//		printf("testing sigint..\n");
	//	}
	printf("rsync_copyD\n");
	return 0;
}
//dst디렉토리 내 파일이름이 같은 다른 파일인 경우 
//dst디렉토리 내 파일을 src파일로 대체함 
int rsync_replaceF(char *src, char *onlysrcfname,char *dst){
	char newdst[PATH_SIZE];
	signal(SIGINT, (void*)quit_rsync);
	memset(newdst,0,PATH_SIZE);
	printf("dst:%s\n",dst);
	sprintf(newdst,"%s/%s",dst,onlysrcfname);
	if(rename(src,newdst)<0){
		fprintf(stderr,"rename error for %s to %s\n",src,newdst);
		exit(1);
	}
	if(remove(dst)!=0){
		fprintf(stderr,"remove error for dst:%s\n",dst);
		exit(1);
	}

	printf("rsync_replace file\n");
	return 0;
}
//dst디렉토리 이름과 src디렉토리가 이름이 같은 경우 
//서브디렉토리를 제외한 디렉토리 내 파일들을 dst디렉토리로 대체(이동)함 (no ropt)
int rsync_replaceD(char *src, char *onlysrcdname,char *dst){
	signal(SIGINT, (void*)quit_rsync);
	char newdst[PATH_SIZE];
	memset(newdst,0,PATH_SIZE);
	printf("dst:%s\n",dst);
	sprintf(newdst,"%s/%s",dst,onlysrcdname);
	if(rename(src,newdst)<0){
		fprintf(stderr,"rename error for %s to %s\n",src,newdst);
		exit(1);
	}//***서브디렉토리까지 이동될듯..
	if(remove(dst)!=0){
		fprintf(stderr,"remove error for dst:%s\n",dst);
		exit(1);
	}

	printf("rsync_replace dir\n");
	return 0;
}
//동기화 작업 중 SIGINT발생시 동기화작업취소되고 그전상태로 유지되어야함 
static void quit_rsync(int signo){
	/*if(rename(src,newdst)<0){
	  fprintf(stderr,"rename error for %s to %s\n",src,newdst);
	  exit(1);
	  }*/

	printf("got SIGINT(%d).. quiting rsync job..\n",SIGINT);
	exit(0);
}

int scan_dst(char *dststr, Node *srcnode){
	int i,countdirp;
	struct dirent **flist;
	struct stat statbuf;
	char dstpath[PATH_SIZE];

	char *ptr;
	ptr=dststr+strlen(dststr);
	*ptr++='/';
	*ptr='\0';

	if((countdirp=scandir(dststr, &flist, 0, alphasort))<0){
		fprintf(stderr,"scandir error for %s\n",dststr);
		return -1;
	}

	i=0;
	while(i<countdirp){
		if(!strcmp(flist[i]->d_name,".")||!strcmp(flist[i]->d_name,"..")){
			i++;
			continue;
		}
		strcpy(ptr,flist[i]->d_name);
		printf("d_name:%s\n",flist[i]->d_name);
		Node *node=(Node *)malloc(sizeof(Node));
		memset(node,0,sizeof(Node));

		if(lstat(dststr,&statbuf)<0){
			fprintf(stderr,"lstat() error for %s\n",flist[i]->d_name);
			return -1;
		}

		strcpy(node->onlydstfname,flist[i]->d_name);
		node->mtime=statbuf.st_mtime;
		node->fsize=statbuf.st_size;
		printf("scan size:%ld\n",statbuf.st_size);

		memset(dstpath,0,PATH_SIZE);
		strcpy(node->dstpath,dststr);//flist[i]->d_name);

		list_dstinsert(node);

		if(S_ISDIR(statbuf.st_mode)){
			scan_dst(dststr,node);//재귀 
		}

		i++;
	}
	return 1;
}
int scan_src(char *srcstr, sNode *srcnode,int nosub){

	if(nosub)//서브디렉토리 제외하고 리스트생성 다시하는 경우 
		shead=NULL;

	int i,countdirp;
	struct dirent **flist;
	struct stat statbuf;
	char srcpath[PATH_SIZE];

	char *ptr;
	ptr=srcstr+strlen(srcstr);
	*ptr++='/';
	*ptr='\0';

	if((countdirp=scandir(srcstr, &flist, 0, alphasort))<0){
		fprintf(stderr,"scandir error for %s\n",srcstr);
		return -1;
	}
	printf("scanning src...\n");
	i=0;
	while(i<countdirp){
		if(!strcmp(flist[i]->d_name,".")||!strcmp(flist[i]->d_name,"..")){
			i++;
			continue;
		}
		strcpy(ptr,flist[i]->d_name);
		printf("d_name:%s\n",flist[i]->d_name);
		sNode *node=(sNode *)malloc(sizeof(sNode));
		memset(node,0,sizeof(sNode));

		if(lstat(srcstr,&statbuf)<0){
			fprintf(stderr,"lstat() error for %s\n",flist[i]->d_name);
			return -1;
		}

		strcpy(node->onlysrcfname,flist[i]->d_name);
		node->mtime=statbuf.st_mtime;
		node->fsize=statbuf.st_size;
		printf("fname:%s mtime:%d size:%ld\n",node->onlysrcfname,node->mtime,node->fsize);


		if(S_ISREG(statbuf.st_mode)){
			//if(nosub)//r옵션 없는 src디렉토리 경우, 서브디렉토리 제외 파일들만 작업해야함 
				strcpy(node->primfname,flist[i]->d_name);

		}
		printf("prim:%s\n",node->primfname);

		memset(srcpath,0,PATH_SIZE);
		strcpy(node->srcpath,srcstr);//flist[i]->d_name);

		list_srcinsert(node);

		if(S_ISDIR(statbuf.st_mode)){
			if(!nosub)
				scan_src(srcstr,node,0);//재귀 
		}

		i++;
	}
	printf("src scan end\n");
	list_srcprint();
	return 1;
}
void list_dstinsert(Node *newnode){
	newnode->next=NULL;
	if(head==NULL)
		head=newnode;
	else{
		Node *dstnode;
		dstnode=head;
		while(dstnode->next!=NULL)
			dstnode=dstnode->next;
		dstnode->next=newnode;
	}
}
void list_srcinsert(sNode *newnode){
	newnode->next=NULL;
	if(shead==NULL)
		shead=newnode;
	else{
		sNode *srcnode;
		srcnode=shead;
		while(srcnode->next!=NULL)
			srcnode=srcnode->next;
		srcnode->next=newnode;
	}
}
//dst디렉토리 내 동일한 파일인지/ 이름만 동일한지 판별 
int list_samenamesearch(char *cmpfname){
	Node *search;
	search=head;
	while(search){
		//printf("dst:%s src:%s\n",search->onlydstfname,cmpfname);
		if(!strcmp(search->onlydstfname,cmpfname))
			return 1;//존재하는 파일/디렉토리 
		search=search->next;
	}
	return 0;//존재하지않는 파일/디렉토리 
}
int list_samefilesearch(char *cmpfname,int cmpmtime, long cmpfsize){
	Node *search;
	search=head;
	while(search){
		printf("src:name:%s mtime:%d size:%ld\n",cmpfname,cmpmtime,cmpfsize);
		printf("dst:name:%s mtime:%d size:%ld\n",search->onlydstfname,search->mtime,search->fsize);
		if(!strcmp(search->onlydstfname,cmpfname)&&(search->mtime==cmpmtime)&&(search->fsize==cmpfsize))
			return 1;//존재하는 파일/디렉토리 
		search=search->next;
	}
	return 0;//존재하지않는 파일/디렉토리 
}

//dst디버깅용 
void list_dstprint(){
	Node *cur;
	cur=head;
	int i=0;
	printf("<<dst list>>\n");
	while(cur->next!=NULL){
		if(cur->dstpath!=NULL)
			printf("%s\n",cur->dstpath);
		cur=cur->next;
	}
	printf("%s\n",cur->dstpath);
}
//src디버깅용 
void list_srcprint(){
	sNode *cur;
	cur=shead;
	int i=0;
	printf("<<src list>>\n");
	while(cur->next!=NULL){
		if(cur->srcpath!=NULL)
			printf("%s\n",cur->srcpath);
		cur=cur->next;
	}
	printf("%s\n",cur->srcpath);
}
void parsechar(char *tmp,char *onlyfname,char *ch){
	while(*tmp){
		if(*tmp==*ch){
			tmp++;
			ch++;
			continue;
		}
		*onlyfname++=*tmp++;
	}
}

