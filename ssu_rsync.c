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
	int srcfsize=0;
	srcmtime=srcbuf.st_mtime;
	srcfsize=srcbuf.st_size;
	printf("src mtime:%d size:%d\n",srcmtime,srcfsize);

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
	//dst 디렉토리 스캔 
	Node *newnode;
	newnode=(Node*)malloc(sizeof(Node));
	scan_dst(dst, newnode);

	int samefile=0;
	int samename=0;
	int nofile=0;
	if(!list_samefilesearch(onlysrcfname,srcmtime, srcfsize)&&!list_samenamesearch(onlysrcfname))
		nofile=1;
	if(list_samefilesearch(onlysrcfname,srcmtime, srcfsize))
		samefile=1;
	if(!list_samefilesearch(onlysrcfname,srcmtime, srcfsize)&&list_samenamesearch(onlysrcfname))
		samename=1;
	printf("no:%d same file:%d, name:%d",nofile,samefile,samename);
	//[OPTION]없는 경우 
	if(strlen(option)==0){
		if(nofile){
			if(file_src)
				rsync_copyF(src,onlysrcfname);
			else if(dir_src)
				rsync_copyD(src,onlysrcfname);
		}
		else if(samename){
			if(file_src)
				rsync_replaceF(src, onlysrcfname);
			else if(dir_src)
				rsync_replaceD(src, onlysrcfname);
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


	//printf("option:%s src:%s dst:%s\n",option,src,dst);
	//list_print();
	return 0;
}
int rsync_copyF(char *src,char *onlysrcfname){
	printf("rsyn_copy file\n");
	return 0;
}
int	rsync_copyD(char *src,char *onlysrcfname){
	printf("rsync_copy dir\n");
	return 0;
}
int rsync_replaceF(char *src, char *onlysrcfname){
	printf("rsync_replace file\n");
	return 0;
}
int rsync_replaceD(char *src, char *onlysrcfname){
	printf("rsync_replace dir\n");
	return 0;
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
		//printf("d_name:%s\n",flist[i]->d_name);
		Node *node=(Node *)malloc(sizeof(Node));
		memset(node,0,sizeof(Node));

		if(lstat(dststr,&statbuf)<0){
			fprintf(stderr,"lstat() error for %s\n",flist[i]->d_name);
			return -1;
		}

		strcpy(node->onlydstfname,flist[i]->d_name);
		node->mtime=statbuf.st_mtime;
		node->fsize=statbuf.st_size;

		memset(dstpath,0,PATH_SIZE);
		/*if(realpath(flist[i]->d_name,dstpath)==NULL){
		  fprintf(stderr,"realpath() error\n");
		  return -1;
		  }*/

		strcpy(node->dstpath,dststr);//flist[i]->d_name);
		list_insert(node);

		if(S_ISDIR(statbuf.st_mode)){
			scan_dst(dststr,node);
		}

		i++;
	}
	return 1;
}
void list_insert(Node *newnode){
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
int list_samefilesearch(char *cmpfname,int cmpmtime, int cmpfsize){
	Node *search;
	search=head;
	while(search){
//			printf("dst:path:%s mtime:%d size:%d\n",search->dstpath,search->mtime,search->fsize);
		if(!strcmp(search->onlydstfname,cmpfname)&&(search->mtime==cmpmtime)&&(search->fsize==cmpfsize))
			return 1;//존재하는 파일/디렉토리 
		search=search->next;
	}
	return 0;//존재하지않는 파일/디렉토리 
}

//디버깅용 
void list_print(){
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

