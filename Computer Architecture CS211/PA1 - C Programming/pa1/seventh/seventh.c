#include <stdio.h>
#include <stdlib.h>

char last_Letter(char*);

char last_Letter(char* word){
  
  char ptr = word[0];
  for(int i = 0; word[i] != '\0'; i++){
    ptr = word[i];
    
  }
  return ptr;
}

int main(int argc, char** argv){
  
  for(int i = 1; i < argc; i++){
    printf("%c", last_Letter(argv[i]));
  }
  return 0;
}
