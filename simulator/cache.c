#include "dogfault.h"
#include "cache.h"
#include <assert.h>
#include <ctype.h>
#include <getopt.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include "config.h"

// HELPER FUNCTIONS USEFUL FOR IMPLEMENTING THE CACHE

unsigned long long address_to_block(const unsigned long long address, const Cache *cache) {
    /*YOUR CODE HERE*/
	int blockBits_count = cache->blockBits;
	return ((address >> blockBits_count)<<blockBits_count);
}

unsigned long long cache_tag(const unsigned long long address, const Cache *cache) {
    /*YOUR CODE HERE*/
	int index_and_offset_bitsCount = cache->blockBits + cache->setBits;
	return ((address >> index_and_offset_bitsCount));
}

unsigned long long cache_set(const unsigned long long address, const Cache *cache) {
    /*YOUR CODE HERE*/
	unsigned long long removed_blockBits = address>>(cache->blockBits);
	return removed_blockBits & ((1 << cache->setBits) - 1);
}

bool probe_cache(const unsigned long long address, const Cache *cache) {
    /*YOUR CODE HERE*/

	unsigned long long set_index = cache_set(address, cache);
	unsigned long long tag = cache_tag(address, cache);

	// Get the correct set
  	Set *set = &cache->sets[set_index];
  	if(set == NULL || set->lines == NULL){
  		printf("Set or line is null\n");
		exit(EXIT_FAILURE);
 	}
	
  	for(int i = 0; i < cache->linesPerSet; i++){
		if(set->lines[i].valid && set->lines[i].tag == tag){
			return true;	//it's a cache hit!
		}
  	}

  	return false;

}

void hit_cacheline(const unsigned long long address, Cache *cache) {
    /*YOUR CODE HERE*/
	unsigned long long set_index = cache_set(address, cache);
 	unsigned long long tag = cache_tag(address, cache);

  	// Get the correct set
  	Set *set = &cache->sets[set_index];

  	for (int i = 0; i < cache->linesPerSet;i++){
  		Line *line = &set->lines[i];
		if(line->valid && line->tag == tag){
			// LRU
			if(cache->lfu == 0){	
				//update LRU counter based on global lru_clock of the corresponding set
				line->lru_clock = ++set->lru_clock;
			// LFU		
			}else{			
				//update LFU counter
				line->access_counter++;
			}
		}

   	}

    // Did not find line, not supposed to be here
    //assert(0);
}

bool insert_cacheline(const unsigned long long address, Cache *cache) {
    /*YOUR CODE HERE*/
    // Getting which set to search and tag for address
   	unsigned long long set_index = cache_set(address,cache);
   	unsigned long long tag = cache_tag(address,cache);

   	// Getting the correct set
   	Set *set = &cache->sets[set_index];
   	if (set == NULL){
   		printf("set is null in insert\n");
		return false;
   	}
   	
   	// Looping through all lines in the set to find 
   	for (int i = 0; i < cache->linesPerSet; i++){
		if(set->lines == NULL){
			printf("null lines in insert\n");
			return false;
		}
		if (!set->lines[i].valid){
  			// If empty line is found, 
			// update its valid bit and assign the address tag to its cache tag
			// Update lru_clock, access counter and block address as well

			set->lines[i].valid = true;
			set->lines[i].tag = tag;
			
			// Update cache line's lru_clock based on global lru_clock
			set->lines[i].lru_clock = set->lru_clock;
			
			// Updating block address as well as access counter
			set->lines[i].block_addr = address_to_block(address, cache);
			set->lines[i].access_counter = 0;
			
			return true;
		}
   	}
        
   	return false;	// If no empty cache line found

    
}

unsigned long long victim_cacheline(const unsigned long long address, const Cache *cache) {
    /*YOUR CODE HERE*/

	unsigned long long set_index = cache_set(address, cache);
  
	// Getting the correct set
 	Set *set = &cache->sets[set_index];

	// Initialize minimum access count and time to first line in set
  	int min_accessCount = set->lines[0].access_counter;
  	int min_accessTime = set->lines[0].lru_clock;      
  	
	int victimIndex = 0;
  
 	// If LFU (Least Frequently Used) policy is to be used	  
 	if (cache->lfu == 1){
      
    		for (int i = 1; i < cache->linesPerSet; i++){
	
			// If access count is equal, check the LRU clock instead
      			if (set->lines[i].access_counter == min_accessCount){
      	 		 	if(set->lines[i].lru_clock < min_accessTime){
	  		  		min_accessTime = set->lines[i].lru_clock;
	  	   			victimIndex = i;
		  		}
      			}	    
  	    		else if (set->lines[i].access_counter < min_accessCount){
			 	min_accessCount = set->lines[i].access_counter;
			 	min_accessTime = set->lines[i].lru_clock;
		 		victimIndex = i;
	      		}
    		}
  	} else {
    		// If LRU (Least Recently Used) policy is to be used
    		for (int i = 1; i < cache->linesPerSet; i++){
			if (set->lines[i].lru_clock < min_accessTime){
           			min_accessTime = set->lines[i].lru_clock;
	   			victimIndex = i;
			}
   		}
  	}

  	// Returning the block address of victim cache line
  	return set->lines[victimIndex].block_addr;

}

