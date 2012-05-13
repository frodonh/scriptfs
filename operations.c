/*
 * =====================================================================================
 *
 *       Filename:  operations.c
 *
 *    Description:  Implementation of operations on script files
 *
 *        Version:  1.0
 *        Created:  08/05/2012 15:13:31
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  François Hissel
 *        Company:  
 *
 * =====================================================================================
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "operations.h"

/********************************************/
/*         DATA TYPES AND FUNCTIONS         */
/********************************************/
void init_resources() {
	persistent.mirror=0;
	persistent.mirror_len=0;
	persistent.test=0;
	persistent.program=0;
	persistent.program_path=0;
	persistent.program_args=0;
	persistent.test_path=0;
	persistent.test_args=0;
}

void free_resources() {
	free(persistent.mirror);
	free(persistent.program_path);
	char **p;
	if (persistent.program_args!=0) {
		p=persistent.program_args;
		while (*p!=0) free(*(p++));
		free(persistent.program_args);
	}
	free(persistent.test_path);
	if (persistent.test_args!=0) {
		p=persistent.test_args;
		while (*p!=0) free(*(p++));
		free(persistent.test_args);
	}
}

/********************************************/
/*              TEST FUNCTIONS              */
/********************************************/
int test_true(const char *file) {
	return 1;
}

int test_shell(const char *file) {
	FILE *f=fopen(file,"r");
	if (f==0) return 0;
	char magic[2];
	size_t s=fread(magic,1,2,f);
	fclose(f);
	if (s<2 || magic[0]!='#' || magic[1]!='!') return 0;
	return 1;
}

int test_program(const char *file) {
	// Create the array of arguments of the program by adding the reference to the file at the very end of it and the path of the program at the beginning
	size_t num=0;
	const char **args=(const char**)persistent.test_args;
	while (*args!=0) {++args;++num;}
	const char **newargs=(const char**)malloc((num+2)*sizeof(char*));
	args=(const char**)persistent.test_args;
	const char **na=newargs;
	*(na++)=persistent.program_path;
	while (*args!=0) {*(na++)=*(args++);}
	*(na++)=file;
	*na=0;
	// Launch the program
	int code=execute_program(persistent.test_path,newargs,0);
	// Release memory and exit
	free(newargs);
	return (code==0);
}

/********************************************/
/*           EXECUTION FUNCTIONS            */
/********************************************/
int program_shell(const char *file,int fd) {
	FILE *f=fopen(file,"r");
	if (f==0) return 1;
	char *line=0;
	size_t nn;
	ssize_t n=getline(&line,&nn,f);
	size_t i=2;
	while (i<n && (line[i]==' ' || line[i]=='\t')) ++i;
	if (i>=n || line[i]=='\n') return 1;
	size_t j=i;
	while (j<n && (line[j-1]=='\\' || (line[j]!=' ' && line[j]!='\t' && line[j]!='\n'))) ++j;
	line[j]=0;
	char *path=line+i;
	const char *args[]={path,file,0};
	int code=execute_program(path,args,fd);
	free(line);
	return code;
}

int program_external(const char *file,int fd) {
	// Create the array of arguments of the program by adding the reference to the file at the very end of it and the path of the program at the beginning
	size_t num=0;
	const char **args=(const char**)persistent.program_args;
	while (*args!=0) {++args;++num;}
	const char **newargs=(const char**)malloc((num+2)*sizeof(char*));
	args=(const char**)persistent.program_args;
	const char **na=newargs;
	*(na++)=persistent.program_path;
	while (*args!=0) {*(na++)=*(args++);}
	*(na++)=file;
	*na=0;
	// Launch the program
	int code=execute_program(persistent.program_path,newargs,fd);
	// Release memory and exit
	free(newargs);
	return code;
}

/********************************************/
/*             OTHER OPERATIONS             */
/********************************************/
int is_script(const char *file) {
	return persistent.test(file);
}

int execute_program(const char *file,const char **args,int fd) {
	pid_t child;	// ID of child process executing external program
	child=fork();
	if (child!=0) {	// Parent process (caller)
		int code;
		waitpid(child,&code,0);
		if (WIFEXITED(code)) return WEXITSTATUS(code);
	} else {	// Child process (external program)
		if (fd!=0) dup2(fd,STDOUT_FILENO);	// Redirect output to fd descriptor
		else dup2(STDERR_FILENO,STDOUT_FILENO);	// Redirect standard output on standard error, to avoid mixing outputs from the external program and the parent process
		close(STDIN_FILENO);	// We do not want the external program to use anything from the common standard input
		execvp(file,(char *const *)args);
		fprintf(stderr,"Error calling external program : %s",file);
		while (args!=0) fprintf(stderr," %s",*(args++));
		fprintf(stderr,"\n");
		abort();
	}
	return 1;
}
