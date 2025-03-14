//========================================================//
//  cache.c                                               //
//  Source file for the Cache Simulator                   //
//                                                        //
//  Implement the I-cache, D-Cache and L2-cache as        //
//  described in the README                               //
//========================================================//

#include "cache.hpp"
#include <stdio.h>
#include <inttypes.h>
// #include <iostream>
// using namespace std;
//
// TODO:Student Information
//
const char *studentName = "NAME";
const char *studentID   = "PID";
const char *email       = "EMAIL";

//------------------------------------//
//        Cache Configuration         //
//------------------------------------//

uint32_t icacheSets;      // Number of sets in the I$
uint32_t icacheAssoc;     // Associativity of the I$
uint32_t icacheBlocksize; // Blocksize of the I$
uint32_t icacheHitTime;   // Hit Time of the I$

uint32_t dcacheSets;      // Number of sets in the D$
uint32_t dcacheAssoc;     // Associativity of the D$
uint32_t dcacheBlocksize; // Blocksize of the D$
uint32_t dcacheHitTime;   // Hit Time of the D$

uint32_t l2cacheSets;     // Number of sets in the L2$
uint32_t l2cacheAssoc;    // Associativity of the L2$
uint32_t l2cacheBlocksize;// Blocksize of the L2$
uint32_t l2cacheHitTime;  // Hit Time of the L2$
uint32_t inclusive;       // Indicates if the L2 is inclusive

uint32_t prefetch;        // Indicate if prefetching is enabled

uint32_t memspeed;        // Latency of Main Memory

uint32_t bit_width(uint32_t data){
  uint32_t bits = 0;
  for(int i = data; i > 1; i = int(i/2)){
    bits += 1;
  }
  return bits;
}

Cache icache, dcache, l2cache;
//------------------------------------//
//          Cache Statistics          //
//------------------------------------//

uint64_t icacheRefs;       // I$ references
uint64_t icacheMisses;     // I$ misses
uint64_t icachePenalties;  // I$ penalties

uint64_t dcacheRefs;       // D$ references
uint64_t dcacheMisses;     // D$ misses
uint64_t dcachePenalties;  // D$ penalties

uint64_t l2cacheRefs;      // L2$ references
uint64_t l2cacheMisses;    // L2$ misses
uint64_t l2cachePenalties; // L2$ penalties

uint64_t compulsory_miss;  // Compulsory misses on all caches
uint64_t other_miss;       // Other misses (Conflict / Capacity miss) on all caches

uint32_t icache_offset_bits;
uint32_t icache_index_bits;
uint32_t icache_tag_bits;
uint32_t dcache_offset_bits;
uint32_t dcache_index_bits;
uint32_t dcache_tag_bits;
uint32_t l2cache_offset_bits;
uint32_t l2cache_index_bits;
uint32_t l2cache_tag_bits;

//------------------------------------//
//        Cache Data Structures       //
//------------------------------------//

//
//TODO: Add your Cache data structures here
//

//------------------------------------//
//          Cache Functions           //
//------------------------------------//

// Initialize the Cache Hierarchy
//

#define SPT_ENTRY_BITS 10

struct SPT_entry {
  uint32_t instruction_address;
  uint32_t memory_address;
  bool valid;
};

struct Stride_Prefetch_Table {
  uint32_t *access_history;
  SPT_entry entry[1 << SPT_ENTRY_BITS];
};

Stride_Prefetch_Table SPT;


void 
cache_init(Cache& cache, uint32_t cacheSets, uint32_t cacheAssoc){
  cache.set = (Cache_Set*)malloc(sizeof(Cache_Set) * cacheSets);
  for(int i = 0; i < cacheSets; i++){
    cache.set[i].block = (Cache_Block*)malloc(sizeof(Cache_Block) * cacheAssoc);
    cache.set[i].access_history = (uint32_t*)malloc(sizeof(uint32_t) * cacheAssoc);
    for(int j = 0; j < cacheAssoc; j++){
      cache.set[i].block[j].tag = 0;
      cache.set[i].access_history[j] = cacheAssoc - j - 1;
    }
  }
}

void
SPT_init(){
  SPT.access_history = (uint32_t*)malloc(sizeof(uint32_t) * (1 << SPT_ENTRY_BITS));
  for(int i = 0; i < (1 << SPT_ENTRY_BITS); i++){
    SPT.entry[i].instruction_address = 0;
    SPT.entry[i].memory_address = 0;
    SPT.entry[i].valid = 0;
    SPT.access_history[i] = i;
  }
}
void
init_cache()
{
  // Initialize cache stats
  icacheRefs        = 0;
  icacheMisses      = 0;
  icachePenalties   = 0;
  dcacheRefs        = 0;
  dcacheMisses      = 0;
  dcachePenalties   = 0;
  l2cacheRefs       = 0;
  l2cacheMisses     = 0;
  l2cachePenalties  = 0;

  compulsory_miss = 0;
  other_miss = 0;
  
  //
  //TODO: Initialize Cache Simulator Data Structures
  //

  // I-cache init
  cache_init(icache, icacheSets, icacheAssoc);

  // D-cache init
  cache_init(dcache, dcacheSets, dcacheAssoc);

  // L2-cache init
  cache_init(l2cache, l2cacheSets, l2cacheAssoc);

  SPT_init();

  icache_offset_bits = bit_width(icacheBlocksize);
  icache_index_bits = bit_width(icacheSets);
  icache_tag_bits = 32 - icache_offset_bits - icache_index_bits;

  dcache_offset_bits = bit_width(dcacheBlocksize);
  dcache_index_bits = bit_width(dcacheSets);
  dcache_tag_bits = 32 - dcache_offset_bits - dcache_index_bits;

  l2cache_offset_bits = bit_width(l2cacheBlocksize);
  l2cache_index_bits = bit_width(l2cacheSets);
  l2cache_tag_bits = 32 - l2cache_offset_bits - l2cache_index_bits;
  printf("finish initialization\n");
}

