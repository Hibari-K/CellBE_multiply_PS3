#include<stdio.h>
#include<stdlib.h>
#include<libspe2.h>
//#include<spu_mfcio.h>

extern spe_program_handle_t spu_split;
extern spe_program_handle_t spu_kernel;


void error_ret(const char *str){
	perror(str);
	exit(1);
}

int main(unsigned long long spe_id, unsigned long long argp, unsigned long long envp){
	
	spe_context_ptr_t spe;
	unsigned int entry_point;
	int retval;
	spe_stop_info_t stop_info;


	unsigned int *a __attribute__ ((aligned(128)));
	unsigned int *b __attribute__ ((aligned(128)));
	unsigned int *t __attribute__ ((aligned(128)));

/*
	a = calloc(768, sizeof(long));
	b = calloc(768, sizeof(long));
	t = calloc(1536, sizeof(long));
*/

	a = calloc(4096, sizeof(long));
	b = &a[1024];
	t = &a[2048];

	printf("a = %x, b = %x, sub = %d\n", a, b, b-a);

	if(!(a && b && t)){
		puts("calloc error");
		exit(1);
	}

	srand(time(NULL));

	int i, j;
	for(i=0; i<4; i++){
		a[i] = 0x11111111;
		b[i] = 0x11111111;
	}

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

	retval = spe_program_load(spe, &spu_split);
	if(retval)
		error_ret("spe_program_load");

	entry_point = SPE_DEFAULT_ENTRY;
	retval = spe_context_run(spe, &entry_point, 0, a, NULL, &stop_info);
	if(retval < 0)
		error_ret("spe_contect_run");

	for(i=0; i<16; i++)
		printf("%d : %x\n", i, a[i]);

	entry_point = SPE_DEFAULT_ENTRY;
	retval = spe_context_run(spe, &entry_point, 0, b, NULL, &stop_info);
	if(retval < 0)
		error_ret("spe_contect_run");

	
/*
	puts("##### Here's PPU space #####");
	for(i=0; i<16; i++)
		printf("%x ", a[i]);
	puts("");
	for(i=0; i<16; i++)
		printf("%x ", b[i]);
	puts("");
*/

	// multiply kernel
	retval = spe_program_load(spe, &spu_kernel);
	if(retval)
		error_ret("spe_context_create");
	
	entry_point = SPE_DEFAULT_ENTRY;
	retval = spe_context_run(spe, &entry_point, 0, a, NULL, &stop_info);
	if(retval < 0)
		error_ret("spe_context_run");
	
	
	printf("0x");
	for(i=7; i>=0; i--)
		printf("%08x", t[i]);
	puts("");

	puts("0x123456789abcdf0123456789abcdf0120fedcba987654320fedcba987654321");

	retval = spe_context_destroy(spe);
	if(retval)
		error_ret("spe_contect_destroy");

	free(a);

	return 0;
}
