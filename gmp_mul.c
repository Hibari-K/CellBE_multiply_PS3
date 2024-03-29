#include<stdio.h>
#include<gmp.h>
#include<stdlib.h>
#include<sys/time.h>


//#define LOOP 30.0
#define BASE 16

void gmp_mul(char* arg1, char* arg2, double LOOP);

int main(int argc, char** argv){

	if(argc != 2){
		printf("Usage : %s num\n", argv[0]);
		puts("1028->32, 2048->64, 3072->96");
		exit(0);
	}

	int lp = atoi(argv[1]);
	double LOOP = lp * 1.0;
	int i;

	char *str1 = malloc(sizeof(char)*2048);
	char *str2 = malloc(sizeof(char)*2048);

	if(!(str1 && str2)){
		puts("malloc error");
		exit(0);
	}

	int temp = 0x11223344;
	char *tmpa = str1;
	char *tmpb = str2;

	for(i=lp-1; i>=0; i--){
		snprintf(tmpa, 9, "%08x", temp);
		snprintf(tmpb, 9, "%08x", temp);
		tmpa += 8;
		tmpb += 8;
	}

	gmp_mul(str1, str2, LOOP);

	
	FILE *result = fopen("result_gmp.txt", "r");
	if(result == NULL){
		puts("file open error");
		exit(0);
	}

	char str[512];
	while(fgets(str, 512, result) != NULL) {
		printf("%s", str);
	}

	fclose(result);
	
	free(str1);
	free(str2);

	return 0;
	
}

void gmp_mul(char* arg1, char* arg2, double LOOP){
	mpz_t a, b, res;

/*
	mpz_init2(a, 8192);
	mpz_init2(b, 8192);
	mpz_init2(res, 16384);
*/
	
	mpz_init(a);
	mpz_init(b);
	mpz_init(res);
	

	int i;
	
	struct timeval s, e;
	double total = 0.0;
	double time = 0.0;
	
	
	mpz_set_str(a,arg1,BASE);
	mpz_set_str(b,arg2,BASE);

	//mpz_out_str(stdout, 16, b);
	//mpz_set(b, a);

	for(i=0; i<LOOP; i++){

		gettimeofday(&s, NULL);
		mpz_mul(res, a, b);
		gettimeofday(&e, NULL);

		total += (e.tv_sec - s.tv_sec) + (e.tv_usec - s.tv_usec)*1.0E-6;
	}

	time = (total / LOOP*1.0)*1000*1000;
	//printf("GMP\t\t: Average time = %lf [us]\n", time);
    printf("%lf\n", time);


	FILE *result = fopen("result_gmp.txt", "w");

	mpz_out_str(result, BASE, res);
	fprintf(result, "\n");

	fclose(result);

	//mpz_out_str(stdout, 16, b);
	//printf("\n");

	mpz_clear(a);
	mpz_clear(b);
	mpz_clear(res);

	//free(str_b);

}
