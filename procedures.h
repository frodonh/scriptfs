/*
 * \file
 *
 * =====================================================================================
 *
 *       Filename:  procedures.h
 *
 *    Description:  Definition of procedures
 *
 *        Version:  1.0
 *        Created:  03/06/2012 15:56:59
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  François Hissel
 *        Company:  
 *
 * =====================================================================================
 */

#ifndef  PROCEDURES_INC
#define  PROCEDURES_INC

#define	MAX_PATH_LENGTH 0x400	//!< Maximal lengths of paths in the file system (used to allocate buffers when needed)
#define	MAX_ARGS_NUMBER 0x100 //!< Maximum number of arguments in a command

#include <regex.h>

/********************************************/
/*                FUNCTIONS                 */
/********************************************/
typedef struct Program *PProgram;	//!< Forward definition of pointer to Program type
typedef struct Test *PTest;	//!< Forward definition of pointer to Test type

/**
 * \brief Type of a test function
 *
 * The test function should be called with a path as its parameter and returns 1 if the file at path location is a script, and 0 otherwise.
 */
typedef int (*TestFunction)(PTest,const char*);

/**
 * \brief Type of a script function
 *
 * The script function is called with a parameter giving the path of a script. It executes the script, writes its output on the file with fd descriptor, and returns the error code of the program.
 */
typedef int (*ProgramFunction)(PProgram,const char*,int fd);

/********************************************/
/*                 PROGRAM                  */
/********************************************/
/**
 * \brief Structure used to define an executable program
 *
 * The executable program is the program that is called against each script file. It converts the script to the result of its execution.
 */
typedef struct Program {
	char *path;	//!< Full path to the program, null if the program is automatically detected or the script itself (see also \ref syntaxdoc "Syntax of command-line").
	char **args;	//!< Array of arguments to send to the program. This variable is null if no external program is defined. If an exclamation mark has been found in the array of arguments, it is replaced by a null element. The filearg variable points to the position of this null element. The last element of the array must be a null pointer. The first element of the array is the name of the executable itself (to comply with the standard way to call a program).
	char **filearg;	//!< If there is an exclamation mark in args, the variable points to the element holding this exclamation mark
	int filter;	//!< Tells if the program is actually a filter. In that case, if the filearg variable is null, the program expects to get content on its standard input
	ProgramFunction func;	//!< Pointer to the program function
} Program;

/**
 * \brief Release the memory allocated to a Program structure
 *
 * \param program Pointer to the Program structure
 */
void free_program(Program *program);

/**
 * \brief Construct a Program structure from a string. 
 *
 * This function creates a Program structure from a string given as argument. The format of the string is described in \ref syntaxdoc "Syntax of command-line". The user is responsible for releasing the memory of the newly-allocated structure.
 * \param str String from which the Program structure is read
 * \return Pointer to a newly-allocated Pointer structure, 0 if something went wrong
 */
Program *get_program_from_string(const char *str);

/********************************************/
/*                   TEST                   */
/********************************************/
/**
 * \brief Structure used to define a test program
 *
 * The test program is used to detect if a given file is a script file.
 */
typedef struct Test {
	char *path;	//!< Full path to the test program, null if the test is not a program (see also \ref syntaxdoc "Syntax of command-line").
	char **args;	//!< Array of arguments to send to the program. This variable is null if no external program is defined. If an exclamation mark has been found in the array of arguments, it is replaced by a null element. The filearg variable points to the position of this null element. The last element of the array must be a null pointer. The first element of the array is the name of the executable itself (to comply with the standard way to call a program).
	char **filearg;	//!< If there is an exclamation mark in args, the variable points to the element holding this exclamation mark
	int filter;	//!< Tells if the test function is actually a filter. In that case, if the filearg variable is null, the function expects to get content on its standard input
	regex_t *compiled;	//!< If the Test function is actually a match against a regular expression, holds the compiled value of the regular expression, otherwise null
	TestFunction func;	//!< Pointer to the test function
} Test;

/**
 * \brief Release the memory allocated to a Test structure
 *
 * \param test Pointer to the Test structure
 */
void free_test(Test *test);

/**
 * \brief Construct a Test structure from a string. 
 *
 * This function creates a Test structure from a string given as argument. The format of the string is described in \ref syntaxdoc "Syntax of command-line". The user is responsible for releasing the memory of the newly-allocated structure.
 * \param str String from which the Test structure is read
 * \return Pointer to a newly-allocated Test structure, 0 if something went wrong
 */
Test *get_test_from_string(const char *str);

/********************************************/
/*                PROCEDURE                 */
/********************************************/
/**
 * \brief Structure gathering information about what do to with a file on the virtual file system
 *
 * A procedure is a set of data telling how the files on the virtual file system shall be dealt with. It holds especially a link to an "executable program" which will be called on each script file, and another link to a "test program" that has to be executed on every file to detect if it is a script file.
 */
typedef struct Procedure {
	Program *program;	//!< Pointer to the Program structure
	Test *test;	//!< Pointer to the Test structure
} Procedure;

/**
 * \brief Release the memory allocated to a Procedure structure
 *
 * \param procedure Pointer to the Procedure structure
 */
void free_procedure(Procedure *procedure);

/**
 * \brief Reads a procedure from a string
 *
 * This function is used to process the command-line \c -p arguments. One such argument is converted to a Procedure structure. The newly-allocated structure must be released by the user when it is not needed any longer.
 * \param str String from which the procedure must be read
 * \return Pointer to a newly-created Procedure structure
 */
Procedure* get_procedure_from_string(const char* str);

/********************************************/
/*                PROCEDURES                */
/********************************************/
/**
 * \brief Chained list of Procedure objects
 *
 * This structure holds a list of Procedure objects. The order of the objects is the same as the one in the command-line, thus it is the order in which the procedures must be processed. The list ends with a null pointer.
 */
typedef struct Procedures {
	Procedure *procedure;	//!< Current Procedure element
	struct Procedures *next;	//!< Next element in the list
} Procedures;

/**
 * \brief Release the memory allocated to a Procedures structure
 *
 * The function release all the memory allocated to a Procedures structure. It goes through the Procedure elements and frees pointers in the elements.
 *
 * \param procedures Pointer to the Procedures structure
 */
void free_procedures(Procedures *procedures);

#endif   /* ----- #ifndef PROCEDURES_INC  ----- */
