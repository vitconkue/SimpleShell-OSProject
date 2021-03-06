#include <stdio.h> 
#include <sys/types.h> 
#include <unistd.h> 
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include<fcntl.h> 
#define MAXLEN 80
#define _GNU_SOURCE
static char *historyCommand[500] = {"\0"};
static int historyIndex = 0;
static int isRedirectInput = 0;
static int isRedirectOutput = 0;

void resetArgumentsList(char *inputArgs[], int numberOfArgument);
int parseToArgumentsList(char** argsList,char* inputCommand);
void execCommandWithArgumetsList(char** argumentList, int numberOfWord);
void execCommand(char* commmand);
void redirectOutput(char **argsList,int pos);
void redirectInput(char **argsList, int pos);
void execHistoryCmd();
int pipeExec(char **argsList, int index);
int checkHistoryCmdExec(char *command);
void execCommandAtPos(char *command);
void execMostRecentCommand(); 
char *appendHistoryCommand(char *history);
char *strcatOverride(char *a, char *b);
char *getParameters(char *str, int pos,int len);
int strlenOverride(char *str);
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
    exit(0);
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
            printf("Child process still running\n");
           //_exit(EXIT_SUCCESS); 
        }
              
    }
}


void execCommand(char* command)
{
    int flag = 0;
    char *argsList[MAXLEN / 2 + 1];
    int numberOfArg = parseToArgumentsList(argsList,command);
    //Get last index of history array;
    int lastIndex = historyIndex > 0 ? historyIndex - 1 : 0;
     for(int i = 0; i < numberOfArg; i++){
        if (strcmp(argsList[i], "|") == 0){
            if(historyCommand[lastIndex]=="\0"||strcmp(historyCommand[lastIndex], command)){
                historyCommand[historyIndex] = (char*)malloc(sizeof(char) * MAXLEN);       
                strcpy(historyCommand[historyIndex++], command);
            }
            pipeExec(argsList, i);
            flag = 1;
            break;  
        }
    }
    for (int i = 0; i < numberOfArg; i++)
    {
        if(strcmp(argsList[i],">")==0){
            //If history is empty or previous not same at current command. 
            //Add current command to history and copy current command to previous command//
            
            if (historyCommand[lastIndex] == "\0" || strcmp(historyCommand[lastIndex], command))
            {
                historyCommand[historyIndex] = (char*)malloc(sizeof(char) * MAXLEN);       
                strcpy(historyCommand[historyIndex++], command);
            }
            //Change flag status
            flag = 1;
            redirectOutput(argsList,i);
        }
        else if(strcmp(argsList[i],"<")==0){
          //Same up there
            if(historyCommand[lastIndex]=="\0"||strcmp(historyCommand[lastIndex], command)){
                historyCommand[historyIndex] = (char*)malloc(sizeof(char) * MAXLEN);       
                strcpy(historyCommand[historyIndex++], command);
            }
            flag = 1;
            redirectInput(argsList, i);    
        }
    }
    if(strcmp(command,"history") == 0){
        if(historyCommand[lastIndex]=="\0"||strcmp(historyCommand[lastIndex], command)){
            historyCommand[historyIndex] = (char*)malloc(sizeof(char) * MAXLEN);       
            strcpy(historyCommand[historyIndex++], command);
        }
        flag = 1;
        execHistoryCmd();
    }
    else if (strcmp(command,"!!") == 0)
    {
        flag = 1;
        execMostRecentCommand();
    }
    else if(strcmp(command, "exit") == 0 )
    {
        exit(0);
    }
    else if(checkHistoryCmdExec(command)){
        flag = 1;
        execCommandAtPos(command);
    }
    //Same up there
    if(historyCommand[lastIndex]=="\0"){
      historyCommand[historyIndex] = (char*)malloc(sizeof(char) * MAXLEN);
      strcpy(historyCommand[historyIndex++], command);
    }
    else if(strcmp(command, "!!") != 0&&isRedirectInput==0&&isRedirectOutput==0&&strcmp(historyCommand[lastIndex], command))
    {
        historyCommand[historyIndex] = (char*)malloc(sizeof(char) * MAXLEN);
        strcpy(historyCommand[historyIndex++], command);
    }
   
    //If flag equals to 0 exec system command normal way
    if(flag==0){
        execCommandWithArgumetsList(argsList,numberOfArg);
        resetArgumentsList(argsList, numberOfArg);
    }
}
void redirectOutput(char** argsList,int pos){
    FILE *fout;
    //Change redirect status (for not add command output to history)
    isRedirectOutput = 1;
    char *command = "\0";
    char *filename = argsList[pos + 1];
    for (int i = 0; i < pos; i++)
    {
        command = strcatOverride(command,argsList[i]);
        if(pos>1&&i<pos-1){
           command = strcatOverride(command," ");
        }
    }
    //Open file to write (auto create if there is no file)
    fout = fopen(filename, "w");
    int file_desc = open(filename,O_WRONLY); 
    int saved_stdout;
    //Save stdout for terminal
    saved_stdout = dup(1);
    //Dup output file to write
    dup2(file_desc, 1) ;
    execCommand(command);
    fclose(fout);
    close(file_desc);
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
        if(pos>1&&i<pos-1){
           command = strcatOverride(command," ");
        }
    }
    int saved_stdout;
    saved_stdout = dup(0);
    int fin = open(filename, O_RDONLY);
    //If cant open file
    if(fin < 0){
        printf("bash: %s: No such file or directory\n", filename);
    }
    else{
        isRedirectInput = 1;
        //Dupplicate to get input from file
        dup2(fin, STDIN_FILENO);
        //Execute command with input from file
        execCommand(command);
    }
    //Close file
    close(fin);
    dup2(saved_stdout, 0);
    close(saved_stdout);
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

