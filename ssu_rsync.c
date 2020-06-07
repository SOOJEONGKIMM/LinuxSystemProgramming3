#include "./ssu_rsync.h"
int main(int argc, char *argv[]){
	gettimeofday(&begin_t, NULL);
	char option[OPT_SIZE];
	char src[FILE_SIZE];
	char dst[FILE_SIZE];
	char cmdstr[PATH_SIZE];
	memset(option,0,OPT_SIZE);
	memset(src,0,FILE_SIZE);
	memset(dst,0,FILE_SIZE);
	memset(cmdstr,0,PATH_SIZE);
	if(argc<3){
		fprintf(stderr,"Usage: %s [option] <src> <dst>\n",argv[0]);
		exit(1);
	}
	//[OPTION]유무 판별
	if(!strcmp(argv[1],"-r")||!strcmp(argv[1],"-t")||!strcmp(argv[1],"-m")){
		if(argc!=4){
			fprintf(stderr,"Usage: %s [option] <src> <dst>\n",argv[0]);
			gettimeofday(&end_t, NULL);
			ssu_runtime(&begin_t, &end_t);
			exit(1);
		}
		strcpy(option,argv[1]);
		strcpy(src,argv[2]);
		strcpy(dst,argv[3]);
		sprintf(cmdstr,"ssu_rsync %s %s %s",argv[1],argv[2],argv[3]);
		printf("1cmd:%s\n",cmdstr);
	}
	else{
		if(argc!=3){
			fprintf(stderr,"Usage: %s [option] <src> <dst>\n",argv[0]);
			exit(1);
		}
		strcpy(src,argv[1]);
		strcpy(dst,argv[2]);
		sprintf(cmdstr,"ssu_rsync %s %s",argv[1],argv[2]);
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
	if(!list_samefilesearch(onlysrcfname,srcmtime, srcfsize)&&!list_samenamesearch(onlysrcfname,JUSTCHECK,NULL))
		nofile=1;
	if(list_samefilesearch(onlysrcfname,srcmtime, srcfsize)){
		printf("same file. rsync job canceled.\n");
		samefile=1;
		exit(0);
	}
	if(!list_samefilesearch(onlysrcfname,srcmtime, srcfsize)&&list_samenamesearch(onlysrcfname,JUSTCHECK,NULL)){
		samename=1;
		printf("Same name file. Rsync job cancelled..\n");
		exit(0);
	}
	if(samefile){
		printf("Same file. Rsync job cancelled..\n");
		exit(0);
	}
	printf("no:%d same file:%d, name:%d",nofile,samefile,samename);

	//src디렉토리 스캔 
	sNode *srcnode;
	srcnode=(sNode*)malloc(sizeof(sNode));
	char srcscan[PATH_SIZE];//스캔하면서 src배열포인터가 변형되므로 임시변수생성 
	memset(srcscan,0,PATH_SIZE);
	strcpy(srcscan,src);

	if(nofile){
		if(file_src&&strcmp(option,"-t"))
			rsync_copyF(src,onlysrcfname,dst,cmdstr);
		else if(dir_src){//r옵션 없는 src디렉토리 경우, 서브디렉토리 제외 파일들만 복사 
			//-r option
			if(!strcmp(option,"-r")){
				printf("HERE\n");
				scan_src(srcscan, srcnode,0,onlysrcfname);
				rsync_copyD(src,dst,cmdstr,1);
			}
			else if(strcmp(option,"-t")){//-r -t옵션 제외 
				scan_src(srcscan, srcnode,1,onlysrcfname);
				rsync_copyD(src,dst,cmdstr,0);
			}

		}
	}
	/*	else if(samename){
		if(file_src&&strcmp(option,"-t"))
		rsync_replaceF(src, onlysrcfname,dst,cmdstr);
		else if(dir_src){
	//-r option
	if(!strcmp(option,"-r")){
	scan_src(srcscan, srcnode,0,onlysrcfname);
	}
	else if(strcmp(option,"-t"))//-r -t 옵션 제외 
	scan_src(srcscan, srcnode,1,onlysrcfname);
	rsync_replaceD(src, onlysrcfname,dst,cmdstr);
	}
	}*/

	char scansrcfname[FILE_SIZE];
	memset(scansrcfname,0,FILE_SIZE);
	strcpy(scansrcfname,onlysrcfname);
	//-t option
	if(!strcmp(option,"-t")){
		if(dir_src){
			scan_src(srcscan, srcnode,0,scansrcfname);
			do_topt(src,onlysrcfname,dst,cmdstr,1);
		}
		else
			do_topt(src,onlysrcfname,dst,cmdstr,0);

		printf("srcscan:%s, only:%s\n",srcscan,onlysrcfname);
	}
	//-m option
	if(!strcmp(option,"-m")){
		scan_src(srcscan, srcnode,1,onlysrcfname);
		newnode=head;
		while(newnode){
			if(mopt_samefilesearch(newnode->onlydstfname,newnode->mtime, newnode->fsize,cmdstr)){
				if(remove(newnode->dstpath)!=0){
					fprintf(stderr,"remove error for dst:%s\n",newnode->dstpath);
					newnode=newnode->next;
					continue;//빈 디렉토리 아닌 경우도 있으므로 넘김 
				}
				else{
					write_rsynclog_files(newnode->onlydstfname,-1);
				}
			}
			newnode=newnode->next;
		}
	}


	printf("option:%s src:%s dst:%s\n",option,src,dst);
	//list_print();
	return 0;
}
void do_topt(char *src, char *onlysrcfname,char *dst, char *cmdstr,int isdir){
	char tarcmd[PATH_SIZE];
	char untarcmd[PATH_SIZE];
	char curdir[PATH_SIZE];
	char tarname[FILE_SIZE];
	char tartocurdir[PATH_SIZE];

	signal(SIGINT, (void*)quit_rsync);

	memset(tarcmd,0,PATH_SIZE);
	memset(untarcmd,0,PATH_SIZE);
	memset(curdir,0,PATH_SIZE);

	strcpy(curdir,getcwd(NULL,0));

	//파일만 동기화할때 상대경로압축위해 chdir할 경로 
	char filesrcpath[PATH_SIZE];
	memset(filesrcpath,0,PATH_SIZE);
	strncpy(filesrcpath,src,strlen(src)-strlen(onlysrcfname)-1);

	if(isdir){
		chdir(src);
		sprintf(tarcmd,"tar cvf %s.tar *",onlysrcfname);
	}
	else{
		chdir(filesrcpath);
		sprintf(tarcmd,"tar cvf %s.tar %s",onlysrcfname,onlysrcfname);
	}
	system(tarcmd);

	sprintf(tarname,"%s.tar",onlysrcfname);
	sprintf(tartocurdir,"%s/%s.tar",curdir,onlysrcfname);
	rename(tarname,tartocurdir);

	printf("tarcmd:%s\n",tarcmd);

	chdir(curdir);

	struct stat tarstat;
	lstat(tarname,&tarstat);

	sprintf(untarcmd,"tar -xvf %s.tar -C %s",onlysrcfname,dst);
	system(untarcmd);
	printf("untarcmd:%s\n",untarcmd);

	remove(tarname);

	write_rsynclog_timecmd(cmdstr);
	FILE *fp;
	char *rsyncfile="ssu_rsync_log";
	if((fp=fopen(rsyncfile,"a"))<0){
		fprintf(stderr,"fopen %s error",rsyncfile);
		exit(1);
	}
	fseek(fp,0,SEEK_END);
	fprintf(fp,"totalSize %ldbytes\n",tarstat.st_size);
	if(isdir){
		sNode *srcd;
		srcd=shead;
		while(srcd){
			fprintf(fp,"%s\n",srcd->subpath);
			srcd=srcd->next;
		}
	}
	else
		fprintf(fp,"%s\n",onlysrcfname);

	fclose(fp);

}
int mopt_samefilesearch(char *cmpfname,int cmpmtime, long cmpfsize,char *cmdstr){
	int exist=0;
	static int count=0;
	sNode *search;
	search=shead;
	while(search){
		printf("mopt...count:%d\n",count);
		printf("dst:name:%s mtime:%d size:%ld\n",cmpfname,cmpmtime,cmpfsize);
		printf("src:name:%s mtime:%d size:%ld\n",search->onlysrcfname,search->mtime,search->fsize);
		//존재하는 파일/디렉토리 
		if(!strcmp(search->onlysrcfname,cmpfname)){
			exist=1;
			printf("%s exists\n",search->srcpath);
			break;
		}
		search=search->next;
	}
	if(!exist){
		count++;
		/*if(count==1)
		  write_rsynclog_timecmd(cmdstr);*/
		return 1;
	}
	return 0;
}
//src파일이 dst디렉토리 내 동일파일 존재하지않으면
//src파일을 dst 디렉토니 내에 복사함 
int rsync_copyF(char *src,char *onlysrcfname,char *dst,char *cmdstr){
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
	memset(sigdst,0,PATH_SIZE);
	strcpy(sigdst,newdst);
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
	write_rsynclog_timecmd(cmdstr);
	write_rsynclog_files(onlysrcfname,srcstat.st_size);
	/*	while(1){
		printf("test sigint..\n");
		sleep(5);
		}
	 */

	printf("rsyn_copy file\n");
	return 0;
}
//서브디렉토리를 제외한 디렉토리 내 파일들을 dst디렉토리로 복사함 (no ropt)
int	rsync_copyD(char *src, char *dst,char *cmdstr,int ropt){
	char newdst[PATH_SIZE];
	FILE *srcfpr;
	FILE *srcfpw;
	signal(SIGINT, (void*)quit_rsync);
	memset(sigdst,0,PATH_SIZE);
	strcpy(sigdst,dst);
	//src디렉토리 바로 아래 파일 복사(후에 srcfpr은 dst디렉토리로 옮겨짐)
	list_srcprint();
	sNode *srcd;
	srcd=shead;
	if(shead->next!=NULL)//빈디렉토리 
		write_rsynclog_timecmd(cmdstr);
	while(srcd){

		printf("copying prim:%s fname:%s\n",srcd->primfname,srcd->srcpath);
		printf("copying fname:%s\n",srcd->srcpath);
		if((srcfpr=fopen(srcd->srcpath,"r"))<0){
			fprintf(stderr,"fopen error for src_read:%s\n",src);
			exit(1);
		}
		struct stat srcstat;
		lstat(srcd->srcpath,&srcstat);

		//src 파일 복사(src안에서) 
		char tmpname[PATH_SIZE];
		memset(tmpname,0,PATH_SIZE);
		if(S_ISREG(srcstat.st_mode)){
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
			fclose(srcfpr);
			fclose(srcfpw);
		}

		memset(newdst,0,PATH_SIZE);
		//sprintf(newdst,"%s/%s",dst,srcd->onlysrcfname);
		sprintf(newdst,"%s/%s",dst,srcd->subpath);
		printf("sub:%s\n",srcd->subpath);
		//sprintf(newdst,"%s/%s",dst,srcd->logpath);
		if(S_ISDIR(srcstat.st_mode)){
			if(ropt){
				mkdir(newdst,srcstat.st_mode);
			}
		}
		else if(S_ISREG(srcstat.st_mode)){
			if(!list_samefilesearch(srcd->onlysrcfname,srcd->mtime,srcd->fsize)){
				//src파일을 dst디렉토리 내로 옮김 
				if(rename(srcd->srcpath,newdst)<0){
					fprintf(stderr,"rename error for %s to %s\n",srcd->srcpath,newdst);
					exit(1);
				}
				memset(sigdst,0,PATH_SIZE);
				strcpy(sigdst,newdst);
				//tmp를 src이름으로 변경 (src안에서)
				if(rename(tmpname,srcd->srcpath)<0){
					fprintf(stderr,"rename error for %s to %s\n",tmpname,src);
					exit(1);
				}
				//복사src를 기존src접근권한으로 변경 
				if(chmod(srcd->srcpath,srcstat.st_mode)<0){
					fprintf(stderr,"chmod error for newsrc:%s\n",src);
					exit(1);
				}
				//sigint받으면 sigsrc는 모두 삭제함 
				strcpy(srcd->sigsrc,newdst);
				printf("sigsrc:%s\n",srcd->sigsrc);
				write_rsynclog_files(srcd->subpath,srcd->fsize);
			}//이름만 같은 경우 대체 
			else if(!list_samefilesearch(srcd->onlysrcfname,srcd->mtime, srcd->fsize)&&list_samenamesearch(srcd->onlysrcfname,JUSTCHECK,NULL)){
				//SIGINT 받는 경우를 위해 dst파일 백업 
				list_samenamesearch(srcd->onlysrcfname,REPLACE,newdst);

				//src파일을 dst디렉토리 내로 옮김 
				if(rename(srcd->srcpath,newdst)<0){
					fprintf(stderr,"rename error for %s to %s\n",srcd->srcpath,newdst);
					exit(1);
				}
				memset(sigdst,0,PATH_SIZE);
				strcpy(sigdst,newdst);
				//tmp를 src이름으로 변경 (src안에서)
				if(rename(tmpname,srcd->srcpath)<0){
					fprintf(stderr,"rename error for %s to %s\n",tmpname,src);
					exit(1);
				}
				//복사src를 기존src접근권한으로 변경 
				if(chmod(srcd->srcpath,srcstat.st_mode)<0){
					fprintf(stderr,"chmod error for newsrc:%s\n",src);
					exit(1);
				}
				//sigint받으면 sigsrc는 모두 삭제함 
				write_rsynclog_files(srcd->subpath,srcd->fsize);
			}
		}
		srcd=srcd->next;
	}
	while(1){
		sleep(3);
		printf("testing sigint..\n");
	}
	printf("rsync_copyD\n");
	return 0;
}
//dst디렉토리 내 파일이름이 같은 다른 파일인 경우 
//dst디렉토리 내 파일을 src파일로 대체함 
int rsync_replaceF(char *src, char *onlysrcfname,char *dst,char *cmdstr){
	char newdst[PATH_SIZE];
	signal(SIGINT, (void*)quit_rsync);
	memset(newdst,0,PATH_SIZE);

	memset(tmpdst,0,PATH_SIZE);
	sprintf(tmpdst,"%s_tmp",dst);
	printf("dst:%s\n",dst);
	sprintf(newdst,"%s/%s",dst,onlysrcfname);
	struct stat srcstat;
	lstat(src,&srcstat);
	if(rename(src,newdst)<0){
		fprintf(stderr,"rename error for %s to %s\n",src,newdst);
		exit(1);
	}
	//SIGINT받는 경우 원상복구 위해 dst백업파일생성 
	memset(sigdst,0,PATH_SIZE);
	strcpy(sigdst,dst);
	if(rename(dst,tmpdst)<0){
		fprintf(stderr,"rename error for dst:%s backup\n",dst);
		exit(1);
	}
	write_rsynclog_timecmd(cmdstr);
	write_rsynclog_files(onlysrcfname,srcstat.st_size);

	printf("rsync_replace file\n");
	return 0;
}
//dst디렉토리 이름과 src디렉토리가 이름이 같은 경우 
//서브디렉토리를 제외한 디렉토리 내 파일들을 dst디렉토리로 대체(이동)함 (no ropt)
int rsync_replaceD(char *src, char *onlysrcdname, char *dst,char *cmdstr){
	signal(SIGINT, (void*)quit_rsync);
	memset(sigdst,0,PATH_SIZE);
	strcpy(sigdst,dst);
	char newdst[PATH_SIZE];
	sNode *srcd;
	srcd=shead;
	write_rsynclog_timecmd(cmdstr);
	while(srcd){
		//서브디렉토리는 동기화에서 제외 
		if(strlen(srcd->primfname)==0){
			srcd=srcd->next;
			continue;
		}
		//src파일이 dst파일과 동일한 파일(이름,크기,수정시간)인 경우 해당파일은 동기화하지않음 
		if(!list_samefilesearch(srcd->onlysrcfname,srcd->mtime,srcd->fsize)){
			//src파일을 dst디렉토리 내로 옮김 
			memset(newdst,0,PATH_SIZE);
			//sprintf(newdst,"%s/%s",dst,srcd->onlysrcfname);
			sprintf(newdst,"%s/%s",dst,srcd->subpath);
			printf("src:%s newdst:%s\n",srcd->srcpath,newdst);
			if(rename(srcd->srcpath,newdst)<0){
				fprintf(stderr,"rename error for %s to %s\n",srcd->srcpath,newdst);
				exit(1);
			}
			write_rsynclog_files(srcd->subpath,srcd->fsize);

		}
		srcd=srcd->next;
	}
	//SIGINT 받는 경우를 위해 dst파일 백업 
	memset(tmpdst,0,PATH_SIZE);
	sprintf(tmpdst,"%s_tmp",dst);
	if(rename(dst,tmpdst)<0){
		fprintf(stderr,"rename error for %s to %s\n",dst,tmpdst);
		exit(1);
	}

	printf("rsync_replace dir\n");
	list_srcprint();
	return 0;
}

//동기화 작업 중 SIGINT발생시 동기화작업취소되고 그전상태로 유지되어야함 
static void quit_rsync(int signo){
	printf("tmp:%s sig:%s\n",tmpdst,sigdst);

	//동기화된애들 삭제 
	sNode *sig;
	sig=shead;
	while(sig){
		//if(access(sig->tmpdst,F_OK)!=0){ 
			if(remove(sig->sigsrc)==0)
				printf("remove %s succeed\n",sig->sigsrc);
			else
				printf("remove %s failed\n",sig->sigsrc);
		//}
		sig=sig->next;
	}

	//빈 디렉토리 다시 삭제처리(위에선 빈디렉토리가 아니라 삭제실패됨)
	sig=shead;
	while(sig){
		//if(access(sig->tmpdst,F_OK)!=0){ 
			if(rmdirs(sig->sigsrc,1)==0)
				printf("rmdirs remove %s succeed\n",sig->sigsrc);
			else
				printf("rmdirs remove %s failed\n",sig->sigsrc);
		//}
		sig=sig->next;
	}
	//copy file한 경우
	if(remove(sigdst)==0)
		printf("remove dst:%s succeed\n",sigdst);
	else
		printf("remove dst:%s failed\n",sigdst);

	//대체한 경우 백업해둔 dst 되살리기 
	if(access(sig->tmpdst,F_OK)==0){//replace한 경우 
		if(rename(sig->tmpdst,sig->sigsrc)<0){
			fprintf(stderr,"rename error for %s to %s\n",sigdst,tmpdst);
			exit(1);
		}
	}

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
int scan_src(char *srcstr, sNode *srcnode,int nosub,char *logpath){

	//	if(nosub)//서브디렉토리 제외하고 리스트생성 다시하는 경우 
	//		shead=NULL;

	int i,countdirp;
	struct dirent **flist;
	struct stat statbuf;
	char srcpath[PATH_SIZE];

	char *ptr;
	ptr=srcstr+strlen(srcstr);
	*ptr++='/';
	*ptr='\0';

	char *logptr;
	logptr=logpath+strlen(logpath);
	*logptr++='/';
	*logptr='\0';

	char *subptr;

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
		strcpy(logptr,flist[i]->d_name);

		printf("srcstr:%s\n",srcstr);
		printf("logpath:%s\n",logpath);
		printf("d_name:%s\n",flist[i]->d_name);
		sNode *node=(sNode *)malloc(sizeof(sNode));
		memset(node,0,sizeof(sNode));

		char subpath[PATH_SIZE];
		memset(subpath,0,PATH_SIZE);
		char *tmp=strchr(logpath,'/');
		parsechar(tmp,subpath,"/");
		strcpy(node->subpath,subpath);
		printf("subpath:%s\n",node->subpath);

		if(lstat(srcstr,&statbuf)<0){
			fprintf(stderr,"lstat() error for %s\n",flist[i]->d_name);
			return -1;
		}

		strcpy(node->onlysrcfname,flist[i]->d_name);
		node->mtime=statbuf.st_mtime;
		node->fsize=statbuf.st_size;
		strcpy(node->logpath,logpath);
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
				scan_src(srcstr,node,0,logpath);//재귀 
		}

		i++;
	}
	printf("src scan end\n");
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
int list_samenamesearch(char *cmpfname,int opt,char *newdst){
	Node *search;//dst
	search=head;
	sNode *srcd;
	srcd=shead;
	while(search){
		//printf("dst:%s src:%s\n",search->onlydstfname,cmpfname);
		if(!strcmp(search->onlydstfname,cmpfname)){
			//SIGINT 받는 경우 위해 dst 백업 
			if(opt==REPLACE){
				memset(tmpdst,0,PATH_SIZE);
				sprintf(tmpdst,"%s_tmp",search->dstpath);
				if(rename(search->dstpath,tmpdst)<0){
					fprintf(stderr,"rename error for %s to %s\n",search->dstpath,tmpdst);
					exit(1);
				}
				strcpy(srcd->sigsrc,newdst);//src로 대체된dst 
				printf("sigsrc:%s\n",srcd->sigsrc);
				strcpy(srcd->tmpdst,tmpdst);//기존dst백업_SIGINT받으면 살려줌 
			}

			return 1;//존재하는 파일/디렉토리 
		}
		search=search->next;
	}
	if(opt==REPLACE)
		list_srcinsert(srcd);

	return 0;//존재하지않는 파일/디렉토리 
}
int list_samefilesearch(char *cmpfname,int cmpmtime, long cmpfsize){
	Node *search;//dst
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
//시간 계산 
void get_time(char *str){
	char timestr[TIME_SIZE];
	char tmp[TIME_SIZE];
	time_t timer=time(NULL);
	struct tm *t=localtime(&timer);

	memset(timestr,0,TIME_SIZE);
	memset(str,0,TIME_SIZE);
	memset(tmp,0,TIME_SIZE);

	//	sprintf(timestr,"[%a %b %02d %02d:%02d:%02d %04d]", p[t->tm_wday],p[t->tm_mon+1],t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec,t->tm_year+1900);

	strncpy(tmp,asctime(t),24);//개행없애기 
	sprintf(timestr,"[%s]",tmp);
	strcpy(str,timestr);
}
//파일 입출력 
void write_rsynclog_timecmd(char *cmdstr){
	//함수호출전에 반드시 curdir로, 함수호출후에 원래workingdir으로 반드시 바꿔놓을것.
	char timestr[TIME_SIZE];
	FILE *fp;
	char *rsyncfile="ssu_rsync_log";
	if((fp=fopen(rsyncfile,"a"))<0){
		fprintf(stderr,"fopen %s error",rsyncfile);
		exit(1);
	}
	get_time(timestr);//current time
	fseek(fp,0,SEEK_END);
	fprintf(fp,"%s %s\n",timestr,cmdstr);
	fclose(fp);
}
void write_rsynclog_files(char *fname, long fsize){
	char timestr[TIME_SIZE];
	FILE *fp;
	char *rsyncfile="ssu_rsync_log";
	if((fp=fopen(rsyncfile,"a"))<0){
		fprintf(stderr,"fopen %s error",rsyncfile);
		exit(1);
	}
	fseek(fp,0,SEEK_END);
	if(fsize==-1)
		fprintf(fp,"%s delete\n",fname);
	else
		fprintf(fp,"%s %ldbytes\n",fname,fsize);
	fclose(fp);
}
int rmdirs(const char *path, int force){
	DIR *dir_ptr=NULL;
	struct dirent *file=NULL;
	struct stat buf;
	char filename[1024];
	if((dir_ptr=opendir(path))==NULL){
		return unlink(path);
	}
	while((file=readdir(dir_ptr))!=NULL){
		if(strcmp(file->d_name,".")==0||strcmp(file->d_name,"..")==0){
			continue;
		}
		sprintf(filename,"%s/%s",path,file->d_name);
		if(lstat(filename,&buf)==-1){
			continue;
		}
		if(S_ISDIR(buf.st_mode)){
			if(rmdirs(filename,force)==-1&&!force){
				return -1;
			}
		}
		else if(S_ISREG(buf.st_mode)||S_ISLNK(buf.st_mode)){
			if(!unlink(filename)==-1&&force){
				return -1;
			}
		}
	}
	closedir(dir_ptr);
	return rmdir(path);
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

