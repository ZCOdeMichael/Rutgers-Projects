#include <stdio.h>
#include <stdlib.h>
#include <math.h>

struct cache{
  struct set** sets;
};

struct set{
  struct block** blocks;
  unsigned int greatest;
};

struct block{
  size_t tag;
  unsigned int valid;
  unsigned int recently_Accessed;

};

/*
Initializes all global variables and tests to see if the inputs are valid
*/
int check(char**);

/*
Initialize Cache Sim
*/
struct cache* init();

/*
Printing results
*/
void print_Results();

/*
Checks if the blocks array is full
If full => -1
If not-full => index of empty block
*/
int isFull_Blocks(struct block**);

/*
Returns index of recently accessed memory
*/
int recently_Accessed(struct block**);

/*
Prefetch hit check
*/
int isHit_Prefetch(struct block**, size_t);



/*
On a hit block not Prefetch
*/
void OnHit_NonP(char, size_t, struct set*, struct block**, int);

/*
On a miss block not Prefetch
*/
void OnMiss_NonP(char, size_t, struct set*, struct block**);

/*
On a hit block Prefetch
*/
void OnHit_P(char, size_t, struct set*, struct block**, int);

/*
On a miss block Prefetch
*/
void OnMiss_P(char, size_t, struct cache*);

/*
Inserts address in cache
*/
void insert(char, size_t, struct cache*);
