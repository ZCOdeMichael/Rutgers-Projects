#include <stdio.h>
#include <stdlib.h>

struct Node{
  int data;
  struct Node* left;
  struct Node* right;

};

void clear_mem(struct Node*);
int search_Num(struct Node*, int);
int insert(struct Node*, int);
int is_Dupe(struct Node*, int);
struct Node* allocate_Node(int);
struct Node* head = NULL;



void clear_mem(struct Node* temp){
  
  if(temp == NULL){
    return;
  }  
  
  struct Node* temp2 = temp;
  
  clear_mem(temp->left);
  clear_mem(temp->right);
  free(temp2);
}

struct Node* allocate_Node(int target){
  struct Node* temp = malloc(sizeof(struct Node));
  temp->data = target;
  temp->left = NULL;
  temp->right = NULL;
  return temp;
}

int is_Dupe(struct Node* temp, int target){
  
  int result = 0;

  if(temp == NULL){
    return 0;
  }
  if(temp->data == target){
    return 1;
  }else if(temp->data < target){
    result = is_Dupe(temp->right, target);
  }else{
    result = is_Dupe(temp->left, target);

  }

  return result;
}

int insert(struct Node* temp, int target){
  if(temp->data == target){
    return 0;
  }
  if(temp->data > target){
    if(temp->left == NULL){
      struct Node* temp1 = allocate_Node(target);
      temp->left = temp1;
      return 2;
    }else{
      return insert(temp->left, target) + 1;
    }
  }else{
    if(temp->right == NULL){
      struct Node* temp2 = allocate_Node(target);
      temp->right = temp2;
      return 2;
    }else{
      return insert(temp->right, target) + 1;
    }
  }
}

int search_Num(struct Node* temp, int target){
  if(temp == NULL){
    return 0;
  }
  if(temp->data == target){
    return 1;
  }else if(temp->data > target){
    return search_Num(temp->left, target) + 1;
  }else{
    return search_Num(temp->right, target) + 1;
  }
}


int main(int argc, char** argv){
  
  FILE* fp = fopen(argv[1],"r");
  if(fp == NULL){
    printf("error");
    exit(0);
  }
  char Cchar;
  int Cnum;
  if(fscanf(fp, "%c\t%d\n", &Cchar, &Cnum) == EOF){
    exit(0);
  }
  
  if(Cchar == 's'){
    printf("absent\n");
  }else{
    struct Node* temp = allocate_Node(Cnum);
    head = temp;
    printf("inserted 1\n");
  }
  
  
  
  while(fscanf(fp, "%c\t%d\n", &Cchar, &Cnum) > 0){
    //printf("%c\t%d\n", Cchar, Cnum);
    if(Cchar == 's' && head == NULL){
      printf("absent\n");
    }else if(Cchar == 'i' && head == NULL){
      printf("inserted 1\n");
      struct Node* temp = allocate_Node(Cnum);
      head = temp;
    }else{
      
      if(Cchar == 'i'){
	if(is_Dupe(head, Cnum) == 1){
	  printf("duplicate\n");
	}else{
	  printf("inserted %d\n", insert(head, Cnum));
	}
      }else{
	if(is_Dupe(head, Cnum) == 1){
	  printf("present %d\n", search_Num(head, Cnum));
	}else{
	  printf("absent\n");
	}
      }
     
    }
  }
  clear_mem(head);
  fclose(fp);
  return 0;
}
