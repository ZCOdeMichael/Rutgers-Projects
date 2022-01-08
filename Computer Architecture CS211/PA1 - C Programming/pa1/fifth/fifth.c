#include <stdio.h>
#include <stdlib.h>

int** allocate_matrix(int);
int has_n_distinct(int**, int);
int is_Magic(int**, int);


int** allocate_matrix(int size){

  int** ret_val = malloc(size * sizeof(int*));
  
  for(int i = 0; i < size; i++){
    ret_val[i] = malloc(size * sizeof(int));
  }

  return ret_val;
}

int has_n_distinct(int** mat, int size){

  for(int n = 1; n <= size*size; n++){
    int count = 0;
    
    for(int i = 0; i < size; i++){
      for(int j = 0; j < size; j++){
	if(mat[i][j] == n){
	  count++;
	}
      }
    }

    if(count != 1){
      return 0;
    }


  }
  
  
  return 1;
}

int is_Magic(int** mat, int size){
  
  int sum = 0;
  
  for(int i = 0; i < size; i++){
    sum += mat[0][i];
  }
  
  //Check Rows
  for(int i = 0; i < size; i++){
    int sum_Rows = 0;
    for(int j = 0; j < size; j++){
      sum_Rows += mat[i][j];
    }
    if(sum_Rows != sum){
      return 0;
    }
  }

  //printf("H");

  //Check Cols
  for(int i = 0; i < size; i++){
    int sum_Cols = 0;
    for(int j = 0; j < size; j++){
      sum_Cols += mat[j][i];
    }
    if(sum_Cols != sum){
      return 0;
    }
  }
  
  //printf("E");
  //Check Diagonals
  int sum_D1 = 0;
  for(int i = 0; i < size; i++){
    sum_D1 += mat[i][i];
  }
  if(sum_D1 != sum){
    return 0;
  }

  //printf("LL");
  int sum_D2 = 0;
  for(int i = size-1; i >= 0; i--){
    sum_D2 += mat[i][i];
  }
  if(sum_D2 != sum){
    return 0;
  }

  return 1;
}

/*
  size == 1 magic
  size <= 0 negative is not magic
*/
int main(int argc, char** argv){

  FILE* fp = fopen(argv[1], "r");
  if(fp == NULL){
    exit(0);
  }
  int size;
  if(fscanf(fp, "%d\n", &size) == EOF){
    exit(0);
  } 
  if(size == 1){
    int temp;
    fscanf(fp, "%d", &temp);
    if(temp != 1){
      printf("not-magic");
    }else{
      printf("magic");
    }
  }
  if(size < 1){
    exit(0);
  }
  
  int** matrix = allocate_matrix(size);

  for(int i = 0; i < size; i++){
    for(int j = 0; j < size; j++){
      fscanf(fp, "%d", &matrix[i][j]);
    }
  }
  
  if(has_n_distinct(matrix, size) == 1){
    if(is_Magic(matrix, size) == 1){
      printf("magic");
    }else{
      printf("not-magic");
    }
  }else{
    printf("not-magic");
  }
  fclose(fp);
  for(int i = 0; i < size; i++){
    free(matrix[i]);
  }
  return 0;
}
