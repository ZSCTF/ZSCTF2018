#include <cstdio>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include<Windows.h>
#include<iostream>
using namespace std;
int ram_n = 0;
char flag[1000000], flag1[100],flag2[100];
//--------------------------------strtonum---------------------------------------
int char2num(char c) {
	int num = 0;
	if ((c >= 'a') && (c <= 'f')) {
		num = 10 + (c - 'a');
	}
	else if ((c >= 'A') && (c <= 'F')) {
		num += 10 + (c - 'A');
	}
	else if ((c >= '0') && (c <= '9')) {
		num += c - '0';
	}
	else {
		printf("type error");
	}
	return num;
}

int str2num(char s[],char *num) {
	int i = 0;
	int length = strlen(s);
	int tmp;

	if (length % 2 != 0) {
		printf("length error\n");
		return 0;
	}


	while (i<length) {
		tmp = 16 * char2num(s[i]) + char2num(s[i + 1]);
		num[i / 2] = tmp;
		i += 2;
	}

	//for (i = 0; i<(length / 2); i++) {
	//	printf("%x ", num[i]);
	//}
	//printf("\n");
	return 1;
}
//-------------------------------------------------------------------------------

//-----------------------------------AES-----------------------------------------
#if 2147483647L+1L == -2147483648L
typedef unsigned long ulong32;
#else
typedef unsigned int ulong32;
#endif

//#pragma comment(lib, "aes.lib")


unsigned int aes_8bit_mul_mod_0x101(unsigned int x, unsigned int y);
unsigned int aes_8bit_mul_mod_0x11B(unsigned int x, unsigned int y);
unsigned int aes_8bit_inverse(unsigned int x);
void aes_polynomial_mul(unsigned char x[4], unsigned char y[4], unsigned char z[4]);
unsigned int get_msb_mask(unsigned int x);
void rol_a_row(unsigned char *p, int n);
void shr_a_row(unsigned char *p, int n);
void ror_a_row(unsigned char *p, int n);
void get_column(unsigned char *p, int j, int r, unsigned char *q);
void put_column(unsigned char *p, unsigned char *q, int j, int r);

void build_sbox(void);
void build_sbox_inverse(void);
void ByteSub(unsigned char *p, int n);
void ByteSubInverse(unsigned char *p, int n);
void ShiftRow(unsigned char *p);
void ShiftRowInverse(unsigned char *p);
void MixColumn(unsigned char *p, unsigned char a[4], int do_mul);
void MixColumnInverse(unsigned char *p, unsigned char a[4], int do_mul);
void AddRoundKey(unsigned char *p, unsigned char *k);
void aes_init(void);
int  aes_set_key(unsigned char *seed_key, int bits, unsigned char *key);
void aes_encrypt(unsigned char *bufin, unsigned char *bufout, unsigned char *key);
void aes_decrypt(unsigned char *bufin, unsigned char *bufout, unsigned char *key);

unsigned char sbox[256];
unsigned char sbox_inverse[256];
int key_rounds = 0;


unsigned int aes_8bit_mul_mod_0x101(unsigned int x, unsigned int y)
{
	/* ����ũ���㷨�� x * y mod (X^8 + 1); */
	/*      8         0
	X         X
	n = 1 0000 0001 = 0x101
	*/
	unsigned int p = 0; /* the product of the multiplication */
	int i;
	for (i = 0; i < 8; i++)
	{
		if (y & 1) /* if y is odd, then add the corresponding y to p */
			p ^= x; /* since we're in GF(2^m), addition is an XOR */
		y >>= 1;   /* equivalent to y/2 */
		x <<= 1;   /* equivalent to x*2 */
		if (x & 0x100) /* GF modulo: if x >= 256, then apply modular reduction */
			x ^= 0x101; /* XOR with the primitive polynomial x^8 + 1 */
						/* Actually, it's is the same as ROL. Commented by Black White. */
	}
	return p;
}

unsigned int aes_8bit_mul_mod_0x11B(unsigned int x, unsigned int y)
{
	/* ����ũ���㷨�� x * y mod (X^8 + X^4 + X^3 + X + 1);
	����ũ���㷨��ο�:
	(1) https://en.wikipedia.org/wiki/Rijndael_Galois_field
	(2) https://en.wikipedia.org/wiki/Multiplication_algorithm
	*/
	/*
	8    4 3 10
	X    X X XX
	n = 1 0001 1011 = 0x11B
	*/
	unsigned int p = 0; /* the product of the multiplication */
	int i;
	for (i = 0; i < 8; i++)
	{
		if (y & 1) /* if y is odd, then add the corresponding y to p */
			p ^= x; /* since we're in GF(2^m), addition is an XOR */
		y >>= 1;   /* equivalent to y/2 */
		x <<= 1;   /* equivalent to x*2 */
		if (x & 0x100) /* GF modulo: if x >= 256, then apply modular reduction */
			x ^= 0x11B; /* XOR with the primitive polynomial x^8 + x^4 + x^3 + x + 1 */

	}
	return p;
}

unsigned int get_msb_mask(unsigned int x)
{
	/* ����x���λ(���5λ)������:
	��x=0x0A, �������λ����=0x08;
	��x=0x1D, �������λ����=0x10;
	aes_8bit_inverse()��Ҫ���ñ�����
	*/
	unsigned int mask = 0x100;
	while (mask != 0 && (mask & x) == 0)
	{
		mask >>= 1;
	}
	return mask;
}

unsigned int aes_8bit_inverse(unsigned int x)
{
	/*      -1      8   4    3
	����x   mod X + X  + X  + X + 1
	����x��������õ�����չŷ������㷨(extended Euclidian algorithm);
	��չŷ������㷨��֤����������ο��̲�p.93��p.94;
	*/
	/*      8    4 3 10
	X    X X XX
	n = 1 0001 1011 = 0x11B
	*/
	unsigned int a1 = 1, b1 = 0, a2 = 0, b2 = 1;
	unsigned int q, r, t, n, nmask, xmask, shift;
	n = 0x11B; /* ��nΪ������ */
	r = x;     /* xΪ���� */
	while (r != 0)
	{
		q = 0;
		nmask = get_msb_mask(n);
		xmask = get_msb_mask(x);
		do
		{
			shift = 0;
			while (xmask < nmask)
			{
				xmask <<= 1;
				shift++;
			}
			if (xmask == nmask)
			{
				q |= 1 << shift; /* qΪ�� */
				n ^= x << shift; /* n = n - (x << shift);
								 ����������GF(2^8)�ļ�������,
								 ����ļ��������Բ�����λ, ������������
								 */
			}
			nmask = get_msb_mask(n);
			xmask = get_msb_mask(x);
		} while (n != 0 && nmask >= xmask);
		r = n;

		t = a1;
		a1 = a2;
		a2 = t ^ aes_8bit_mul_mod_0x11B(q, a2); /* a2 = a1 - q*a2; */
		t = b1;
		b1 = b2;
		b2 = t ^ aes_8bit_mul_mod_0x11B(q, b2); /* b2 = b1 - q*b2; */
		n = x;
		x = r;
	}
	return b1;
}


