//========================================================//
//  cache.c                                               //
//  Source file for the Cache Simulator                   //
//                                                        //
//  Implement the I-cache, D-Cache and L2-cache as        //
//  described in the README                               //
//========================================================//

#include "cache.hpp"
#include <stdio.h>
#include <stdint.h>
//
// TODO:Student Information
//
const char *studentName = "Sin-Yu Chen";
const char *studentID   = "A59026047";
const char *email       = "sic030@ucsd.edu";

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

//------------------------------------//
//        Cache Data Structures       //
//------------------------------------//

//
//TODO: Add your Cache data structures here
//
struct Block {
  uint32_t tag;
  // uint32_t *data;
};

struct Set {
  Block *block;
};

struct Cache {
  Set *set;
};

Cache icache, dcache, l2cache;
uint32_t icache_offset_bits;
uint32_t icache_index_bits;
uint32_t icache_tag_bits;
uint32_t dcache_offset_bits;
uint32_t dcache_index_bits;
uint32_t dcache_tag_bits;
uint32_t l2cache_offset_bits;
uint32_t l2cache_index_bits;
uint32_t l2cache_tag_bits;
uint32_t *i_PC, *d_PC;
uint32_t *i_address, *d_address;

//------------------------------------//
//          Cache Functions           //
//------------------------------------//

// Initialize the Cache Hierarchy
//
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

  icache.set = (Set *)malloc(icacheSets*sizeof(Set));
  for(int i = 0; i < icacheSets; i++)
  {
    icache.set[i].block = (Block *)malloc(icacheAssoc*sizeof(Block));
    for(int j = 0; j < icacheAssoc; j++)
    {
      icache.set[i].block[j].tag = 0;
    }
  }
  dcache.set = (Set *)malloc(dcacheSets*sizeof(Set));
  for(int i = 0; i < dcacheSets; i++)
  {
    dcache.set[i].block = (Block *)malloc(dcacheAssoc*sizeof(Block));
    for(int j = 0; j < dcacheAssoc; j++)
    {
      dcache.set[i].block[j].tag = 0;
    }
  }
  l2cache.set = (Set *)malloc(l2cacheSets*sizeof(Set));
  for(int i = 0; i < l2cacheSets; i++)
  {
    l2cache.set[i].block = (Block *)malloc(l2cacheAssoc*sizeof(Block));
    for(int j = 0; j < l2cacheAssoc; j++)
    {
      l2cache.set[i].block[j].tag = 0;
    }
  }
  
  for(int i = icacheBlocksize; i > 1; i = int(i/2))
  {
    icache_offset_bits += 1;
  }
  for(int i = icacheSets; i > 1; i = int(i/2))
  {
    icache_index_bits += 1;
  }
  icache_tag_bits = 32 - icache_index_bits - icache_offset_bits;
  for(int i = dcacheBlocksize; i > 1; i = int(i/2))
  {
    dcache_offset_bits += 1;
  }
  for(int i = dcacheSets; i > 1; i = int(i/2))
  {
    dcache_index_bits += 1;
  }
  dcache_tag_bits = 32 - dcache_index_bits - dcache_offset_bits;
  for(int i = l2cacheBlocksize; i > 1; i = int(i/2))
  {
    l2cache_offset_bits += 1;
  }
  for(int i = l2cacheSets; i > 1; i = int(i/2))
  {
    l2cache_index_bits += 1;
  }
  l2cache_tag_bits = 32 - l2cache_index_bits - l2cache_offset_bits;

  i_PC = (uint32_t *)malloc((uint64_t)(1<<30) * sizeof(uint32_t));
  i_address = (uint32_t *)malloc((uint64_t)(1<<30) * sizeof(uint32_t));
  d_PC = (uint32_t *)malloc((uint64_t)(1<<30) * sizeof(uint32_t));
  d_address = (uint32_t *)malloc((uint64_t)(1<<30) * sizeof(uint32_t));
  for(long long i = 0; i < (1<<30); i++)
  {
    i_PC[i] = 0;
    i_address[i] = 0;
    d_PC[i] = 0;
    d_address[i] = 0;
  }
}

// Clean Up the Cache Hierarchy
//
void
clean_cache()
{
  //
  //TODO: Clean Up Cache Simulator Data Structures
  //
  for(int i = 0; i < icacheSets; i++)
  {
    free(icache.set[i].block);
  }
  free(icache.set);
  for(int i = 0; i < dcacheSets; i++)
  {
    free(dcache.set[i].block);
  }
  free(dcache.set);
  for(int i = 0; i < l2cacheSets; i++)
  {
    free(l2cache.set[i].block);
  }
  free(l2cache.set);
  free(i_PC);
  free(d_PC);
  free(i_address);
  free(d_address);
}

