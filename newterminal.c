#include <stdio.h> 
#include <sys/types.h> 
#include <unistd.h> 
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include<fcntl.h> 
#define MAXLEN 80
#define _GNU_SOURCE
static char* historyCommand[100];
static int historyIndex = 0; 


void resetArgumentsList(char* inputArgs[], int numberOfArgument);
int parseToArgumentsList(char** argsList,char* inputCommand);
void execCommandWithArgumetsList(char** argumentList, int numberOfWord);
void execCommand(char* commmand);
void redirectOutput(char **argsList,int pos);
void redirectInput(char **argsList, int pos);
char *strcatOverride(char *a, char *b);
int strlenOverride(char *str);
int pipeline(char** argList, int numberOfArg);
int main()
{ 
    

    char* args[MAXLEN/2 + 1];


    int shouldRun = 1; 

    while (shouldRun)
    {
        printf("osh> "); 
        fflush(stdout); 

        char enteredCommand[MAXLEN+1]; 

        fflush(stdin); 
        scanf("%[^\n]s",enteredCommand);
        while(getchar() != '\n');
        execCommand(enteredCommand); 

        //printf("\nTest here\n"); 

    }
   

} 


void resetArgumentsList(char* inputArgs[], int numberOfArgument)
{
    for(int i=0; i < numberOfArgument; ++i)
    {
        free(inputArgs[i]); 
    }
    return; 
}

int parseToArgumentsList(char** argsList,char* inputCommand)
{
    //TODO: parse Commandto Arguments List
 
    char* ptrCommand = inputCommand;

    
    int argIndex = 0;

    char delimInOut = '>';

    while(*ptrCommand != '\0')
        {
            while(*ptrCommand == ' ')
            {
                ptrCommand++; 
            }
            argsList[argIndex] = (char*)malloc(sizeof(char)*MAXLEN);  
        
            int i =0; 
            while (*ptrCommand != ' ' && *ptrCommand != '\0')
            {
                *(argsList[argIndex] + i)  = *ptrCommand;
                ptrCommand++;
                i++; 
            }
            *(argsList[argIndex] + i)  = '\0';
            argIndex++; 
        }
       
        for (int i = 0;i<argIndex;i++){
            if(strcmp(argsList[i],">")==0){
                historyCommand[historyIndex] = (char*)malloc(sizeof(char) * MAXLEN); 
                strcpy(historyCommand[historyIndex++], inputCommand);
                redirectOutput(argsList,i);
                return -1;
            }
            else if(strcmp(argsList[i],"<")==0){
                historyCommand[historyIndex] = (char*)malloc(sizeof(char) * MAXLEN); 
                strcpy(historyCommand[historyIndex++], inputCommand);
                redirectInput(argsList, i);
                return -1;
            } 
        }
        return argIndex; 
}

void execCommandWithArgumetsList(char** argumentList, int numberOfWord)
{
    int childStatus;
    int willWait = 1; 
    if(strcmp(argumentList[numberOfWord-1], "&") == 0)
    {
        willWait =  0; 
        argumentList[numberOfWord-1] = NULL; 
    }
    else
    {
        argumentList[numberOfWord] = NULL ; 
    }
    pid_t pid = fork(); 
    if(pid == 0)
    {
        if(strcmp(argumentList[0],"!!")==0){
            if (historyIndex >= 1)
            {
                printf("%s\n", historyCommand[historyIndex-1]);
                execCommand(historyCommand[historyIndex-1]); 
            }
            else{
                printf("empty\n");
            }
        }
        else{
            execvp(argumentList[0], argumentList); 
        }
        _exit(EXIT_SUCCESS);
    }
    else if(pid == -1)
    {
        printf("\nUnable to create process\n"); 
    }
    else
    {
            // If last word is & => dont have to wait

        if(willWait)
        {
            wait( &childStatus);
        }
        else
        {
                
        }
        _exit(EXIT_SUCCESS);     
    }
}