void build_sbox(void)
{
	/* ����sbox:
	��a��[0,255], ��
	-1                8
	sbox[a] = ((a   *  0x1F) mod (X + 1)) ^ 0x63;

	-1
	����a * a   = 1 mod (X^8+X^4+X^3+X+1);
	*/
	int i;
	for (i = 0; i<256; i++)
	{
		sbox[i] = aes_8bit_mul_mod_0x101(aes_8bit_inverse(i), 0x1F) ^ 0x63;
	}
}

void build_sbox_inverse(void)
{
	/* ����sbox����sbox����� */
	int i, j;
	for (i = 0; i<256; i++)
	{
		for (j = 0; j<256; j++)
		{
			if (sbox[j] == i)
				break;
		}
		sbox_inverse[i] = j;
	}
}

void aes_init(void)
{
	/* ����sbox��sbox����� */
	build_sbox();
	build_sbox_inverse();
}

int aes_set_key(unsigned char *seed_key, int bits, unsigned char *key)
{
	/* ����������Կ����������Կ:
	128bit������Կ(16�ֽ�): ����(1+10)*4��long32, step=4, loop=10
	192bit������Կ(24�ֽ�): ����(1+12)*4��long32, step=6, loop=8
	256bit������Կ(32�ֽ�): ����(1+14)*4��long32, step=8, loop=7
	*/
	int i, j, step, loop;
	ulong32 *pk;
	if (seed_key == NULL || key == NULL)
		return 0;
	if (bits == 128)      /* 16�ֽ�������Կ */
	{
		key_rounds = 10;  /* ���ܻ������Ҫ��10��ѭ�� */
		step = 4;         /* ������Կ��ѭ������Ϊ4��long32 */
		loop = 10;        /* ������Կ��Ҫ��10��ѭ�� */
	}
	else if (bits == 192) /* 24�ֽ�������Կ */
	{
		key_rounds = 12;  /* ���ܻ������Ҫ��12��ѭ�� */
		step = 6;
		loop = 8;
	}
	else if (bits == 256) /* 32�ֽ�������Կ */
	{
		key_rounds = 14;  /* ���ܻ������Ҫ��14��ѭ�� */
		step = 8;
		loop = 7;
	}
	else
	{
		key_rounds = 0;
		step = 0;
		return 0;
	}
	memcpy(key, seed_key, bits / 8);
	pk = (ulong32 *)(key + 4 * step);
	for (i = step; i<step + step*loop; i += step)
	{
		unsigned int r;
		/* �ٶ����ɵ���Կk��long32���͵�����, i�����±�,
		��i!=0 && i%step==0ʱ, k[i]�ڼ���ʱ��������
		�����⴦��:
		*/
		pk[0] = pk[-1];
		rol_a_row(key + i * 4, 1);
		ByteSub(key + i * 4, 4);
		r = 1 << ((i - step) / step);
		if (r <= 0x80)
			r = aes_8bit_mul_mod_0x11B(r, 1);
		else
			r = aes_8bit_mul_mod_0x11B(r / 4, 4);
		key[i * 4] ^= r;
		pk[0] ^= pk[-step];

		for (j = 1; j<step; j++) /* i+j����Կk���±�, ��(i+j)%step != 0ʱ, */
		{                     /* k[i+j]ֻ�����򵥵������ */
			if ((step == 6 && i == step*loop && j >= step - 2) ||
				(step == 8 && i == step*loop && j >= step - 4))
				break; /* 128-bit key does not need to discard any steps:
					   4 + 4*10 - 0= 4+40 = 44 == 4+4*10
					   192-bit key should discard last 2 steps:
					   6 + 6*8 - 2 = 4+48 = 52 == 4+4*12
					   256-bit key should discard last 4 steps:
					   8 + 8*7 - 4 = 4+56 = 60 == 4+4*14
					   */
			if (step == 8 && j == 4) /* ����256bit��Կ, ��(i+j)%4==0ʱ�������⴦�� */
			{
				ulong32 k;
				k = pk[3];
				ByteSub((unsigned char *)&k, 4); /* k = scrambled pk[3] */
				pk[4] = k ^ pk[4 - 8];
			}
			else /* ��(i+j)%step != 0ʱ, k[i+j]ֻ������������� */
				pk[j] = pk[j - step] ^ pk[j - 1];
		}
		pk += step;
	} /* for(i=step; i<step+step*loop; i+=step) */
	return 1;
}


void AddRoundKey(unsigned char *p, unsigned char *k)
{
	/* ��p��ָ���4*4���ľ�����k��ָ���4*4��Կ�������������:
	p -> 4byte * 4byte matrix
	k -> 4byte * 4byte key
	*/
	ulong32 *pp, *kk;
	int i;
	pp = (ulong32 *)p;
	kk = (ulong32 *)k;
	for (i = 0; i<4; i++)
	{
		pp[i] ^= kk[i];
	}
}

void ByteSub(unsigned char *p, int n)
{
	/* ��p��ָ���n�ֽ��滻��sbox�е�ֵ */
	int i;
	for (i = 0; i<n; i++)
	{
		p[i] = sbox[p[i]];
	}
}



void rol_a_row(unsigned char *p, int n)
{
	/* ��p��ָ���4���ֽ�ѭ������n���ֽ� */
	int i;
	unsigned char t;
	for (i = 0; i<n; i++)
	{
		t = p[0];
		memcpy(p, p + 1, 3);
		p[3] = t;
	}
}

void shr_a_row(unsigned char *p, int n)
{
	/* ��p��ָ���4���ֽ�����n���ֽ� */
	int i, j;
	for (i = 0; i<n; i++)
	{
		for (j = 3; j >= 1; j--)
		{
			p[j] = p[j - 1];
		}
		p[0] = 0;
	}
}

void ror_a_row(unsigned char *p, int n)
{
	/* ��p��ָ���4���ֽ�ѭ������n���ֽ� */
	int i, j;
	unsigned char t;
	for (i = 0; i<n; i++)
	{
		t = p[3];
		for (j = 3; j >= 1; j--)
		{
			p[j] = p[j - 1];
		}
		p[0] = t;
	}
}


