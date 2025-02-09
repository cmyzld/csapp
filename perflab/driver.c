/*******************************************************************
 * 
 * driver.c - Driver program for CS:APP Performance Lab
 * 
 * In kernels.c, students generate an arbitrary number of flip and
 * sharpen test functions, which they then register with the driver
 * program using the add_flip_function() and add_sharpen_function()
 * functions.
 * 
 * The driver program runs and measures the registered test functions
 * and reports their performance.
 * 
 * Copyright (c) 2002, R. Bryant and D. O'Hallaron, All rights
 * reserved.  May not be used, modified, or copied without permission.
 *
 ********************************************************************/

#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <assert.h>
#include <math.h>
#include "fcyc.h"
#include "defs.h"
#include "config.h"

/* Team structure that identifies the students */
extern struct student_t student;

/* Keep track of a number of different test functions */
#define MAX_BENCHMARKS	100
#define DIM_CNT			8
#define MAXBITVALUE		256

/* Misc constants */
#define BSIZE			64				/* cache block size in bytes */
#define MAX_DIM			(8192+256)		/* 4096 + 256 */
//#define ODD_DIM			96				/* not a power of 2 */
#define ODD_DIM			320				/* not a power of 2 */

/* fast versions of min and max */
#define min(a,b) (a < b ? a : b)
#define max(a,b) (a > b ? a : b)

int array_index(int i, int j, int n)
{
	return (i*n) + j;
}

/* This struct characterizes the results for one benchmark test */
struct bench_t
{
	lab_test_func tfunct; /* The test function */
	double cpes[DIM_CNT]; /* One CPE result for each dimension */
	char *description;    /* ASCII description of the test function */
	unsigned short valid; /* The function is tested if this is non zero */
};

/* The range of image dimensions that we will be testing */
//static int test_dim_flip[] = {64, 96, 128, 256, 512, 1024, 2048, 4096, 8192, 16384};
//static int test_dim_sharpen[] = {32, 64, 96, 128, 256, 512, 1024, 2048, 4096, 8192, 16384};
static int test_dim_flip[] =	{64, 128, 256, 320, 512, 1024, 2048, 8192};
static int test_dim_sharpen[] = {64, 128, 256, 320, 512, 1024, 2048, 8192};

/* Baseline CPEs (see config.h) */
//static double flip_baseline_cpes[] = {R64, R96, R128, R256, R512, R1024, R2048, R4096, R8192, R16384};
//static double sharpen_baseline_cpes[] = {S32, S64, S96, S128, S256, S512, S1024, S2048, S4096, S8192, S16384};
static double flip_baseline_cpes[] = 	{R64, R128, R256, R320, R512, R1024, R2048, R8192};
static double sharpen_baseline_cpes[] = {S64, S128, S256, S320, S512, S1024, S2048, S8192};

/* These hold the results for all benchmarks */
static struct bench_t benchmarks_flip[MAX_BENCHMARKS];
static struct bench_t benchmarks_sharpen[MAX_BENCHMARKS];

/* These give the sizes of the above lists */
static int flip_benchmark_count = 0;
static int sharpen_benchmark_count = 0;

/*
 * An image is a dimxdim matrix of pixels stored in a 1D array.  The
 * data array holds three images (the input original, a copy of the original,
 * and the output result array. There is also an additional BSIZE bytes
 * of padding for alignment to cache block boundaries.
 */
static struct pixel_t data[(3*MAX_DIM*MAX_DIM) + (BSIZE/sizeof(struct pixel_t))];

/* Various image pointers */
static struct pixel_t *orig = NULL;			/* original image */
static struct pixel_t *copy_of_orig = NULL;	/* copy of original for checking result */
static struct pixel_t *result = NULL;		/* result image */

/* Keep track of the best flip and sharpen score for grading */
double flip_maxmean = 0.0;
char *flip_maxmean_desc = NULL;

double sharpen_maxmean = 0.0;
char *sharpen_maxmean_desc = NULL;


/******************** Functions begin *************************/

void add_sharpen_function(lab_test_func f, char *description)
{
	benchmarks_sharpen[sharpen_benchmark_count].tfunct = f;
	benchmarks_sharpen[sharpen_benchmark_count].description = description;
	benchmarks_sharpen[sharpen_benchmark_count].valid = 0;
	sharpen_benchmark_count++;
}