// Clean Up the Cache Hierarchy
//
void
clean_cache()
{
  //
  //TODO: Clean Up Cache Simulator Data Structures
  //
  for(int i = 0; i < icacheSets; i++){
    free(icache.set[i].block);
    free(icache.set[i].access_history);
  }
  free(icache.set);
  for(int i = 0; i < dcacheSets; i++){
    free(dcache.set[i].block);
    free(dcache.set[i].access_history);
  }
  free(dcache.set);
  for(int i = 0; i < l2cacheSets; i++){
    free(l2cache.set[i].block);
    free(l2cache.set[i].access_history);
  }
  free(l2cache.set);
}

// Perform a memory access through the icache interface for the address 'addr'
// Return the access time for the memory operation
//

void cache_update_history(Cache& cache, uint32_t set_index, uint32_t order_index){
  uint32_t block_index = cache.set[set_index].access_history[order_index];
  for(int i = order_index; i > 0; i--){
    cache.set[set_index].access_history[i] = cache.set[set_index].access_history[i-1];
  }
  cache.set[set_index].access_history[0] = block_index;
}

void cache_update(Cache& cache, uint32_t set_index, uint32_t order_index, Cache_Block new_block){
  // put in the new data
  uint32_t block_index = cache.set[set_index].access_history[order_index];
  cache.set[set_index].block[block_index] = new_block;
  // update the access history
  cache_update_history(cache, set_index, order_index);

}

void debug_info(uint32_t cache_index_bits, uint32_t cache_tag_bits, uint32_t cache_offset_bits, uint32_t addr, uint32_t tag){
  printf("icache_index_bits = %d\n", cache_index_bits);
  printf("icache_tag_bits = %d\n", cache_tag_bits);
  printf("icache_offset_bits = %d\n", cache_offset_bits);
  printf("addr = %d\n", addr);
  printf("tag = %d\n", tag);

}
uint32_t
icache_access(uint32_t addr)
{
  //
  //TODO: Implement I$
  //
  icacheRefs ++;
  uint32_t tag = (addr >> (icache_index_bits + icache_offset_bits)) & ((1 << icache_tag_bits) - 1);
  uint32_t index = (addr >> icache_offset_bits) & ((1 << icache_index_bits) - 1);
  
  // debug_info(uint32_t icache_index_bits, uint32_t icache_tag_bits, uint32_t icache_offset_bits, uint32_t addr, uint32_t tag);
  
  // hit -> update access history 
  for(int i = 0; i < icacheAssoc; i++){
    uint32_t order_index = icache.set[index].access_history[i];
    if(icache.set[index].block[order_index].tag == tag){
      cache_update_history(icache, index, i);
      return icacheHitTime;
    }
  }
  // miss
  icacheMisses ++;
  // non-inclusive: no matter if l2 is a hit or not -> put the data in l1 and do nothing about l2
  // put in the new data with LRU, put the evicted data into l2
  Cache_Block new_block;
  new_block.tag = tag;
  cache_update(icache, index, icacheAssoc-1, new_block);
  uint32_t l2_time = l2cache_access(addr);
  icachePenalties += l2_time;
  return l2_time;
}

// Perform a memory access through the dcache interface for the address 'addr'
// Return the access time for the memory operation
//
uint32_t
dcache_access(uint32_t addr)
{
  dcacheRefs ++;
  uint32_t tag = (addr >> (dcache_index_bits + dcache_offset_bits)) & ((1 << dcache_tag_bits) - 1);
  uint32_t index = (addr >> dcache_offset_bits) & ((1 << dcache_index_bits) - 1);
  
  // debug_info(uint32_t dcache_index_bits, uint32_t dcache_tag_bits, uint32_t dcache_offset_bits, uint32_t addr, uint32_t tag);
  
  // hit -> update access history 
  for(int i = 0; i < dcacheAssoc; i++){
    uint32_t order_index = dcache.set[index].access_history[i];
    if(dcache.set[index].block[order_index].tag == tag){
      cache_update_history(dcache, index, i);
      return dcacheHitTime;
    }
  }
  // miss
  dcacheMisses ++;
  // non-inclusive: no matter if l2 is a hit or not -> put the data in l1 and do nothing about l2
  // put in the new data with LRU, put the evicted data into l2
  Cache_Block new_block;
  new_block.tag = tag;
  cache_update(dcache, index, dcacheAssoc-1, new_block);
  uint32_t l2_time = l2cache_access(addr);
  dcachePenalties += l2_time;
  return l2_time;
}

