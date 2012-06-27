/*
 * \file
 *
 * =====================================================================================
 *
 *       Filename:  procedures.c
 *
 *    Description:  Implementation of procedures
 *
 *        Version:  1.0
 *        Created:  03/06/2012 20:16:52
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  François Hissel
 *        Company:  
 *
 * =====================================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <regex.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "procedures.h"
#include "operations.h"

/********************************************/
/*                UTILITIES                 */
/********************************************/
/**
 * \brief Read a single word in a command-line
 *
 * This function reads a word in a string. The word is either a group of successive non-blank characters or escaped blanks, or a string delimitated by quotes.
 * \param str Pointer to the start of the string that should be read, points to the start of the next word after the function is executed
 * \return Word that has been read. Memory is allocated to hold this word, so the user has to release when it is not needed.
 */
char *read_word(const char **str) {
	if (*str==0) return 0;
	while (**str!=0 && (**str==' ' || **str=='\t')) (*str)++;
	if (**str==0) return 0;
	const char *s=*str;
	char word[MAX_PATH_LENGTH];
	size_t num=0;
	int state=1;
	while (*s!=0 && state!=0 && num<MAX_PATH_LENGTH-1) {
		switch (state) {
			case 1:
				if (*s=='"') state=2;
				else if (*s=='\'') state=3;
				else if (*s=='\\') state=4;
				else if (*s==' ' || *s=='\n' || *s=='\t') state=0;
				else word[num++]=*s;
				break;
			case 2:
				if (*s=='"') state=1;
				else if (*s=='\\') state=5;
				else word[num++]=*s;
				break;
			case 3:
				if (*s=='\'') state=1;
				else word[num++]=*s;
				break;
			case 4:
				if (*s=='\'' || *s=='"' || *s=='\\') word[num++]=*s;
				else if (*s=='t') word[num++]='\t';
				else if (*s=='n') word[num++]='\n';
				else if (*s=='r') word[num++]='\r';
				state=1;
				break;
			case 5:
				if (*s=='"' || *s=='\\') word[num++]=*s;
				else {word[num++]='\\';word[num++]=*s;}
				state=2;
				break;
		}
		++s;
	}
	word[num++]=0;
	char *res=(char*)malloc(num*sizeof(char));
	strcpy(res,word);
	*str=s;
	return res;
}

/**
 * \brief Analyze a command and extract the name of the executable and the arguments
 *
 * This function analyzes the command in string str as it was a command-line in a shell. The first word is the name of the executable and is sved in the path variable. All other words or group of letters delimitated by quotes are saved in the args array, the first element of this array is again the name of the executable. A null pointer is added at the end of the array. Finally if an exclamation mark is found in one argument, this word is not saved in the args element and the element at the corresponding position is set to a null pointer, while the position is saved in the filearg variable.
 * \param str String which will be tokenized
 * \param path Pointer to the name of the executable, initialized by the function
 * \param args Pointer to an array of arguments, initialized by the function
 * \param filearg Pointer to a position in the array args, which represents the element that will be replaced by the name of the script file
 */
void tokenize_command(const char *str,char **path,char ***args,char ***filearg) {
	*path=0;
	*filearg=0;
	*args=0;
	if (str==0) return;
	*path=read_word(&str);
	if (*path==0) return;
	size_t num=1;
	*args=(char**)malloc((MAX_ARGS_NUMBER+1)*sizeof(char*));
	(*args)[0]=(char*)malloc((strlen(*path)+1)*sizeof(char));
	strcpy((*args)[0],*path);
	while (*str!=0 && num<MAX_ARGS_NUMBER) {
		(*args)[num]=read_word(&str);
		if ((*args)[num]!=0 && (*args)[num][0]=='!' && (*args)[num][1]==0) {
			free((*args)[num]);
			(*args)[num]=0;
			*filearg=*args+num;
			++num;
		}
		if ((*args)[num]!=0) ++num;
	}
	(*args)[num++]=0;
	*args=realloc(*args,num*sizeof(char*));
}

/********************************************/
/*                 PROGRAM                  */
/********************************************/
void free_program(Program *program) {
	if (program==0) return;
	free(program->path);
	char **a=program->args;
	if (a!=0) {
		while (*a) free(*(a++));
		free(program->args);
	}
	free(program);
}