void ShiftRow(unsigned char *p)
{
	/* ��p��ָ���4*4����������ѭ�����Ʋ���:
	��0��: ���ƶ�;
	��1��: ����1�ֽ�
	��2��: ����2�ֽ�
	��3��: ����3�ֽ�
	*/
	int i;
	for (i = 1; i<4; i++)
	{
		rol_a_row(p + i * 4, i);
	}
}



void get_column(unsigned char *p, int j, int r, unsigned char *q)
{
	/* ��p��ָ���4*4����m����ȡ��j��, �����4��unsigned char, ���浽q��;
	����rָ����j��4��Ԫ���е���Ԫ�������к�, ����get_column(p, 3, 3, q)
	��ȡ����: m[3][3], m[0][3], m[1][3], m[2][3]
	*/
	int i;
	for (i = 0; i<4; i++)
	{
		q[i] = p[(r + i) % 4 * 4 + j];
	}
}

void put_column(unsigned char *p, unsigned char *q, int j, int r)
{
	/* ��p��ָ��4��unsigned char���浽q��ָ����m�еĵ�j��;
	����rָ����j��4��Ԫ���е���Ԫ�������к�, ����put_column(p, q, 2, 2)
	�������: m[2][2], m[3][2], m[0][2], m[1][2]
	*/
	int i;
	for (i = 0; i<4; i++)
	{
		q[(r + i) % 4 * 4 + j] = p[i];
	}
}

void aes_polynomial_mul(unsigned char x[4], unsigned char y[4], unsigned char z[4]) {
	int i;
	unsigned int result[4] = { 0 };
	for (i = 0; i<4; i++) {
		result[i] ^= aes_8bit_mul_mod_0x11B((unsigned int)x[(3 - i) % 4], (unsigned int)y[0]);
		result[i] ^= aes_8bit_mul_mod_0x11B((unsigned int)x[(4 - i) % 4], (unsigned int)y[1]);
		result[i] ^= aes_8bit_mul_mod_0x11B((unsigned int)x[(5 - i) % 4], (unsigned int)y[2]);
		result[i] ^= aes_8bit_mul_mod_0x11B((unsigned int)x[(6 - i) % 4], (unsigned int)y[3]);
	}

	for (i = 0; i<4; i++) {
		z[0] = (unsigned char)result[0];
		z[1] = (unsigned char)result[1];
		z[2] = (unsigned char)result[2];
		z[3] = (unsigned char)result[3];
	}
}


void MixColumn(unsigned char *p, unsigned char a[4], int do_mul)
{
	/* (1) ��pָ���4*4����m�е�4�����˷�����;
	(2) ����ĳ˷���ָ������GF(2^8)����ʽģ(X^4+1)�˷�,���岽����ο��̲�p.61��p.62;
	(3) aes����ʱ���õı�����aΪ����ʽ3*X^3 + X^2 + X + 2, �������ʾΪ
	unsigned char a[4]={0x03, 0x01, 0x01, 0x02};
	(4) aes����ʱ���õı�����aΪ�������ö���ʽ����, ��{0x0B, 0x0D, 0x09, 0x0E};
	(5) ����m�е�4�а�����˳��ֱ���a���˷�����:
	��0��: ��m[0][0], m[1][0], m[2][0], m[3][0]����
	��3��: ��m[1][3], m[2][3], m[3][3], m[0][3]����
	��2��: ��m[2][2], m[3][2], m[0][2], m[1][2]����
	��1��: ��m[3][1], m[0][1], m[1][1], m[2][1]����
	(6) �˷�����4��ת��4��, ���浽p��, �滻��p��ԭ�еľ���;
	(7) do_mul���������Ƿ�Ҫ���˷�����, �������һ�ּ����ܵ�һ��do_mul=0;
	*/
	unsigned char b[4];
	unsigned char t[4][4];
	int j;
	for (j = 0; j<4; j++)
	{
		get_column(p, (4 - j) % 4, j, b); /* ��p��ָ����m����ȡ��j��, ���浽����b��;
										  ��j����Ԫ�ص��к�Ϊj:
										  column 0: 0 1 2 3
										  column 3: 1 2 3 0
										  column 2: 2 3 0 1
										  column 1: 3 0 1 2
										  */
		if (do_mul) /* �ڼ������һ���Լ����ܵ�һ�ֵ�MixColumn�����в���Ҫ���˷�; */
			aes_polynomial_mul(a, b, b); /* �����ֶ�Ҫ���˷�: b = a*b mod (X^4+1); */
		memcpy(t[j], b, 4); /* �ѳ˷����ý�����Ƶ�t�е�j�� */
	}
	memcpy(p, t, 4 * 4); /* ����t�о���p, �滻��p��ԭ�о��� */
}


//void aes_encrypt(unsigned char *bufin, unsigned char *bufout, unsigned char *key)
//{
//	int i;
//	unsigned char a[4] = { 0x03, 0x01, 0x01, 0x02 }; /* �������ʽ3*X^3+X^2+X+2 */
//	unsigned char matrix[4][4];
//	memcpy(matrix, bufin, 4 * 4); /* ��������16�ֽڵ�matrix */
//	AddRoundKey((unsigned char *)matrix, key); /* ��0��ֻ��AddRoundKey() */
//	for (i = 1; i <= key_rounds; i++)
//	{  /* ��1��key_rounds��, �����²���: ByteSub, ShiftRow, MixColumn, AddRoundKey */
//		ByteSub((unsigned char *)matrix, 16);
//		ShiftRow((unsigned char *)matrix);
//		if (i != key_rounds)
//			MixColumn((unsigned char *)matrix, a, 1); /* do mul */
//		else
//			MixColumn((unsigned char *)matrix, a, 0); /* don't mul */
//		AddRoundKey((unsigned char *)matrix, key + i*(4 * 4));
//	}
//	memcpy(bufout, matrix, 4 * 4); /* ���ĸ��Ƶ�bufout */
//}


void ByteSubInverse(unsigned char *p, int n) {
	/* ��p��ָ���n�ֽ��滻��sbox������е�ֵ */
	int i;
	for (i = 0; i<n; i++)
	{
		p[i] = sbox_inverse[p[i]];
	}
}

void ShiftRowInverse(unsigned char *p) {
	/* ��p��ָ���4*4����������ѭ�����Ʋ���:
	��0��: ���ƶ�;
	��1��: ����1�ֽ�
	��2��: ����2�ֽ�
	��3��: ����3�ֽ�
	*/
	int i;
	for (i = 1; i<4; i++)
	{
		ror_a_row(p + i * 4, i);
	}
}

