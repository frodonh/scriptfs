ScriptFS is a new file system which merely replicates a local file system, but replaces all files detected as "scripts" by the result of their execution.

Any script is supported. In a usual way of working, ScriptFS reads on its command line the name of two programs : one that is the script interpretor, and the other one a lightweight program that tells fast if a file on the disk is a script that can be read by the first program.

If no script interpretor is provided, ScriptFS reads the first line of a file and considers it is a script if it starts with #! (like Bash). Then the appropriate script interpretor is called.

Currently, scripts are read-only. Any other file will behave exactly like its counterpart on the local folder.

For additional help about the command-line syntax to mount a filesystem, see [Documentation of syntax](sfs_syntaxdoc.md). Also see an example session [here](sfs_example.md).