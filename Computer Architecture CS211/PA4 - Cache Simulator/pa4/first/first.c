#include "first.h"

int Mem_reads;
int Mem_writes;
int Cache_hits;
int Cache_misses;

int Mem_reads_P;
int Mem_writes_P;
int Cache_hits_P;
int Cache_misses_P;

int Cache_Size;
int Block_Size;
int Set_Size;
int Prefetch_Size;
int Associativity_number;
int block_Offset;
int set_Index;

int FIFO;
int IRU;
int main(int argc, char** argv){

  
  if(argc != 7){
    printf("error");
    exit(0);
  }
  int temp = check(argv);
  if(temp == 0){
    exit(0);
  }
   
  //printf("Cache_Size %d\n Block_Size %d\n Set_Size %d\n Prefetch_Size %d\n Associativity_number %d\n", Cache_Size, Block_Size, Set_Size, Prefetch_Size, Associativity_number); 
  
  FILE* fp = fopen(argv[6], "r");
  if(fp == NULL){
    printf("error");
    exit(0);
  }
  
  struct cache* cache_NoPrefetch = init();
  struct cache* cache_Prefetch = init();
  
  
  block_Offset = log2(Block_Size);
  set_Index = log2(Cache_Size / (Associativity_number * Block_Size));
  
  char W_R_check;
  size_t current;
  
  while(fscanf(fp, "%c%zx", &W_R_check, &current) > 0){
    size_t tag = (current >> block_Offset) >> set_Index;
    int index = (current >> block_Offset) & ((1 << set_Index) - 1);
    
    //printf("%c ", W_R_check);
    //printf("%zx\n", current);
    if(W_R_check == '#'){
      break;
    }
    
    if(FIFO == 1){
      //FIFO
      //No Prefetch
      int isHit = 0;
      struct block** curr_blocks = cache_NoPrefetch->sets[index]->blocks;
      struct set* curr_set = cache_NoPrefetch->sets[index];
      for(int i = 0; i < Associativity_number; i++){
	if(curr_blocks[i]->tag == tag){
	  //It's a hit
	  OnHit_NonP(W_R_check, tag, curr_set, curr_blocks, i);
	  isHit = 1;
	  break;
	}
      }
      if(isHit == 0){
	//It was not a hit
	OnMiss_NonP(W_R_check, tag, curr_set, curr_blocks);
      }
      //printf("\n");
      //With Prefetch
      
      int isHit_P = 0;
      struct block** curr_blocks_P = cache_Prefetch->sets[index]->blocks;
      struct set* curr_set_P = cache_Prefetch->sets[index];
      for(int i = 0; i < Associativity_number; i++){
	if(curr_blocks_P[i]->tag == tag){
	  //It's a hit
	  OnHit_P(W_R_check, tag, curr_set_P, curr_blocks_P, i);
	  isHit_P = 1;
	  break;
	}
      }
      if(isHit_P == 0){
	//It was not a hit
	OnMiss_P(W_R_check, current, cache_Prefetch);
      }
      
    }else{
      //LRU
    }
    
  }
  
  print_Results();
  for(int i = 0; i < Set_Size; i++){
    for(int j = 0; j < Associativity_number; j++){
      free(cache_NoPrefetch->sets[i]->blocks[j]);
      free(cache_Prefetch->sets[i]->blocks[j]);
    }
    free(cache_NoPrefetch->sets[i]->blocks);
    free(cache_Prefetch->sets[i]->blocks);
    free(cache_NoPrefetch->sets[i]);
    free(cache_Prefetch->sets[i]);
  }
  free(cache_NoPrefetch->sets);
  free(cache_Prefetch->sets);
  free(cache_NoPrefetch);
  free(cache_Prefetch);
  fclose(fp);
  return 0;
}


