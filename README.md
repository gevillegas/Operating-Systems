# COP4610
Project 1 - Implementing a Shell

Team: Franco Marcoccia, Elizabeth Cobb, Guillermo Villegas 

Division of Labor: (Prompt) Franco
                   (Shortcut Resolution) Elizabeth, Guillermo, Franco
                   ($PATH Resolution) Guillermo
                   (Execution) Franco
                   (I/O Redirection) Elizabeth, Guillermo
                   (Pipes) Guillermo, Franco
                   (Background Processing) Franco, Elizabeth
                   (Built-ins):
                               exit- Franco
                               cd - Guillermo
                               echo - Elizabeth
                               alias/unalias - Elizabeth

Date Submitted: 2/6/2019

Summary: a basic shell interface that supports input/output redirection, pipes, and a series of built in functions

How to Compile: 
 type make (in same directory where project1.c is)

How to Run: 
 ./main or main

How to Clean: 
 type make clean

How to Exit: 
 exit or Ctrl+C for force exit

Contents of Tar file:

1. README

2. Makefile- Compiles the program
    1. gcc: for compiling 
    2. CFLAGS: -I. -ansi -pedantic -Wall 
      -I. is included so that gcc will look in the current directory
      -ansi ansi language option 
      -pendantic generates more warnings 
      -Wall shows all warnings
    3. -g is included for valgrind
    4. -c is for compiling without linking
    5. -o is for renaming the executable 
    6. executable: main
    7. make clean removs project1.o and the executable file main

3. project1.c- source code

4. log.docx: Development Log- Documents progression of project over time in the form of a word document(screenshots from Github commits)

Bugs/Unfinished Portions:

All of the following bugs are runtime bugs:

1. Shortcut resolution ~ is only to be used on its own. Can't handle things such as ~/home/etc
2. alias's memory allocation is wrong and sometimes aborts the program- tried to fix with valgrind but still couldn't fix it. Alias also only works once. For example you set an alias then run that command, it works, then the second time that same alias is called it does not work and aborts the program. Again, I tried to use valgrind and I still don't know how to fix it. 
3. executables still work if adding as many '/' before the commands. Such as ////////ls. We do not know why this happens because the execv should not execute this but still does.
4. CMD > should be an error and is, but displays invalid file descriptor. Program still runs as long as you type another command.
5. Multiple redirection works for CMD < FILE > OUTPUT except it outputs it to the terminal when it isn't suppose to alongside a "no such    file or directory" message, but still outputs it to the file as well as intended. Redirection CMD > OUTPUT < FILE not implemented/working properly. 
6. Background Processing only handles error checking. Not fully implemented due to shortage of time.
7. echo $ should technically display a '$' but treats it as an invalid env variable.  This happens because our code checks the 2nd token for $ and displays the enviromental variable. 
8. echo with other implementations such as i/o will just treat it as an echo commands the way our code is set up. For example, echo $USER > apple.txt outputs "user" > apple.txt. This is due to the way our code is set up that checks for commands based off what is in index 0 of tokens.
9. Single pipes only work when it is the first command ran in the program. If not, it outputs a weird message. For example, doing ls then ls|wc won't work, but doing ls|wc right off the back works.
10. Two pipes not implemented- ran out of time.
11. There can be more than 10 Aliases. The error doesnt output when more than 10 are added. This is because how the strut is set up.

All of these bugs were either attempted to be fixed with valgrind when it dealt with memory leaks, were too complex to fix due to the way our code was structured, or didn't have enough time to keep working on certain parts.

Special considerations:

1. Foundfile and Founddir functions were based off stat structs from C reference websites.
2. The amount of commands ran used by exit include invalid commands.
3. Used the parsing provided to us (without add null).
4. Thanks to Leah for her massive help throughout the whole project.
