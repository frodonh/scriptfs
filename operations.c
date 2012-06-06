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
#include <fcntl.h>
#include "procedures.h"
#include "operations.h"

/********************************************/
/*         DATA TYPES AND FUNCTIONS         */
/********************************************/
void init_resources() {
	persistent.mirror=0;
	persistent.mirror_len=0;
	persistent.procs=0;
}

void free_resources() {
	free(persistent.mirror);
	free_procedures(persistent.procs);
}

/********************************************/
/*              TEST FUNCTIONS              */
/********************************************/
int test_true(PTest test,const char *file) {return 1;}
int test_false(PTest test,const char *file) {return 0;}

int test_shell(PTest test,const char *file) {
	FILE *f=fopen(file,"r");
	if (f==0) return 0;
	char magic[2];
	size_t s=fread(magic,1,2,f);
	fclose(f);
	if (s<2 || magic[0]!='#' || magic[1]!='!') return 0;
	return 1;
}

int test_executable(PTest test,const char *file) {
	return access(file,X_OK)==0;
}

int test_pattern(PTest test,const char *file) {
	if (test->compiled==0) return 0;
	return regexec(test->compiled,file,0,0,0)==0;
}

int test_program(PTest test,const char *file) {
	// Create the array of arguments of the program by replacing the exclamation mark with the name of the file
	if (test->args!=0 && test->filearg!=0) *(test->filearg)=(char*)file;
	// If the program is a filter that requires standard input, add the name of the file in the arguments of the call to execute_program
	const char *f=(test->filter)?file:0;
	// Launch the program
	int code=execute_program(test->path,(const char**)(test->args),0,f);
	return (code==0);
}

/********************************************/
/*           EXECUTION FUNCTIONS            */
/********************************************/
int program_shell(PProgram program,const char *file,int fd) {
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
	int code=execute_program(path,args,fd,0);
	free(line);
	return code;
}

int program_self(PProgram program,const char *file,int fd) {
	const char *args[]={file,0};
	int code=execute_program(file,args,fd,0);
	return code;
}

int program_external(PProgram program,const char *file,int fd) {
	// Create the array of arguments of the program by replacing the exclamation mark with the name of the file
	if (program->args!=0 && program->filearg!=0) *(program->filearg)=(char*)file;
	// If the program is a filter that requires standard input, add the name of the file in the arguments of the call to execute_program
	const char *f=(program->filter)?file:0;
	// Launch the program
	int code=execute_program(program->path,(const char**)(program->args),fd,f);
	return code;
}

/********************************************/
/*             OTHER OPERATIONS             */
/********************************************/
Procedure* get_script(const Procedures *procs,const char *file) {
	Procedure *res=0;
	while (res==0 && procs!=0) {
		if (procs->procedure->test->func(procs->procedure->test,file)!=0) res=procs->procedure;
	}
	return res;
}

int execute_program(const char *file,const char **args,int out,const char* path_in) {
	pid_t child;	// ID of child process executing external program
	int fds[2];	// Handles of the two ends of the pipe, only used if input has to be provided to the standard input of the external program
	int in;
	if (path_in!=0) {	// Prepare a pipe to feed standard input of the external program, fork and copy the file to the pipe
		pipe(fds);
		child=fork();
		close(fds[0]);	// Close input descriptor
		in=open(path_in,O_RDONLY);
		if (in<0) path_in=0; else {	// Copy file to standard input
			char buffer[0x1000];
			ssize_t num,numw,num2;
			do {
				num=read(in,buffer,0x1000);
				numw=0;
				while (numw<num) {
					num2=write(fds[1],buffer+numw,num-numw);
					if (num2<0) numw=num; else numw+=num2;
				}
			} while (num>0);
		}
		fsync(fds[1]);
		close(fds[1]);
	} else child=fork();	// Only create the pipe
	if (child!=0) {	// Parent process (caller)
		int code;
		waitpid(child,&code,0);
		if (WIFEXITED(code)) return WEXITSTATUS(code);
	} else {	// Child process (external program)
		if (out!=0) dup2(out,STDOUT_FILENO);	// Redirect output to out descriptor
		else dup2(STDERR_FILENO,STDOUT_FILENO);	// Redirect standard output on standard error, to avoid mixing outputs from the external program and the parent process
		if (path_in==0) {
			close(STDIN_FILENO);	// We do not want the external program to use anything from the common standard input
		} else {
			close(fds[1]);	// Close output descriptor
			dup2(fds[0],STDIN_FILENO);	// Redirect standard input to pipe output
		}
		execvp(file,(char *const *)args);
		fprintf(stderr,"Error calling external program : %s",file);
		while (args!=0) fprintf(stderr," %s",*(args++));
		fprintf(stderr,"\n");
		abort();
	}
	return 1;
}
