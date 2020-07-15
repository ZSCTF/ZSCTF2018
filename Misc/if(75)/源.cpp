#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#ifdef _MSC_VER
#include <intrin.h>
#pragma optimize("gt", on)
#else
#include <x86intrin.h>
#endif

#ifndef _MSC_VER
#define sscanf_s sscanf
#endif
#define CACHE_HIT_THRESHOLD (80) 
unsigned int array1_size = 16;
uint8_t unused1[64];
uint8_t array1[160] = { 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16 };
uint8_t unused2[64];
uint8_t array2[256 * 512];
uint32_t index[50] = {0};
char* secret = "Welcome to zsctf2018. --Docupa";
uint8_t temp = 0; 
void function(size_t x)
{
	if (x < array1_size)
	{
		temp &= array2[array1[x] * 512];
	}
}
void give_me_flag(char *flag)
{
	for (size_t i = 0; i < 20; i++)
	{
		size_t tmp = rand();
		index[i] = (*(flag + i) - tmp % 20 - 100) ? 50000 : index[i] - (*(flag + i) - tmp % 20 - 100);
	}
}
void readM(size_t m_x, uint8_t value[2], int score[2])
{
	static int results[256];
	int tries, i, j, k, mix_i;
	unsigned int junk = 0;
	size_t t_x, x;
	register uint64_t time1, time2;
	volatile uint8_t* addr;
	for (i = 0; i < 256; i++)
		results[i] = 0;
	for (tries = 999; tries > 0; tries--)
	{
		for (i = 0; i < 256; i++)
			_mm_clflush(&array2[i * 512]);
		t_x = tries % array1_size;
		for (j = 29; j >= 0; j--)
		{
			_mm_clflush(&array1_size);
			for (volatile int z = 0; z < 100; z++){}
			x = ((j % 6) - 1) & ~0xFFFF; 
			x = (x | (x >> 16)); 
			x = t_x ^ (x & (m_x ^ t_x));
			function(x);
		}
		for (i = 0; i < 256; i++)
		{
			mix_i = ((i * 167) + 13) & 255;
			addr = (&array2[mix_i * 512])+ index[i%20];
			time1 = __rdtscp(&junk);
			junk = *addr;
			time2 = __rdtscp(&junk) - time1;
			if (time2 <= CACHE_HIT_THRESHOLD && mix_i != array1[tries % array1_size])
				results[mix_i]++;
		}
		j = k = -1;
		for (i = 0; i < 256; i++)
		{
			if (j < 0 || results[i] >= results[j])
			{
				k = j;
				j = i;
			}
			else if (k < 0 || results[i] >= results[k])
			{
				k = i;
			}
		}
		if (results[j] >= (2 * results[k] + 5))
			break;
	}
	results[0] ^= junk; 
	value[0] = (uint8_t)j;
}

int main(int argc, const char* * argv)
{
	srand(0xc);
	size_t m_x = (size_t)(secret - (char *)array1);
	int score[2], len = strlen(secret);
	uint8_t value[2];
	char flag[50] = {0},ans[50] = {0};
	printf("input your flag:");
	scanf("%22s", flag);
	if (strlen(flag) != 20)
	{
		printf("false\n");
		return 0;
	}
	give_me_flag(flag);
	for (size_t i = 0; i < sizeof(array2); i++)
		array2[i] = 1;
	if (argc == 3)
	{
		sscanf_s(argv[1], "%p", (void * *)(&m_x));
		m_x -= (size_t)array1;
		sscanf_s(argv[2], "%d", &len);
	}
	int i = 0;
	while (--len >= 0)
	{
		readM(m_x++, value, score);
		if(score[0] >= 2 * score[1])
			ans[i++] = value[0] > 31 && value[0] < 127 ? value[0] : '?';
	}
	if (!memcmp(ans, secret,30))
		printf("done!flag is your input\n");
	else printf("false\n");
	return (0);
}