void MixColumnInverse(unsigned char *p, unsigned char a[4], int do_mul) {
	/* (1) ��pָ���4*4����m�е�4�����˷�����;
	(2) ����ĳ˷���ָ������GF(2^8)����ʽģ(X^4+1)�˷�,���岽����ο��̲�p.61��p.62;
	(3) aes����ʱ���õı�����aΪ�������ö���ʽ����, ��{0x0B, 0x0D, 0x09, 0x0E};
	(4) ����m�е�4����a���˷������, ������˳��ת�ɾ���t�е���:
	��0��:
	t[0][0] = m[0][0]
	t[1][0] = m[0][1]
	t[2][0] = m[0][2]
	t[3][0] = m[0][3]
	��3��:
	t[1][3] = m[1][0]
	t[2][3] = m[1][1]
	t[3][3] = m[1][2]
	t[0][3] = m[1][3]
	��2��:
	t[2][2] = m[2][0]
	t[3][2] = m[2][1]
	t[0][2] = m[2][2]
	t[1][2] = m[2][3]
	��1��:
	t[3][1] = m[3][0]
	t[0][1] = m[3][1]
	t[1][1] = m[3][2]
	t[2][1] = m[3][3]
	(5) ����t��p��, �滻��p��ԭ�еľ���;
	(6) do_mul���������Ƿ�Ҫ���˷�����, �������һ�ּ����ܵ�һ��do_mul=0;
	*/
	unsigned char b[4];
	unsigned char t[4][4];
	int i, j;
	for (j = 0; j<4; j++)
	{
		for (i = 0; i<4; i++) {				//ȡ��
			b[i] = p[4 * j + i];
		}
		if (do_mul) /* �ڼ������һ���Լ����ܵ�һ�ֵ�MixColumn�����в���Ҫ���˷�; */
			aes_polynomial_mul(a, b, b); /* �����ֶ�Ҫ���˷�: b = a*b mod (X^4+1); */

		for (i = 0; i<4; i++) {						//����ת��
			t[(i + j) % 4][(4 - j) % 4] = b[i];
		}
	}
	memcpy(p, t, 4 * 4); /* ����t�о���p, �滻��p��ԭ�о��� */
}

void aes_decrypt(unsigned char *bufin, unsigned char *bufout, unsigned char *key)
{
	int i;
	unsigned char a[4] = { 0x0B, 0x0D, 0x09, 0x0E }; /* �������ʽB*X^3+D*X^2+9*X+E */
	unsigned char matrix[4][4];
	memcpy(matrix, bufin, 4 * 4); /* ��������16�ֽڵ�matrix */
	for (i = key_rounds; i >= 1; i--)
	{  /* ��key_rounds����1��, �����²���: AddRoundKey, MixColumn, ShiftRow, ByteSub*/
		AddRoundKey((unsigned char *)matrix, key + i*(4 * 4));
		if (i != key_rounds)
			MixColumnInverse((unsigned char *)matrix, a, 1); /* do mul */
		else
			MixColumnInverse((unsigned char *)matrix, a, 0); /* don't mul */
		ShiftRowInverse((unsigned char *)matrix);
		ByteSubInverse((unsigned char *)matrix, 16);
	}
	AddRoundKey((unsigned char *)matrix, key); /* ��0��ֻ��AddRoundKey() */
	memcpy(bufout, matrix, 4 * 4); /* ���ĸ��Ƶ�bufout */
}
//-------------------------------------------------------------------------------

//------------------------------------VM-----------------------------------------
unsigned int std_opc_value(unsigned int **vm, unsigned int addr, unsigned int size, unsigned int value)
{
	unsigned int mask;
	switch (size)
	{
	case 1:
		mask = 0xff;
		break;;
	case 2:
		mask = 0xffff;
		break;;
	case 4:
		mask = 0xffffffff;
		break;;
	default:
		mask = 0;
	}
	*(unsigned int *)((char *)*vm + addr) ^= (*(unsigned int *)((char *)*vm + addr) & mask) ^ (value & mask);
	return(value);
}


unsigned int get_mem_value(unsigned int **vm, unsigned int addr, unsigned int size)
{
	unsigned int    value = 0;
	unsigned int    mask;
	switch (size)
	{
	case 1:
		mask = 0xff;
		break;;
	case 2:
		mask = 0xffff;
		break;;
	case 4:
		mask = 0xffffffff;
		break;;
	default:
		mask = 0;
	}
	value = *(unsigned int *)((char *)vm[3] + addr) & mask;
	return(value);
}


unsigned int std_mem_value(unsigned int **vm, unsigned int addr, unsigned int size, unsigned int value)
{
	unsigned int mask;
	switch (size)
	{
	case 1:
		mask = 0xff;
		break;;
	case 2:
		mask = 0xffff;
		break;;
	case 4:
		mask = 0xffffffff;
		break;;
	default:
		mask = 0;
	}
	*(unsigned int *)((char *)vm[3] + addr) ^= (*(unsigned int *)((char *)vm[3] + addr) & mask) ^ (value & mask);
	return(value);
}


unsigned int cmp_value(unsigned int **vm, unsigned int value1, unsigned int value2)
{
	if (value1 == value2)
	{
		*(*(vm + 1) + 7) = 1;
	}
	else if (value1 > value2)
	{
		*(*(vm + 1) + 7) = 2;
	}
	else {
		*(*(vm + 1) + 7) = 0;
	}
	return(value1);
}


unsigned int get_reg_value(unsigned int **vm, unsigned int idx, unsigned int size)
{
	unsigned int    value = 0;
	unsigned int    mask;
	if (idx < 8)
	{
		switch (size)
		{
		case 1:
			mask = 0xff;
			break;;
		case 2:
			mask = 0xffff;
			break;;
		case 4:
			mask = 0xffffffff;
			break;;
		default:
			mask = 0;
		}
		value = vm[1][idx] & mask;
	}
	return(value);
}

unsigned int get_ram_value(unsigned int **vm, unsigned int idx, unsigned int size)
{
	unsigned int    value = 0;
	unsigned int    mask;
	if (1)
	{
		switch (size)
		{
		case 1:
			mask = 0xff;
			break;;
		case 2:
			mask = 0xffff;
			break;;
		case 4:
			mask = 0xffffffff;
			break;;
		default:
			mask = 0;
		}
		value = vm[4][idx] & mask;
	}
	return(value);
}
unsigned int std_reg_value(unsigned int **vm, unsigned int idx, unsigned int size, unsigned int value)
{
	unsigned int mask;
	if (idx < 8)
	{
		switch (size)
		{
		case 1:
			mask = 0xff;
			break;;
		case 2:
			mask = 0xffff;
			break;;
		case 4:
			mask = 0xffffffff;
			break;;
		default:
			mask = 0;
		}
		vm[1][idx] ^= (vm[1][idx] & mask) ^ (value & mask);
	}
	return(value);
}


unsigned int push(unsigned int **vm, unsigned int value)
{
	*(vm + 2) -= 4;
	*vm[2] = value;
	return(value);
}


