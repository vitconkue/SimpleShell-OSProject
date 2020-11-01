#include <stdio.h> 
#include <sys/types.h> 
#include <unistd.h> 
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#define MAXLEN 80

static char* historyCommand[100];
static int historyIndex = 0; 


void resetArgumentsList(char* inputArgs[], int numberOfArgument);
int parseToArgumentsList(char** argsList,char* inputCommand);
void execCommandWithArgumetsList(char** argumentList, int numberOfWord);
void execCommand(char* commmand);


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

            //printf("%s\n", args[argIndex]);
        
            argIndex++; 
        }

    // parse

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
                    
    }
}


void execCommand(char* commmand)
{
    char* args[MAXLEN/2 +1]; 
    int numberOfArg = parseToArgumentsList(args,commmand); 

    execCommandWithArgumetsList(args, numberOfArg);

    if(strcmp(commmand, "!!") != 0)
    {
        historyCommand[historyIndex] = (char*)malloc(sizeof(char) * MAXLEN); 

        strcpy(historyCommand[historyIndex++], commmand); 
    }

 

    resetArgumentsList(args, numberOfArg); 

    
}
