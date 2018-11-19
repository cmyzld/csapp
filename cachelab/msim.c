/*
 * msim.c - A cache simulator that implements LRU and MSI protocol for
 *   cache coherency.
 *
 * You must implement this file.
 */
#include <stdio.h>
#include <stdlib.h>
#include "cache.h"
#include "msim.h"
#include "trace-stream.h"

static int s, E, b, cache_num;

cache_line_t **initialiseCache();

//core_traces
sim_results_t runSimulator(cache_config_t* config) {
  	sim_results_t results;
	s = config->set_bits;
	E = config->associativity;
	b = config->block_bits;
	cache_num = config->num_cores;
	cache_line_t **caches[4];
	for(int i = 0; i < cache_num; i++)
	{
		caches[i] = initialiseCache();
		results.cores[i].hits = results.cores[i].misses = results.cores[i].evictions = results.cores[i].invalidations = 0;
	}
	unsigned long long int t = 0;
	while(1)
	{
		int mightHasNext = 0;
		for(int i = 0; i < cache_num; i++)
		{
			trace_entry_t *trace = traceStreamNext(&(config->core_traces[i]));
			if(!trace)
				continue;
			mightHasNext = 1;
			mem_addr_t index = (trace->addr >> b) & ((1 << s) - 1);
			mem_addr_t tag = (trace->addr >> ( b + s ));
			int j = 0, incache = 0, minlru_line = 0, invalidated = 0;
			if(trace->op == 'L')
			{
				for(; j < E; j++)
				{
					state_t state = caches[i][index][j].state;
					if(caches[i][index][j].age < caches[i][index][minlru_line].age)
						minlru_line = j;
					if(state == SHARED || state == MODIFIED)
					{
						if(caches[i][index][j].tag == tag)
						{
							incache = 1;
							results.cores[i].hits++;
							break;
						}
					}
					else
						break;
				}
				if(incache == 0)
				{
					results.cores[i].misses++;
					for(int k = 0; k < cache_num; k++)
						for(int l = 0; l < E; l++)
							if(k != i && caches[k][index][l].state == MODIFIED && caches[k][index][l].tag == tag)
							{
								caches[k][index][l].state = SHARED;
								invalidated = 1;
							}
					if(j >= E)
					{
						results.cores[i].evictions++;
						j = minlru_line;
					}
					caches[i][index][j].tag = tag;
				}
				caches[i][index][j].state = SHARED;
				caches[i][index][j].age = t++;
			}
			else
			{
				for(; j < E; j++)
				{
					state_t state = caches[i][index][j].state;
					if(caches[i][index][j].age < caches[i][index][minlru_line].age)
						minlru_line = j;
					if(state == MODIFIED || state == SHARED)
					{
						if(caches[i][index][j].tag == tag)
						{
							results.cores[i].hits++;
							incache = 1;
							if(state == SHARED)
							{
								for(int k = 0; k < cache_num; k++)
									for(int l = 0; l < E; l++)
										if(k != i && caches[k][index][l].state == SHARED && caches[k][index][l].tag == tag)
										{
											invalidated = 1;
											caches[k][index][l].state = INVALID;
										}
							}
							break;
						}
					}
					else
						break;
				}
				if(incache == 0)
				{
					results.cores[i].misses++;
					if(j >= E)
					{
						results.cores[i].evictions++;
						j = minlru_line;
					}
					caches[i][index][j].tag = tag;
					for(int k = 0; k < cache_num; k++)
						for(int l = 0; l < E; l++)
						{
							state_t state = caches[k][index][l].state;
							if(k != i && (state == SHARED || state == MODIFIED) && caches[k][index][l].tag == tag)
							{
								invalidated = 1;
								caches[k][index][l].state = INVALID;
							}
						}
				}
				if(invalidated == 1)
						results.cores[i].invalidations++;
				caches[i][index][j].state = MODIFIED;
				caches[i][index][j].age = t++;
			}
		}
		if(!mightHasNext)
			break;
	}
  	return results;
}

cache_line_t **initialiseCache()
{
	int i, j, set = (2 << s);
 	cache_line_t **cache = (cache_line_t **)malloc(sizeof(cache_line_t *) * set);  
  	for(i = 0; i < set; i++)
  	{
    	cache[i] = (cache_line_t *)malloc(E * sizeof(cache_line_t));
    	for(j = 0; j < E; j++)
    	{
      		cache[i][j].state = INVALID;
      		cache[i][j].age = 0;
    	}
  	}
  	return cache;
}
