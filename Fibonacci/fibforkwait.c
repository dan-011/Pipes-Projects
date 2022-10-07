#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <sys/wait.h>

int fib(int n){
	if (n <= 1)
		return n;
	else return fib(n-1) + fib(n-2);
}

int main(void){
	pid_t value = fork();
	int i;
	if (value == 0){
		for (i = 0; i < 16; i++)
			printf("fib(%2d) = %d\n", i*3, fib(i*3));
	}
	else{
		long begin = time(NULL);
		for (i = 0; i < 10; i++){
			sleep(1);
			printf("Elapsed time in parent: %ld\n", time(NULL)-begin);
		}
		int exitStatus;
		pid_t deadChild = wait(&exitStatus); // exitStatus picks up the exit status of the child, and deadChild holds the identifier of the child who died
		printf("Child %d died\n", deadChild);
		printf("Exited normally? [%s] with status %d\n", WIFEXITED(exitStatus) ? "yes" : "no", WEXITSTATUS(exitStatus)); // macro WIFEXITED returns 0 or 1 if the child exited normally, macro WEXITSTATUS returns what the child returned
	}
	return 0;
}
