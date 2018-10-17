#include<stdio.h>
#include<stdlib.h>
#include<spu_intrinsics.h>
#include<spu_mfcio.h>

//int split_12bit(unsigned short *data, unsigned short *result, int digits){
int main(unsigned long long spu_id, unsigned long long argp, unsigned long long envp){


	// initialize
	vec_int4 and1 = {0, 0, 0, 0xfff};
	vec_int4 and2 = {0, 0, 0, 0xfff000};
	vec_int4 and3 = {0, 0, 0xf, 0xff000000};
	vec_int4 and4 = {0, 0, 0xfff0, 0};
	vec_int4 and5 = {0, 0, 0xfff0000, 0};
	vec_int4 and6 = {0, 0xff, 0xf0000000, 0};
	vec_int4 and7 = {0, 0xfff00, 0, 0};
	vec_int4 and8 = {0, 0xfff00000, 0, 0};

	int32_t data[1024] __attribute__ ((aligned(128)));
	vec_int4 result[256] __attribute__ ((aligned(128)));

	mfc_get(data, argp, sizeof(data), 0, 0, 0);
	mfc_write_tag_mask(1<<0);
	mfc_read_tag_status_all();

	int i, j;
	int digits = 16;
	vec_int4 tmp, res;

	
	/* DEBUG */
	/*
	printf("#######IN SPLIT##########");
	for(i=0; i<8; i++)
		printf("%x ", data[i]);
	puts("");
*/

	// so many data dependency
	// it is better to change this algorithm
	for(i=0, j=0; i<digits; i+=3, j++){
		
		vec_int4 vector = {data[i], data[i+1], data[i+2], data[i+3]};

		// 0
		res = spu_and(and1, vector);

		// 1
		tmp = spu_and(and2, vector);
		tmp = spu_slqw(tmp, 4);
		res = spu_xor(tmp, res);

		// 2
		tmp = spu_and(and3, vector);
		tmp = spu_slqwbyte(tmp, 1);
		res = spu_xor(tmp, res);

		// 3
		tmp = spu_and(and4, vector);
		tmp = spu_slqw(tmp, 4);
		tmp = spu_slqwbyte(tmp, 1);
		res = spu_xor(tmp, res);

		// 4
		tmp = spu_and(and5, vector);
		tmp = spu_slqwbyte(tmp, 2);
		res = spu_xor(tmp, res);

		// 5
		tmp = spu_and(and6, vector);
		tmp = spu_slqw(tmp, 4);
		tmp = spu_slqwbyte(tmp, 2);
		res = spu_xor(tmp, res);

		// 6
		tmp = spu_and(and7, vector);
		tmp = spu_slqwbyte(tmp, 3);
		res = spu_xor(tmp, res);

		// 7
		tmp = spu_and(and8, vector);
		tmp = spu_slqw(tmp, 4);
		tmp = spu_slqwbyte(tmp, 3);
		res = spu_xor(tmp, res);

		result[j] = res;
	}


/*
	for(i=0; i<32; i++)
		printf("%x ", result[0][i]);
	puts("");
*/

	mfc_put(result, argp, sizeof(result), 0, 0, 0);
	mfc_write_tag_mask(1<<0);
	mfc_read_tag_status_all();

	return 0;
}

