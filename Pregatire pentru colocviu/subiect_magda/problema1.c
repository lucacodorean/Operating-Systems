#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define PROCESS_START(id) printf("P[%d] start.\n", id);
#define   PROCESS_END(id) printf("P[%d] end.\n", id);
#define CHILD 		  0
#define SUCCESS 	  0

int main(void) {

	pid_t first_process = fork();
	
	if(first_process == CHILD) {
		PROCESS_START(getpid());
		
		pid_t second_process = fork();
		if(second_process == CHILD) {
			PROCESS_START(getpid());
			PROCESS_END(getpid());
			exit(SUCCESS);
		}
		
		pid_t third_process = fork();
		if(third_process == CHILD) {
			PROCESS_START(getpid());
			
			pid_t fifth_process = fork();
			if(fifth_process == CHILD) {
				PROCESS_START(getpid());
			
				pid_t sixth_process = fork();
				if(sixth_process == CHILD) {
					PROCESS_START(getpid());
					PROCESS_END(getpid());
					exit(SUCCESS);
				}
					
				waitpid(sixth_process, NULL, 0);
				PROCESS_END(getpid());
				exit(SUCCESS);
			}
			
			waitpid(fifth_process, NULL, 0);
			
			pid_t forth_process = fork();
			if(forth_process == CHILD) {
				PROCESS_START(getpid());
				PROCESS_END(getpid());
				exit(SUCCESS);
			}
			
			waitpid(forth_process, NULL, 0);
			PROCESS_END(getpid());
			exit(SUCCESS);
		}
		
		waitpid(second_process, NULL, 0);
		waitpid(third_process, NULL, 0);
		PROCESS_END(getpid());
		exit(SUCCESS);
	}

	return SUCCESS;
}
