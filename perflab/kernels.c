/********************************************************
 * Kernels to be optimized for the CS:APP Performance Lab
 ********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defs.h"

/* Please fill in the following team struct */
struct student_t student = {
/* Full name, email address */
	"",
	"",
};

/***************
 * FLIP KERNEL
 ***************/

/******************************************************
 * Your different versions of the flip kernel go here
 ******************************************************/

/*
 * naive_flip - The naive baseline version of flip
 */
char naive_flip_descr[] = "naive_flip: Naive baseline implementation";
void naive_flip(int dim, struct pixel_t *src, struct pixel_t *dst)
{
	for(int i = 0; i < dim; i+=1)
	{
		for(int j = 0; j < dim; j+=1)
		{
			dst[RIDX(dim-1-i, dim-1-j, dim)].red = src[RIDX(i, j, dim)].red;
			dst[RIDX(dim-1-i, dim-1-j, dim)].green = src[RIDX(i, j, dim)].green;
			dst[RIDX(dim-1-i, dim-1-j, dim)].blue = src[RIDX(i, j, dim)].blue;
			dst[RIDX(dim-1-i, dim-1-j, dim)].unused = src[RIDX(i, j, dim)].unused;
		}
	}
}

//I[RIDX(i,j,n)] : M(i,j) n is dimension of image matrix
//RIDX(i,j,n) = i*n + j

/* 
 * flip - Your current working version of flip
 * IMPORTANT: This is the version you will be graded on
 */

char flip_descr[] = "flip: Current working version";
void flip(int dim, struct pixel_t *src, struct pixel_t *dst)
{	
	unsigned long temp1,temp2;
	for(int i = 0; i < dim; i++)
	{
		int index = RIDX(dim-1-i, 0, dim);
		for(int j = dim-4; j >= 0; j-=4)
		{
			temp1 = *(unsigned long *)(src+RIDX(i,j,dim));
			temp2 = *(unsigned long *)(src+RIDX(i,j+2,dim));
			temp1 = ((temp1 & 0xFFFFFFFF) << 32) + (temp1 >> 32);
			temp2 = ((temp2 & 0xFFFFFFFF) << 32) + (temp2 >> 32);
			*(unsigned long *)(dst+index) = temp2;
			*(unsigned long *)(dst+index+2) = temp1;
			index+=4;
		}
	}

}

/*********************************************************************
 * register_flip_functions - Register all of your different versions
 *     of the flip kernel with the driver by calling the
 *     add_flip_function() for each test function. When you run the
 *     driver program, it will test and report the performance of each
 *     registered test function.  
 *********************************************************************/

void register_flip_functions()
{
    add_flip_function(&naive_flip, naive_flip_descr);
    add_flip_function(&flip, flip_descr);
	/* ... Register additional test functions here */
}


/***************
 * SHARPEN KERNEL
 **************/

/***************************************************************
 * Various typedefs and helper functions for the sharpen function
 * You may modify these any way you like.
 **************************************************************/

/* Compute min and max of two integers, respectively */
static int min(int a, int b) { return (a < b ? a : b); }
static int max(int a, int b) { return (a > b ? a : b); }

/******************************************************
 * Your different versions of the sharpen kernel go here
 ******************************************************/

/*
 * naive_sharpen - The naive baseline version of sharpen 
 */
char naive_sharpen_descr[] = "naive_sharpen: Naive baseline implementation";
void naive_sharpen(int dim, struct pixel_t *src, struct pixel_t *dst)
{
	for(int i = 0; i < dim; i++)
	{
		for(int j = 0; j < dim; j++)
		{
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

			dst[RIDX(i, j, dim)].red = r;
			dst[RIDX(i, j, dim)].green = g;
			dst[RIDX(i, j, dim)].blue = b;
			dst[RIDX(i, j, dim)].unused = 0;
		}
	}
}

/*
 * sharpen - Your current working version of sharpen. 
 * IMPORTANT: This is the version you will be graded on
 */
