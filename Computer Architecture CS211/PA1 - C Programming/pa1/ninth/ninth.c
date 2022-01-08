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
void deletion(struct Node*);
struct Node* allocate_Node(int);
struct Node* find_Delete(struct Node*, int);
struct Node* find_Prev(struct Node*, struct Node*);
struct Node* find_Min(struct Node*);
struct Node* head = NULL;

struct Node* find_Min(struct Node* temp){
  struct Node* temp1 = temp->right;
  while(temp1->left != NULL){
    temp1 = temp1->left;
  }
  
  return temp1;
  
  
}

struct Node* find_Delete(struct Node* temp, int target){
  struct Node* result = temp;
  if(temp->data == target){
    return result;
  }else if(temp->data < target){
    result = find_Delete(temp->right, target);
  }else{
    result = find_Delete(temp->left, target);
  }
  return result;
  
}

//Assuming target is not Head
struct Node* find_Prev(struct Node* temp, struct Node* target){
  struct Node* result = temp;
  if(temp->data < target->data){
    if(temp->right == target){
      return temp;
    }else{
      result = find_Prev(temp->right, target);
    }
  }else{
    if(temp->left == target){
      return temp;
    }else{
      result = find_Prev(temp->left, target);
    }
  }
  return result;
}

void deletion(struct Node* temp){
  if(temp->left == NULL && temp->right == NULL){
    //No Child - Leaf Node
    if(temp == head){
      struct Node* temp1 = temp;
      head = NULL;
      free(temp1);
    }else{
      struct Node* temp1 = find_Prev(head, temp);
      if(temp1->right == temp){
	temp1->right = NULL;
	free(temp);
      }else{
	temp1->left = NULL;
	free(temp);
      }
    }
    
  }else if((temp->left == NULL && temp->right != NULL) || (temp->left != NULL && temp->right == NULL)){
    //One Child
    if(temp->left == NULL){
      if(temp == head){
	struct Node* temp1 = temp;
	head = temp->right;
	free(temp1);

      }else{
	struct Node* prev = find_Prev(head, temp);
	struct Node* temp1 = temp;
	if(prev->right == temp){
	  prev->right = temp->right;
	  free(temp1);
	  
	}else{
	  prev->left = temp->right;
	  free(temp1);
	  
	}
      }
    }else{
      if(temp == head){
	struct Node* temp1 = temp;
	head = temp->left;
	free(temp1);
      
      }else{
	struct Node* prev = find_Prev(head, temp);
	struct Node* temp1 = temp;
	if(prev->right == temp){
	  prev->right = temp->left;
	  free(temp1);
	}else{
	  prev->left = temp->left;
	  free(temp1);
	}
      }
    }
  }else{
    //Two Child
    if(temp == head){
      /*
	Delete Head
	Min has no left
	

       */
      struct Node* temp1 = temp;
      struct Node* min = find_Min(temp);
      struct Node* prev = find_Prev(head, min);
      if(prev->left == min){
	prev->left = min->right;
	min->left = temp1->left;
	min->right = temp1->right;
	head = min;
	free(temp1);
      }else{
	prev->right = min->right;
        min->left = temp1->left;
        min->right = temp1->right;
	head = min;
	free(temp1);
      }
      
      
    }else{
      /*
	Delete Node
	Min has no left

	
       */
      struct Node* target = temp;
      struct Node* min = find_Min(temp);
      struct Node* prev_Min = find_Prev(head, min);
      struct Node* prev_Target = find_Prev(head, target);
      
      if(prev_Min->left == min){
	if(prev_Target->right == target){
	  prev_Target->right = min;
	}else{
	  prev_Target->left = min;
	}
	min->left = target->left;
	
	if(prev_Min->right == min){
	  prev_Min->right = NULL;
	}else{
	  prev_Min->left = NULL;
	}
	
        free(target);
      }else{
	if(prev_Target->right == target){
	  prev_Target->right = min;
	}else{
	  prev_Target->left = min;
	}

	min->left = target->left;

	if(prev_Min->right == min){
	  prev_Min->right = NULL;
	}else{
	  prev_Min->left = NULL;
	}

        free(target);
      }


    }
  }
  
}

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
  }else if(Cchar == 'd'){
    printf("fail\n");
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
    }else if(Cchar == 'd' && head == NULL){
      printf("fail\n");
    }else{
      
      if(Cchar == 'i'){
	if(is_Dupe(head, Cnum) == 1){
	  printf("duplicate\n");
	}else{
	  printf("inserted %d\n", insert(head, Cnum));
	}
      }else if(Cchar == 's'){
	if(is_Dupe(head, Cnum) == 1){
	  printf("present %d\n", search_Num(head, Cnum));
	}else{
	  printf("absent\n");
	}
      }else{
	if(is_Dupe(head, Cnum) == 1){
	  struct Node* temp = find_Delete(head, Cnum);
	  //printf("%p\n", temp);
	  deletion(temp);
	  printf("success\n");
	}else{
	  printf("fail\n");
	}
	
      }
     
    }
  }
  clear_mem(head);
  fclose(fp);
  return 0;
}