unsigned int pop(unsigned int **vm)
{
	unsigned int *value = vm[2];
	*(vm + 2) += 4;
	return(*value);
}


unsigned int reg_add(unsigned int **vm, unsigned int reg1, unsigned int reg2)
{
	unsigned int    v1 = get_reg_value(vm, reg1, 4);
	unsigned int    v2 = get_reg_value(vm, reg2, 4);
	return(std_reg_value(vm, reg1, 4, v1 + v2));
}


unsigned int reg_sub(unsigned int **vm, unsigned int reg1, unsigned int reg2)
{
	unsigned int    v1 = get_reg_value(vm, reg1, 4);
	unsigned int    v2 = get_reg_value(vm, reg2, 4);
	return(std_reg_value(vm, reg1, 4, v1 - v2));
}


unsigned int reg_mul(unsigned int **vm, unsigned int reg1, unsigned int reg2)
{
	unsigned int    v1 = get_reg_value(vm, reg1, 4);
	unsigned int    v2 = get_reg_value(vm, reg2, 4);
	return(std_reg_value(vm, reg1, 4, v1 * v2));
}


unsigned int reg_div(unsigned int **vm, unsigned int reg1, unsigned int reg2)
{
	unsigned int    v1 = get_reg_value(vm, reg1, 4);
	unsigned int    v2 = get_reg_value(vm, reg2, 4);
	return(std_reg_value(vm, reg1, 4, v1 / v2));
}


unsigned int reg_and(unsigned int **vm, unsigned int reg1, unsigned int reg2)
{
	unsigned int    v1 = get_reg_value(vm, reg1, 4);
	unsigned int    v2 = get_reg_value(vm, reg2, 4);
	return(std_reg_value(vm, reg1, 4, v1 & v2));
}


unsigned int reg_or(unsigned int **vm, unsigned int reg1, unsigned int reg2)
{
	unsigned int    v1 = get_reg_value(vm, reg1, 4);
	unsigned int    v2 = get_reg_value(vm, reg2, 4);
	return(std_reg_value(vm, reg1, 4, v1 | v2));
}


unsigned int reg_xor(unsigned int **vm, unsigned int reg1, unsigned int reg2)
{
	unsigned int    v1 = get_reg_value(vm, reg1, 4);
	unsigned int    v2 = get_reg_value(vm, reg2, 4);
	return(std_reg_value(vm, reg1, 4, v1 ^ v2));
}


unsigned int reg_cmp(unsigned int **vm, unsigned int reg1, unsigned int reg2)
{
	unsigned int    v1 = get_reg_value(vm, reg1, 4);
	unsigned int    v2 = get_reg_value(vm, reg2, 4);
	return(cmp_value(vm, v1, v2));
}


unsigned int inc_reg(unsigned int **vm, unsigned int reg1)
{
	unsigned int v1 = get_reg_value(vm, reg1, 4);
	*(char *)vm = *(char *)vm - 1;
	return(std_reg_value(vm, reg1, 4, v1 + 1));
}


unsigned int dec_reg(unsigned int **vm, unsigned int reg1)
{
	unsigned int v1 = get_reg_value(vm, reg1, 4);
	*(char *)vm = *(char *)vm - 1;
	return(std_reg_value(vm, reg1, 4, v1 - 1));
}


unsigned int push_imm(unsigned int **vm, unsigned int value)
{
	*(char *)vm = *(char *)vm + 2;
	return(push(vm, value));
}


unsigned int push_reg(unsigned int **vm, unsigned int reg1)
{
	unsigned int v1 = get_reg_value(vm, reg1, 4);
	*(char *)vm = *(char *)vm - 1;
	return(push(vm, v1));
}


unsigned int pop_reg(unsigned int **vm, unsigned int reg1)
{
	unsigned int v1 = pop(vm);
	*(char *)vm = *(char *)vm - 1;
	return(std_reg_value(vm, reg1, 4, v1));
}


unsigned int reg_mov_reg(unsigned int **vm, unsigned int reg1, unsigned int reg2)
{
	unsigned int v2 = get_reg_value(vm, reg2, 4);
	return(std_reg_value(vm, reg1, 4, v2));
}


unsigned int mem_mov_reg(unsigned int **vm, unsigned int reg1, unsigned int addr, unsigned int size)
{
	unsigned int v2 = get_mem_value(vm, addr, size);
	//*(char *)vm = *(char *)vm + 3;
	return(std_reg_value(vm, reg1, size, v2));
}

unsigned int nmem_mov_reg(unsigned int **vm, unsigned int reg1, unsigned int addr, unsigned int size)
{
	unsigned int n = get_reg_value(vm, addr, size);
	unsigned int v2 = get_mem_value(vm, n, size);
	//*(char *)vm = *(char *)vm + 3;
	return(std_reg_value(vm, reg1, size, v2));
}

unsigned int nram_mov_reg(unsigned int **vm, unsigned int reg1, unsigned int addr, unsigned int size)
{
	unsigned int v2 = get_ram_value(vm, ram_n++, size);
	//*(char *)vm = *(char *)vm + 3;
	return(std_reg_value(vm, reg1, size, v2));
}

unsigned int reg_mov_mem(unsigned int **vm, unsigned int reg1, unsigned int addr, unsigned int size)
{
	unsigned int v1 = get_reg_value(vm, reg1, size);
	//*(char *)vm = *(char *)vm + 3;
	return(std_mem_value(vm, addr, size, v1));
}
unsigned int reg_mov_nmem(unsigned int **vm, unsigned int reg1, unsigned int addr, unsigned int size)
{
	unsigned int n = get_reg_value(vm, addr, size);
	unsigned int v1 = get_reg_value(vm, reg1, size);
	//*(char *)vm = *(char *)vm + 3;
	return(std_mem_value(vm, n, size, v1));
}


unsigned int jmp(unsigned int **vm, unsigned int addr)
{
	*(char *)vm = *(char *)vm - addr - 3;
	return(0);
}


unsigned int je(unsigned int **vm, unsigned int addr)
{
	if (*(*(vm + 1) + 7) == 1)
	{
		jmp(vm, addr);
	}
	else {
		*(char *)vm = *(char *)vm + 2;
	}
	return(0);
}

void checkcheck(unsigned int **vm, unsigned int reg1, unsigned int addr, unsigned int size)
{
	if (IsDebuggerPresent())
		exit(0);
}
unsigned int jne(unsigned int **vm, unsigned int addr)
{
	if (*(*(vm + 1) + 7) != 1)
	{
		jmp(vm, addr);
	}
	else {
		*(char *)vm = *(char *)vm + 2;
	}
	return(0);
}