void OnHit_NonP(char W_R, size_t tag, struct set* curr_s, struct block** curr, int i){
  //On a hit 
  if(W_R == 'W'){
    //Write and it's a hit
    //printf("1");
    Cache_hits++;
    Mem_writes++;
    //curr_s->greatest++;
    //curr[i]->recently_Accessed = curr_s->greatest;
    return;
  }else if(W_R == 'R'){
    //Read and it's a hit
    //printf("2");
    Cache_hits++;
    //curr_s->greatest++;
    //curr[i]->recently_Accessed = curr_s->greatest;
    return;
  }
}

void OnMiss_NonP(char W_R, size_t tag, struct set* curr_s,  struct block** curr){
  //On a miss
  if(W_R == 'W'){
    //Write and Miss
    Cache_misses++;
    Mem_writes++;
    Mem_reads++;
    
    int index_Fill = isFull_Blocks(curr);
    if(index_Fill == -1){
      //Set is full of Valid Blocks
      //printf("3");
      int num_Recently = recently_Accessed(curr);
      curr[num_Recently]->tag = tag;
      curr[num_Recently]->valid = 1;
      curr_s->greatest++;
      curr[num_Recently]->recently_Accessed = curr_s->greatest;
      return;
    }else{
      //printf("4");
      //Set is not full of Valid Blocks
      curr[index_Fill]->tag = tag;
      curr[index_Fill]->valid = 1;
      curr_s->greatest++;
      curr[index_Fill]->recently_Accessed = curr_s->greatest;
      return;
    }
  }else if(W_R == 'R'){
    //Read and Miss
    Cache_misses++;
    Mem_reads++;
    
    int index_Fill = isFull_Blocks(curr);
    if(index_Fill == -1){
      //The Set is full of valid Blocks
      //printf("5");
      int num_Recently = recently_Accessed(curr);
      curr[num_Recently]->tag = tag;
      curr[num_Recently]->valid = 1;
      curr_s->greatest++;
      curr[num_Recently]->recently_Accessed = curr_s->greatest;
      return;
    }else{
      //printf("6");
      //The Set is not full of valid Blocks
      curr[index_Fill]->tag = tag;
      curr[index_Fill]->valid = 1;
      curr_s->greatest++;
      curr[index_Fill]->recently_Accessed = curr_s->greatest;
      return;
    }
  }
}

