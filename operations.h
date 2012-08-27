/**
 * \file
 *
 * =====================================================================================
 *
 *       Filename:  operations.h
 *
 *    Description:  Different operations on script files
 *
 *        Version:  1.0
 *        Created:  08/05/2012 15:12:30
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  François Hissel
 *        Company:  
 *
 * =====================================================================================
 */

#ifndef  OPERATIONS_INC
#define  OPERATIONS_INC

#include "procedures.h"

#define	FILENAME_MAX_LENGTH 0x400	//!< Maximum length of a path name in the virtual filesystem

/********************************************/
/*         DATA TYPES AND FUNCTIONS         */
/********************************************/
/**
 * \brief Persistent data
 *
 * This structure holds the persistent data needed for the application to be able to execute scripts properly.
 */
struct Persistent {
	char** envp;	//!< Array of environment variables as defined when the program is called
	char* mirror;	//!< Path to the mirror folder
	size_t mirror_len;	//!< Length of the mirror string
	int mirror_fd;	//!< File descriptor of the mirror folder
	Procedures *procs;	//!< List of procedures describing what to do with files
} persistent;	//!< Variable holding all the persistent data needed by the application

/**
 * \brief Data saved about an opened file
 *
 * When a file is opened on the virtual file system, FUSE gives it a handle that points to this structure and gives access to information about this file to the program.
 */
typedef struct FileStruct {
	/**
	 * \brief	Type structure, gives the type of the file or folder on which this handle is pointing
	 */
	enum Type {
		T_FILE,	//!< Regular file
		T_SCRIPT,	//!< Script file (detected in such a way by the file system, so it should not be overwritten)
		T_FOLDER	//!< Directory
	} type;	//!< Type of the file
	int file_handle;	//!< Handle of the corresponding item on the mirror file system if the system is a file
	void* dir_handle; //!< Pointer to the directory flow if the file is actually a directory
	//int dirfd;	//!< Handle of the directory if the file is a directory. This handle is kept to close the open directory when it is no longer used, but it should not be used by the application
	char filename[FILENAME_MAX_LENGTH];	//!< Name of the file
} FileStruct;

/**
 * \brief Initialize resources used to communicate with the remote website
 *
 * This function initializes some resources. It should be called at the start of the program, before any use of the external program.
 */
void init_resources();

/**
 * \brief Free all resources used by the program
 *
 * This function releases all the resources allocated by the program, to allow for a normal and clean exit. It frees the memory and closes the files.
 */
void free_resources();

/********************************************/
/*              TEST FUNCTIONS              */
/********************************************/
/**
 * \brief Test if a file is a shell script
 *
 * This function is one possible implementation of a TestFunction function. This one is used when the script interpretor is the shell. It tests if a particular file is a script by looking at the two first bytes and checking if they are a shebang.
 * \param test Pointer to the Test structure from which the function is called.
 * \param file Path of the file that has to be tested
 * \return 1 if the file is a shell script, 0 otherwise
 */
int test_shell(PTest test,const char *file);

/**
 * \brief Test function that always returns 1
 *
 * This function returns 1 without checking anything on the actual file. It can be used as a script TestFunction function so that every file is interpretated as a script and read by an external program.
 * \param test Pointer to the Test structure from which the function is called.
 * \param file Path of the file, not used
 * \return Always 1
 */
int test_true(PTest test,const char *file);

/**
 * \brief Test function that always returns 0
 *
 * This function returns 0 without checking anything on the actual file. It can be used as a script TestFunction function so that no file is interpretated as a script.
 * \param test Pointer to the Test structure from which the function is called.
 * \param file Path of the file, not used
 * \return Always 0
 */
int test_false(PTest test,const char *file);

/**
 * \brief Test if a file is executable
 *
 * This function is one possible implementation of a TestFunction function. It tests if the file in argument has the executable attribute in the mirror file system.
 * \param test Pointer to the Test structure from which the function is called.
 * \param file Path of the file that has to be tested
 * \return 1 if the file is executable, 0 otherwise
 */
int test_executable(PTest test,const char *file);

/**
 * \brief Test is the file would be executable in a shell
 *
 * This function checks if the file would be executable in a shell, that is either it is a shell script or it is an executable file. Thus the result of this function is the binary-or of test_shell and test_executable.
 * \param test Pointer to the Test structure from which the function is called.
 * \param file Path of the file that has to be tested
 * \return 1 if the file is executable in a shell, 0 otherwise
 */
int test_shell_executable(PTest test,const char *file);

