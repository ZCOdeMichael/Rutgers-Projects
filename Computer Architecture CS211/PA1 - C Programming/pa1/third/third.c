#include <stdio.h>
#include <stdlib.h>

struct node{
  int data;
  struct node* next;

};
  
struct node* arr[1000];

int search(int);
int insert(int);

int search(int target){
  int index;
  
  if(target < 0){
    index = (-1*target)%1000;
  }else{
    index = target%1000;
  }

  if(arr[index] == NULL){
    return 0;
  }
  for(struct node* ptr = arr[index]; ptr != NULL; ptr = ptr->next){
    if(ptr->data == target){
      return 1;
    }
    
  }
  return 0;
}

int insert(int target){
  int index;
  if(target < 0){
    index = (-1*target)%1000;
  }else{
    index = target%1000;
  }
 
  
    
  if(search(target) == 1){  
    return 0;
    
  }else{
      
    if(arr[index] == NULL){
	
      arr[index] = malloc(sizeof(struct node));
      arr[index]->data = target;
      arr[index]->next = NULL;
      return 1;
    }else{
      struct node* temp = malloc(sizeof(struct node));
      temp->data = target;
      temp->next = arr[index];
      
      arr[index] = temp;
      return 1;
    }
  }

}


int main(int argc, char** argv){
  

  FILE* fp = fopen(argv[1], "r");
  if(fp == NULL){
    exit(0);
  }
  char Cchar;
  int Cnum;

  if(fscanf(fp, "%c\t%d\n", &Cchar, &Cnum) == EOF){
    exit(0);
  }
  
  for(int i = 0; i < 1000; i++){
    arr[i] = malloc(sizeof(struct node));
    arr[i] = NULL;
  }
  
  if(Cchar == 'i'){
    if(insert(Cnum) == 0){
      printf("duplicate\n");
    }else{
      printf("inserted\n");
    }
  }else{
    if(search(Cnum) == 0){
      printf("absent\n");
    }else{
      printf("present\n");
    }

  }
  
  while(fscanf(fp, "%c\t%d\n", &Cchar, &Cnum) > 0){
    if(Cchar == 'i'){
      if(insert(Cnum) == 0){
	printf("duplicate\n");
      }else{
	printf("inserted\n");
      }
    }else{
      if(search(Cnum) == 0){
	printf("absent\n");
      }else{
	printf("present\n");
      }

    }

  }

  /*
  if(arr[0] == NULL){
    arr[0] = malloc(sizeof(struct node));
    arr[0]->data = 0;
    arr[0] = NULL;
  }
    
  struct node* head = arr[0];
  
  for(int i = 0; i < 5; i++){
    struct node* temp = malloc(sizeof(struct node));
    temp->data = i*2;
    temp->next = head;
    head = temp;
  }
  
  for(; head != NULL; head = head->next){
    printf("%d\n", head->data);
  }
  */
  
  for(int i = 0; i < 1000; i++){
    free(arr[i]);
  }
  fclose(fp);
  return 0;

}
