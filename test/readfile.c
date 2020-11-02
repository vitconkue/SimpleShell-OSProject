#define _GNU_SOURCE
#include <stdio.h>
#include<stdlib.h> 
#include<unistd.h> 
#include<stdio.h> 
#include<fcntl.h> 
  
int main() 
{ 
    int file_desc = open("text.txt",O_WRONLY | O_APPEND); 
      
    dup2(file_desc, 1) ;  
          

    FILE *pipe;

   size_t len = 0;
   ssize_t read;
   char * line = NULL;
   pipe = popen("ls" ,"r");

   if (NULL == pipe) {
      perror("pipe");
      exit(1);
   } 
   while ((read = getline(&line, &len, pipe)) != -1) {     
        printf("%s", line);
   }

    return 0; 
} 