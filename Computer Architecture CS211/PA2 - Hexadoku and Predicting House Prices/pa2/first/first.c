#include <stdio.h>
#include <stdlib.h>

char hexaDec[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

struct sepBox{
  int row1;
  int row2;
  int col1;
  int col2;

};


char** hexadokuArr;

char** init();
char hexaReplace(int, int);
void printHexa();
int isSolution();
struct sepBox* identifyBox(int, int);


struct sepBox* identifyBox(int i, int j){
  struct sepBox *result = malloc(sizeof(struct sepBox));
  
  //i,j 0 - 3
  //i,j 4 - 7
  //i,j 8 - 11
  //i,j 12 - 15

  //Check Row
  if(i >= 0 && i <= 3){
    result->row1 = 0;
    result->row2 = 3;
  }else if(i >= 4 && i <= 7){
    result->row1 = 4;
    result->row2 = 7;
  }else if(i >= 8 && i <= 11){
    result->row1 = 8;
    result->row2 = 11;
  }else{
    result->row1 = 12;
    result->row2 = 15;
  }
  
  //Check Col
  if(j >= 0 && j <= 3){
    result->col1 = 0;
    result->col2 = 3;
  }else if(j >= 4 && j <= 7){
    result->col1 = 4;
    result->col2 = 7;
  }else if(j >= 8 && j <= 11){
    result->col1 = 8;
    result->col2 = 11;
  }else{
    result->col1 = 12;
    result->col2 = 15;
  }

  return result;
}

char** init(){
  char** result = malloc(sizeof(char*) * 16);

  for(int i = 0; i < 16; i++){
    result[i] = malloc(sizeof(char) * 16);
  }
  
  return result;
}

void printHexa(){
  for(int i = 0; i < 16; i++){
    for(int j = 0; j < 16; j++){
      printf("%c\t", hexadokuArr[i][j]);
      
    }
    printf("\n");
  }
}

int isSolution(){
  for(int i = 0; i < 16; i++){
    for(int j = 0; j < 16; j++){
      char current = hexadokuArr[i][j];
      
      if(current != '-'){
	struct sepBox *temp = identifyBox(i, j);
	
	//Row Check
	for(int col = 0; col < 16; col++){
	  if(current == hexadokuArr[i][col] && col != j){
	    free(temp);
	    return 0;
	  }
	}
	//Col Check
	for(int row = 0; row < 16; row++){
	  if(current == hexadokuArr[row][j] && row != i){
	    free(temp);
	    return 0;
	  }
	}
	
	for(int row = temp->row1; row <= temp->row2; row++){
	  for(int col = temp->col1; col <= temp->col2; col++){
	    if(current == hexadokuArr[row][col] && row != i && col != j){
	      //printf("%d %d\n", temp->row1, temp->row2);
	      //printf("%d %d\n", temp->col1, temp->col2);
	      //printf("%d %d\n", row, col);
	      //printf("%d %d", i, j);
	      free(temp);
	      return 0;
	    }
	  }
	}
	
	free(temp);
      }
    }
  }
  return 1;
}

char hexaReplace(int i, int j){
  char* rowNon = malloc(sizeof(char) * 16);
  char* colNon = malloc(sizeof(char) * 16);
  char* boxNon = malloc(sizeof(char) * 16);
  
  char* dupes = malloc(sizeof(char) * 16);
  char* dupes1 = malloc(sizeof(char) * 16);
  
  for(int num = 0; num < 16; num++){
    rowNon[num] = '-';
    colNon[num] = '-';
    boxNon[num] = '-';
    
    dupes[num] = '-';
    dupes1[num] = '-';
  } 
  
  int genCount = 0;
  //Check Row
  for(int k = 0; k < 16; k++){
    char current = hexaDec[k];
    int isNon = 1;
    for(int l = 0; l < 16; l++){
      if(hexadokuArr[i][l] == current){
	isNon = 0;
	break;
      }
    }
    
    if(isNon){
      colNon[genCount] = current;
      genCount++;
    }
  }

  genCount = 0;
  //Check Col
  for(int k = 0; k < 16; k++){
    char current = hexaDec[k];
    int isNon = 1;
    for(int l = 0; l < 16; l++){
      if(hexadokuArr[l][j] == current){
        isNon = 0;
        break;
      }
    }

    if(isNon){
      rowNon[genCount] = current;
      genCount++;
    }
  }

  genCount = 0;
  //Check Box

  for(int k = 0; k < 16; k++){
    char current = hexaDec[k];
    int isNon = 1;
    struct sepBox *temp = identifyBox(i, j);
    for(int row = temp->row1; row < temp->row2; row++){
      for(int col = temp->col1; col < temp->col2; col++){
	if(hexadokuArr[row][col] == current){
	  isNon = 0;
	  break;
	}
      }
    }
    
    if(isNon){
      boxNon[genCount] = current;
      genCount++;
    }
    free(temp);
  }

  genCount = 0;
  //Dupes for row and col
  for(int k = 0; k < 16; k++){
    if(rowNon[k] != '-'){
      char current = rowNon[k];
      for(int l = 0; l < 16; l++){
	if(current == colNon[l]){
	  dupes[genCount] = current;
	  genCount++;
	  break;
	}
      }
    }else{
      break;
    }
  }
  
  genCount = 0;
  //Dupes for dupes and box
  for(int k = 0; k < 16; k++){
    if(dupes[k] != '-'){
      char current = dupes[k];
      for(int l = 0; l < 16; l++){
	if(current == boxNon[l]){
	  dupes1[genCount] = current;
	  genCount++;
	  break;
	}
      }
    }else{
      break;
    }
  }

  char result = '-';

  if(dupes1[1] == '-'){
    result = dupes1[0];
  }
  
  free(rowNon);
  free(colNon);
  free(boxNon);
  free(dupes);
  free(dupes1);

  return result;;
}

int main(int argc, char** argv){
  
  FILE* fp = fopen(argv[1], "r");
  if(fp == NULL){
    return 0;
  }

  hexadokuArr = init();
  char hex;

  for(int i = 0; i < 16; i++){
    for(int j = 0; j < 16; j++){
      fscanf(fp, "%c\t", &hex);
      hexadokuArr[i][j] = hex;
    }
  }
  fclose(fp);
  //printHexa();
  
  if(!isSolution()){
    printf("no-solution");
    //FREE MEM
    for(int i = 0; i < 16; i++){
      free(hexadokuArr[i]);
    }
    free(hexadokuArr);
    exit(0);
  }else{
    for(int row = 0; row < 16; row++){
      for(int i = 0; i <= row; i++){
	for(int j = 0; j < 16; j++){
	  if(hexadokuArr[i][j] == '-'){
	    char temp = hexaReplace(i, j);
	    hexadokuArr[i][j] = temp;
	  }
	}
      }
    }
    
    for(int i = 0; i < 16; i++){
      for(int j = 0; j < 16; j++){
	if(hexadokuArr[i][j] == '-'){
	  printf("no-solution");
	  for(int i = 0; i < 16; i++){
	    free(hexadokuArr[i]);
	  }
	  free(hexadokuArr);
	  exit(0);
	}
      }
    }
    
    printHexa();
    for(int i = 0; i < 16; i++){
      free(hexadokuArr[i]);
    }
    free(hexadokuArr);
  }
  return 0;
}
