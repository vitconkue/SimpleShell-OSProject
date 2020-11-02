#include <string.h>
#include <stdio.h>
#include<stdlib.h>
int main () {
   char bash_cmd[256] = "ls";
   char buffer[1000];
   FILE *pipe;

   size_t len = 0;
   ssize_t read;
   char * line = NULL;
   pipe = popen(bash_cmd, "r");

   if (NULL == pipe) {
      perror("pipe");
      exit(1);
   } 
   while ((read = getline(&line, &len, pipe)) != -1) {     
        printf("%s", line);
   }
// fgets(buffer, sizeof(buffer), pipe);

    
// len = strlen(buffer);
// buffer[len-1] = '\0'; 

pclose(pipe);
printf("%s", buffer);
return (0);
}