void add_flip_function(lab_test_func f, char *description)
{
	benchmarks_flip[flip_benchmark_count].tfunct = f;
	benchmarks_flip[flip_benchmark_count].description = description;
	benchmarks_flip[flip_benchmark_count].valid = 0;
	flip_benchmark_count++;
}

/*
 * random_in_interval - Returns random integer in interval [low, high)
 */
static int random_in_interval(int low, int high)
{
	int size = high - low;
	return (rand()% size) + low;
}

/*
 * create - creates a dimxdim image aligned to a BSIZE byte boundary
 */
static void create(int dim)
{
    int i, j;

    /* Align the images to BSIZE byte boundaries */
    orig = data;
    while ((unsigned long long)orig % BSIZE)
	{
		char* tmp = (char *)orig;
		tmp++;
		orig = (struct pixel_t *)tmp;
	}
    result = orig + dim*dim;
    copy_of_orig = result + dim*dim;

	for(i = 0; i < dim; i++)
	{
		for(j = 0; j < dim; j++)
		{
			/* Original image initialized to random colors */
			orig[RIDX(i,j,dim)].red = random_in_interval(0, MAXBITVALUE);
			orig[RIDX(i,j,dim)].green = random_in_interval(0, MAXBITVALUE);
			orig[RIDX(i,j,dim)].blue = random_in_interval(0, MAXBITVALUE);

			/* Copy of original image for checking result */
			copy_of_orig[RIDX(i,j,dim)].red = orig[RIDX(i,j,dim)].red;
			copy_of_orig[RIDX(i,j,dim)].green = orig[RIDX(i,j,dim)].green;
			copy_of_orig[RIDX(i,j,dim)].blue = orig[RIDX(i,j,dim)].blue;

			/* Result image initialized to all black */
			result[RIDX(i,j,dim)].red = 0;
			result[RIDX(i,j,dim)].green = 0;
			result[RIDX(i,j,dim)].blue = 0;
		}
	}
}


/*
 * compare_pixels - Returns 1 if the two arguments don't have same RGB
 *    values, 0 o.w.
 */
static int compare_pixels(struct pixel_t p1, struct pixel_t p2)
{
    return (p1.red != p2.red) || (p1.green != p2.green) || (p1.blue != p2.blue);
}


/* Make sure the orig array is unchanged */
static int check_orig(int dim)
{
    int i, j;
    for(i = 0; i < dim; i++)
	{
		for(j = 0; j < dim; j++)
		{
		    if(compare_pixels(orig[RIDX(i,j,dim)], copy_of_orig[RIDX(i,j,dim)]))
			{
				printf("\nError: Original image has been changed!\n");
				return 1;
			}
		}
	}
    return 0;
}

/* 
 * check_flip - Make sure the flip actually works. 
 * The orig array should not  have been tampered with! 
 */
static int check_flip(int dim) 
{
    int err = 0;
    int i, j;
    int badi = 0;
    int badj = 0;
    struct pixel_t orig_bad = {0,0,0,0}, res_bad = {0,0,0,0};

    /* return 1 if the original image has been changed */
    if(check_orig(dim))
	{
		return 1;
	}

    for(i = 0; i < dim; i++)
	{
		for(j = 0; j < dim; j++)
		{
//			if(compare_pixels(orig[RIDX(i,j,dim)], result[RIDX(dim-1-i,j,dim)]))
			if(compare_pixels(orig[RIDX(i,j,dim)], result[RIDX(dim-1-i,dim-1-j,dim)]))
			{
				err++;
				badi = i;
				badj = j;
				orig_bad = orig[RIDX(i,j,dim)];
				res_bad = result[RIDX(dim-1-i,j,dim)];
			}
		}
	}

	if(err)
	{
		printf("\n");
		printf("ERROR: Dimension=%d, %d errors\n", dim, err);
		printf("E.g., The following two pixels should have equal value:\n");
		printf("src[%d][%d].{red,green,blue} = {%d,%d,%d}\n", badi, badj, orig_bad.red, orig_bad.green, orig_bad.blue);
		printf("dst[%d][%d].{red,green,blue} = {%d,%d,%d}\n", (dim-1-badi), badj, res_bad.red, res_bad.green, res_bad.blue);
    }
    return err;
}

