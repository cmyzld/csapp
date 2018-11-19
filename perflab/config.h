/*********************************************************
 * config.h - Configuration data for the driver.c program.
 *********************************************************/
#ifndef _CONFIG_H_
#define _CONFIG_H_

/* 
 * CPEs for the baseline (naive) version of the flip function that
 * was handed out to the students. Rd is the measured CPE for a dxd
 * image. Run the driver.c program on your system to get these
 * numbers.  
 */
#define R64		4.0
#define R128	4.0
#define R256	4.0
#define R320	4.0
#define R512	4.0
#define R1024	4.1
#define R2048	4.4
#define R8192	4.4

/* 
 * CPEs for the baseline (naive) version of the sharpen function that
 * was handed out to the students. Sd is the measure CPE for a dxd
 * image. Run the driver.c program on your system to get these
 * numbers.  
 */
#define S64		70.0
#define S128	70.0
#define S256	70.1
#define S320	70.2
#define S512	70.2
#define S1024	76.2
#define S2048	76.4
#define S8192	76.7


#endif /* _CONFIG_H_ */
