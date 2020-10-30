#include <stdio.h> 
#include <sys/types.h> 
#include <unistd.h> 
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#define MAXLEN 80
int main() 
{ 
    

    char* args[MAXLEN/2 + 1];

    int shouldRun = 1; 
    
    while(shouldRun)
    {

        int childStatus;
        printf("osh> "); 
        fflush(stdout);


        char enteredCommand[MAXLEN + 1];
        char* ptrEnteredCommand = enteredCommand; 


        fflush(stdin); 
        scanf("%[^\n]s",enteredCommand);
        while(getchar() != '\n');

        int argIndex = 0 ; 
        // parse
        while(*ptrEnteredCommand != '\0')
        {
            while(*ptrEnteredCommand == ' ')
            {
                ptrEnteredCommand++; 
            }
            args[argIndex] = (char*)malloc(sizeof(char)*MAXLEN);  
        
            int i =0; 
            while (*ptrEnteredCommand != ' ' && *ptrEnteredCommand != '\0')
            {
                *(args[argIndex] + i)  = *ptrEnteredCommand;
                ptrEnteredCommand++;
                i++; 
            }

            //printf("%s\n", args[argIndex]);
        
            argIndex++; 
        }

        args[argIndex] = NULL ;

        //

        pid_t pid = fork(); 
        if(pid == 0)
        {
            execvp(args[0], args); 
        }
        else if(pid == -1)
        {
             printf("\nUnable to create process\n"); 
        }
        else
        {
            wait( &childStatus); 
        }
        
        

    }

    

   

    
    


} 
