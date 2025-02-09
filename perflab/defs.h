/*
 * driver.h - Various definitions for the Performance Lab.
 * 
 * DO NOT MODIFY ANYTHING IN THIS FILE
 */
#ifndef _DEFS_H_
#define _DEFS_H_

#include <stdlib.h>

#define RIDX(i,j,n) ((i)*(n)+(j))

struct student_t
{
	char *name, *email;
};

extern struct student_t student;

struct pixel_t
{
	unsigned short red : 8;
	unsigned short green : 8;
	unsigned short blue : 8;
	unsigned short unused : 8;
};

typedef void (*lab_test_func) (int, struct pixel_t*, struct pixel_t*);

void flip(int, struct pixel_t *, struct pixel_t *);
void register_flip_functions(void);
void add_flip_function(lab_test_func, char*);


void sharpen(int, struct pixel_t *, struct pixel_t *);
void register_sharpen_functions(void);
void add_sharpen_function(lab_test_func, char*);

#endif /* _DEFS_H_ */

