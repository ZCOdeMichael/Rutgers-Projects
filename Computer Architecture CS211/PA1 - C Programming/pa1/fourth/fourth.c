#include <stdio.h>
#include <stdlib.h>



int** allocate_matrix(int, int);
int** solution_matrix(int**, int**, int, int);
int calc_num(int**, int**, int, int);
void print_matrix(int**, int, int);
void free_mem(int**, int);

int rows_fMatrix;
int cols_fMatrix;
int rows_sMatrix;
int cols_sMatrix;

void free_mem(int** ptr, int size){
  for(int i = 0; i < size; i++){
    free(ptr[i]);
  }

}

void print_matrix(int** mat, int rows, int cols){
  for(int i = 0; i < rows; i++){
    for(int j = 0; j < cols; j++){
      printf("%d\t", mat[i][j]);
    }
    printf("\n");
  }

}

int** allocate_matrix(int rows, int cols){
  int** ret_val = malloc(rows * sizeof(int*));

  for(int i = 0; i < rows; i++){
    ret_val[i] = malloc(cols * sizeof(int));
  }

  return ret_val;

}

int** solution_matrix(int** first, int** second, int rows, int cols){
  int** sol_val = allocate_matrix(rows, cols);
  
  for(int i = 0; i < rows; i++){
    for(int j = 0; j < cols; j++){      
      sol_val[i][j] = calc_num(first, second, i, j);
      
    }
  }

  return sol_val;
  
}

int calc_num(int** first, int** second, int fRows, int sCols){
  int sol_val = first[fRows][0] * second[0][sCols];
  //printf("%d\n", sol_val);
  
  for(int i = 1; i < cols_fMatrix; i++){
    sol_val += first[fRows][i] * second[i][sCols];
    //printf("%d\n", sol_val);
  }
  //printf("%d\n", sol_val);
  //printf("\n");
  return sol_val;

}

/*
  negative dim
  zero dim
*/
int main(int argc, char** argv){
  
  FILE* fp = fopen(argv[1], "r");
  if(fp == NULL){
    exit(0);
  }

  rows_fMatrix = 0;
  cols_fMatrix = 0;
  rows_sMatrix = 0;
  cols_sMatrix = 0;


  if(fscanf(fp, "%d\t%d", &rows_fMatrix, &cols_fMatrix) == EOF){
    exit(0);
  }
  
  //printf("%d\t%d\n", rows_fMatrix, cols_fMatrix);
  
  int** first_Matrix = allocate_matrix(rows_fMatrix, cols_fMatrix);

  for(int i = 0; i < rows_fMatrix; i++){
    for(int j = 0; j < cols_fMatrix; j++){
      fscanf(fp, "%d", &first_Matrix[i][j]);
      //printf("%d\n", first_Matrix[i][j]);
    }
  }
  
  fscanf(fp, "%d\t%d", &rows_sMatrix, &cols_sMatrix);
  
  int** second_Matrix = allocate_matrix(rows_sMatrix, cols_sMatrix);
  
  for(int i = 0; i < rows_sMatrix; i++){
    for(int j = 0; j < cols_sMatrix; j++){
      fscanf(fp, "%d", &second_Matrix[i][j]);
    }
  }
  
  int** solution = NULL;
  
  //print_matrix(first_Matrix, rows_fMatrix, cols_fMatrix);
  //printf("\n");
  //print_matrix(second_Matrix, rows_sMatrix, cols_sMatrix);
  //printf("\n");

  if(rows_fMatrix <= 0 || cols_fMatrix <= 0 || rows_sMatrix <= 0 || cols_sMatrix <= 0){
    exit(0);
  }else if(cols_fMatrix != rows_sMatrix){
    printf("bad-matrices");
  }else{
    solution = solution_matrix(first_Matrix, second_Matrix, rows_fMatrix, cols_sMatrix);
  }
  
  if(solution != NULL){
    for(int i = 0; i < rows_fMatrix; i++){
      for(int j = 0; j < cols_sMatrix; j++){
	printf("%d\t", solution[i][j]);
      }
      printf("\n");
    }
    free_mem(solution, rows_fMatrix);
  }

  
  fclose(fp);
  free_mem(first_Matrix, rows_fMatrix);
  free_mem(second_Matrix, rows_sMatrix);
  
  return 0;

}
