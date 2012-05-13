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

/********************************************/
/*         DATA TYPES AND FUNCTIONS         */
/********************************************/
/**
 * \brief Type of a test function
 *
 * The test function should be called with a path as its parameter and returns 1 if the file at path location is a script, and 0 otherwise.
 */
typedef int (*Test)(const char*);

/**
 * \brief Type of a script function
 *
 * The script function is called with a parameter giving the path of a script. It executes the script, writes its output on the file with fd descriptor, and returns the error code of the program.
 */
typedef int (*Program)(const char*,int fd);

/**
 * \brief Persistent data
 *
 * This structure holds the persistent data needed for the application to be able to execute scripts properly.
 */
struct Persistent {
	char* mirror;	//!< Path to the mirror folder
	size_t mirror_len;	//!< Length of the mirror string
	Test test;	//!< Function that will be called to test if a file is a script
	Program program;	//!< Program to execute on scripts
	char *program_path;	//!< Path of the external program
	char **program_args;	//!< Array of arguments to send to the external program before the name of the file
	char *test_path;	//!< Path of the external program which will be used to test if a file is a script
	char **test_args;	//!< Array of arguments to send to the test program
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
	int handle;	//!< Handle of the corresponding item on the mirror file system if the system is a file, pointer to the directory flow if the file is actually a directory
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
 * This function is one possible implementation of a Test function. This one is used when the script interpretor is the shell. It tests if a particular file is a script by looking at the two first bytes and checking if they are a shebang.
 * \param file Path of the file that has to be tested
 * \return 1 if the file is a shell script, 0 otherwise
 */
int test_shell(const char *file);

/**
 * \brief Test function that always returns 1
 *
 * This function returns 1 without checking anything on the actual file. It can be used as a script Test function so that every file is interpretated as a script and read by an external program.
 * \param file Path of the file, not used
 * \return Always 1
 */
int test_true(const char *file);

/**
 * \brief Test function that checks the return value of the program
 *
 * This test function only checks the return value of the underlying program. If this value is not 0 (which means an error occurred), the file is not considered as a good script.
 * \param file Path of the file which has to be tested
 * \return 1 if the file may be regarded as a script, 0 otherwise
 */
int test_program(const char *file);

/********************************************/
/*           EXECUTION FUNCTIONS            */
/********************************************/
/**
 * \brief Execute a script with the help of an interpretor
 *
 * This function of the Program type executes a script as it would be done by a shell. It is assumed the file (script) starts with a shebang #!, then the path of the interpretor is written on the same first line. This interpretor is used to execute the script. The function spawns a new process that loads the interpretor. The standard output is redirected to the file which descriptor is given.
 * \param file Path of the script file
 * \param fd Descriptor of the file on which the output of the program will be written. The file should already be opened and ready to accept input
 * \return Error code of the external program after its execution
 */
int program_shell(const char *file,int fd);

/**
 * \brief Execute an external program and write its output on given file
 *
 * This function is a simple wrapper of the execute_program function and publishes it as a Program type. It executes the external program on the specified file and redirects its output on the file which descriptor is given.
 * \param file Path of the file on which the program will be executed. The file will be the last argument of the line which invokes the external program
 * \param fd Descriptor of the file on which the output of the program will be written. The file should already be opened and ready to accept input
 * \return Error code of the external program after its execution
 */
int program_external(const char *file,int fd);

/********************************************/
/*             OTHER OPERATIONS             */
/********************************************/
/**
 * \brief Tells if the file in argument is actually a script
 *
 * This function tests the file in argument and tells if it is a script. Since it is called very often (each time a folder is explored and a file is opened), it should be very fast and not really too much on external programs.
 * \param file Full path of the actual file
 * \return 1 if the file is a script, 0 otherwise
 */
int is_script(const char *file);

/**
 * \brief Spawn a process that executes an external program
 *
 * This function creates a new process which will execute the external program located at file. The second argument is a file descriptor on which the output will be written. If the descriptor is null, no output will be written at all.
 * \param file Path to the executable file
 * \param args Array of arguments to be added after the name of the program. The array must end with a null pointer. By convention, the first element of the array should be the path of the program itself but this function that not take care of adding the path of the program (file) at the beginning of the array.
 * \param fd Descriptor of the file on which the output will be redirected, 0 if no output is required
 * \return Error code of the program after the end of its execution
 */
int execute_program(const char *file,const char **args,int fd);

#endif   /* ----- #ifndef OPERATIONS_INC  ----- */