char sharpen_descr[] = "sharpen: Current working version";
void sharpen(int dim, struct pixel_t *src, struct pixel_t *dst)
{
	double red = 0, green = 0, blue = 0;
	int i,j;
	struct pixel_t *p1,*p2,*p3;
	struct pixel_t *dst1;
	p1 = src;
	p2 = p1 + dim;
	
	/* left-top pixel*/
	red = ((p1->red) << 2) + p1->red - (p1->red + (p1+1)->red + p2->red + (p2+1)->red);
	green =((p1->green) << 2) + p1->green -(p1->green + (p1+1)->green + p2->green + (p2+1)->green);
	blue =((p1->blue)<<2) + p1->blue -(p1->blue + (p1+1)->blue + p2->blue + (p2+1)->blue);
	dst->red = min(max(0,(int)red), 255);
	dst->green = min(max(0,(int)green),255);
	dst->blue = min(max(0,(int)blue),255);
	dst->unused = 0;
	dst++;	

	/* top border except left and right border*/
    for(i=1;i<dim-1;i++)  
	{      
        red = (((p1+1)->red) << 3) - (p1+1)->red - (p1->red + (p1+1)->red +(p1+2)->red + p2->red + (p2+1)->red+(p2+2)->red);
		green =(((p1+1)->green) << 3) - (p1+1)->green -(p1->green + (p1+1)->green+(p1+2)->green + p2->green + (p2+1)->green+(p2+2)->green);
		blue =(((p1+1)->blue)<<3) - (p1+1)->blue -(p1->blue + (p1+1)->blue +(p1+2)->blue + p2->blue + (p2+1)->blue+(p2+2)->blue);
		dst->red = min(max(0,(int)red), 255);
		dst->green = min(max(0,(int)green),255);
		dst->blue = min(max(0,(int)blue),255);
		dst->unused = 0;
		dst++;	
		p1++;
		p2++;
    }     

	/* top-right pixel*/
	red = (((p1+1)->red) << 2) + (p1+1)->red - (p1->red + (p1+1)->red + p2->red + (p2+1)->red);
	green =(((p1+1)->green) << 2) + (p1+1)->green -(p1->green + (p1+1)->green + p2->green + (p2+1)->green);
	blue =(((p1+1)->blue)<<2) + (p1+1)->blue -(p1->blue + (p1+1)->blue + p2->blue + (p2+1)->blue);
	dst->red = min(max(0,(int)red), 255);
	dst->green = min(max(0,(int)green),255);
	dst->blue = min(max(0,(int)blue),255);
	dst->unused = 0;
	dst++;	
	p1 = src;
	p2 = p1+dim;
	p3 = p2+dim;
	
	/* row 1 - dim-1*/
	for(i = 1; i < dim-1;i++)
	{
		red = ((p2->red) << 3) - p2->red - (p1->red + (p1+1)->red + p2->red + (p2+1)->red +p3->red +(p3+1)->red);
		green =((p2->green) << 3) - p2->green -(p1->green + (p1+1)->green + p2->green + (p2+1)->green+p3->green+(p3+1)->green);
		blue =((p2->blue)<<3) - p2->blue -(p1->blue + (p1+1)->blue + p2->blue + (p2+1)->blue+p3->blue+(p3+1)->blue);
		dst->red = min(max(0,(int)red), 255);
		dst->green = min(max(0,(int)green),255);
		dst->blue = min(max(0,(int)blue),255);
		dst->unused = 0;
		dst++;
		dst1 = dst+1;
		for(j = 1; j < dim-2;j+=2)
		{
		 	red=((((p2+1)->red) << 3) + (((p2+1)->red) << 1))-(p1->red+(p1+1)->red+(p1+2)->red+p2->red+(p2+1)->red+(p2+2)->red+p3->red+(p3+1)->red+(p3+2)->red);  
            green=((((p2+1)->green) << 3) + (((p2+1)->green) << 1))-(p1->green+(p1+1)->green+(p1+2)->green+p2->green+(p2+1)->green+(p2+2)->green+p3->green+(p3+1)->green+(p3+2)->green);            
            blue=((((p2+1)->blue) << 3) + (((p2+1)->blue) << 1))-(p1->blue+(p1+1)->blue+(p1+2)->blue+p2->blue+(p2+1)->blue+(p2+2)->blue+p3->blue+(p3+1)->blue+(p3+2)->blue); 
            dst->red = min(max(0,(int)red), 255);
			dst->green = min(max(0,(int)green),255);
			dst->blue = min(max(0,(int)blue),255);
			dst->unused = 0;
            red=((((p2+2)->red) << 3) + (((p2+2)->red) << 1)) - ((p1+3)->red+(p1+1)->red+(p1+2)->red+(p2+3)->red+(p2+1)->red+(p2+2)->red+(p3+3)->red+(p3+1)->red+(p3+2)->red);             
            green=((((p2+2)->green) << 3) + (((p2+2)->green) << 1)) -((p1+3)->green+(p1+1)->green+(p1+2)->green+(p2+3)->green+(p2+1)->green+(p2+2)->green+(p3+3)->green+(p3+1)->green+(p3+2)->green);             	blue=((((p2+2)->blue) << 3) + (((p2+2)->blue) << 1)) - ((p1+3)->blue+(p1+1)->blue+(p1+2)->blue+(p2+3)->blue+(p2+1)->blue+(p2+2)->blue+(p3+3)->blue+(p3+1)->blue+(p3+2)->blue);     
            dst1->red = min(max(0,(int)red), 255);
			dst1->green = min(max(0,(int)green),255);
			dst1->blue = min(max(0,(int)blue),255);
			dst1->unused = 0;    
            dst+=2;dst1+=2;p1+=2;p2+=2;p3+=2; 	
		}
		for(;j<dim-1;j++)     
		{           
            red=((((p2+1)->red)<< 3) + (((p2+1)->red) << 1)) - (p1->red+(p1+1)->red+(p1+2)->red+p2->red+(p2+1)->red+(p2+2)->red+p3->red+(p3+1)->red+(p3+2)->red);          
            green=((((p2+1)->green) << 3) + (((p2+1)->green) << 1)) -(p1->green+(p1+1)->green+(p1+2)->green+p2->green+(p2+1)->green+(p2+2)->green+p3->green+(p3+1)->green+(p3+2)->green);          
            blue=((((p2+1)->blue) << 3) + (((p2+1)->blue) << 1)) -(p1->blue+(p1+1)->blue+(p1+2)->blue+p2->blue+(p2+1)->blue+(p2+2)->blue+p3->blue+(p3+1)->blue+(p3+2)->blue);     
            dst->red = min(max(0,(int)red), 255);
			dst->green = min(max(0,(int)green),255);
			dst->blue = min(max(0,(int)blue),255);
			dst->unused = 0;      
            dst++;       
			p1++;p2++;p3++;       
        }       
        red=(((p2+1)->red) << 3) - (p2+1)->red -(p1->red+(p1+1)->red+p2->red+(p2+1)->red+p3->red+(p3+1)->red);        
        green=(((p2+1)->green) << 3) - (p2+1)->green -(p1->green+(p1+1)->green+p2->green+(p2+1)->green+p3->green+(p3+1)->green);       
        blue=(((p2+1)->blue) << 3) - (p2+1)->blue -(p1->blue+(p1+1)->blue+p2->blue+(p2+1)->blue+p3->blue+(p3+1)->blue);    
        dst->red = min(max(0,(int)red), 255);
		dst->green = min(max(0,(int)green),255);
		dst->blue = min(max(0,(int)blue),255);
		dst->unused = 0;      
        dst++;      
		p1+=2;      p2+=2;      p3+=2;  				
	}
        red=(((p2->red) << 2) + p2->red) - (p1->red+(p1+1)->red+p2->red+(p2+1)->red);       
        green=(((p2->green) << 2) + p2->green) - (p1->green+(p1+1)->green+p2->green+(p2+1)->green);      
        blue=(((p2->blue) << 2) + p2->blue) - (p1->blue+(p1+1)->blue+p2->blue+(p2+1)->blue); 
        dst->red = min(max(0,(int)red), 255);
		dst->green = min(max(0,(int)green),255);
		dst->blue = min(max(0,(int)blue),255);
		dst->unused = 0;      
        dst++;
   
         for(i=1;i<dim-1;i++)     {         
         	red = (((p2+1)->red) << 3) - (p2+1)->red - (p1->red + (p1+1)->red +(p1+2)->red + p2->red + (p2+1)->red+(p2+2)->red);
			green =(((p2+1)->green) << 3) - (p2+1)->green -(p1->green + (p1+1)->green+(p1+2)->green + p2->green + (p2+1)->green+(p2+2)->green);
			blue =(((p2+1)->blue)<<3) - (p2+1)->blue -(p1->blue + (p1+1)->blue +(p1+2)->blue + p2->blue + (p2+1)->blue+(p2+2)->blue);
			dst->red = min(max(0,(int)red), 255);
			dst->green = min(max(0,(int)green),255);
			dst->blue = min(max(0,(int)blue),255);
			dst->unused = 0;
			dst++;	
			p1++;
			p2++;   
         }     //右下角像素处理       

		red = (((p2+1)->red) << 2) + (p2+1)->red - (p1->red + (p1+1)->red + p2->red + (p2+1)->red);
		green =(((p2+1)->green) << 2) + (p2+1)->green -(p1->green + (p1+1)->green + p2->green + (p2+1)->green);
		blue =(((p2+1)->blue)<<2) + (p2+1)->blue -(p1->blue + (p1+1)->blue + p2->blue + (p2+1)->blue);
		dst->red = min(max(0,(int)red), 255);
		dst->green = min(max(0,(int)green),255);
		dst->blue = min(max(0,(int)blue),255);
		dst->unused = 0;
}



/********************************************************************* 
 * register_sharpen_functions - Register all of your different versions
 *     of the sharpen kernel with the driver by calling the
 *     add_sharpen_function() for each test function.  When you run the
 *     driver program, it will test and report the performance of each
 *     registered test function.  
 *********************************************************************/

void register_sharpen_functions() {
    add_sharpen_function(&sharpen, sharpen_descr);
    add_sharpen_function(&naive_sharpen, naive_sharpen_descr);
    /* ... Register additional test functions here */
}