// Perform a memory access through the icache interface for the address 'addr'
// Return the access time for the memory operation
//
uint32_t
icache_access(uint32_t addr)
{
  //
  //TODO: Implement I$
  //
  icacheRefs += 1;
  uint32_t index = (uint32_t)(addr << icache_tag_bits) >> (icache_tag_bits + icache_offset_bits);
  uint32_t tag = (uint32_t)(addr >> (icache_index_bits + icache_offset_bits));
  for(int i = 0; i < icacheAssoc; i++)
  {
    if(icache.set[index].block[i].tag == tag)
    {
      // printf("Hit\n");
      Block temp;
      temp.tag = icache.set[index].block[i].tag;
      for(int j = i-1; j >= 0; j--)
      {
        icache.set[index].block[j+1].tag = icache.set[index].block[j].tag;
      }
      icache.set[index].block[0].tag = temp.tag;
      if(inclusive) l2cache_access(addr);
      return icacheHitTime;
    }
  }
  icacheMisses += 1;     // I$ misses
  // printf("Miss\n");
  uint32_t l2_time = l2cache_access(addr);
  icachePenalties += l2_time;  // I$ penalties
  for(int j = icacheAssoc-2; j >= 0; j--)
  {
    icache.set[index].block[j+1].tag = icache.set[index].block[j].tag;
  }
  icache.set[index].block[0].tag = tag;
  return (icacheHitTime + l2_time);
 
}

// Perform a memory access through the dcache interface for the address 'addr'
// Return the access time for the memory operation
//
uint32_t
dcache_access(uint32_t addr)
{
  //
  //TODO: Implement D$
  //
  dcacheRefs += 1;
  uint32_t index = (uint32_t)(addr << dcache_tag_bits) >> (dcache_tag_bits + dcache_offset_bits);
  uint32_t tag = (uint32_t)(addr >> (dcache_index_bits + dcache_offset_bits));
  for(int i = 0; i < dcacheAssoc; i++)
  {
    if(dcache.set[index].block[i].tag == tag)
    {
      Block temp;
      temp.tag = dcache.set[index].block[i].tag;
      for(int j = i-1; j >= 0; j--)
      {
        dcache.set[index].block[j+1].tag = dcache.set[index].block[j].tag;
      }
      dcache.set[index].block[0].tag = temp.tag;
      if(inclusive) l2cache_access(addr);
      return dcacheHitTime;
    }
  }
  dcacheMisses += 1;     // I$ misses
  uint32_t l2_time = l2cache_access(addr);
  dcachePenalties += l2_time;  // I$ penalties
  Block temp;
  temp.tag = dcache.set[index].block[0].tag;
  for(int j = dcacheAssoc-2; j >= 0; j--)
  {
    dcache.set[index].block[j+1].tag = dcache.set[index].block[j].tag;
  }
  dcache.set[index].block[0].tag = tag;
  return dcacheHitTime + l2_time;
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
  l2cacheRefs += 1;
  uint32_t index = (uint32_t)(addr << l2cache_tag_bits) >> (l2cache_tag_bits + l2cache_offset_bits);
  uint32_t tag = (uint32_t)(addr >> (l2cache_index_bits + l2cache_offset_bits));
  for(int i = 0; i < l2cacheAssoc; i++)
  {
    if(l2cache.set[index].block[i].tag == tag)
    {
      // printf("Hit\n");
      Block temp;
      temp.tag = l2cache.set[index].block[i].tag;
      for(int j = i-1; j >= 0; j--)
      {
        l2cache.set[index].block[j+1].tag = l2cache.set[index].block[j].tag;
      }
      l2cache.set[index].block[0].tag = temp.tag;
      return l2cacheHitTime;
    }
  }
  l2cacheMisses += 1;     // I$ misses
  // printf("Miss\n");
  // uint32_t l2_time = l2cache_access(addr);
  l2cachePenalties += memspeed;  // I$ penalties
  Block temp;
  temp.tag = l2cache.set[index].block[0].tag;
  for(int j = l2cacheAssoc-2; j >= 0; j--)
  {
    l2cache.set[index].block[j+1].tag = l2cache.set[index].block[j].tag;
  }
  l2cache.set[index].block[0].tag = tag;

  return l2cacheHitTime + memspeed;
}

