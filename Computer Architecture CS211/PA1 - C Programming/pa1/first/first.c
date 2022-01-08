#include <stdio.h>
#include <stdlib.h>

int isTruncPrime(int temp);
int isPrime(int num);
int multipleOfThree(int num);

int main(int argc, char** argv){

  FILE* fp = fopen(argv[1], "r");
  if(fp == NULL){
    exit(0);
  }
  
  int current;  
  int size;
  if(fscanf(fp,"%d", &size) == EOF){
    exit(0);
  }
  
  if(size == 0){
    exit(0);
  }
  
  //printf("%d\n", current);
  int *arr = malloc(sizeof(int)*(size));
  int i = 0;

  while(fscanf(fp, "%d\n", &current) >  0){
    //printf("%d\n", current);
    arr[i] = current;
    i++;
  }

  for(int i = 0; i < size; i++){
    //printf("%d\n", arr[i]);
    int temp = isTruncPrime(arr[i]);
    //printf("%d\n", arr[i]);   
     if(temp == 1){
       printf("yes\n");
     }else{
       printf("no\n");
     }
  }
  
  free(arr);
  
  fclose(fp);
  return 0;
}

int isTruncPrime(int temp){
  
  for(int i = temp; i != 0; i = i / 10){
    //printf("Test Trunc Prime: %d\n", i);
    if(isPrime(i) != 1){
      return 0;
    }
  }
  return 1;

}

int isPrime(int num){
  if(num == 0 || num == 1){
    return 0;
  }
  if(num == 2 || num == 3){
    return 1;
  }

  if(num % 2 == 0 ||(num % 10 == 5 && num > 5)){
    return 0;
  }else if(multipleOfThree(num) == 1){
    return 0;
  }else{
    return 1;
  }

}

int multipleOfThree(int num){
  int total = 0;
  for(int i = num; i != 0; i = i/10){
    total += i%10;
  }
  if(total % 3 == 0){
    return 1;
  }
  return 0;

}
