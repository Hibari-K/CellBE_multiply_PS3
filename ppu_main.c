#include<stdio.h>
#include<stdlib.h>
#include<libspe2.h>
#include<sys/time.h>
//#include<spu_mfcio.h>

extern spe_program_handle_t spu_split;
extern spe_program_handle_t spu_kernel;


void error_ret(const char *str){
	perror(str);
	exit(1);
}

//int main(unsigned long long spe_id, unsigned long long argp, unsigned long long envp){
int main(int argc, char **argv){
	
	spe_context_ptr_t spe;
	unsigned int entry_point;
	int retval;
	spe_stop_info_t stop_info;


	unsigned int *a __attribute__ ((aligned(128)));
	unsigned int *b __attribute__ ((aligned(128)));
	unsigned int *t __attribute__ ((aligned(128)));

	if(argc != 2){
		printf("Usage : %s num\n", argv[0]);
		puts("1028->32, 2048->64, 3072->96");
		exit(0);
	}

	int lp = atoi(argv[1]);
	double LOOP = lp * 1.0;

	if(lp > 96){
		printf("%d is too large.", lp);
		exit(0);
	}

	a = calloc(4096, sizeof(long));
	if(!a){
		puts("calloc error");
		exit(1);
	}

	b = &a[1024];
	t = &a[2048];

	srand(time(NULL));

	int i, j;
	for(i=0; i<lp; i++){
		a[i] = 0x11223344;
		b[i] = 0x11223344;
	}


	// check length
	/*
	int size;
	for(i=96; a[i] == 0; i--);
	size = i+1;
	for(i=96; b[i] == 0; i--);
	size = i+1;

	if(size != lp){
		printf("lp = %d, size = %d\n", lp, size);
		exit(0);
	}
*/


	printf("a = ");
	for(i=3; i>=0; i--)
		printf("%08x ", a[i]);
	puts("");
	
	printf("b = ");
	for(i=3; i>=0; i--)
		printf("%08x ", b[i]);
	puts("");

	spe = spe_context_create(0, NULL);

	if(!spe)
		error_ret("spe_context_create");
/*
	retval = spe_program_load(spe, &spu_split);
	if(retval)
		error_ret("spe_program_load");

	entry_point = SPE_DEFAULT_ENTRY;
	retval = spe_context_run(spe, &entry_point, 0, a, NULL, &stop_info);
	if(retval < 0)
		error_ret("spe_contect_run");

	entry_point = SPE_DEFAULT_ENTRY;
	retval = spe_context_run(spe, &entry_point, 0, b, NULL, &stop_info);
	if(retval < 0)
		error_ret("spe_contect_run");
*/	
	
	// multiply kernel
	retval = spe_program_load(spe, &spu_kernel);
	if(retval)
		error_ret("spe_context_create");
	
	struct timeval s, e;
	double ptime = 0.0;
	double total = 0.0;
	
	//for(i=0; i<LOOP; i++){

	//	gettimeofday(&s, NULL);

		entry_point = SPE_DEFAULT_ENTRY;
		retval = spe_context_run(spe, &entry_point, 0, a, NULL, &stop_info);
		if(retval < 0)
			error_ret("spe_context_run");

	//	gettimeofday(&e, NULL);
	//}
	
/*	
	for(i=0; i<LOOP; i++){

		gettimeofday(&s, NULL);

		entry_point = SPE_DEFAULT_ENTRY;
		retval = spe_context_run(spe, &entry_point, 0, a, NULL, &stop_info);
		if(retval < 0)
			error_ret("spe_context_run");

		gettimeofday(&e, NULL);
		total += (e.tv_sec - s.tv_sec) + (e.tv_usec - s.tv_usec)*1.0E-6;
	}
	
	ptime = (total / LOOP) * 1000 * 1000;
	printf("PPU Time = %lf\n", ptime);
*/
	for(i=lp*2-1; i>=0; i--)
		printf("%08x", t[i]);
	puts("");

	retval = spe_context_destroy(spe);
	if(retval)
		error_ret("spe_contect_destroy");

	free(a);

	return 0;
}
