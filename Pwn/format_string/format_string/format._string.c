#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

void shell(void)
{
    system("/bin/sh");
}

int main(void)
{
	setbuf(stdout, 0);
	printf("---------------------------------\n\n");
    printf("Welcome to zsctf2018. --zhakul\n\n");
	printf("---------------------------------\n\n");
    char buf[40];
	printf("please enter your name:\n");
    gets(buf);
    printf(buf);								//format string  Vulnerability
	printf("please enter your password:\n");
	gets(buf);          						//StackOverflow
	
	return 0;
}


//32位  开  NX   Canary
