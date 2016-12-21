#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <time.h>
#include <pwd.h>
#include <unistd.h> /*for fork*/
#include <sys/types.h> /*for pid_t*/
#include <sys/wait.h> /*for wait*/
#define MAX_LINE_LENGTH 256

//Function used to display prompt and get input from user
char* getInput(char* prompt, char line[], int len) {
    printf ("%s\n", prompt);
    return fgets(line, len, stdin);
}

//Function to make substring from an existing string
void substring(char s[], char sub[], int p, int l) {
    int c = 0;
    while (c < l) {
        sub[c] = s[p+c-1];
        c++;
    }
    sub[c] = '\0';
}

int main() {
    //Global Variables
    char* prompt = "Prompt Instructions: ";
    char origDirectory[MAX_LINE_LENGTH];
    getcwd(origDirectory,sizeof(origDirectory));
    char prev[MAX_LINE_LENGTH];
    char word[MAX_LINE_LENGTH];
    int background = 0;
    int status = 0;
    
    //Variables for standard in and standard out
    int stdin_copy = dup(0);
    int stdout_copy = dup(1);
    
    //While loop to continuously ask user for input
    printf("To exit custom shell, input 'q' or 'Q' \n");
    while(getInput(prompt, word, sizeof(word))) {
    //Exit if input q
        if (toupper(word[0]) == 'Q' && word[1] == '\n')
            break;

        //Looping variables
        int i = 0;
        int j = 0;
        int k = 0;
        int size = strlen(word);
        word[size-1] = '\0';
        char inputLine[MAX_LINE_LENGTH][MAX_LINE_LENGTH]; 
        char temp[MAX_LINE_LENGTH];
        
        //Parsing the input
        while (i < size) {
            //Case if the character is a '
            if (word[i] == '\'') {
                j = i+1;
                while (word[j] != '\''){
                    ++j;
                }
                substring(word, temp, i+2, j-i-1);
                strcpy(inputLine[k], temp);
                ++k;
                i = j;
            }
            //Case if the character is a "
            else if (word[i] == '\"') {
                j = i+1;
                while (word[j] != '\"'){ 
                    ++j;
                }
                substring(word, temp, i+2, j-i-1);
                strcpy(inputLine[k], temp);
                ++k;
                i = j;
            }
            //Case if the character is a |
            else if (word[i] == '|') {
                strcpy(inputLine[k], "|");
                ++k;
            }
            //Case if the character is >
            else if (word[i] == '>') {
                strcpy(inputLine[k], "|");
                ++k;
                strcpy(inputLine[k], ">");
                ++k;
            }
            //Case if the character is <
            else if (word[i] == '<'){
                j = i+2;
                char temp2[MAX_LINE_LENGTH];
                while (word[j] != ' ' && j<size) {
                    ++j;
                }
                substring(word, temp2, i+3, j-i-2);
                FILE* file = fopen(temp2, "r");
                //Makes sure that the file existed
                if(file == NULL){
                    perror("Error");
                }
                else{
                    //If successful get input from the file
                    char line[MAX_LINE_LENGTH];
                    word[i] = ' ';
                    fgets(line, sizeof(line), file);
                    fclose(file);
                    strncpy(temp, word, i-1);
                    strcat(temp, " ");
                    strcat(temp, line);
                    
                    strcat(temp, word+j);
                    strcpy(word, temp);
                    --i;
                    size = strlen(word);
                }
            }
            //Case for other characters
            else if (word[i] == '.' || word[i] == '-' || word[i] == '/' || isalpha(word[i]) || isdigit(word[i])) {
                j = i+1;
                while (word[j] != ' ' && j<size) {
                    ++j;
                }
                substring(word, temp, i+1, j-i);
                strcpy(inputLine[k], temp);
                ++k;
                i = j;
            }
            else if (word[i] == '&'){
                background = 1;
            }
            ++i;
        }

        int b = 0;
        int fd[2];
        //Loop to execute the processes
        while(b < k){
            char* templine[MAX_LINE_LENGTH];
            int c = b;
            for(i = 0; i < k-c; ++i){
                if(strcmp(inputLine[b], "|") == 0) {
                    break; 
                }
                else if(strcmp(inputLine[b], ">") == 0) {
                        templine[i] = inputLine[b];
                        templine[i+1] = inputLine[b+1];
                        i = i+2;
                        break;
                    }
                else {
                    templine[i] = inputLine[b];
                }
                ++b;
            }
            b = i+c+1;
            templine[i] = NULL;
            pipe(fd);
            FILE* fds;
            
            //Fork another process
            pid_t pid = fork();
            //Case if its the child
            if (pid == 0) {
                //Checks if background process
                if(background){
                    fclose(stdin);
                    fopen("/dev/null", "r");
                    execvp(templine[0], templine);
                    exit(1);
                }
                else{
                    //Case if output to a file
                    if (strcmp(templine[0], ">") == 0) {
                        fds = freopen(templine[1], "w", stdout);
                        char* cmd[] = {"cat", NULL};
                        execvp(cmd[0],cmd);
                        fclose(fds);
                        exit(0);
                    }
                    //Case if cd or -t
                    else if(strcmp(templine[0], "cd") == 0 || strcmp(templine[0], "-t") == 0 || strcmp(templine[0], "custom") == 0){
                        exit(1);
                    }
                    //Case if there are more processes to be piped
                    else if (b < k-1) {
                        dup2(fd[1], 1);
                        close(fd[0]);
                        execvp(templine[0],templine);
                        perror("FAILED \n");
                        exit(1);
                    }
                    //Case if it is the last process or no pipe
                    else {
                        execvp(templine[0],templine);
                        perror("FAILED \n");
                        exit(1);
                    }
                }
            }
            //Case if it is the parent
            else{
                char cwd[256];
                //Checks if background process
                if(background){
                    background = 0;
                }
                else{
                    //Remove prompt in case of -t
                    if(strcmp(templine[0], "-t") == 0) {
                        prompt = ">>";
                        printf("%s\n", prompt);
                    }
                    //Custom function
                    else if(strcmp(templine[0], "custom") == 0){
                        time_t current_time;
                        char* c_time_string;
                        char *user = NULL;
                        current_time = time(NULL);
                        c_time_string = ctime(&current_time);
                        printf("Current time and date is: %s", c_time_string);
                        getcwd(cwd,sizeof(cwd));
                        printf("Current directory is: %s\n", cwd);
                        struct passwd *pw;
                        pw = getpwuid(geteuid());
                        if (pw)
                            user = pw->pw_name;
                        else if ((user = getenv("USER")) == NULL) {
                            fprintf(stderr, "I don't know!\n");
                            return 1;
                        }
                        printf("Username: %s\n", user);
                    }
                    //CD Function
                    else if(strcmp(templine[0], "cd") == 0){
                        //Case for just cd
                        if(templine[1] == NULL){
                            chdir(origDirectory);
                        }
                        //Case for cd /....
                        else if(templine[1][0] == '/'){
                            getcwd(prev,sizeof(prev));
                            strcat(cwd,templine[1]);
                            chdir(cwd);
                        }
                        //Case for cd ..
                        else if(strcmp(templine[1], "..") == 0){
                            getcwd(prev,sizeof(prev));
                            printf("%s\n", cwd);
                            chdir("..");
                            
                        }
                        //Case for cd -
                        else if(strcmp(templine[1], "-") == 0){
                            chdir(prev);
                        }
                        //Case for cd path_name
                        else{
                            getcwd(cwd,sizeof(cwd));
                            strcat(cwd, "/");
                            strcat(cwd,templine[1]);
                            chdir(cwd);
                        }
                    }
                    waitpid (pid, 0, 0);
                    dup2(fd[0], 0);
                    close(fd[1]);
                }
            }
        }
        //Reopen standard in and standard out
        dup2(stdin_copy, 0);
        dup2(stdout_copy, 1);
        
    }
    return 0;
}