static struct pixel_t check_average(int dim, int i, int j, struct pixel_t *src) {
	struct pixel_t result;

	double red = 0.0, green = 0.0, blue = 0.0;

	int neighbors = 0;
	for(int fX = max(i-1, 0); fX <= min(i+1, dim-1); fX++)
	{
		for (int fY = max(j-1, 0); fY <= min(j+1, dim-1); fY++)
		{
			red -= src[RIDX(fX, fY, dim)].red;
			green -= src[RIDX(fX, fY, dim)].green;
			blue -= src[RIDX(fX, fY, dim)].blue;
			neighbors++;
		}
	}
	if(neighbors == 4)
	{
		red += 5 * src[RIDX(i,j,dim)].red;
		green += 5 * src[RIDX(i,j,dim)].green;
		blue += 5 * src[RIDX(i,j,dim)].blue;
	}
	else if(neighbors == 6)
	{
		red += 7 * src[RIDX(i,j,dim)].red;
		green += 7 * src[RIDX(i,j,dim)].green;
		blue += 7 * src[RIDX(i,j,dim)].blue;
	}
	else if(neighbors == 9)
	{
		red += 10 * src[RIDX(i,j,dim)].red;
		green += 10 * src[RIDX(i,j,dim)].green;
		blue += 10 * src[RIDX(i,j,dim)].blue;
	}
	else
	{
		//Invalid neighbor count
		fprintf(stderr, "Invalid neighbor count of %d\n", neighbors);
	}

	int r = min(max(0, (int)red), 255);
	int g = min(max(0, (int)green), 255);
	int b = min(max(0, (int)blue), 255);

	result.red = (unsigned short)r;
	result.green = (unsigned short)g;
	result.blue = (unsigned short)b;
	result.unused = 0;
	return result;
}


/*
 * check_sharpen - Make sure the sharpen function actually works.  The
 * orig array should not have been tampered with!
 */
static int check_sharpen(int dim) {
    int err = 0;
    int i, j;
    int badi = 0;
    int badj = 0;
    struct pixel_t right = {0,0,0,0}, wrong = {0,0,0,0};

    /* return 1 if original image has been changed */
    if(check_orig(dim))
	{
		return 1;
	}

    for(i = 0; i < dim; i++)
	{
		for(j = 0; j < dim; j++)
		{
			struct pixel_t sharpened = check_average(dim, i, j, orig);
			if(compare_pixels(result[RIDX(i,j,dim)], sharpened))
			{
				err++;
				badi = i;
				badj = j;
				wrong = result[RIDX(i,j,dim)];
				right = sharpened;
			}
		}
	}

    if(err)
	{
		printf("\n");
		printf("ERROR: Dimension=%d, %d errors\n", dim, err);
		printf("E.g., \n");
		printf("You have dst[%d][%d].{red,green,blue} = {%d,%d,%d}\n", badi, badj, wrong.red, wrong.green, wrong.blue);
		printf("It should be dst[%d][%d].{red,green,blue} = {%d,%d,%d}\n", badi, badj, right.red, right.green, right.blue);
	}

    return err;
}


void func_wrapper(void *arglist[])
{
    struct pixel_t *src, *dst;
    int mydim;
    lab_test_func f;

    f = (lab_test_func) arglist[0];
    mydim = *((int *) arglist[1]);
    src = (struct pixel_t *) arglist[2];
    dst = (struct pixel_t *) arglist[3];

    (*f)(mydim, src, dst);

    return;
}

void run_flip_benchmark(int idx, int dim) 
{
	benchmarks_flip[idx].tfunct(dim, orig, result);
}

