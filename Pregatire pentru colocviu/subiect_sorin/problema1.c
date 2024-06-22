#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/stat.h>

#define CHILD 	 0
#define SUCCESS  0
#define READ 	 0
#define WRITE	 1 
#define ERROR   -1

int create_nameless_pipe(int[]);
int write_to_pipe(int, char*, unsigned int);

int open_file(char*, int);

int main(int argc, char** argv) {

	int  PARENT_FIRST_CHILD[2] = {0};
	int PARENT_SECOND_CHILD[2] = {0};
	
	sem_t* FIRST_CHILD_TO_PARENT  = NULL;
	sem_t* SECOND_CHILD_TO_PARENT = NULL;
	sem_t* PARENT_TO_SECOND_CHILD = NULL;
	
	FIRST_CHILD_TO_PARENT  = sem_open("/FIRST_CHILD_TO_PARENT",  O_CREAT, 0644, 1);
	SECOND_CHILD_TO_PARENT = sem_open("/SECOND_CHILD_TO_PARENT", O_CREAT, 0644, 0);
	PARENT_TO_SECOND_CHILD = sem_open("/PARENT_TO_SECOND_CHILD", O_CREAT, 0644, 0);

	int shmFd = shm_open("SHAREDMEM", O_CREAT | O_RDWR, 0644);
	if(shmFd < 0) {
		perror("[PARENT]: Can't create the SHM");
		exit(ERROR);
	}

	ftruncate(shmFd, sizeof(unsigned int));
	volatile unsigned int* SHARED_MEMORY = (volatile unsigned int*)mmap(NULL, sizeof(unsigned int), PROT_READ | PROT_WRITE, MAP_SHARED, shmFd, 0);
	if(SHARED_MEMORY == MAP_FAILED) {
		perror("[PARENT]: Can't alloc SHARED_MEMORY.\n");
		exit(ERROR);
	}

	int EVALUATOR = -1;
	EVALUATOR = create_nameless_pipe(PARENT_FIRST_CHILD);
	if(EVALUATOR == ERROR) return ERROR;

	EVALUATOR = create_nameless_pipe(PARENT_SECOND_CHILD);
	if(EVALUATOR == ERROR) return ERROR;


	pid_t first_child = fork();
	if(first_child == CHILD) {
		close(PARENT_FIRST_CHILD[READ]);
		
		char path[256]; memset(path, 0, 256);
		printf("[FIRST CHILD]: Introduceti numele fisierului: ");
		scanf("%s", path);
		
		if(write_to_pipe(PARENT_FIRST_CHILD[WRITE], path, strlen(path)) == ERROR) exit(ERROR);
		
		sem_post(FIRST_CHILD_TO_PARENT);
		sem_wait(SECOND_CHILD_TO_PARENT);
		
		int freq = *(volatile unsigned int*)SHARED_MEMORY;

		printf("[FIRST CHILD]: In fisierul %s se secventa apare de %d ori.\n", path, freq);
		printf("[FIRST CHILD]: Procesul se finalizeaza.\n");
		exit(SUCCESS);
	} else {
		
		pid_t second_child = fork();
		if(second_child == CHILD) {
			close(PARENT_SECOND_CHILD[WRITE]);
			sem_wait(PARENT_TO_SECOND_CHILD);

			printf("[SECOND CHILD]: Se incepe procesul copil.\n");

			int size = 0, freq = 0;
			read(PARENT_SECOND_CHILD[READ], &size, sizeof(int));

			char received_path[256]; memset(received_path, 0, 256);
			read(PARENT_SECOND_CHILD[READ], received_path, size);

			int fd = open_file(received_path, O_RDONLY);
			if(fd == ERROR) exit(ERROR);
			

			size = lseek(fd, 0, SEEK_END);
			lseek(fd, 0, SEEK_SET);
			
			for(int i = 0; i<size-2; i++) {
				char buffer[3]; memset(buffer, 0, 3 * sizeof(char));

				int success = read(fd, buffer, 3 * sizeof(char));
				if(success != 3 * sizeof(char)) exit(ERROR);

				lseek(fd, -2, SEEK_CUR);
				if(strncmp(buffer, "abc", 3) == 0) freq++;
				printf("%s\n", buffer);
			}
			
			*(volatile unsigned int*)(SHARED_MEMORY) = freq;		
			sem_post(SECOND_CHILD_TO_PARENT);
			printf("[SECOND CHILD]: Procesul se finalizeaza.\n");

			close(fd);
			exit(SUCCESS);
		} else {
			int PROCESS_STATUS = -1;
			sem_wait(FIRST_CHILD_TO_PARENT);
			
			close(PARENT_FIRST_CHILD[WRITE]);
			close(PARENT_SECOND_CHILD[READ]);
		
			unsigned int size = 0;
			EVALUATOR = read(PARENT_FIRST_CHILD[READ], &size, sizeof(unsigned int));
			if(EVALUATOR != sizeof(unsigned int)) return ERROR;
			
			
			char path[256]; memset(path, 0, 256);
			EVALUATOR = read(PARENT_FIRST_CHILD[READ], path, size * sizeof(char));
			if(EVALUATOR != size * sizeof(char)) return ERROR;
		
			printf("[PARENT]: Parintele a primit string-ul: %s.\n", path);

			if(write_to_pipe(PARENT_SECOND_CHILD[WRITE], path, strlen(path)) == ERROR) exit(ERROR);
			printf("[PARENT]: Parintele a scris in pipe-ul procesului 2.\n");

			sem_post(PARENT_TO_SECOND_CHILD);
		
			waitpid(second_child, NULL, 0);
			waitpid(first_child, NULL, 0);
			
			
			sem_close(FIRST_CHILD_TO_PARENT); 
			sem_close(SECOND_CHILD_TO_PARENT);
			sem_unlink("SECOND_CHILD_TO_PARENT");
			sem_unlink("FIRST_CHILD_TO_PARENT");
			
			return SUCCESS;
		}
	}
}

int create_nameless_pipe(int fd[2]) {
	do{
		int STATUS = pipe(fd);
		if(STATUS != SUCCESS) {
			perror("[CREATE PIPE]: Can't create the pipe.\n");
			break;
		}
		return SUCCESS;
	} while(1);
	
	return ERROR;
}

int write_to_pipe(int fd, char* path, unsigned int size) {
	do {
		int STATUS = write(fd, &size, sizeof(unsigned int));
		if(STATUS != sizeof(unsigned int)) {
			perror("[WRITE PIPE]: Can't write in pipe. (1)\n");
			break;
		}
		
		STATUS = write(fd, path, size * sizeof(char));
		if(STATUS != size * sizeof(char)) {
			perror("[WRITE PIPE]: Can't write in pipe. (2)\n");
			break;
		}
		
		return SUCCESS;
	} while(1);
	
	return ERROR;
}

int open_file(char* path, int MODE) {

	do {
		int STATUS = open(path, MODE);
		if(STATUS < 0) {
			printf("[OPEN FILE]: Can't open file at: %s", path);
			break;
		}
		return STATUS;
	} while(1);
	return ERROR;
}
