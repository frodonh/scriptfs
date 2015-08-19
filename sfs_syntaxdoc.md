# Command line #

The new filesystem is mounted with the following command: `scriptfs [options] mirror_path mountpoint`

# Mandatory arguments #

`mirror_path`
> Path of the original folder which will be replicated in the virtual file system
`mountpoint`
> Path of the mount point folder. The folder should be empty before using the command, otherwise existing files will not be accessible any longer.


# Optional arguments #

Optional arguments are used to define which files have to be considered as scripts and what to do with them. Two types of executable files should be provided:
  * **Program files** define the program that has to be called on each detected script file to transform its contents;
  * **Test files** are also executable files that point to programs used to detect if a file is a script.


For each script file detected as such, the program file is executed and its standard output is captured and sent back as the content of the virtual file (replacing the content of the original script file on the mirror file system). Standard error is left unchanged and written to the standard error of the filesystem program (thus enabling to log errors on a file).

`-p program[;test]`
> Define an executable program and a corresponding test program to use. The command may be repeated several times to define other executable programs. When this is the case, each description will be used in the order they are defined to detect if the file is a script. As soon as the file is detected by a script, the corresponding program is executed on it. The other remaining definitions are not used. `program` can be either of the following string.
  * Full command line. If `program` is a full shell command-line (starting with the name of an executable program, with arguments), the corresponding program, located by the first word on the command-line is used on each script file, detected as such by the test program. All the arguments are used as they are written. If the command-line holds the "!" character, it is replaced by the name of the script file. If no such character is found, the content of the script file is provided as the standard input of the external program.
  * `auto`. When the `auto` string is found, the filesystem behaves almost as would a standard shell do, that is each file is read to find if it is a proper executable script (starting with a shebang `#!`) or an executable program. No test program has to be provided. If the file is a shell script, the string after `#!` defines the path of the executable program that will be launched to execute the content of the script.


`test` is an optional part of the description and can be one of the following:
  * Full command line. The behaviour is similar to the one used when `program` is a full command-line. The command-line is used on each file to detect if it is a script. All the arguments are used as they are written. If the command-line holds the "!" character, it is replaced by the name of the file. If no such character is found, the content of the file is provided as the standard input of the test program. The standard output of the test program is discarded. Since the test program is executed on every file on the filesystem, it should be quite fast. A file is recognized as a script if the exit code of the test program is zero (normal exit). Otherwise, it is not considered as a script.
  * `always`. When the `always` string is read, all the files in the virtual file system are considered as script files.
  * `executable`. When the `executable` string is found, only files that the current user can execute are considered as script files.
  * Pattern. A pattern is an expression which starts with the '&' character. The full name of the file (including the path) is tested against the pattern and if it matches, the file is considered as a script file.


If no test procedure is provided and the program procedure is a full command-line, the same command-line will be used for the test program. Thus every file will first be executed to detect if they should be regarded as script files. If the program procedure is `self`, and no test procedure is provided, the `executable` mode will be used for the test procedure, and only executable files will be considered as script files.


When no procedure (`-p`) is set, the program behaves as is only one procedure `-p auto` was used.

See also [Example session](sfs_example.md)