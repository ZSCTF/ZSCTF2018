#include<unistd.h>
#include<stdio.h>
#include<seccomp.h>
#include<linux/seccomp.h>
#include<stdlib.h>
void init_seccomp(void)
{
	scmp_filter_ctx ctx= seccomp_init(SCMP_ACT_ALLOW);
	seccomp_rule_add(ctx, SCMP_ACT_KILL, SCMP_SYS(execve), 0);
	seccomp_load(ctx);
}
char shellcode[100];
void init(void)
{
	setbuf(stdout, 0);
	setbuf(stdin, 0);
	init_seccomp();
}
void here(void)
{
	char buf[40];  
	write(1, "Input:\n", 7);
	read(0,buf,200);
}
int main(void)
{
	init();
    here();
	return 0;
}

//开 NX
//gcc -no-pie -o rop rop.c -lseccomp
