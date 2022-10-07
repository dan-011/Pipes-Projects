#include <stdio.h>
#include <unistd.h>
#include <time.h>

int fib(int n){
	if (n <= 1)
		return n;
	else return fib(n-1) + fib(n-2);
}

int main(void){
	pid_t value = fork();
	int i;
	if (value == 0){
		for (i = 0; i < 30; i++)
			printf("fib(%2d) = %d\n", i*5, fib(i*5));
	}
	else{
		long begin = time(NULL);
		for (i = 0; i < 10; i++){
			sleep(1);
			printf("Elapsed time in parent: %ld\n", time(NULL)-begin);
		}
	}
	return 0;
}