void replace_cacheline(const unsigned long long victim_block_addr, const unsigned long long insert_addr, Cache *cache) {
    /*YOUR CODE HERE*/

	unsigned long long set_index = cache_set(insert_addr, cache);
  	unsigned long long tag = cache_tag(insert_addr, cache);

	// Getting the correct set
  	Set *set = &cache->sets[set_index];

	// Looping through all lines in set
  	for (int i = 0; i < cache->linesPerSet; i++){
		// When victim cache line is found, update all properties
    		if (set->lines[i].block_addr == victim_block_addr){
    			set->lines[i].valid = true;
			set->lines[i].tag = tag;
			set->lines[i].access_counter = 0;
			set->lines[i].lru_clock = ++set->lru_clock;
			set->lines[i].block_addr = address_to_block(insert_addr, cache);
			return;
    		}
 	}
  	return;

    // Not supposed to be here
    //assert(0);
}

void cacheSetUp(Cache *cache, char *name) {
    cache->hit_count = 0;
    /*YOUR CODE HERE*/

    int num_of_sets = 1 << (cache->setBits);
	cache->sets = (Set*)malloc(num_of_sets*sizeof(Set));

	if(cache->sets == NULL){
		printf("Failed to allocate memory for the cache sets\n");
		return;
	}

	for(int i = 0; i < num_of_sets;i++){
		cache->sets[i].lines = (Line *)malloc(cache->linesPerSet*sizeof(Line));
		if(cache->sets[i].lines == NULL){
			printf("Failed to allocate memory for cache lines in a set\n");
			return;
		}
		cache->sets[i].lru_clock = 0;

		for(int j = 0; j < cache->linesPerSet;j++){
			cache->sets[i].lines[j].valid = 0;
			cache->sets[i].lines[j].tag = 0;
			cache->sets[i].lines[j].lru_clock = 0;
			cache->sets[i].lines[j].access_counter = 0;
			cache->sets[i].lines[j].block_addr = 0;
		}
	}

	//initialize cache name
	cache->name = name;
	if(cache->name == NULL){
		printf("Failed to allocate memory for the cache name \n");
		exit(EXIT_FAILURE);
	}

}

void deallocate(Cache *cache) {
    /*YOUR CODE HERE*/

	int num_of_sets = 1 << cache->setBits;

	for(int i = 0; i < num_of_sets;i++){
		free(cache->sets[i].lines);	//free lines
	}
	free(cache->sets);	//free sets

}

result operateCache(const unsigned long long address, Cache *cache) {
    result r;
    /*YOUR CODE HERE*/


	//find the corresp. set
	unsigned long long set_index = cache_set(address, cache);

	Set *set = &cache->sets[set_index];
	if(set == NULL){
		printf("set is null here\n");
		exit(EXIT_FAILURE);
	}

	set->lru_clock++;

	if(probe_cache(address, cache)){	//address is in the cache
		hit_cacheline(address, cache);	//update counters
		r.status = CACHE_HIT;		//hit!
		cache->hit_count++;		//update hit count
	}else{	//cache miss:(
		if(insert_cacheline(address, cache)){	//miss!
			r.status = CACHE_MISS;
			r.insert_block_addr = address_to_block(address, cache);
			cache->miss_count++;
		}
		else{	//eviction!
			r.status = CACHE_EVICT;
			r.victim_block_addr = victim_cacheline(address, cache);
			r.insert_block_addr = address_to_block(address, cache);
			replace_cacheline(r.victim_block_addr,r.insert_block_addr, cache);
			cache->miss_count++;
			cache->eviction_count++;
		}
	}
	
	char* format;
	switch(r.status){
		case CACHE_HIT:
			format = CACHE_HIT_FORMAT;
			break;
		case CACHE_MISS:
			format = CACHE_MISS_FORMAT;
			break;
		case CACHE_EVICT:
			format = CACHE_EVICTION_FORMAT;
			break;
		default:
			break;
	}


    #ifdef PRINT_CACHE_TRACES 
    printf(format, address); 
    #endif 
    return r;
    /*YOUR CODE HERE*/


}

int processCacheOperation(unsigned long address, Cache *cache) {
    result r;
    /*YOUR CODE HERE*/

    r = operateCache(address, cache);

    if (r.status == CACHE_HIT) {
        return CACHE_HIT_LATENCY;
    } else if (r.status == CACHE_MISS)
    {
        return CACHE_MISS_LATENCY;
    }
    else
    {
        return CACHE_OTHER_LATENCY;
    }
}