void OnHit_P(char W_R, size_t tag, struct set* curr_s, struct block** curr, int i){
  //On hit
  if(W_R == 'W'){
    //Write and hit
    //printf("1");
    Cache_hits_P++;
    Mem_writes_P++;
    //curr_s->greatest++;
    //curr[i]->recently_Accessed = curr_s->greatest;
  }else if(W_R == 'R'){
    //Read and hit
    //printf("2");
    Cache_hits_P++;
    //curr_s->greatest++;
    //curr[i]->recently_Accessed = curr_s->greatest;
  }
}
/*
void OnMiss_P(char W_R, size_t tag, struct set* curr_s, struct block** curr, struct cache* cache){
  //On a miss
  if(W_R == 'W'){
    //Write and Miss
    Cache_misses_P ++;
    Mem_writes_P ++;
    Mem_reads_P++;
    int index_Fill = isFull_Blocks(curr);
    if(index_Fill == -1){
      //Set is full of valid blocks
      int num_Recently = recently_Accessed(curr);
      size_t prefetch_address = tag;
      curr[num_Recently]->tag = prefetch_address;
      curr[num_Recently]->valid = 1;
      
      //Mem_reads_P++;
      for(int i = 0; i < Prefetch_Size; i++){
	prefetch_address = prefetch_address + Block_Size;
	size_t tag = (prefetch_address >> block_Offset) >> set_Index;
	int index = (prefetch_address >> block_Offset) & ((1 << set_Index) -1);
	
	struct block** block_P = cache->sets[index]->blocks;
	
	if(isHit_Prefetch(block_P, tag) == -1){
	  num_Recently = recently_Accessed(block_P);
	  block_P[num_Recently]->tag = tag;
	  Mem_reads_P++;
	  //Cache_misses_P++;
	  curr_s->greatest++;
	  curr[num_Recently]->recently_Accessed = curr_s->greatest;
	}else{
	 
	  //Cache_hits_P++;
	  //Might be a hit
	  curr_s->greatest++;
	  curr[isHit_Prefetch(curr,prefetch_address)]->recently_Accessed = curr_s->greatest;
	}
      }
    }else{
      //Set is not full of valid blocks
      size_t prefetch_address = tag;
      curr[index_Fill]->tag = prefetch_address;
      curr[index_Fill]->valid = 1;
      curr_s->greatest++;
      curr[index_Fill]->recently_Accessed = curr_s->greatest;
      //Mem_reads_P++;
      for(int i = 0; i < Prefetch_Size; i++){
	prefetch_address = prefetch_address + Block_Size;
	
	if(isHit_Prefetch(curr, prefetch_address) == -1){
	  //Not a prefetch hit
	  int index_Fill_P = isFull_Blocks(curr);
	  if(index_Fill_P == -1){
	    //Full of valid blocks and not a prefetch hit
	    //if(isHit_Prefetch(curr, prefetch_address == -1)){
	    int num_Recently = recently_Accessed(curr);
	    curr[num_Recently]->tag = prefetch_address;
	    curr_s->greatest++;
	    curr[num_Recently]->recently_Accessed = curr_s->greatest;
	    Mem_reads_P++;
	    //Cache_misses_P++;
	    //}else{
	    //Cache_hits_P++;
	    //zero_Accessed(curr);
	    //curr[isHit_Prefetch(curr, prefetch_address)]->recently_Accessed = 1;
	    //}
	        
	  }else{
	    //Not full of valid blocks and not a prefetch hit
	    //if(isHit_Prefetch(curr, prefetch_address == -1)){
	    curr[index_Fill_P]->tag = prefetch_address;
	    curr[index_Fill_P]->valid = 1;
	    curr_s->greatest++;
	    curr[index_Fill_P]->recently_Accessed = curr_s->greatest;
	    Mem_reads_P++;
	    //Cache_misses_P++;
	    //}else{
	    //Cache_bits_P++;
	    //zero_Accessed(curr);
	    //curr[isHit_Prefetch(curr, prefetch_address)]->recently_Accessed = 1;
	    //}
	  }
	  //Mem_reads_P++;
	}else{
	  //Prefetch hits
	    
	
	  //Cache_hits_P++;
	  //Might be a hit
	  curr_s->greatest++;
	  curr[isHit_Prefetch(curr, prefetch_address)]->recently_Accessed = curr_s->greatest;
	}
	//Mem_reads_P++
      }
    }
  }else if(W_R == 'R'){
    //Read and Miss
    Cache_misses_P++;
    Mem_reads_P++;
    int index_Fill = isFull_Blocks(curr);
    if(index_Fill == -1){
      //Set is full of valid blocks
      int num_Recently = recently_Accessed(curr);
      size_t prefetch_address = tag;
      curr[num_Recently]->tag = prefetch_address;
      curr[num_Recently]->valid = 1;
      //Mem_reads_P++;
      for(int i = 0; i < Prefetch_Size; i++){
        prefetch_address = prefetch_address + Block_Size;
        if(isHit_Prefetch(curr, prefetch_address) == -1){
	  //Not a pretch hit and full of valid blocks
          num_Recently = recently_Accessed(curr);
          curr[num_Recently]->tag = prefetch_address;
          Mem_reads_P++;
          //Cache_misses_P++;
          curr_s->greatest++;
          curr[num_Recently]->recently_Accessed = curr_s->greatest;
        }else{
	  //Prefetch hit and full of valid blocks
         
          //Cache_hits_P++;
          //Might be a hit
          curr_s->greatest++;
	  curr[isHit_Prefetch(curr,prefetch_address)]->recently_Accessed = curr_s->greatest;
        }
      }

      
    }else{
      //Set is not full of valid blocks
      size_t prefetch_address = tag;
      curr[index_Fill]->tag = prefetch_address;
      curr[index_Fill]->valid = 1;
      curr_s->greatest++;
      curr[index_Fill]->recently_Accessed = curr_s->greatest;
      //Mem_reads_P++;
      for(int i = 0; i < Prefetch_Size; i++){
        prefetch_address = prefetch_address + Block_Size;
        if(isHit_Prefetch(curr, prefetch_address) == -1){
	  //Not a prefetch hit
          int index_Fill_P = isFull_Blocks(curr);
          if(index_Fill_P == -1){
	    //Not a prefetch hit and full of valid blocks
            //if(isHit_Prefetch(curr, prefetch_address == -1)){
	    int num_Recently = recently_Accessed(curr);
	    curr[num_Recently]->tag = prefetch_address;
	    curr_s->greatest++;
	    curr[num_Recently]->recently_Accessed = curr_s->greatest;
	    Mem_reads_P++;
	    //Cache_misses_P++;
	    //}else{
	    //zero_Accessed(curr);
	    //curr[isHit_Prefetch(curr, prefetch_address)]->recently_Accessed = 1;
	    //}
	        
          }else{
	    //Not a prefetch hit and not full of valid blocks
            curr[index_Fill_P]->tag = prefetch_address;
            curr[index_Fill_P]->valid = 1;
            curr_s->greatest++;
            curr[index_Fill_P]->recently_Accessed = curr_s->greatest;
            Mem_reads_P++;
	    //Cache_misses_P++;
	  }

          //Mem_reads_P++;
        }else{
	  //Prefetch hit
	  //Cache_hits_P++;
          //Might be a hit
	  curr_s->greatest++;
          curr[isHit_Prefetch(curr, prefetch_address)]->recently_Accessed = curr_s->greatest;
        }
        //Mem_reads_P++;
      }

    }
  }
}
*/

