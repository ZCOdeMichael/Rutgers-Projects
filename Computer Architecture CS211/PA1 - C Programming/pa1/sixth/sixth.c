#include <stdio.h>
#include <stdlib.h>


char* word_To_PigLatin(char*);
int is_Vowel(char);
int is_Letter(char);
char to_Lower(char);

char vowels[] = {'a', 'e', 'i', 'o', 'u'};
char cap_Vowels[] = {'A', 'E', 'I', 'O', 'U'};

char to_Lower(char letter){
  for(int i = 0; i < 5; i++){
    if(letter == cap_Vowels[i]){
      return vowels[i];
    }
  }

  return letter;
}

int is_Vowel(char letter){
  for(int i = 0; i < 5; i++){
    if(vowels[i] == to_Lower(letter)){
      return 1;
    }
  }
  return 0;
}

char* word_To_PigLatin(char* word){
  
  int length = 0;
  for(int i = 0; word[i] != '\0'; i++){
    length++;
  }
  
  if(is_Vowel(word[0]) == 1){
    char* temp = malloc(sizeof(char)*(length + 3));
    for(int i = 0; i < length; i++){
      temp[i] = word[i];
    }
    temp[length] = 'y';
    temp[length+1] = 'a';
    temp[length+2] = 'y';
    
    return temp;
  }else{
    char* temp = malloc(sizeof(char)*(length + 2));
    int to_Vowel = 0;
    for(int i = 0; word[i] != '\0'; i++){
      if(is_Vowel(word[i]) == 1){
	break;
      }
      to_Vowel++;
    }
    int count = 0;
    for(int i = to_Vowel; word[i] != '\0'; i++){
      temp[count] = word[i];
      count++;
    }
    for(int i = 0; i < to_Vowel; i++){
      temp[count] = word[i];
      count++;
    }
    temp[count] = 'a';
    temp[count+1] = 'y';
    
    return temp;

  }
}


int main(int argc, char** argv){
  /*
  for(int i = 0; i < argc; i++){
    printf("%s ", word_To_PigLatin(argv[0][i]);
    
  }
*/

  for(int i = 1; i < argc; i++){
    char* temp = word_To_PigLatin(argv[i]);
    printf("%s ", temp);
    free(temp);
  }
  return 0;
  
}