void execHistoryCmd(){
    for (int i = 0; i < historyIndex;i++){
        printf("%d %s\n", i + 1, historyCommand[i]);
    }
}

void execMostRecentCommand()
{
    int lastIndex = historyIndex - 1;
    //If history not empty exec last command in history command
    if (lastIndex >= 0)
    {
        printf("%s\n", historyCommand[lastIndex]);
        execCommand(historyCommand[lastIndex]);
    }
    else
    {
        fflush(stdout); 
        printf("No command in history\n"); 
    }
    
}
void execCommandAtPos(char *command){
    int index = 0;
    int stopPosition = 0;
    char *parameters = "\0";
    int len = strlenOverride(command);
    for (int i = 1; i < len;i++){
        int temp = (int)command[i] - 48;
        if (temp<0||temp>9)
        {
            stopPosition = i;
            break;
        }
    }
    if(stopPosition==0){
        int temp = (int)command[1];
        temp = temp - 48;
        index += temp;
    }
    else if(stopPosition!=1)
    {
        parameters = getParameters(command, stopPosition, len);
  
        for (int i = 1; i < stopPosition; i++)
        {
            int temp = (int)command[i];
            temp = temp - 48;
            index += temp;
            if (i < stopPosition - 1)
            {
                index *= 10;
            }
            
        }
    }
    if(index>historyIndex||index==0){
        if(stopPosition==1){
            printf("bash: %s: event not found\n",command);
        }
        else{
            printf("bash: %d: event not found\n",index);
        }
    }
    else{
        char *newCommand = strcatOverride(historyCommand[index - 1], parameters);
        printf("%s\n", newCommand);
        execCommand(newCommand);
    }
}

char *getParameters(char *str, int pos,int len){
    int resultLen = len - pos;
    char *result = (char *)malloc(resultLen* sizeof(char));
    for (int i = 0; i < resultLen;i++){
        result[i] = str[i + pos];
    }
    return result;
}

int checkHistoryCmdExec(char *command){
    if(command[0]=='!' && command[1] != '\0'){
        return 1;
    }
    return 0;
}

int pipeExec(char** argsList, int index){
    char* temp = argsList[index];
    argsList[index] = NULL;
    int fd[2];
    if (pipe(fd)==-1) { 
        printf("Pipe Failed" ); 
        return 1; 
    } 
    pid_t p = fork();
    if (p < 0){
        printf("\nUnable to create process\n"); 
        argsList[index] = temp;
        return 1;
    } else if (p == 0) {
        dup2(fd[1], STDOUT_FILENO); //Put output to pipe
        close(fd[0]);
        close(fd[1]);
        execvp(argsList[0], argsList);
        //child
    }
    pid_t p2 = fork();
    if (p2 < 0){
        printf("\nUnable to create process\n"); 
        argsList[index] = temp;
        return 1;
    } else if (p2 == 0){
        dup2(fd[0], STDIN_FILENO);
        close(fd[0]);
        close(fd[1]);
        execvp(argsList[index + 1], &argsList[index + 1]);
        exit(EXIT_SUCCESS);
    }
    close(fd[0]);
    close(fd[1]);
    waitpid(p, NULL, 0);
    waitpid(p2, NULL, 0);
    argsList[index] = temp;
    return 0;
}