unsigned int jg(unsigned int **vm, unsigned int addr)
{
	if (*(*(vm + 1) + 7) == 2)
	{
		jmp(vm, addr);
	}
	else {
		*(char *)vm = *(char *)vm + 2;
	}
	return(0);
}


unsigned int jl(unsigned int **vm, unsigned int addr)
{
	if (*(*(vm + 1) + 7) == 0)
	{
		jmp(vm, addr);
	}
	else {
		*(char *)vm = *(char *)vm + 2;
	}
	return(0);
}


unsigned int jge(unsigned int **vm, unsigned int addr)
{
	if (*(*(vm + 1) + 7) >= 1)
	{
		jmp(vm, addr);
	}
	else {
		*(char *)vm = *(char *)vm + 2;
	}
	return(0);
}


unsigned int jle(unsigned int **vm, unsigned int addr)
{
	if (*(*(vm + 1) + 7) <= 1)
	{
		jmp(vm, addr);
	}
	else {
		*(char *)vm = *(char *)vm + 2;
	}
	return(0);
}


unsigned int smc(unsigned int **vm, unsigned int reg1, unsigned int size)
{
	unsigned int v1 = get_reg_value(vm, reg1, size);
	*(char *)vm = *(char *)vm - 1;
	return(std_opc_value(vm, 5, size, v1));
}
//                      push reg6                                xor reg5,reg5;   inc reg4                 pop reg6               pop reg3                                      mov reg1,[ram_n]        mov mem[reg5],reg         cmp reg5,reg3
unsigned char opc[] = { 103,6,0,0,0,0,103,5,0,0,0,0,112,6,6,0,0,0,0,112,5,5,0,0,0,12,4,0,200,0,0,0,0,203,5,0,203,6,0,0,5,229,24,0,0,0,203,3,0,0,0,0,0,0,0,0,0,0,1,139,0,5,0,0,0,0,212,1,0,0,0,0,46,0,0,0,112,0,1,132,0,5,0,0,0,0,12,5,0,195,5,3,0,0,251,36,0,0,0,0,255 };
//                            push reg5          xor reg6,reg6                                    pop reg5            push 24                                        mov reg,mem[reg5]    check46      xor reg,reg1            inc reg5              jl 30

void vm_handler(unsigned int **vm)
{
	unsigned char    code;
	unsigned int *vm_re = vm[0], *vm_temp;
	if (vm[1][4] > 3)
		return;
	while (1)
	{
		code = vm[0][0];
		switch (code)
		{
		case 193:
			reg_add(vm, (unsigned int)((char *)*vm)[1], (unsigned int)((char *)*vm)[2]);
			break;
		case 133:
			reg_sub(vm, (unsigned int)((char *)*vm)[1], (unsigned int)((char *)*vm)[2]);
			break;
		case 21:
			reg_mul(vm, (unsigned int)((char *)*vm)[1], (unsigned int)((char *)*vm)[2]);
			break;
		case 183:
			reg_div(vm, (unsigned int)((char *)*vm)[1], (unsigned int)((char *)*vm)[2]);
			break;
		case 49:
			reg_and(vm, (unsigned int)((char *)*vm)[1], (unsigned int)((char *)*vm)[2]);
			break;
		case 169:
			reg_or(vm, (unsigned int)((char *)*vm)[1], (unsigned int)((char *)*vm)[2]);
			break;
		case 112:
			reg_xor(vm, (unsigned int)((char *)*vm)[1], (unsigned int)((char *)*vm)[2]);
			break;
		case 195:
			reg_cmp(vm, (unsigned int)((char *)*vm)[1], (unsigned int)((char *)*vm)[2]);
			break;
		case 12:
			inc_reg(vm, (unsigned int)((char *)*vm)[1]);
			break;
		case 160:
			dec_reg(vm, (unsigned int)((char *)*vm)[1]);
			break;
		case 229:
			push_imm(vm, *(unsigned int *)((char *)*vm + 1));
			break;
		case 103:
			push_reg(vm, (unsigned int)((char *)*vm)[1]);
			break;
		case 203:
			pop_reg(vm, (unsigned int)((char *)*vm)[1]);
			break;
		case 200:
			vm_temp = vm[0];
			vm[0] = vm_re;
			vm_handler(vm);
			vm[0] = vm_temp;
			break;
		case 45:
			reg_mov_reg(vm, (unsigned int)((char *)*vm)[1], (unsigned int)((char *)*vm)[2]);
			break;
		case 46:
			checkcheck(vm, (unsigned int)((char *)*vm)[1], *(unsigned int *)((char *)*vm + 2), 1);
			break;
		case 138:
			mem_mov_reg(vm, (unsigned int)((char *)*vm)[1], *(unsigned int *)((char *)*vm + 2), 1);
			break;
		case 139:
			nmem_mov_reg(vm, (unsigned int)((char *)*vm)[1], *(unsigned int *)((char *)*vm + 2), 1);
			break;
		case 242:
			mem_mov_reg(vm, (unsigned int)((char *)*vm)[1], *(unsigned int *)((char *)*vm + 2), 2);
			break;
		case 243:
			nmem_mov_reg(vm, (unsigned int)((char *)*vm)[1], *(unsigned int *)((char *)*vm + 2), 2);
			break;
		case 254:
			mem_mov_reg(vm, (unsigned int)((char *)*vm)[1], *(unsigned int *)((char *)*vm + 2), 4);
			break;
		case 253:
			nmem_mov_reg(vm, (unsigned int)((char *)*vm)[1], *(unsigned int *)((char *)*vm + 2), 4);
			break;
		case 212:
			nram_mov_reg(vm, (unsigned int)((char *)*vm)[1], *(unsigned int *)((char *)*vm + 2), 1);
			break;
		case 131:
			reg_mov_mem(vm, (unsigned int)((char *)*vm)[1], *(unsigned int *)((char *)*vm + 2), 1);
			break;
		case 132:
			reg_mov_nmem(vm, (unsigned int)((char *)*vm)[1], *(unsigned int *)((char *)*vm + 2), 1);
			break;
		case 233:
			reg_mov_mem(vm, (unsigned int)((char *)*vm)[1], *(unsigned int *)((char *)*vm + 2), 2);
			break;
		case 234:
			reg_mov_nmem(vm, (unsigned int)((char *)*vm)[1], *(unsigned int *)((char *)*vm + 2), 2);
			break;
		case 153:
			reg_mov_mem(vm, (unsigned int)((char *)*vm)[1], *(unsigned int *)((char *)*vm + 2), 4);
			break;
		case 154:
			reg_mov_nmem(vm, (unsigned int)((char *)*vm)[1], *(unsigned int *)((char *)*vm + 2), 4);
			break;
		case 172:
			jmp(vm, *(unsigned int *)((char *)*vm + 1));
			break;
		case 20:
			je(vm, *(unsigned int *)((char *)*vm + 1));
			break;
		case 164:
			jne(vm, *(unsigned int *)((char *)*vm + 1));
			break;
		case 54:
			jg(vm, *(unsigned int *)((char *)*vm + 1));
			break;
		case 251:
			jl(vm, *(unsigned int *)((char *)*vm + 1));
			break;
		case 115:
			jge(vm, *(unsigned int *)((char *)*vm + 1));
			break;
		case 42:
			jle(vm, *(unsigned int *)((char *)*vm + 1));
			break;
		case 134:
			smc(vm, (unsigned int)((char *)*vm)[1], 1);
			break;
		case 249:
			smc(vm, (unsigned int)((char *)*vm)[1], 2);
			break;
		case 113:
			smc(vm, (unsigned int)((char *)*vm)[1], 4);
			break;
		case 255:
			return;
		default:
			*(unsigned char *)vm = *(unsigned char *)vm - 2;
			break;
		}
		*(unsigned char *)vm = *(unsigned char *)vm + 3;
	}
}

