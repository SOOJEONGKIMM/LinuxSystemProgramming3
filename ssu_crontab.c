#include "./ssu_crontab.h"

int addOpt = 0;
int removeOpt = 0;
int exitOpt = 0;
int invalidOpt = 0;

void ssu_crontab_prompt(void) {
	char cmdbuf[BUFFER_SIZE];
	int cmd;

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

	printf("add opt\n");

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
