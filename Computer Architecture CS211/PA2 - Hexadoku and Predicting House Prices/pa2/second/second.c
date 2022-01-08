#include <stdio.h>
#include <stdlib.h>

char hexaDec[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

struct sepBox{
  int row1;
  int row2;
  int col1;
  int col2;

};

struct guess{
  char current;
  char* guesses;

  int index;
  int numGuesses;
};


struct guess*** hexadokuArr;

void init();
void printHexa();

int isSolution();
int chechGuess(int, int); // TODO NEED TO IMPLEMENT 
struct sepBox* identifyBox(int, int);
char* hexaGuess(int, int);
void updateHexa();

void updateHexa(){
  for(int i = 0; i < 16; i++){
    for(int j = 0; j < 16; j++){
      if(hexadokuArr[i][j]->numGuesses != -1){
	struct guess *temp = hexadokuArr[i][j];
	temp->current = temp->guesses[temp->index];
      }
    }
  }
}

int checkGuess(int i, int j){
  char current = hexadokuArr[i][j]->current;
  struct sepBox *temp = identifyBox(i, j);
  
  //Row Check
  for(int col = 0; col < 16; col++){
    if(current == hexadokuArr[i][col]->current && col != j){
      free(temp);
      return 0;
    }
  }

  //Col Check
  for(int row = 0; row < 16; row++){
    if(current == hexadokuArr[row][j]->current && row != i){
      free(temp);
      return 0;
    }
  }

  //Box Check
  for(int row = temp->row1; row <= temp->row2; row++){
    for(int col = temp->col1; col <= temp->col2; col++){
      if(current == hexadokuArr[row][col]->current && row != i && col != j){
	free(temp);
	return 0;
      }
    }
  }
  free(temp);
  return 1;
}

char* hexaGuess(int i, int j){
  char* rowNon = malloc(sizeof(char) * 16);
  char* colNon = malloc(sizeof(char) * 16);
  char* boxNon = malloc(sizeof(char) * 16);

  char* dupesTemp = malloc(sizeof(char) * 16);
  char* dupes = malloc(sizeof(char) * 16);
  
  for(int num = 0; num < 16; num++){
    rowNon[num] = '-';
    colNon[num] = '-';
    boxNon[num] = '-';

    dupesTemp[num] = '-';
    dupes[num] = '-';
  }
  
  int genCount = 0;
  //Check Row
  for(int k = 0; k < 16; k++){
    char current = hexaDec[k];
    int isNon = 1;
    for(int l = 0; l < 16; l++){
      if(hexadokuArr[i][l]->current == current){
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
      if(hexadokuArr[l][j]->current == current){
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
	if(hexadokuArr[row][col]->current == current){
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
	  dupesTemp[genCount] = current;
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
    if(dupesTemp[k] != '-'){
      char current = dupes[k];
      for(int l = 0; l < 16; l++){
	if(current == boxNon[l]){
	  dupes[genCount] = current;
	  genCount++;
	  break;
	}
      }
    }else{
      break;
    }
  }

  free(rowNon);
  free(colNon);
  free(boxNon);
  free(dupesTemp);
  

  return dupes;
  
}

void init(){
  hexadokuArr = malloc(sizeof(struct guess **) * 16);
  for(int i = 0; i < 16; i++){
    hexadokuArr[i] = malloc(sizeof(struct guess *) * 16);
  }
  
}

int isSolution(){
  for(int i = 0; i < 16; i++){
    for(int j = 0; j < 16; j++){
      char current = hexadokuArr[i][j]->current;
      
      if(current != '-'){
	struct sepBox *temp = identifyBox(i, j);
	
	//Row Check
	for(int col = 0; col < 16; col++){
	  if(current == hexadokuArr[i][col]->current && col != j){
	    return 0;
	  }
	}
	//Col Check
	for(int row = 0; row < 16; row++){
	  if(current == hexadokuArr[row][j]->current && row != i){
	    return 0;
	  }
	}
	
	//Box Check
	for(int row = temp->row1; row <= temp->row2; row++){
	  for(int col = temp->col1; col <= temp->col2; col++){
	    if(current == hexadokuArr[row][col]->current && row != i && col != j){
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


void printHexa(){
  for(int i = 0; i < 16; i++){
    for(int j = 0; j < 16; j++){
      printf("%c\t", hexadokuArr[i][j]->current);
    }
    printf("\n");
  }
}

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

int main(int argc, char** argv){
  
  FILE* fp = fopen(argv[1], "r");
  if(fp == NULL){
    return 0;
  }
  init();
  
  char hex;
  
  for(int i = 0; i < 16; i++){
    for(int j = 0; j < 16; j++){
      fscanf(fp, "%c\t", &hex);
      hexadokuArr[i][j] = malloc(sizeof(struct guess));
      hexadokuArr[i][j]->current = hex;
      if(hex != '-'){
	hexadokuArr[i][j]->numGuesses = -1;
      }else{
	hexadokuArr[i][j]->numGuesses = 0;
	hexadokuArr[i][j]->index = 0;
      }
    }
  }
  
  fclose(fp);
  
  if(!isSolution()){
    printf("no-solution");
    
    for(int i = 0; i < 16; i++){
      for(int j = 0; j < 16; j++){
	free(hexadokuArr[i][j]->guesses);
	free(hexadokuArr[i][j]);
      }
    }
    for(int i = 0; i < 16; i++){
      free(hexadokuArr[i]);
    }
    free(hexadokuArr);
  }else{
    for(int i = 0; i < 16; i++){
      for(int j = 0; j < 16; j++){
	
	if(hexadokuArr[i][j]->current == '-'){
	  hexadokuArr[i][j]->guesses = hexaGuess(i, j);
	  for(int k = 0; k < 16; k++){
	    if(hexadokuArr[i][j]->guesses[i] != '-'){
	      hexadokuArr[i][j]->numGuesses++;
	    }else{
	      break;
	    }
	  }
	}
      }
    }

    
    for(int i = 0 ; i < 16; i++){
      for(int j = 0; j < 16; j++){
	
	if(hexadokuArr[i][j]->numGuesses != -1){
	  struct guess *temp = hexadokuArr[i][j];
	  temp->current = temp->guesses[0];
	  if(!checkGuess(i, j)){
	    for(int i = 1; i < temp->numGuesses; i++){
	      temp->current = temp->guesses[i];
	      if(checkGuess(i, j)){
		break;
	      }
	      temp->index++;
	    }
	    if(temp->index == temp->numGuesses -1){
	      
	    }
	  }
	}
      }
    }
    
    
    printHexa();
    
    
    for(int i = 0; i < 16; i++){
      for(int j = 0; j < 16; j++){
        free(hexadokuArr[i][j]->guesses);
	free(hexadokuArr[i][j]);
      }
    }
    for(int i = 0; i < 16; i++){
      free(hexadokuArr[i]);
    }
    free(hexadokuArr);
   }
  
  return 0;
}
