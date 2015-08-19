# Introduction #

In this example session, a file system is mounted over itself with ScriptFS. In the first part, all the executable (shell-executable or real executable) files are replaced by their output. In the second part, a filter (BASH script) is applied to all files, prefixing each line with its number.


# Example session, first part #

```
hissel@xxx:~/dev/scriptfs$ ls -l test
total 20
-rwxr--r-- 1 hissel francois   46 juil. 30 17:51 filter
-rwxr-xr-x 1 hissel francois 7159 juil. 30 17:51 hello_c
-rwxr--r-- 1 hissel francois   33 août   7 13:45 hello_script
-rw-r--r-- 1 hissel francois   16 août   7 13:45 hello_text

hissel@xxx:~/dev/scriptfs$ cat test/hello_text
Hello everybody

hissel@xxx:~/dev/scriptfs$ cat test/hello_script
#! /bin/bash

echo "Hello world"

hissel@xxx:~/dev/scriptfs$ test/hello_c
Bonjour C

hissel@xxx:~/dev/scriptfs$ ./scriptfs -o nonempty -p auto test test

hissel@xxx:~/dev/scriptfs$ ls -l test
total 20
-r-xr--r-- 1 hissel francois   46 juil. 30 17:51 filter
-r-xr-xr-x 1 hissel francois 7159 juil. 30 17:51 hello_c
-r-xr--r-- 1 hissel francois   33 août   7 13:45 hello_script
-rw-r--r-- 1 hissel francois   16 août   7 13:45 hello_text

hissel@xxx:~/dev/scriptfs$ cat test/hello_text
Hello everybody

hissel@xxx:~/dev/scriptfs$ cat test/hello_script
Hello world

hissel@xxx:~/dev/scriptfs$ cat test/hello_c
Bonjour C

hissel@xxx:~/dev/scriptfs$ fusermount -u test
```

# Example session, second part #

```
hissel@xxx:~/dev/scriptfs$ cat mirror/filter
#!/bin/bash

awk -F "\n" '{print FNR,$0;}' $1

hissel@xxx:~/dev/scriptfs$ ./scriptfs -o nonempty -p "/home/hissel/dev/scriptfs/mirror/filter" test test

hissel@xxx:~/dev/scriptfs$ ls -l test
total 20
-r-xr--r-- 1 hissel francois   46 juil. 30 17:51 filter
-r-xr-xr-x 1 hissel francois 7159 juil. 30 17:51 hello_c
-r-xr--r-- 1 hissel francois   33 août   7 13:45 hello_script
-r--r--r-- 1 hissel francois   16 août   7 13:45 hello_text

hissel@xxx:~/dev/scriptfs$ cat test/hello_text
1 Hello everybody

hissel@xxx:~/dev/scriptfs$ cat test/hello_script
1 #! /bin/bash
2 
3 echo "Hello world"

hissel@xxx:~/dev/scriptfs$ cat test/filter
1 #!/bin/bash
2 
3 awk -F "\n" '{print FNR,$0;}' $1

hissel@xxx:~/dev/scriptfs$ fusermount -u test
```