void execCommand(char* command)
{
    char* args[MAXLEN/2 +1]; 
    int numberOfArg = parseToArgumentsList(args,command); 
    if(numberOfArg!=-1){
    execCommandWithArgumetsList(args, numberOfArg);
    //pipeline(args, numberOfArg);
    if(strcmp(command, "!!") != 0)
    {
        historyCommand[historyIndex] = (char*)malloc(sizeof(char) * MAXLEN); 

        strcpy(historyCommand[historyIndex++], command); 
    }
   

    }
    resetArgumentsList(args, numberOfArg); 
}
void redirectOutput(char** argsList,int pos){
    FILE *fout;
    char *command = "\0";
    char *filename = argsList[pos + 1];
    for (int i = 0; i < pos; i++)
    {
        command = strcatOverride(command,argsList[i]);
        if(pos>1&&i<pos-1){
           command = strcatOverride(command," ");
        }
    }
    fout = fopen(filename, "w");
    int file_desc = open(filename,O_WRONLY); 
    int saved_stdout;
    //Save stdout for terminal
    saved_stdout = dup(1);
    //Dup output file to write
    dup2(file_desc, 1) ;           
    execCommand(command);
    fclose(fout);
    //Back to output terminal
    dup2(saved_stdout, 1);
    close(saved_stdout);
}

void redirectInput(char **argsList, int pos){
    char *filename = argsList[pos + 1];
    char *command = "\0";
    for (int i = 0; i < pos; i++)
    {
        command = strcatOverride(command,argsList[i]);
        command = strcatOverride(command," ");
    }
    command = strcatOverride(command, filename);
    execCommand(command);
}

char *strcatOverride(char *a, char *b) {
    int i = 0, j = 0;
    int count = 0;
    int lenA = strlenOverride(a);
    int lenB = strlenOverride(b);
    int desLen = lenA+lenB + 1;
    char *result = (char*)malloc(desLen * sizeof(char));

    for(i = 0; i < lenA; i++) {
        result[i] = a[i];
    }

    for (j = i; j < lenA +lenB; j++)
    {
         result[j] = b[count++];
    }
    result[desLen - 1] = '\0';
    return result;
}

int strlenOverride(char *str){
    int len = 0;
    while(str[len]){
        len++;
    }
    return len;
}

int pipeline(char** argList, int numberOfArg){
    for (int i = 0; i < numberOfArg; i++ )
        if (strcmp(argList[i], "|") == 0){
            int parent_n_of_arg = i;
            argList[i] = NULL;
            int fd[2];
            char str[100];
            pid_t p;
            int saved_stdout;
        
            if (pipe(fd)==-1) 
            { 
                printf("Pipe Failed" ); 
                return 1; 
            } 
            int parent_done = 0;
            p = fork(); 
            if (p < 0){
                printf("\nUnable to create process\n"); 
                return 1;
            } else if (p > 0) {
                //parent
                close(fd[0]);
                //saved_stdout = dup(1);
                //dup2(fd[1], 1);
                //execvp(argList[0], argList);  
                execCommandWithArgumetsList(argList, parent_n_of_arg); 
                parent_done = 1;
                printf("parent");
                fflush(stdout);
            } else {
                //child
                printf("child1");
                fflush(stdout);
                while (parent_done != 1)
                {
                }
                printf("adasdas");
                fflush(stdout);
                /* char output[100];
                read(fd[0], output, 100);        
                char* command = "\0";
                for (int j = i + 1; j < numberOfArg; j++)
                {
                    command = strcatOverride(command,argList[j]);
                    command = strcatOverride(command," ");
                }
                command = strcatOverride(command, output);
                char** child_command;
                int child_n_of_arg = parseToArgumentsList(child_command, command);
                pipeline(child_command, child_n_of_arg); */
            }
            //dup2(saved_stdout, 1);
            //close(saved_stdout);
            return 0;
        }
}