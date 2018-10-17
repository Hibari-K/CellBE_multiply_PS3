#include<stdio.h>
#include<spu_intrinsics.h>

int main(unsigned long long spe_id, unsigned long long argp, unsigned long long envp){

	// 32bit * 4
	vec_short8 a = {1, 2, 3, 4, 5, 6, 7, 8};
	vec_short8 b = {1, 2, 3, 4, 5, 6, 7, 8};
	vec_ushort8 ub = {1, 2, 3, 4, 5, 6, 7, 8};
	vec_int4 c = {0xffffff00, 0xffffff00, 0xffffff00, 0xffffff00};
	
	//vec_int4 d = spu_mulo(a, b);
	// int = short short int
	vec_int4 d = spu_madd(a, b, c);

	// make mask register
	char x = 3;
	ub = spu_maskh(x);

	short hoge = 5;
	int y = 8;
	vec_uchar16 z = {0,1,2,3,4,5,6,7,8,9,10,11,14,15,14,15};
	vec_short8 e = spu_shuffle(a, b, z);

	printf("d = {%u, %u, %u, %u}\n", d[0], d[1], d[2], d[3]);
	//printf("d = {%u, %u, %u, %u, %u, %u, %u, %u}\n", e[0], e[1], e[2], e[3], e[4]);

	vec_uint4 uc = {0xffffffff, 0, 0xffffffff, 0};
	uc = spu_slqwbyte(uc, 4);
	printf("uc = {%x, %x, %x, %x}\n", uc[0], uc[1], uc[2], uc[3]);
	
	int i;
	for(i=0; i<8; i++)
		printf("%u, ", e[i]);
	puts("");

	return 0;
}
