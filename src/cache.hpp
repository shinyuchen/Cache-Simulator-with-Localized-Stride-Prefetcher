//========================================================//
//  cache.h                                               //
//  Header file for the Cache Simulator                   //
//                                                        //
//  Includes function prototypes and global variables     //
//  and defines for the cache simulator                   //
//========================================================//

#ifndef CACHE_HPP
#define CACHE_HPP

#include <stdint.h>
#include <stdlib.h>
// #undef uint64_t
#define uint64_t unsigned long
//
// Student Information
//
extern const char *studentName;
extern const char *studentID;
extern const char *email;

//------------------------------------//
//          Global Defines            //
//------------------------------------//

#define TRUE 1
#define FALSE 0

//------------------------------------//
//        Cache Configuration         //
//------------------------------------//

extern uint32_t icacheSets;      // Number of sets in the I$
extern uint32_t icacheAssoc;     // Associativity of the I$
extern uint32_t icacheBlocksize; // Blocksize of the I$
extern uint32_t icacheHitTime;   // Hit Time of the I$

extern uint32_t dcacheSets;      // Number of sets in the D$
extern uint32_t dcacheAssoc;     // Associativity of the D$
extern uint32_t dcacheBlocksize; // Blocksize of the D$
extern uint32_t dcacheHitTime;   // Hit Time of the D$

extern uint32_t l2cacheSets;     // Number of sets in the L2$
extern uint32_t l2cacheAssoc;    // Associativity of the L2$
extern uint32_t l2cacheBlocksize;// Blocksize of the L2$
extern uint32_t l2cacheHitTime;  // Hit Time of the L2$
extern uint32_t inclusive;       // Indicates if the L2 is inclusive
extern uint32_t prefetch;

extern uint32_t blocksize;       // Block/Line size
extern uint32_t memspeed;        // Latency of Main Memory


struct Cache_Block{
  uint32_t tag;
};

struct Cache_Set{
  Cache_Block* block;
  uint32_t* access_history;
};

struct Cache{
  Cache_Set* set;
};
//------------------------------------//
//          Cache Statistics          //
//------------------------------------//

extern uint64_t icacheRefs;       // I$ references
extern uint64_t icacheMisses;     // I$ misses
extern uint64_t icachePenalties;  // I$ penalties

extern uint64_t dcacheRefs;       // D$ references
extern uint64_t dcacheMisses;     // D$ misses
extern uint64_t dcachePenalties;  // D$ penalties

extern uint64_t l2cacheRefs;      // L2$ references
extern uint64_t l2cacheMisses;    // L2$ misses
extern uint64_t l2cachePenalties; // L2$ penalties

extern uint64_t compulsory_miss;
extern uint64_t other_miss;

//------------------------------------//
//      Cache Function Prototypes     //
//------------------------------------//

// Initialize the predictor
//
void init_cache();

void clean_cache();

// Perform a memory access through the icache interface for the address 'addr'
// Return the access time for the memory operation
//
uint32_t icache_access(uint32_t addr);

// Perform a memory access through the dcache interface for the address 'addr'
// Return the access time for the memory operation
//
uint32_t dcache_access(uint32_t addr);

// Perform a memory access to the l2cache for the address 'addr'
// Return the access time for the memory operation
//
uint32_t l2cache_access(uint32_t addr);

uint32_t icache_prefetch_addr(uint32_t pc, uint32_t addr, char r_or_w);

uint32_t dcache_prefetch_addr(uint32_t pc, uint32_t addr, char r_or_w);

void icache_prefetch(uint32_t addr);

void dcache_prefetch(uint32_t addr);

#endif