/**
 * \brief Test a file by matching its full name against a regular expression
 *
 * This function is one possible implementation of a TestFunction function. It matches the name of the file in argument with the regular expression hold by the test variable. A regular expression is a standard POSIX basic regular expression. It accepts characters like ., *, ?, +,... See also the man page of grep for more details about basic regular expressions.
 * \param test Pointer to the Test structure from which the function is called.
 * \param file Path of the file that has to be tested
 * \return 1 if the file matches the regular expression, 0 otherwise
 */
int test_pattern(PTest test,const char *file);

/**
 * \brief Test function that checks the return value of the program
 *
 * This test function only checks the return value of the underlying program. If this value is not 0 (which means an error occurred), the file is not considered as a good script.
 * \param test Pointer to the Test structure from which the function is called. The structure holds data used to locate the test program.
 * \param file Path of the file which has to be tested
 * \return 1 if the file may be regarded as a script, 0 otherwise
 */
int test_program(PTest test,const char *file);

/********************************************/
/*           EXECUTION FUNCTIONS            */
/********************************************/
/**
 * \brief Execute a script with the help of an interpretor
 *
 * This function of the ProgramFunction type executes a script as it would be done by a shell. It is assumed the file (script) starts with a shebang #!, then the path of the interpretor is written on the same first line. This interpretor is used to execute the script. The function spawns a new process that loads the interpretor. The standard output is redirected to the file which descriptor is given.
 * \param program Pointer to the Program structure from which the function is called. The structure holds data used to locate the executable and get its arguments.
 * \param file Path of the script file
 * \param fd Descriptor of the file on which the output of the program will be written. The file should already be opened and ready to accept input
 * \return Error code of the external program after its execution
 */
int program_shell(PProgram program,const char *file,int fd);

/**
 * \brief Execute an external program and write its output on given file
 *
 * This function is a simple wrapper of the execute_program function and publishes it as a ProgramFunction type. It executes the external program on the specified file and redirects its output on the file which descriptor is given.
 * \param program Pointer to the Program structure from which the function is called. The structure holds data used to locate the executable and get its arguments.
 * \param file Path of the file on which the program will be executed. The file will be the last argument of the line which invokes the external program
 * \param fd Descriptor of the file on which the output of the program will be written. The file should already be opened and ready to accept input
 * \return Error code of the external program after its execution
 */
int program_external(PProgram program,const char *file,int fd);

/********************************************/
/*             OTHER OPERATIONS             */
/********************************************/
/**
 * \brief Find the script associated with a file
 *
 * This function tests the file in argument and tells if it is a script. It goes through all the procedures in the list given as argument, in the order in which they are stored. As soon as a test succeeds, the file is recognized as a script and a pointer to the corresponding procedure is returned. If no matching procedure is found, the function returns a null pointer. Since it is called very often (each time a folder is explored and a file is opened), it should be very fast and not rely too much on external programs.
 * \param procs List of procedures that will be tested against the file
 * \param file Path of the actual file
 * \return Pointer to a procedure which test function succeeds when applied to the file, null if no procedure is found
 */
Procedure* get_script(const Procedures *procs,const char *file);

/**
 * \brief Detect if a file is a shell script or a classic executable and executes it
 *
 * The function checks if the file in the first argument is a shell script or a classic executable file. It then executes it according to its nature. The executed program replaces the current process and nothing is done with input and output file descriptors which should be redirected before the call to the function if needed. Since the current process is replaced by a new one, the function should never return. The mere fact that it returns tells that an error occured.
 * \param file Path to the program to be executed
 * \param args Array of arguments to be added after the name of the program. Whether the program is a shell script or a real executable, the array of arguments will not be changed and will be sent as such to the execvp call.
 */
void call_program(const char *file,const char **args);

/**
 * \brief Spawn a process that executes an external program
 *
 * This function creates a new process which will execute the external program located at file. The third argument is a file descriptor on which the output will be written. If the descriptor is null, no output will be written at all. The last argument is a path to a file which content should be provided on the standard input of the external program. If nothing has to be sent to the external program, the user should give a null value to this parameter.
 * \param file Path to the executable file
 * \param args Array of arguments to be added after the name of the program. The array must end with a null pointer. By convention, the first element of the array should be the path of the program itself but this function does not take care of adding the path of the program (file) at the beginning of the array.
 * \param out Descriptor of the file on which the output will be redirected, 0 if no output is required
 * \param path_in Path of the file that should be provided to the standard output, 0 if no file has to be provided
 * \return Error code of the program after the end of its execution
 */
int execute_program(const char *file,const char **args,int out,const char *path_in);

#endif   /* ----- #ifndef OPERATIONS_INC  ----- */