void OnMiss_P(char W_R, size_t address, struct cache* cache){
  //On a miss
  size_t prefetch_Address = address;
  size_t tag = (address >> block_Offset) >> set_Index;
  int index = (address >> block_Offset) & ((1 << set_Index) - 1);
  struct block** curr = cache->sets[index]->blocks;
  struct set* curr_S = cache->sets[index];
  if(W_R == 'W'){
    //Miss and it's a write
    Cache_misses_P++;
    Mem_writes_P++;    
    Mem_reads_P++;
    
    int index_Fill = isFull_Blocks(curr);
    if(index_Fill == -1){
      //Set is full of valid blocks
      //printf("3");
      int num_Recently = recently_Accessed(curr);
      curr[num_Recently]->tag = tag;
      curr[num_Recently]->valid = 1;
      
      curr_S->greatest++;
      curr[num_Recently]->recently_Accessed = curr_S->greatest;

      for(int i = 0; i < Prefetch_Size; i++){
	prefetch_Address = prefetch_Address + Block_Size;
	insert(W_R, prefetch_Address, cache);
	//Mem_reads_P++;
      }
    }else{
      //Set is not full of valid blocks
      //printf("4");
      curr[index_Fill]->tag = tag;
      curr[index_Fill]->valid = 1;
      
      curr_S->greatest++;
      curr[index_Fill]->recently_Accessed = curr_S->greatest;

      for(int i = 0; i < Prefetch_Size; i++){
	prefetch_Address = prefetch_Address + Block_Size;
	insert(W_R, prefetch_Address, cache);
	//Mem_reads_P++;
      }
    }
    
  }else if(W_R == 'R'){
    
    Cache_misses_P++;
    Mem_reads_P++;
    

    int index_Fill = isFull_Blocks(curr);
    if(index_Fill == -1){
      //Set is full of valid blocks
      //printf("5");
      int num_Recently = recently_Accessed(curr);
      curr[num_Recently]->tag = tag;
      curr[num_Recently]->valid = 1;

      curr_S->greatest++;
      curr[num_Recently]->recently_Accessed = curr_S->greatest;
      
      for(int i = 0; i < Prefetch_Size; i++){
	prefetch_Address = prefetch_Address + Block_Size;
	insert(W_R, prefetch_Address, cache);
	//Mem_reads_P++;
      }
    }else{
      //Set is not full of valid blocks
      //printf("6");
      curr[index_Fill]->tag = tag;
      curr[index_Fill]->valid = 1;
      
      curr_S->greatest++;
      curr[index_Fill]->recently_Accessed = curr_S->greatest;
      
      for(int i = 0; i < Prefetch_Size; i++){
	prefetch_Address = prefetch_Address + Block_Size;
	insert(W_R, prefetch_Address, cache);
	//Mem_reads_P++;
      }
    }
  }
}

