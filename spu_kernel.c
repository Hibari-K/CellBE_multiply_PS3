#include<stdio.h>
#include<stdlib.h>
#include<spu_intrinsics.h>
#include<spu_mfcio.h>


void multiply(vec_short8 *a, vec_short8 *b, vec_int4 *t, vec_int4*, vec_int4*, vec_int4*);
void split_12bit(int32_t *data, vec_int4 *result, int digits);
void calc_carry(vec_int4 *t, vec_int4 *u, vec_int4 *v, vec_int4 *w, int digits);
void combine_16bit(vec_int4 *data, int32_t *result, int digits);

int main(unsigned long long spu_id, unsigned long long argp, unsigned long long envp){

	int32_t a_data[256] __attribute__ ((aligned(128)));
	int32_t b_data[256] __attribute__ ((aligned(128)));
	vec_short8 a[256] __attribute__ ((aligned(128)));
	vec_short8 b[256] __attribute__ ((aligned(128)));
	int32_t *t __attribute__ ((aligned(128)));

	t = calloc(512*16, sizeof(long));
	vec_int4 *t_tmp = calloc(512*16, sizeof(long));
	vec_int4 *u = calloc(512*16, sizeof(long));
	vec_int4 *v = calloc(512*16, sizeof(long));
	vec_int4 *w = calloc(512*16, sizeof(long));

	mfc_get(a_data, argp, sizeof(a_data), 0, 0, 0);
	mfc_get(b_data, argp+(4096), sizeof(b_data), 0, 0, 0);
	mfc_write_tag_mask(1<<0);
	mfc_read_tag_status_all();
	
	int i,j;
	
	puts("~~~~~ KERNEL AREA ~~~~~");

	int digits, offset;

	// check length
	for(i=95; a_data[i] == 0; i--);
	digits = i+1;
	digits *= 3;

	// ######### split 12bit #########
	split_12bit(a_data, (vec_int4*)a, digits);
	split_12bit(b_data, (vec_int4*)b, digits);


	for(i=0; i<digits; i++){
		for(j=0; j<digits; j++){

			offset = (i+j) * 2;		
			multiply(a+j, b+i, t_tmp+offset, u+offset, v+offset, w+offset);
		}
	}

	// ########## calc carry  ############

	calc_carry(t_tmp, u, v, w, digits*2);

	combine_16bit(t_tmp, t, digits);

	mfc_put(t, argp+(8192), 8192, 0, 0, 0);
	mfc_write_tag_mask(1<<0);
	mfc_read_tag_status_all();

	free(t);	
	free(t_tmp);	
	free(u);	
	free(v);	
	free(w);	

	return 0;
}