void vm(char *key, unsigned int ram[])
{
	char        *mem = (char *)malloc(sizeof(char) * 128);
	char        *stk = (char *)malloc(sizeof(char) * 256);
	unsigned int    *reg = (unsigned int *)malloc(sizeof(unsigned int) * 8);
	unsigned int    **vm = (unsigned int * *)malloc(sizeof(unsigned int*) * 5);
	memset(stk, 0, 64);
	memset(mem, 0, 64);
	memset(reg, 0, 32);
	memcpy(mem, key, 24);//��С�ǵø�

	*vm = (unsigned int *)opc;
	*(vm + 1) = (unsigned int *)reg;
	*(vm + 2) = (unsigned int *)stk + 32;
	*(vm + 3) = (unsigned int *)mem;
	*(vm + 4) = (unsigned int *)ram;
	vm_handler(vm);
	memcpy(key, mem, 24);//��С�ǵø�

	free(mem);
	free(stk);
	free(reg);
	free(vm);
}
//-------------------------------------------------------------------------------
//------------------------------------game---------------------------------------
char map1[11][21] = { "##########","#........#","#........#" ,"#........#" ,"#........#" ,"#........#" ,"#........#","#........#" ,"#........#" ,"##########" };
char map2[11][21] = { "####################","*********.**********","*********.**********" ,"*********.**********" ,"*********.**********" ,"*********.**********" ,"*********.**********","*********.**********" ,"*********.**********" ,"####################" };
char map3[11][21] = { "********************","********************", "********************", "********************", "********************", "********************", "********************", "********************", "********************", "********************" };
char *p1;
char jump_table[] = { 1,1,7,2,6,6,9,8,9,8 ,9,8 ,9,8 ,9,8 ,9,8 ,9,8 ,9,8 ,9,8 ,9,8 ,9,8 ,9,8 ,9,8 ,9,8 ,9,8 ,9,8 ,9,8 ,9,8 ,9,8 ,9,8 ,9,8 };
char jump_table_index = 0;
struct MyStruct
{
	int x, y, pre_x, pre_y;
}point;
void delay(int a)
{
	time_t start, end;
	time(&start);
	time(&end);
	while (difftime(end, start) < a)
	{
		time(&end);
	}
}
void sorry()
{
	printf("sorry,error key\n");
	delay(3);
	exit(0);
}
void pan(int flag, MyStruct &locat)
{
	if (flag == 1)
	{
		if (map1[locat.x][locat.y] != '.')
			sorry();
	}
	else if (flag == 2)
	{
		if (map2[locat.x][locat.y] != '.')
			sorry();
	}
	else
	{
		if (map3[locat.x][locat.y] != '.')
			sorry();
	}
}
void up(int flag, MyStruct &locat)
{
	if (flag == 1)
		if (locat.x <= 1)
			sorry();
	locat.x = locat.x - 1;
	pan(flag, locat);
}
void down(int flag, MyStruct &locat)
{
	if (flag == 1)
		if (locat.x >= 8)
			sorry();
	locat.x = locat.x + 1;
	pan(flag, locat);
}
void left(int flag, MyStruct &locat)
{
	if (flag == 1)
		if (locat.y <= 1)
			sorry();
	locat.y = locat.y - 1;
	pan(flag, locat);
}
void right(int flag, MyStruct &locat)
{
	if (flag == 1)
		if (locat.y >= 8)
			sorry();
	locat.y = locat.y + 1;
	pan(flag, locat);
}
void upper_left(int flag, MyStruct &locat)
{
	if (flag == 2)
	{
		locat.x = locat.x - 1;
		locat.y = locat.y - 1;
	}
	else if (flag == 3)
	{
		locat.x = locat.x - 1;
		locat.y = locat.y - 1;
		jump_table_index += 2;
	}
	pan(flag, locat);
}
void upper_right(int flag, MyStruct &locat)
{
	if (flag == 2)
	{
		locat.x = locat.x - 1;
		locat.y = locat.y + 1;
	}
	else if (flag == 3)
	{
		locat.x = locat.x - 1;
		locat.y = locat.y + 1;
		jump_table_index += 2;
	}
	pan(flag, locat);
}
void lower_right(int flag, MyStruct &locat)
{
	if (flag == 2)
	{
		locat.x = locat.x + 1;
		locat.y = locat.y + 1;
	}
	else if (flag == 3)
	{
		locat.x = locat.x + 1;
		locat.y = locat.y + 1;
		jump_table_index += 2;
	}
	pan(flag, locat);
}
void lower_left(int flag, MyStruct &locat)
{
	if (flag == 2)
	{
		locat.x = locat.x + 1;
		locat.y = locat.y - 1;
	}
	else if (flag == 3)
	{
		locat.x = locat.x + 1;
		locat.y = locat.y - 1;
		jump_table_index += 2;
	}
	pan(flag, locat);
}
void jump(MyStruct &locat)
{
	locat.x = jump_table[jump_table_index];
	locat.y = jump_table[jump_table_index + 1];
	jump_table_index += 2;
	pan(3, locat);
}
void level_1_move(MyStruct &locat)
{
	switch (*p1)
	{
	case 'a':up(1, locat); break;
	case 'b':down(1, locat); break;
	case 'c':left(1, locat); break;
	case 'd':right(1, locat); break;
	default:sorry();
		break;
	}
	++p1;
}
void level_2_move(MyStruct &locat)
{
	switch (*p1)
	{
	case 'e':up(2, locat); break;
	case 'f':down(2, locat); break;
	case 'g':left(2, locat); break;
	case 'h':right(2, locat); break;
	case 'i':upper_left(2, locat); break;
	case 'j':upper_right(2, locat); break;
	case 'k':lower_left(2, locat); break;
	case 'l':lower_right(2, locat); break;
	default:sorry();
		break;
	}
	if (locat.pre_x >= locat.x)
		sorry();
	++p1;
}
void level_3_move(MyStruct &locat)
{
	switch (*p1)
	{
	case 'm':up(3, locat); break;
	case 'n':down(3, locat); break;
	case 'o':left(3, locat); break;
	case 'p':right(3, locat); break;
	case 'q':upper_left(3, locat); break;
	case 'r':upper_right(3, locat); break;
	case 's':lower_left(3, locat); break;
	case 't':lower_right(3, locat); break;
	case 'u':jump(locat); break;
	default:sorry();
		break;
	}
	++p1;
}
void move(int flag, MyStruct &locat)
{
	switch (flag)
	{
	case 1:level_1_move(locat); break;
	case 2:level_2_move(locat); break;
	case 3:level_3_move(locat); break;
	}
}
void print(char p[][21])
{
	for (int i = 0; i < 11; ++i)
	{
		printf("%s\n", p[i]);
	}
}
void sp_function(MyStruct &locat)
{
	int i, n;
	map2[locat.x - 1][locat.y - 1] = '.';
	map2[locat.x][locat.y] = '*';
	for (i = 0; i < 20; ++i)
	{
		if (map2[locat.x + 1][i] == '.')
		{
			n = i;
			map2[locat.x + 1][i] = '*';
		}
	}
	switch (*p1)
	{
	case 'v':up(2, locat); break;
	case 'w':down(2, locat); break;
	case 'x':left(2, locat); break;
	case 'y':right(2, locat); break;
	case 'z':upper_left(2, locat); break;
	case '1':upper_right(2, locat); break;
	case '2':lower_left(2, locat); break;
	case '3':lower_right(2, locat); break;
	default:sorry();
		break;
	}
	++p1;
	if (map2[locat.x][locat.y] != '.')
		sorry();
	map2[locat.x + 2][n] = '.';
	map2[locat.x][locat.y] = '*';
	map2[locat.x + 1][locat.y + 1] = '.';
	switch (*p1)
	{
	case 'v':up(2, locat); break;
	case 'w':down(2, locat); break;
	case 'x':left(2, locat); break;
	case 'y':right(2, locat); break;
	case 'z':upper_left(2, locat); break;
	case '1':upper_right(2, locat); break;
	case '2':lower_left(2, locat); break;
	case '3':lower_right(2, locat); break;
	default:sorry();
		break;
	}
	++p1;
	if (map2[locat.x][locat.y] != '.')
		sorry();
	map2[locat.x][locat.y] = '*';
}
void level_1()
{
	int i, j, k, l, n = 9, temp;
	MyStruct locat;
	locat.x = 8;
	locat.y = 1;
	for (k = 0; k < 7; ++k)
	{
		temp = n;
		for (i = 1; i < 9; ++i)
		{
			for (j = 1; j < temp&&j<9; ++j)
			{
				map1[i][j] = '!';
			}
			--temp;
		}
		/*system("CLS");
		print(map1);
		delay(1);*/
		move(1, locat);
		n++;
	}
}
void level_2()
{
	int i, j, k, l, flag_5 = 0;
	MyStruct locat;
	locat.x = 1;
	locat.y = 9;
	locat.pre_x = -1;
	locat.pre_y = -1;
	for (i = 2; i < 9; ++i)
	{
		if (i == 5 && !flag_5)
		{
			--i;
			sp_function(locat);
			flag_5 = 1;
			continue;
		}
		for (j = i; j < 8; ++j)
		{
			for (k = 0;; k++)
			{
				if (map2[j][k] == '.')
				{
					if (i % 2)
					{
						if (j % 2)
						{
							map2[j][k] = '*';
							map2[j][k + 1] = '.';
						}
					}
					else
					{
						if (j % 2)
						{
							map2[j][k] = '*';
							map2[j][k - 1] = '.';
						}
					}
					break;
				}
			}
		}
		/*system("CLS");
		print(map2);
		delay(1);*/
		move(2, locat);
		locat.pre_x = locat.x;
		locat.pre_y = locat.y;
		map2[locat.x][locat.y] = '*';
	}
}
void level_3()
{
	int i, j, k;
	MyStruct locat;
	locat.x = 2;
	locat.y = 4;
	for (i = 0; i < 8; ++i)
	{
		if (i <= 4)
		{
			map3[locat.x][locat.y] = '*';
			map3[locat.x][locat.y + 1] = '.';
			move(3, locat);
			//locat.y += 1;
		}
		else if (i == 5)
		{
			map3[locat.x][locat.y] = '*';
			map3[locat.x + 1][locat.y - 1] = '.';
			map3[locat.x + 1][locat.y] = '.';
			move(3, locat);
			/*locat.x += 1;
			locat.y -= 1;*/
		}
		else if (i == 6)
		{
			map3[locat.x][locat.y + 1] = '*';
			map3[locat.x][locat.y] = '*';
			map3[7][2] = '.';
			move(3, locat);
			/*locat.x = 7;
			locat.y = 2;*/
		}
		else
		{
			map3[locat.x][locat.y] = '*';
			map3[locat.x + 1][locat.y] = '.';
			move(3, locat);
			//locat.x += 1;
		}
		/*system("CLS");
		print(map3);
		delay(1);*/
	}
}
//-------------------------------------------------------------------------------
void donedone()
{
	printf("done!!!,flag is flag{%s}\n",flag);
	system("pause");
}
int main()
{
	printf("Input your flag:");
	scanf("%s", flag);
	if (strlen(flag) != 64)
		sorry();
	for (int i = 0; i < 64; ++i)
	{
		if (flag[i] >= '0'&&flag[i] <= '9' || flag[i] >= 'A'&&flag[i] <= 'F')
			continue;
		sorry();
	}
	str2num(flag, flag1);
	unsigned char seed_key[] = "DocupaDocupaDocupaDocupa";
	int key_size =192;
	unsigned char key[(56 + 4) * 4];
	int i, j, k;
	char buf[100];
	aes_init();
	aes_set_key(seed_key, key_size, key);
	aes_decrypt((unsigned char*)flag1, (unsigned char*)flag2, key);
	aes_decrypt((unsigned char*)flag1 + 16, (unsigned char*)flag2 + 16, key);
	unsigned int ram[1000];
	srand(10);
	for (int i = 0; i < 1000; ++i)
		ram[i] = rand() % 255;
	vm(flag2,ram);
	p1 = flag2;
	level_1();
	level_2();
	level_3();
	donedone();
	return 0;
}