void test_flip(int bench_index) 
{
    int i;
    int test_num;
    char *description = benchmarks_flip[bench_index].description;

    for (test_num = 0; test_num < DIM_CNT; test_num++)
	{
		int dim;

		/* Check for odd dimension */
		create(ODD_DIM);
		run_flip_benchmark(bench_index, ODD_DIM);
		if(check_flip(ODD_DIM))
		{
			printf("Benchmark \"%s\" failed correctness check for dimension %d.\n", benchmarks_flip[bench_index].description, ODD_DIM);
			return;
		}

		/* Create a test image of the required dimension */
		dim = test_dim_flip[test_num];
		create(dim);
#ifdef DEBUG
		printf("DEBUG: Running benchmark \"%s\"\n", benchmarks_flip[bench_index].description);
#endif

		/* Check that the code works */
		run_flip_benchmark(bench_index, dim);
		if(check_flip(dim))
		{
			printf("Benchmark \"%s\" failed correctness check for dimension %d.\n", benchmarks_flip[bench_index].description, dim);
			return;
		}

		/* Measure CPE */
		{
	    	double num_cycles, cpe;
	    	int tmpdim = dim;
	    	void *arglist[4];
	    	double dimension = (double) dim;
	    	double work = dimension*dimension;
#ifdef DEBUG
			printf("DEBUG: dimension=%.1f\n",dimension);
			printf("DEBUG: work=%.1f\n",work);
#endif
			arglist[0] = (void *) benchmarks_flip[bench_index].tfunct;
			arglist[1] = (void *) &tmpdim;
			arglist[2] = (void *) orig;
			arglist[3] = (void *) result;

			create(dim);
			num_cycles = fcyc_v((test_funct_v)&func_wrapper, arglist); 
			cpe = num_cycles/work;
			benchmarks_flip[bench_index].cpes[test_num] = cpe;
		}
	}

    /* Print results as a table */
	printf("Flip Version = %s:\n", description);
	printf("Dim\t");
	for(i = 0; i < DIM_CNT; i++)
	{
		printf("\t%d", test_dim_flip[i]);
	}
	printf("\tMean\n");

	printf("Your CPEs");
	for(i = 0; i < DIM_CNT; i++)
	{
		printf("\t%.1f", benchmarks_flip[bench_index].cpes[i]);
	}
    printf("\n");

	printf("Baseline CPEs");
    for(i = 0; i < DIM_CNT; i++)
	{
		printf("\t%.1f", flip_baseline_cpes[i]);
	}
	printf("\n");

	/* Compute Speedup */
	{
		double prod, ratio, mean;
		prod = 1.0; /* Geometric mean */
		printf("Speedup\t");
		for(i = 0; i < DIM_CNT; i++)
		{
			if(benchmarks_flip[bench_index].cpes[i] > 0.0)
			{
				ratio = flip_baseline_cpes[i]/benchmarks_flip[bench_index].cpes[i];
			}
			else
			{
				printf("Fatal Error: Non-positive CPE value...\n");
				exit(EXIT_FAILURE);
			}
			prod *= ratio;
			printf("\t%.1f", ratio);
		}

		/* Geometric mean */
		mean = pow(prod, 1.0/(double) DIM_CNT);
		printf("\t%.1f", mean);
		printf("\n\n");
		if(mean > flip_maxmean)
		{
			flip_maxmean = mean;
			flip_maxmean_desc = benchmarks_flip[bench_index].description;
		}
	}

#ifdef DEBUG
	fflush(stdout);
#endif
}

void run_sharpen_benchmark(int idx, int dim)
{
	benchmarks_sharpen[idx].tfunct(dim, orig, result);
}