Program *get_program_from_string(const char *str) {
	if (str==0) return 0;
	Program *prog=(Program*)malloc(sizeof(Program));
	prog->path=0;
	prog->args=0;
	prog->filearg=0;
	prog->filter=0;
	if (*str==0 || strncasecmp(str,"AUTO",4)==0) {	// Program is either a shell script or an executable that can be executed by itself
		prog->func=&program_shell;
	} else {	// The program is located by a path name
		tokenize_command(str,&(prog->path),&(prog->args),&(prog->filearg));
		if (prog->path!=0) {
			struct stat fileinfo;
			if (!(stat(prog->path,&fileinfo)==0 && S_ISREG(fileinfo.st_mode) && access(prog->path,X_OK)==0)) {
				fprintf(stderr,"%s can not be found or executed\n",prog->path);
			} else {
				prog->func=&program_external;
				prog->filter=1;
			}
		}
	}
	if (prog->func==0) {	// If no program function was set, clear the structure
		free_program(prog);
		prog=0;
	}
	return prog;
}

/********************************************/
/*                   TEST                   */
/********************************************/
void free_test(Test *test) {
	if (test==0) return;
	free(test->path);
	char **a=test->args;
	if (a!=0) {
		while (*a) free(*(a++));
		free(test->args);
	}
	if (test->compiled) regfree(test->compiled);
	free(test->compiled);
	free(test);
}

Test *get_test_from_string(const char *str) {
	if (str==0) return 0;
	Test *test=(Test*)malloc(sizeof(Test));
	test->path=0;
	test->args=0;
	test->filearg=0;
	test->compiled=0;
	test->filter=0;
	test->func=0;
	if (*str==0 || strncasecmp(str,"ALWAYS",6)==0) {	// Consider all files are executable
		test->func=test_true;
	} else if (strncasecmp(str,"EXECUTABLE",10)==0) {	// Only files marked as executable in the mirror file system are considered as executable
		test->func=test_executable;
	} else if (*str=='&') {	// A regexp is used to select the names of the files
		regex_t *reg=(regex_t*)malloc(sizeof(regex_t));
		if (regcomp(reg,str+1,REG_NOSUB)!=0) {	// If the regular expression is invalid, no file is recognized as a script
			test->func=test_false;
			free(reg);
		} else {
			test->func=test_pattern;
			test->compiled=reg;
		}
	} else {	// The program is located by a path name
		tokenize_command(str,&(test->path),&(test->args),&(test->filearg));
		if (test->path!=0) {
			struct stat fileinfo;
			if (!(stat(test->path,&fileinfo)==0 && S_ISREG(fileinfo.st_mode) && access(test->path,X_OK)==0)) {
				fprintf(stderr,"%s can not be found or executed\n",test->path);
			} else {
				test->func=&test_program;
				test->filter=1;
			}
		}
	}
	if (test->func==0) {	// If no test function was set, clear the structure
		free_test(test);
		test=0;
	}
	return test;
}

/********************************************/
/*                PROCEDURE                 */
/********************************************/
void free_procedure(Procedure *procedure) {
	if (procedure==0) return;
	free_program(procedure->program);
	free_test(procedure->test);
	free(procedure);
}

Procedure* get_procedure_from_string(const char* str) {
	if (str==0 || *str==0) return 0;
	Procedure *proc=(Procedure*)malloc(sizeof(Procedure));
	const char *p=str;
	// Find the limit between the program and the test
	while (*p!=0 && *p!=';') ++p;
	// Read program
	char *q=(char*)malloc((p-str+1)*sizeof(char));
	strncpy(q,str,p-str);
	q[p-str]=0;
	proc->program=get_program_from_string(q);
	// Read test
	if (proc->program!=0) {
		if (*p==0) {
			if (proc->program->func==&program_external) {	// Choose same external program for the test function
				proc->test=get_test_from_string(q);
			} else if (proc->program->func==&program_shell) {	// Choose corresponding test function for shell scripts
				proc->test=(Test*)malloc(sizeof(Test));
				proc->test->func=&test_shell_executable;
				proc->test->path=0;
				proc->test->args=0;
				proc->test->filearg=0;
				proc->test->filter=0;
				proc->test->compiled=0;
			} else proc->test=0;
			free(q);
		}
		else {
			free(q);
			str=p+1;
			p=str;
			while (*p!=0) ++p;
			q=(char *)malloc((p-str+1)*sizeof(char));
			strncpy(q,str,p-str);
			q[p-str]=0;
			proc->test=get_test_from_string(q);
			free(q);
		}
	} else { // If nothing was declared, release the Procedure structure
		free(proc);
		proc=0;
	}
	// Return the Procedure object
	return proc;
}

/********************************************/
/*                PROCEDURES                */
/********************************************/
void free_procedures(Procedures *procedures) {
	if (procedures==0) return;
	Procedures *p=procedures;
	Procedures *q;
	while (p) {
		q=p->next;
		free_procedure(p->procedure);
		free(p);
		p=q;
	}
}
