#include<iostream>
#include<string>
using namespace std;

char mu[] = { 0xd7,0xd5,0xa3,0xd9,0xda,0xa9,0xd8,0xe4,0xad,0xf5,0xc8,
0xa5,0xf5,0xde,0xbe,0xcf,0xd3,0x98,0xd1,0xdd,0xb8,0xc9,0xc8,0xb6 };
char x[3] = { 0xAA,0xBB,0xCC };

void cr(char *flag,char *x)
{
	int first, last;
	for (first = 0, last = strlen(flag)-1; first < last; first++, last--)
	{
		flag[first] ^= flag[last];
		flag[last] ^= flag[first];
		flag[first] ^= flag[last];
	}
	for (first = 0,last=0; first < strlen(flag); first++,last++)
	{
		if (last >= 3)
			last = 0;
		flag[first] ^= x[last];
	}
}

bool cmp(char *flag)
{
	int i;
	for (i = 0; i < 24; i++)
	{
		if (flag[i] != mu[i])
			return 0;
	}
	return 1;
}

int main(void)
{
	char flag[50];
	int i;
	printf("welcome to zsctf!\n");
	printf("Input your flag:");
	scanf_s("%s", flag,50);
	for (i = 0; i < strlen(flag); ++i)
	{
		if (flag[i] >= 'A'&&flag[i] <= '}')
			continue;
		printf("error flag\n");
		system("pause");
		return 0;
	}

	cr(flag,x);
	
	if (cmp(flag))
		printf("Congratulation\n");
	else
		printf("error flag\n");
	system("pause");
	return 0;
}