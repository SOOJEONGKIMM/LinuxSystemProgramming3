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

	//[OPTION]없는 경우 
	if(strlen(option)==0){
		printf("no option\n");
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
	return 0;
}
