#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <assert.h>
#include <stdlib.h>

int main(int argc, char ** argv)
{
   char *cmd = "python3";

   int par_to_py[2];
   int py_to_par[2];

   if(pipe(par_to_py) < 0) printf("par to py went wrong\n");
   if(pipe(py_to_par) < 0) printf("py to par went wrong\n");

   pid_t pid = fork();

   if(pid == 0){ // read par_to_py, write py_to_par
      dup2(par_to_py[0], 0); // stdin redireced to read end of pipe
      close(par_to_py[0]);
      close(par_to_py[1]);

      dup2(py_to_par[1],1); // stdout redirected to write end of pipe
      close(py_to_par[0]);
      close(py_to_par[1]);

      int f = open("/dev/null", O_WRONLY);
      dup2(f, 2);

      execlp(cmd, cmd, "-i", NULL);
   }
   else{
      close(par_to_py[0]);
      close(py_to_par[1]);
   }
   
   // parent writes to par_to_py[1]
   // parent reads from py_to_par[0]

   while(1){
      int n;
      int e;
      
      int check = scanf("%d %d", &n, &e);
      if(check < 0) break;
      char* str = (char*)calloc(1024, sizeof(char));
      snprintf(str, 1024, "(%d) ** (%d)\n", n, e);
      int length = 0;
      while(str[length]){
         length++;
      }
      char* res = (char*)calloc(1024, sizeof(char));
      write(par_to_py[1], str, length);
      read(py_to_par[0], res, 1024);

      printf("(%d) ** (%d) = %s", n, e, res);
      free(str);
      free(res);
   }
   close(par_to_py[1]);
   close(py_to_par[0]);
   int status;
   waitpid(pid, &status, 0);
   printf("Child exit status: %d\n", status);
   return 0;
}
