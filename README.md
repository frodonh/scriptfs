# scriptfs

ScriptFS is a new file system which merely replicates a local file system, but replaces all files detected as "scripts" by the result of their execution.

Any script is supported. In a usual way of working, ScriptFS reads on its command line the name of two programs : one that is the script interpretor, and the other one a lightweight program that tells fast if a file on the disk is a script that can be read by the first program.
