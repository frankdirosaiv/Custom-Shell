Partner 1: Briana Martinez
UIN 1: 124001108
Partner 2: Frank DiRosa
UIN 2: 223008529


To compile our shell program use the makefile provided. It will make an executable called shell. It was compiled using the linux.cse.tamu.edu.

Our shell programs implements all of the functions that were required. It can pipe an infinite amount of times. It also supports input and output to a file. If the input file does not exist, it will throw an error. If trying to output to a file, if the file does not already exist, it will make it. Using the & will allow the process to run in the background, hitting the enter button will take you to prompt line and you can input another command. Whenever the background process is done running, it will print the result to the screen.

cd: By simply typing cd, it will take you back to the original directory of the program. Hence it will be the folder native to the program. The rest of the cd functions will function the same as a normal terminal. 

-t mode: Since there were not any examples of how -t would be called, it was assumed that it would always just be its own line. If it is called, the shell will no longer display the prompt message.

BONUS:

Custom function for bonus: The custom function is simply titled “custom”. By typing that in the shell, it will display the current time and date, username, and current directory.  

Our function is able to handle double quotes without the contents between the quotes being tokenized.