void insert(char W_R, size_t address, struct cache* cache){
  size_t tag = (address >> block_Offset) >> set_Index;
  int index = (address >> block_Offset) & ((1 << set_Index) - 1);
  struct block** curr = cache->sets[index]->blocks;
  struct set* curr_S = cache->sets[index];
  
  int hit = isHit_Prefetch(curr, tag);
  if(hit == -1){
    Mem_reads_P++;
    int index_Fill = isFull_Blocks(curr);
    
    if(index_Fill == -1){
      //Set is full of valid blocks
      int num_Recently = recently_Accessed(curr);
      curr[num_Recently]->tag = tag;
      curr[num_Recently]->valid = 1;
      
      curr_S->greatest++;
      curr[num_Recently]->recently_Accessed = curr_S->greatest;
    }else{
      //Set is not full of valid blocks
      curr[index_Fill]->tag = tag;
      curr[index_Fill]->valid = 1;
      
      curr_S->greatest++;
      curr[index_Fill]->recently_Accessed = curr_S->greatest;
    }
  }else{
    
    //curr_S->greatest++;
    //curr[hit]->recently_Accessed = curr_S->greatest;
  }
}

int isHit_Prefetch(struct block** check, size_t tag){
  for(int i = 0; i < Associativity_number; i++){
    if(check[i]->tag == tag){
      return i;
    }
  }
  return -1;
}

int recently_Accessed(struct block** recent){
  int curr = recent[0]->recently_Accessed;
  int result = 0;
  for(int i = 1; i < Associativity_number; i++){
    if((curr > recent[i]->recently_Accessed) && (recent[i]->valid == 1)){
      curr = recent[i]->recently_Accessed;
      result = i;
    }
  }
  return result;
}

int isFull_Blocks(struct block** check){
  for(int i = 0; i < Associativity_number; i++){
    if(check[i]->valid == 0){
      return i;
    }
  }
  return -1;
}

void print_Results(){
  printf("no-prefetch\n");
  printf("Memory reads: %d\n", Mem_reads);
  printf("Memory writes: %d\n", Mem_writes);
  printf("Cache hits: %d\n", Cache_hits);
  printf("Cache misses: %d\n", Cache_misses);
  printf("with-prefetch\n");
  printf("Memory reads: %d\n", Mem_reads_P);
  printf("Memory writes: %d\n", Mem_writes_P);
  printf("Cache hits: %d\n", Cache_hits_P);
  printf("Cache misses: %d", Cache_misses_P);
}

struct cache* init(){
  struct cache* result = malloc(sizeof(struct cache));
  struct set** sets = malloc(sizeof(struct set*) * Set_Size);
  for(int i = 0; i < Set_Size; i++){
    sets[i] = malloc(sizeof(struct set));
  }
  
  for(int i = 0; i < Set_Size; i++){
    struct block** blocks = malloc(sizeof(struct block*) * Associativity_number);
    for(int j = 0; j < Associativity_number; j++){
      blocks[j] = malloc(sizeof(struct block));
      blocks[j]->valid = 0;
      blocks[j]->recently_Accessed = 0;
      //blocks[j]->tag = -1;
    }
    sets[i]->blocks = blocks;
    sets[i]->greatest = 1;
  }
  
  result->sets = sets;
  
  return result;
}