// Perform a memory access to the l2cache for the address 'addr'
// Return the access time for the memory operation
//
uint32_t
l2cache_access(uint32_t addr)
{
  //
  //TODO: Implement L2$
  //
  l2cacheRefs ++;
  uint32_t tag = (addr >> (l2cache_index_bits + l2cache_offset_bits)) & ((1 << l2cache_tag_bits) - 1);
  uint32_t index = (addr >> l2cache_offset_bits) & ((1 << l2cache_index_bits) - 1);
  
  // debug_info(uint32_t l2cache_index_bits, uint32_t l2cache_tag_bits, uint32_t l2cache_offset_bits, uint32_t addr, uint32_t tag);
  
  // hit -> update access history 
  for(int i = 0; i < l2cacheAssoc; i++){
    uint32_t order_index = l2cache.set[index].access_history[i];
    if(l2cache.set[index].block[order_index].tag == tag){
      cache_update_history(l2cache, index, i);
      return l2cacheHitTime;
    }
  }

  // miss
  l2cacheMisses ++;
  // non-inclusive: put the data in l2
  Cache_Block new_block;
  new_block.tag = tag;
  cache_update(l2cache, index, l2cacheAssoc-1, new_block);
  l2cachePenalties += memspeed;
  return memspeed;
}

void
SPT_info(uint32_t pc, uint32_t addr, uint32_t spt_index){
  printf("pc = %d\n", pc);
  printf("stored pc = %d\n", SPT.entry[spt_index].instruction_address);
  printf("cur MA = ");
  printf("%" PRIu32 "\n", addr);
  printf("last MA = %d\n", SPT.entry[spt_index].memory_address);
}

// Predict an address to prefetch on icache with the information of last icache access:
// 'pc':     Program Counter of the instruction of last icache access
// 'addr':   Accessed Address of last icache access
// 'r_or_w': Read/Write of last icache access
uint32_t
icache_prefetch_addr(uint32_t pc, uint32_t addr, char r_or_w)
{
  return addr + icacheBlocksize;
}

// Predict an address to prefetch on dcache with the information of last dcache access:
// 'pc':     Program Counter of the instruction of last dcache access
// 'addr':   Accessed Address of last dcache access
// 'r_or_w': Read/Write of last dcache access
uint32_t
dcache_prefetch_addr(uint32_t pc, uint32_t addr, char r_or_w)
{
  uint32_t addr_index = (addr >> dcache_offset_bits);
  uint32_t spt_index = pc & ((1 << SPT_ENTRY_BITS) - 1);

  if(!SPT.entry[spt_index].valid || (SPT.entry[spt_index].instruction_address != pc)){
    SPT.entry[spt_index].valid = 1;
    SPT.entry[spt_index].instruction_address = pc;
    SPT.entry[spt_index].memory_address = addr_index;
    return addr;
  }
  
  int stride = addr_index - SPT.entry[spt_index].memory_address;
  SPT.entry[spt_index].instruction_address = pc;
  SPT.entry[spt_index].memory_address = addr_index;
  return addr + stride * dcacheBlocksize; 
}

// Perform a prefetch operation to I$ for the address 'addr'
// 1. check if the data block is already in the cache
// 2. if yes -> update the access history
// 3. if no -> update the cache
void
icache_prefetch(uint32_t addr)
{
  //
  //TODO: Implement I$ prefetch operation
  //
  // replace with LRU
  uint32_t tag = (addr >> (icache_index_bits + icache_offset_bits)) & ((1 << icache_tag_bits) - 1);
  uint32_t index = (addr >> icache_offset_bits) & ((1 << icache_index_bits) - 1);

  // hit
  for(int i = 0; i < icacheAssoc; i++){
    uint32_t order_index = icache.set[index].access_history[i];
    if(icache.set[index].block[order_index].tag == tag){
      cache_update_history(icache, index, i);
      return;
    }
  }

  // miss
  Cache_Block new_data;
  new_data.tag = tag;
  cache_update(icache, index, icacheAssoc-1, new_data);

}

// Perform a prefetch operation to D$ for the address 'addr'
void
dcache_prefetch(uint32_t addr)
{
  //
  //TODO: Implement D$ prefetch operation
  //
  uint32_t tag = (addr >> (dcache_index_bits + dcache_offset_bits)) & ((1 << dcache_tag_bits) - 1);
  uint32_t index = (addr >> dcache_offset_bits) & ((1 << dcache_index_bits) - 1);

  // hit
  for(int i = 0; i < dcacheAssoc; i++){
    uint32_t order_index = dcache.set[index].access_history[i];
    if(dcache.set[index].block[order_index].tag == tag){
      cache_update_history(dcache, index, i);
      return;
    }
  }

  // miss
  Cache_Block new_data;
  new_data.tag = tag;
  cache_update(dcache, index, dcacheAssoc-1, new_data);
  
}