void multiply(vec_short8 *vec_a, vec_short8 *vec_b, vec_int4 *t, vec_int4* u, vec_int4 *v, vec_int4 *w){
	

	vec_short8 a0 = vec_a[0];
	//vec_short8 a1 = vec_a[1];
	vec_short8 b0 = vec_b[0];
	//vec_short8 b1 = vec_b[1];

	vec_uchar16 pat_fed = {0x80,0x80,8,9,0x80,0x80,10,11,0x80,0x80,12,13,0x80,0x80,14,15 };
	vec_uchar16 pat_765 = {0x80,0x80,0,1,0x80,0x80,2,3,0x80,0x80,4,5,0x80,0x80,6,7};
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

	// For debug
	//printf("t0 ==== %x %x %x %x %x %x %x %x %x %x %x %x\n", t[0][0], t[0][1], t[0][2], t[0][3], t[0][4], t[0][5], t[0][6], t[0][7], t[0][8], t[0][9], t[0][10], t[0][11]);
	//printf("u0 ==== %x %x %x %x %x %x %x %x %x %x %x %x\n", u[0][0], u[0][1], u[0][2], u[0][3], u[0][4], u[0][5], u[0][6], u[0][7], u[0][8], u[0][9], u[0][10], u[0][11]);
	//printf("v0 ==== %x %x %x %x %x %x %x %x %x %x %x %x\n", v[0][0], v[0][1], v[0][2], v[0][3], v[0][4], v[0][5], v[0][6], v[0][7], v[0][8], v[0][9], v[0][10], v[0][11]);
	//printf("w0 ==== %x %x %x %x %x %x %x %x %x %x %x %x\n", w[0][0], w[0][1], w[0][2], w[0][3], w[0][4], w[0][5], w[0][6], w[0][7], w[0][8], w[0][9], w[0][10], w[0][11]);

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

void split_12bit(int32_t *data, vec_int4 *result, int digits){

	// initialize
	vec_int4 and1 = {0, 0, 0, 0xfff};
	vec_int4 and2 = {0, 0, 0, 0xfff000};
	vec_int4 and3 = {0, 0, 0xf, 0xff000000};
	vec_int4 and4 = {0, 0, 0xfff0, 0};
	vec_int4 and5 = {0, 0, 0xfff0000, 0};
	vec_int4 and6 = {0, 0xff, 0xf0000000, 0};
	vec_int4 and7 = {0, 0xfff00, 0, 0};
	vec_int4 and8 = {0, 0xfff00000, 0, 0};

	vec_uchar16 pat_short = {14,15,12,13,10,11,8,9,6,7,4,5,2,3,0,1};

	int i, j;
	//int digits = 16;
	vec_int4 tmp, res;

	digits *= 2;
	
	// caution : PS3 is big-endian architecture
	// so many data dependency
	// it is better to change this algorithm
	for(i=0, j=0; i<=digits; i+=3, j++){
		
		vec_int4 vector = {data[i+3], data[i+2], data[i+1], data[i]};

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
		tmp = spu_slqwbyte(tmp, 1);
		tmp = spu_slqw(tmp, 4);
		res = spu_xor(tmp, res);

		// 4
		tmp = spu_and(and5, vector);
		tmp = spu_slqwbyte(tmp, 2);
		res = spu_xor(tmp, res);

		// 5
		tmp = spu_and(and6, vector);
		tmp = spu_slqwbyte(tmp, 2);
		tmp = spu_slqw(tmp, 4);
		res = spu_xor(tmp, res);

		// 6
		tmp = spu_and(and7, vector);
		tmp = spu_slqwbyte(tmp, 3);
		res = spu_xor(tmp, res);

		// 7
		tmp = spu_and(and8, vector);
		tmp = spu_slqwbyte(tmp, 3);
		tmp = spu_slqw(tmp, 4);
		res = spu_xor(tmp, res);

		res = spu_shuffle(res, res, pat_short);

		result[j] = res;
	}
}


void calc_carry(vec_int4 *t, vec_int4 *u, vec_int4 *v, vec_int4 *w, int digits){

	vec_short8 *t_short = (vec_short8 *)&t[0][0];
	vec_short8 *u_short = (vec_short8 *)&u[0][0];
	vec_short8 *v_short = (vec_short8 *)&v[0][0];
	vec_short8 *w_short = (vec_short8 *)&w[0][0];
	
	int32_t tmp = 0;
	int32_t and12 = 0xfff;

	int i;

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


//void combine_16bit(int32_t *data, int32_t *result){
void combine_16bit(vec_int4 *data, int32_t *result, int digits){

	vec_uchar16 pat_rev = {14,15,12,13,10,11,8,9,6,7,4,5,2,3,0,1};
	
	vec_short8 and1 = {0, 0, 0, 0, 0, 0, 0, 0xfff};
	vec_short8 and2 = {0, 0, 0, 0, 0, 0, 0xff, 0xf000};
	vec_short8 and3 = {0, 0, 0, 0, 0, 0xf, 0xff00, 0};
	vec_short8 and4 = {0, 0, 0, 0, 0, 0xfff0, 0, 0};
	vec_short8 and5 = {0, 0, 0, 0, 0xfff, 0, 0, 0};
	vec_short8 and6 = {0, 0, 0, 0xff, 0xf000, 0, 0, 0};
	vec_short8 and7 = {0, 0, 0xf, 0xff00, 0, 0, 0, 0};
	vec_short8 and8 = {0, 0, 0xfff0, 0, 0, 0, 0, 0};

	int i, j;

	vec_short8 tmp, res;
	
	for(i=0, j=0; j<digits; i++, j+=3){

		vec_short8 vector = (vec_short8)data[i];

		vector = spu_shuffle(vector, vector, pat_rev);

		//printf("vector = %04x %04x %04x %04x %04x %04x %04x %04x\n", vector[0], vector[1], vector[2], vector[3], vector[4], vector[5], vector[6], vector[7]);

		// 0
		res = spu_and(and1, vector);
		//printf("res0 = %x %x %x %x %x %x %x %x\n", res[0], res[1], res[2], res[3], res[4], res[5], res[6], res[7]);

		// 1
		vector = (vec_short8)si_rotqmbii((qword)vector,-4);
		res = spu_xor(res, spu_and(and2, vector));
		//printf("res1 = %x %x %x %x %x %x %x %x\n", res[0], res[1], res[2], res[3], res[4], res[5], res[6], res[7]);

		// 2
		vector = (vec_short8)si_rotqmbii((qword)vector,-4);
		res = spu_xor(res, spu_and(and3, vector));
		//printf("res2 = %x %x %x %x %x %x %x %x\n", res[0], res[1], res[2], res[3], res[4], res[5], res[6], res[7]);

		// 3
		vector = (vec_short8)si_rotqmbii((qword)vector,-4);
		res = spu_xor(res, spu_and(and4, vector));
		//printf("res3 = %x %x %x %x %x %x %x %x\n", res[0], res[1], res[2], res[3], res[4], res[5], res[6], res[7]);

		// 4
		vector = (vec_short8)si_rotqmbii((qword)vector,-4);
		res = spu_xor(res, spu_and(and5, vector));
		//printf("res4 = %x %x %x %x %x %x %x %x\n", res[0], res[1], res[2], res[3], res[4], res[5], res[6], res[7]);
	
		// 5
		vector = (vec_short8)si_rotqmbii((qword)vector,-4);
		res = spu_xor(res, spu_and(and6, vector));
		//printf("res5 = %x %x %x %x %x %x %x %x\n", res[0], res[1], res[2], res[3], res[4], res[5], res[6], res[7]);

		// 6
		vector = (vec_short8)si_rotqmbii((qword)vector,-4);
		res = spu_xor(res, spu_and(and7, vector));
		//printf("res6 = %x %x %x %x %x %x %x %x\n", res[0], res[1], res[2], res[3], res[4], res[5], res[6], res[7]);
		
		// 7
		vector = (vec_short8)si_rotqmbii((qword)vector,-4);
		res = spu_xor(res, spu_and(and8, vector));
		//printf("res7 = %x %x %x %x %x %x %x %x\n", res[0], res[1], res[2], res[3], res[4], res[5], res[6], res[7]);


		result[j]   = ((vec_int4)res)[3];
		result[j+1] = ((vec_int4)res)[2];
		result[j+2] = ((vec_int4)res)[1];
	
		//printf("%x %x %x\n", result[j+2], result[j+1], result[j]);
	}
}