int check(char** input){
  
  Mem_reads = 0;
  Mem_writes = 0;
  Cache_hits = 0;
  Cache_misses = 0;

  Mem_reads_P = 0;
  Mem_writes_P = 0;
  Cache_hits_P = 0;
  Cache_misses_P = 0;

  Cache_Size = atoi(input[1]);
  Block_Size = atoi(input[2]);
  char* Cache_Policy = input[3];
  char* Associativity = input[4];
  Prefetch_Size = atoi(input[5]);

  if(fmod(log2(Cache_Size), 1) != 0){
    printf("error");
    return 0;
  }
  if(fmod(log2(Block_Size), 1) != 0){
    printf("error");
    return 0;
  }
  
  char* direct = "direct";
  char* assoc = "assoc";
  char* fifo = "fifo";
  char* iru = "iru";

  int isDirect = 1;
  int isAssoc = 1;
  int isFIFO = 1;
  int isIRU = 1;

  /*
    Check inputs for Direct
  */
  for(int i = 0; Associativity[i] != '\0'; i++){
    if(direct[i] != '\0'){
      if(Associativity[i] != direct[i]){
	isDirect = 0;
	break;
      }
      if(direct[i+1] != '\0' && Associativity[i+1] == '\0'){
	isDirect = 0;
	break;
      }
    }
    else{
      if(Associativity[i] != '\0'){
	isDirect = 0;
      }
      break;
    }

  }
  /*
    Check inputs for Assoc
  */
  for(int i = 0; Associativity[i] != '\0'; i++){
    if(assoc[i] != '\0'){
      if(Associativity[i] != assoc[i]){
	isAssoc = 0;
	break;
      }
      if(assoc[i+1] != '\0' && Associativity[i+1] == '\0'){
	isAssoc = 0;
	break;
      }
    }else{
      if(Associativity[i] != ':' && Associativity != '\0'){
	isAssoc = 0;
      }
      break;
    }
  }
  
  if(isDirect == 0 && isAssoc == 0){
    printf("error");
    //free(direct);
    //free(assoc);
    //free(fifo);
    //free(iru);
    return 0;
  }else if(isDirect == 1){
    //Direct
    Associativity_number = 1;
    Set_Size = Cache_Size / (Block_Size * Associativity_number);
  }else{
    if(Associativity[5] != '\0'){
      //Set Associative
      char assoc[100];
      int temp = 0;
      for(int i = 6; Associativity[i] != '\0'; i++){
	assoc[temp] = Associativity[i];
	temp++;
      }
      Associativity_number = atoi(assoc);
      Set_Size = Cache_Size / ( Block_Size * Associativity_number); /////////////

    }else{
      //Fully Associative
      Associativity_number = Cache_Size / Block_Size;
      Set_Size = Cache_Size / (Block_Size * Associativity_number);
    }
  }
  
  /*
    Inputs for Cache_Policy (FIFO)
  */
  for(int i = 0; Cache_Policy[i] != '\0'; i++){
    if(fifo[i] != '\0'){
      if(fifo[i] != Cache_Policy[i]){
	isFIFO = 0;
	break;
      }
      if(fifo[i+1] != '\0' && Cache_Policy[i+1] == '\0'){
	isFIFO = 0;
	break;
      }
    }else{
      if(Cache_Policy[i] != '\0'){
	isFIFO = 0;
      }
      break;
    }
  }

  /*
    Inputs for Cache_Policy (IRU)
  */
  for(int i = 0; Cache_Policy[i] != '\0'; i++){
    if(iru[i] != '\0'){
      if(iru[i] != Cache_Policy[i]){
        isIRU = 0;
        break;
      }
      if(iru[i+1] != '\0' && Cache_Policy[i+1] == '\0'){
        isIRU = 0;
        break;
      }

    }else{
      if(Cache_Policy[i] != '\0'){
        isIRU = 0;
      }
      break;
    }
  }
  
  if(isIRU == 0 && isFIFO == 0){
    printf("error");
    return 0;
  }else if(isFIFO == 1){
    FIFO = 1;
    IRU = 0;
  }else{
    FIFO = 0;
    IRU = 1;
  }
  
  return 1;
}
