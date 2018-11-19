/*
 * csim.c - A cache simulator that implements cache with LRU policy
 *   for replacement.
 *
 * You must implement this file.
 */
#include <stdio.h>
#include <stdlib.h>
#include "cache.h"
#include "csim.h"
#include "trace-stream.h"

typedef struct{ int valid, lru; unsigned long long tag;} Line;
static int s, E, b;

Line** initialiseCache();

sim_result_t runSimulator(cache_config_t* config) {
  	sim_result_t results;
  	s = config->set_bits;
  	E = config->associativity;
  	b = config->block_bits;
  	Line** cache = initialiseCache();
  	trace_entry_t *trace;
  	unsigned long long int t = 0;
  	while((trace = traceStreamNext(&(config->trace)))!= NULL)
  	{
   		unsigned long long int index = (trace->addr >> b) & ((1 << s)- 1);
    	unsigned long long int tag = (trace->addr >> (b + s));
		int i, j = 0;
    	for(i = 0; i < E; i++)
    	{
      		if(cache[index][i].valid == 1)
      		{
        		if(cache[index][i].tag == tag)
        		{
  	      			cache[index][i].lru = t++;
					results.hits++;
					break;
        		}
      		}
			else
			{
				cache[index][i].valid = 1;
				cache[index][i].tag = tag;
				cache[index][i].lru = t++;
				results.misses++;
				break;
			}
			if(cache[index][i].lru < cache[index][j].lru)
				j = i;
    	}
		if(i >= E)
		{
			results.misses++;
			results.evictions++;
			cache[index][j].tag = tag;
			cache[index][j].lru = t++;
		}
  	}
  	return results;
}

Line** initialiseCache()
{
	int i, j, set = (2 << s);
 	Line**cache = (Line **)malloc(sizeof(Line *) * set);  
  	for(i = 0; i < set; i++)
  	{
    	cache[i] = (Line *)malloc(E * sizeof(Line));
    	for(j = 0; j < E; j++)
    	{
      		cache[i][j].valid = 0;
      		cache[i][j].lru = 0;
			cache[i][j].tag = 0;
    	}
  	}
  	return cache;
}
