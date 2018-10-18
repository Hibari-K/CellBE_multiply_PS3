#include<stdio.h>
#include<stdlib.h>
#include<spu_intrinsics.h>
#include<spu_mfcio.h>


void multiply(vec_short8 *a, vec_short8 *b, vec_int4 *t, vec_int4*, vec_int4*, vec_int4*);
void calc_carry(vec_int4 *t, vec_int4 *u, vec_int4 *v, vec_int4 *w);
void combine_16bit(int32_t *data, int32_t *result);

int main(unsigned long long spu_id, unsigned long long argp, unsigned long long envp){

	vec_short8 a[256] __attribute__ ((aligned(128)));
	vec_short8 b[256] __attribute__ ((aligned(128)));
	vec_int4 *t __attribute__ ((aligned(128)));

	t = calloc(512*16, sizeof(long));
	vec_int4 *u = calloc(512*16, sizeof(long));
	vec_int4 *v = calloc(512*16, sizeof(long));
	vec_int4 *w = calloc(512*16, sizeof(long));

	mfc_get(a, argp, sizeof(a), 0, 0, 0);
	mfc_get(b, argp+(4096), sizeof(b), 0, 0, 0);
	mfc_write_tag_mask(1<<0);
	mfc_read_tag_status_all();
	
	int i,j;
	/*
	for(i=0; i<1; i++){
		for(j=0; j<8; j++)
			printf("%x ", a[i][j]);
		puts("");
	}
*/
	
	puts("~~~~~ KERNEL AREA ~~~~~");

	int digits = 16;
	for(i=0; i<digits; i+=2){
		multiply(a+i, b+i, t+i, u+i, v+i, w+i);
	}

	for(i=0; i<8; i++){
		printf("%x ", t[0][i]);
	}
	puts("");

	// ##########  ############

	calc_carry(t, u, v, w);

	free(t);	

	return 0;
}


void multiply(vec_short8 *vec_a, vec_short8 *vec_b, vec_int4 *t, vec_int4* u, vec_int4 *v, vec_int4 *w){
	

	vec_short8 a0 = vec_a[0];
	//vec_short8 a1 = vec_a[1];
	vec_short8 b0 = vec_b[0];
	//vec_short8 b1 = vec_b[1];

	vec_uchar16 pat_fed = {8,9,8,9,10,11,10,11,12,13,12,13,14,15,14,15};
	vec_uchar16 pat_765 = {0,1,0,1,2,3,2,3,4,5,4,5,6,7,6,7};
	vec_uchar16 pat_000 = {0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1};
	vec_uchar16 pat_111 = {2,3,2,3,2,3,2,3,2,3,2,3,2,3,2,3};
	vec_uchar16 pat_222 = {4,5,4,5,4,5,4,5,4,5,4,5,4,5,4,5};
	vec_uchar16 pat_333 = {6,7,6,7,6,7,6,7,6,7,6,7,6,7,6,7};
	vec_uchar16 pat_444 = {8,9,8,9,8,9,8,9,8,9,8,9,8,9,8,9};
	vec_uchar16 pat_555 = {10,11,10,11,10,11,10,11,10,11,10,11,10,11,10,11};
	vec_uchar16 pat_666 = {12,13,12,13,12,13,12,13,12,13,12,13,12,13,12,13};
	vec_uchar16 pat_777 = {14,15,14,15,14,15,14,15,14,15,14,15,14,15,14,15};
	//vec_uchar16 add = {2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2};

	vec_short8 a0h, a0l, a1h, a1l;
	a0h = spu_shuffle(a0, a0, pat_fed);
	a0l = spu_shuffle(a0, a0, pat_765);
	//a1h = spu_shuffle(a1, a1, pat_fed);
	//a1l = spu_shuffle(a1, a1, pat_765);

	vec_short8 b00 = spu_shuffle(b0, b0, pat_000);
	vec_short8 b04 = spu_shuffle(b0, b0, pat_444);
	//vec_short8 b10 = spu_shuffle(b1, b1, pat_000);
	//vec_short8 b14 = spu_shuffle(b1, b1, pat_444);

	// 444 <= 555
	//pat_444 = spu_add(pat_444, add);

	vec_short8 b01 = spu_shuffle(b0, b0, pat_111);
	vec_short8 b05 = spu_shuffle(b0, b0, pat_555);
	//vec_short8 b11 = spu_shuffle(b1, b1, pat_444);
	//vec_short8 b15 = spu_shuffle(b1, b1, pat_444);

	// H
	t[0] = spu_madd(a0l, b00, t[0]);
	// G
	u[0] = spu_madd(a0l, b01, u[0]);
	// A + a
	t[1] = spu_madd(a0h, b00, spu_madd(a0l, b04, t[1]));
	// B + b
	u[1] = spu_madd(a0h, b01, spu_madd(a0l, b05, u[1]));
	// h
	t[2] = spu_madd(a0h, b04, t[2]);
	// g
	u[2] = spu_madd(a0h, b05, u[2]);

	// 111 <= 222, 444 <= 666
	//pat_111 = spu_add(pat_111, add);
	//pat_444 = spu_add(pat_444, add);
	vec_short8 b02 = spu_shuffle(b0, b0, pat_222);
	vec_short8 b06 = spu_shuffle(b0, b0, pat_666);
	// 111 <= 333, 444 <= 777
	//pat_111 = spu_add(pat_111, add);
	//pat_444 = spu_add(pat_444, add);
	vec_short8 b03 = spu_shuffle(b0, b0, pat_333);
	vec_short8 b07 = spu_shuffle(b0, b0, pat_777);
	
	// F
	v[0] = spu_madd(a0l, b02, v[0]);
	// E
	w[0] = spu_madd(a0l, b03, w[0]);
	// C + c
	v[1] = spu_madd(a0h, b02, spu_madd(a0l, b06, v[1]));
	// D + d
	w[1] = spu_madd(a0h, b03, spu_madd(a0l, b07, w[1]));
	// f
	v[2] = spu_madd(a0h, b06, v[2]);
	// e
	w[2] = spu_madd(a0h, b07, w[2]);


}


void calc_carry(vec_int4 *t, vec_int4 *u, vec_int4 *v, vec_int4 *w){

	vec_short8 *t_short = (vec_short8 *)&t[0][0];
	vec_short8 *u_short = (vec_short8 *)&u[0][0];
	vec_short8 *v_short = (vec_short8 *)&v[0][0];
	vec_short8 *w_short = (vec_short8 *)&w[0][0];
	
	int32_t tmp = 0;
	int32_t and12 = 0xfff;

	int i;
	int digits = 16;

	// 0
	tmp = t[0][0];
	t_short[0][0] = t[0][0] & and12;
	tmp >>= 12;

	// 1
	tmp += t[0][1] + u[0][0];
	t_short[0][1] = tmp & and12;
	tmp >>= 12;

	// 2
	tmp += t[0][2] + u[0][1] + v[0][0];
	t_short[0][2] = tmp & and12;
	tmp >>= 12;

	// 3
	tmp += t[0][3] + u[0][2] + v[0][1] + w[0][0];
	t_short[0][3] = tmp & and12;
	tmp >>= 12;

	for(i=4; i<=digits; i++){
		
		tmp += t[0][i] + u[0][i-1] + v[0][i-2] + w[0][i-3];
		t_short[0][i] = tmp & and12;
		tmp >>= 12;
	}
	
}


void combine_16bit(int32_t *data, int32_t *result){

	
}