void test_sharpen(int bench_index)
{
    int i;
    int test_num;
    char *description = benchmarks_sharpen[bench_index].description;

    for(test_num=0; test_num < DIM_CNT; test_num++)
	{
		int dim;

		/* Check correctness for odd (non power of two dimensions */
		create(ODD_DIM);
		run_sharpen_benchmark(bench_index, ODD_DIM);
		if(check_sharpen(ODD_DIM))
		{
	    	printf("Benchmark \"%s\" failed correctness check for dimension %d.\n", benchmarks_sharpen[bench_index].description, ODD_DIM);
			return;
		}

		/* Create a test image of the required dimension */
		dim = test_dim_sharpen[test_num];
		create(dim);

#ifdef DEBUG
		printf("DEBUG: Running benchmark \"%s\"\n", benchmarks_sharpen[bench_index].description);
#endif
		/* Check that the code works */
		run_sharpen_benchmark(bench_index, dim);
		if(check_sharpen(dim))
		{
	    	printf("Benchmark \"%s\" failed correctness check for dimension %d.\n", benchmarks_sharpen[bench_index].description, dim);
			return;
		}

		/* Measure CPE */
		{
		    double num_cycles, cpe;
		    int tmpdim = dim;
		    void *arglist[4];
		    double dimension = (double) dim;
		    double work = dimension*dimension;
#ifdef DEBUG
		    printf("DEBUG: dimension=%.1f\n",dimension);
		    printf("DEBUG: work=%.1f\n",work);
#endif
			arglist[0] = (void *) benchmarks_sharpen[bench_index].tfunct;
		    arglist[1] = (void *) &tmpdim;
		    arglist[2] = (void *) orig;
		    arglist[3] = (void *) result;

		    create(dim);
		    num_cycles = fcyc_v((test_funct_v)&func_wrapper, arglist);
		    cpe = num_cycles/work;
		    benchmarks_sharpen[bench_index].cpes[test_num] = cpe;
		}
	}

    /* Print results as a table */
    printf("Sharpen: Version = %s:\n", description);
    printf("Dim\t");
    for(i = 0; i < DIM_CNT; i++)
	{
		printf("\t%d", test_dim_sharpen[i]);
	}
    printf("\tMean\n");

    printf("Your CPEs");
    for(i = 0; i < DIM_CNT; i++)
	{
		printf("\t%.1f", benchmarks_sharpen[bench_index].cpes[i]);
    }
    printf("\n");

    printf("Baseline CPEs");
    for(i = 0; i < DIM_CNT; i++)
	{
		printf("\t%.1f", sharpen_baseline_cpes[i]);
    }
    printf("\n");

    /* Compute speedup */
    {
		double prod, ratio, mean;
		prod = 1.0; /* Geometric mean */
		printf("Speedup\t");
		for(i = 0; i < DIM_CNT; i++)
		{
		    if(benchmarks_sharpen[bench_index].cpes[i] > 0.0)
			{
				ratio = sharpen_baseline_cpes[i] / benchmarks_sharpen[bench_index].cpes[i];
		    }
		    else
			{
				printf("Fatal Error: Non-positive CPE value...\n");
				exit(EXIT_FAILURE);
		    }
		    prod *= ratio;
		    printf("\t%.1f", ratio);
		}
		/* Geometric mean */
		mean = pow(prod, 1.0/(double) DIM_CNT);
		printf("\t%.1f", mean);
		printf("\n\n");
		if(mean > sharpen_maxmean)
		{
		    sharpen_maxmean = mean;
		    sharpen_maxmean_desc = benchmarks_sharpen[bench_index].description;
		}
	}
}


void usage(char *progname)
{
    fprintf(stderr, "Usage: %s [-hqg] [-f <func_file>] [-d <dump_file>]\n", progname);
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  -h         Print this message\n");
    fprintf(stderr, "  -q         Quit after dumping (use with -d )\n");
    fprintf(stderr, "  -g         Autograder mode: checks only flip() and sharpen()\n");
    fprintf(stderr, "  -f <file>  Get test function names from dump file <file>\n");
    fprintf(stderr, "  -d <file>  Emit a dump file <file> for later use with -f\n");
    exit(EXIT_FAILURE);
}



