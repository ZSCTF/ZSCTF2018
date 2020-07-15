#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<string.h>
#include <unistd.h>
int main()
{
	int n=1,time_check=99,done=0,str_right=1,tttt;
	setvbuf(stdout, NULL, _IOLBF, 0);
	int p[1024];
	char input[120];
	srand(time(0));
	printf("Welcome to zsctf2018\n");
	printf("这里有个好玩的游戏，通关了就有 flag 了\n");
	printf("有 n 个数，a1,a2,a3...an，其中 an ∈ [0,1024)，你的目标是找出这 n 个数分别是什么");
	printf("你可以通过发送格式 \"? l r\" 0 <= l < r < 1024的字串来查询在区间 [l,r) 内存在多少个你需要找到的数,每关你拥有99次查询机会\n");
	printf("当你已经确认了所有数，通过格式 \"! a1 a2 a3 ... an\" 来提交你的答案\n");
	printf("那我们开始吧\n");
	while (n < 15)
	{
		time_check = 99;
		done = 0;
		printf("n=%d\n", n);
		memset(p, 0, sizeof(p));
		for (int i = 0; i < n; ++i)
		{
			tttt = rand() % 1024;
			p[tttt] = 1;
		}
		while (time_check)
		{
			memset(input, 0, sizeof(input));
			fgets(input, 100, stdin);
			if (strlen(input) < 4)
			{
				printf("len error\n");
				return 0;
			}
			if (input[0] == '?' && input[1] == ' ')
			{
				char *str = &input[2];
				char number1[10] = { 0 };
				char number2[10] = { 0 };
				int tmp = 0,number_1,number_2,n_num=0;
				while (*str != ' ')
				{
					if (*str<'0' || *str>'9')
					{
						printf("格式错误，请重新输入\n");
						return 0;
					}
					number1[tmp++] = *str;
					if (tmp > 9)
					{
						printf("小于 1024\n");
						return 0;
					}
					++str;
				}
				++str;
				tmp = 0;
				while (*str != '\n')
				{
					if (*str<'0' || *str>'9')
					{
						printf("格式错误，请重新输入\n");
						return 0;
					}
					number2[tmp++] = *str;
					if (tmp > 9)
					{
						printf("小于 1024\n");
						return 0;
					}
					++str;
				}
				number_1 = atoi(number1);
				number_2 = atoi(number2);
				if (number_1 >= 1024 || number_2 >= 1024)
				{
					printf("老哥越界了 :P\n");
					return 0;
				}
				for (int i = number_1; i < number_2; ++i)
					if (p[i] == 1)
						++n_num;
				printf("%d\n", n_num);
			}
			else if (input[0] == '!' && input[1] == ' ')
			{
				char *str = &input[2];
				char number[10];
				int n_num=0,tmp=0,num;
				memset(number, 0, sizeof(number));
				while (*str != '\n'&& *str != 0)
				{
					while (*str != ' ' && *str != '\n' && *str != 0)
					{
						if (*str<'0' || *str>'9')
						{
							printf("格式错误，请重新输入\n");
							return 0;
						}
						number[tmp++] = *str;
						if (tmp > 9)
						{
							printf("小于 1024\n");
							return 0;
						}
						++str;
					}
					++str;
					tmp = 0;
					num = atoi(number);
					if (num >= 1024)
					{
						printf("老哥越界了 :P\n");
						return 0;
					}
					p[num] = !p[num];
					memset(number, 0, sizeof(number));
				}
				int k = 0;
				for (k = 0; k < 1024; ++k)
				{
					if (p[k] != 0)
					{
						done = 0;
						break;
					}
				}
				if (k == 1024)
				{
					done = 1;
					break;
				}
				else {
					printf("Try again(:3)\n");
					return 0;
				}
			}
			else
			{
				printf("格式错误，请重新输入\n");
			}
			--time_check;
			if (!time_check)
			{
				printf("查询机会用尽，告辞\n");
				return 0;
			}
		}
		if (n >= 2)
			n += 2;
		else ++n;
	}
	if (done)
		printf("Done!!!The flag is zsctf{Y0u_win_th1s_One_Bin5ry_Sea6ch}\n");
	else printf("Try again(:3)\n");
	return 0;
}