// Predict an address to prefetch on icache with the information of last icache access:
// 'pc':     Program Counter of the instruction of last icache access
// 'addr':   Accessed Address of last icache access
// 'r_or_w': Read/Write of last icache access
uint32_t i_stride = 1;
uint32_t d_r_stride = 1;
uint32_t d_w_stride = 8;
uint32_t
icache_prefetch_addr(uint32_t pc, uint32_t addr, char r_or_w)
{
  uint32_t new_addr = addr >> icache_offset_bits;
  uint32_t new_pc = pc >> 2;
  if(i_address[new_addr] == 0)
  {
    i_address[new_addr] = new_pc;
    return addr + i_stride * icacheBlocksize;
  }
  else
  {
    int difference = new_pc - i_address[new_addr];
    if(difference > 0)
    {
      i_address[new_addr] = new_pc;
      i_PC[new_pc+difference] = new_addr;
    }
  }
  uint32_t return_address = (i_PC[new_pc+1] == 0) ? addr + i_stride * icacheBlocksize : i_PC[new_pc+1] << icache_offset_bits;
  return return_address; // Next line prefetching
  //
  //TODO: Implement a better prefetching strategy
  //
  return addr + i_stride * icacheBlocksize;
}

// Predict an address to prefetch on dcache with the information of last dcache access:
// 'pc':     Program Counter of the instruction of last dcache access
// 'addr':   Accessed Address of last dcache access
// 'r_or_w': Read/Write of last dcache access
uint32_t
dcache_prefetch_addr(uint32_t pc, uint32_t addr, char r_or_w)
{
  uint32_t stride = (r_or_w == 'R') ? d_r_stride : d_w_stride;
  // uint32_t new_addr = addr >> dcache_offset_bits;
  // uint32_t new_pc = pc >> 2;
  // if(d_address[new_addr] == 0)
  // {
  //   d_address[new_addr] = new_pc;
  //   return addr + stride * dcacheBlocksize;
  // }
  // else
  // {
  //   int difference = new_pc - d_address[new_addr];
  //   if(difference > 0)
  //   {
  //     d_address[new_addr] = new_pc;
  //     d_PC[new_pc+difference] = new_addr;
  //   }
  // }
  // uint32_t return_address = (d_PC[new_pc+1] == 0) ? addr + stride * dcacheBlocksize : d_PC[new_pc+1] << dcache_offset_bits;
  // return return_address; // Next line prefetching
  return addr + stride * dcacheBlocksize; // Next line prefetching
}

// Perform a prefetch operation to I$ for the address 'addr'
void
icache_prefetch(uint32_t addr)
{
  //
  //TODO: Implement I$ prefetch operation
  //
  uint32_t index = (uint32_t)(addr << icache_tag_bits) >> (icache_tag_bits + icache_offset_bits);
  uint32_t tag = (uint32_t)(addr >> (icache_index_bits + icache_offset_bits));
  for(int i = 0; i < icacheAssoc; i++)
  {
    if(icache.set[index].block[i].tag == tag)
    {
      // printf("Hit\n");
      Block temp;
      temp.tag = icache.set[index].block[i].tag;
      for(int j = i-1; j >= 0; j--)
      {
        icache.set[index].block[j+1].tag = icache.set[index].block[j].tag;
      }
      icache.set[index].block[0].tag = temp.tag;
      if(inclusive) l2cache_access(addr);
      return;
    }
  }
  for(int j = icacheAssoc-2; j >= 0; j--)
  {
    icache.set[index].block[j+1].tag = icache.set[index].block[j].tag;
  }
  icache.set[index].block[0].tag = tag;
  
  return;
}

// Perform a prefetch operation to D$ for the address 'addr'
void
dcache_prefetch(uint32_t addr)
{
  //
  //TODO: Implement D$ prefetch operation
  //
  uint32_t index = (uint32_t)(addr << dcache_tag_bits) >> (dcache_tag_bits + dcache_offset_bits);
  uint32_t tag = (uint32_t)(addr >> (dcache_index_bits + dcache_offset_bits));
  for(int i = 0; i < dcacheAssoc; i++)
  {
    if(dcache.set[index].block[i].tag == tag)
    {
      // printf("Hit\n");
      Block temp;
      temp.tag = dcache.set[index].block[i].tag;
      for(int j = i-1; j >= 0; j--)
      {
        dcache.set[index].block[j+1].tag = dcache.set[index].block[j].tag;
      }
      dcache.set[index].block[0].tag = temp.tag;
      if(inclusive) l2cache_access(addr);
      return;
    }
  }
  for(int j = dcacheAssoc-2; j >= 0; j--)
  {
    dcache.set[index].block[j+1].tag = dcache.set[index].block[j].tag;
  }
  dcache.set[index].block[0].tag = tag;

  return;
}