int main(int argc, char *argv[])
{
    int i;
    int quit_after_dump = 0;
    int skip_teamname_check = 0;
    int autograder = 0;
    int seed = 1729;
    char c = '0';
    char *bench_func_file = NULL;
    char *func_dump_file = NULL;

    /* register all the defined functions */
    register_flip_functions();
    register_sharpen_functions();

    /* parse command line args */
    while((c = getopt(argc, argv, "tgqf:d:s:h")) != -1)
	{
		switch(c)
		{
		case 't': /* skip team name check (hidden flag) */
		    skip_teamname_check = 1;
		    break;
		case 's': /* seed for random number generator (hidden flag) */
		    seed = atoi(optarg);
		    break;
		case 'g': /* autograder mode (checks only flip() and sharpen()) */
		    autograder = 1;
		    break;
		case 'q':
		    quit_after_dump = 1;
		    break;
		case 'f': /* get names of benchmark functions from this file */
		    bench_func_file = strdup(optarg);
		    break;
		case 'd': /* dump names of benchmark functions to this file */
		    func_dump_file = strdup(optarg);
		    {
				int i;
				FILE *fp = fopen(func_dump_file, "w");	
				if(fp == NULL)
				{
				    printf("Can't open file %s\n",func_dump_file);
				    exit(-5);
				}

				for(i = 0; i < flip_benchmark_count; i++)
				{
				    fprintf(fp, "R:%s\n", benchmarks_flip[i].description);
				}
				for(i = 0; i < sharpen_benchmark_count; i++)
				{
				    fprintf(fp, "S:%s\n", benchmarks_sharpen[i].description);
				}
				fclose(fp);
	    	}
		    break;
		case 'h': /* print help message */
		    usage(argv[0]);
		default: /* unrecognized argument */
		    usage(argv[0]);
		}
	}

    if(quit_after_dump)
	{
		exit(EXIT_SUCCESS);
	}


    /* Print team info */
    if(!skip_teamname_check)
	{
		if(strcmp("bovik", student.name) == 0)
		{
		    printf("%s: Please fill in the team struct in kernels.c.\n", argv[0]);
		    exit(1);
		}
		printf("Student: %s\n", student.name);
		printf("Email: %s\n", student.email);
		printf("\n");
    }

    srand(seed);

    /* 
     * If we are running in autograder mode, we will only test
     * the flip() and bench() functions.
     */
    if(autograder)
	{
		flip_benchmark_count = 1;
		sharpen_benchmark_count = 1;

		benchmarks_flip[0].tfunct = flip;
		benchmarks_flip[0].description = "flip() function";
		benchmarks_flip[0].valid = 1;

		benchmarks_sharpen[0].tfunct = sharpen;
		benchmarks_sharpen[0].description = "sharpen() function";
		benchmarks_sharpen[0].valid = 1;
    }
    /* 
     * If the user specified a file name using -f, then use
     * the file to determine the versions of flip and sharpen to test
     */
    else if(bench_func_file != NULL)
	{
		char flag;
		char func_line[256];
		FILE *fp = fopen(bench_func_file, "r");

		if(fp == NULL)
		{
		    printf("Can't open file %s\n",bench_func_file);
		    exit(-5);
		}

		while(func_line == fgets(func_line, 256, fp))
		{
		    char *func_name = func_line;
		    char **strptr = &func_name;
		    char *token = strsep(strptr, ":");
		    flag = token[0];
		    func_name = strsep(strptr, "\n");
#ifdef DEBUG
		    printf("Function Description is %s\n",func_name);
#endif
		    if(flag == 'R')
			{
				for(i=0; i<flip_benchmark_count; i++)
				{
		    		if(strcmp(benchmarks_flip[i].description, func_name) == 0)
					{
						benchmarks_flip[i].valid = 1;
					}
				}
	    	}
	    	else if(flag == 'S')
			{
				for(i=0; i<sharpen_benchmark_count; i++)
				{
		    		if(strcmp(benchmarks_sharpen[i].description, func_name) == 0)
					{
						benchmarks_sharpen[i].valid = 1;
					}
				}
	    	}
		}
		fclose(fp);
    }
    /* 
     * If the user didn't specify a dump file using -f, then 
     * test all of the functions
     */
    else
	{ /* set all valid flags to 1 */
		for(i = 0; i < flip_benchmark_count; i++)
		{
	    	benchmarks_flip[i].valid = 1;
		}
		for(i = 0; i < sharpen_benchmark_count; i++)
		{
	    	benchmarks_sharpen[i].valid = 1;
		}
    }

    /* Set measurement (fcyc) parameters */
    set_fcyc_cache_size(1 << 14); /* 16 KB cache size */
    set_fcyc_clear_cache(1); /* clear the cache before each measurement */
    set_fcyc_compensate(1); /* try to compensate for timer overhead */

    for(i = 0; i < flip_benchmark_count; i++)
	{
		if(benchmarks_flip[i].valid)
		{
	 	   test_flip(i);
		}
	}

    for(i = 0; i < sharpen_benchmark_count; i++)
	{
		if(benchmarks_sharpen[i].valid)
		{
	    	test_sharpen(i);
		}
    }


    if(autograder)
	{
		printf("\nbestscores:%.1f:%.1f:\n", flip_maxmean, sharpen_maxmean);
    }
    else
	{
		printf("Summary of Your Best Scores:\n");
		printf("  Flip: %3.1f (%s)\n", flip_maxmean, flip_maxmean_desc);
		printf("  Sharpen: %3.1f (%s)\n", sharpen_maxmean, sharpen_maxmean_desc);
    }

    return 0;
}
