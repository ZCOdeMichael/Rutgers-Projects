#include <stdio.h>
#include <stdlib.h>


typedef struct Nodes {
  int data;
  struct Nodes* next;
  
}Node;


void printLL(Node* head);
int hasElement(Node* head, int target);

void printLL(Node* head){
  int total = 0;
  for(Node* ptr = head; ptr != NULL; ptr = ptr->next){
    total ++;
  }
  printf("%d\n", total);
  for(Node* ptr = head; ptr != NULL; ptr = ptr->next){
    printf("%d\t", ptr->data);
    
  }
  
  Node* ptr = head;
  
  while(ptr != NULL){
    Node* temp = ptr;
    ptr = ptr->next;
    free(temp);
  }
 


}

int hasElement(Node* head, int target){
  for(Node* ptr = head; ptr != NULL; ptr = ptr->next){
    if(ptr->data == target){
      return 1;
    }
  }
  return 0;

}

int main(int argc, char** argv){


  FILE* fp = fopen(argv[1], "r");
  if(fp == NULL){
    printf("error");
    exit(0);
  }

  Node* head = NULL;
  
  int Cnum;
  char Cchar;

  if(fscanf(fp, "%c\t%d\n", &Cchar, &Cnum) == EOF){ 
    /* Checks if the file is empty or not */

    printf("%d\n", 0);
    exit(0);

  }else{ 
    /* The file is not empty if the first item is i than it'll set it to the head
       but if first item is d than do nothing (since there are not elements in the LL
    */

    //printf("%c\n", Cchar);
    //printf("%d\n", Cnum);

    if(Cchar == 'i'){
      head = malloc(sizeof(Node));
      head->data = Cnum;
      head->next = NULL;
    }
  }


  while(fscanf(fp, "%c\t%d\n", &Cchar, &Cnum) > 0){
    /* While loop checks if there is a next line and executes
       Check if head is NULL, item is i or d
     */
    
    
    //printf("%c\t%d\n", Cchar, Cnum);
    
    if(head == NULL){
      /* If the head is NULL */
      
      if(Cchar == 'i'){
	/* If current item is i */
	
	head = malloc(sizeof(Node));
	head->data = Cnum;
	head->next = NULL;

      }
	/* If current itme is d than do ABSOLUTELY NOTHING */
	
    }else{
      /* If the head is not NULL */
      
      if(Cchar == 'i'){
	/* If current item is i */
	
	if(!hasElement(head, Cnum)){
	  /* If the number does not exists in the LL */
	  
	  Node* prev = NULL;
	  for(Node* ptr = head; ptr != NULL; ptr = ptr->next){
	    /* Goes through all elements */
	    if(ptr->data > Cnum){
	      /* If the current element should be placed before the ptr */
	      if(prev == NULL){
		/* Checks if the ptr is the head and puts the current element before the head */
		
		Node* temp = malloc(sizeof(Node));
		temp->data = Cnum;
		temp->next = head;
		/* Makes element */
		
		head = temp;
		break;

	      }else{
		/* Checks if the ptr is not the head and puts the current element before the ptr*/
		
		Node* temp = malloc(sizeof(Node));
		temp->data = Cnum;
		temp->next = ptr;
		/* Makes element */
		
		prev->next = temp;
		break;
	      }

	    }else{
	      /* If the current element should be placed after the ptr */
	     
		if(ptr->next == NULL){
		  Node* temp = (Node*)malloc(sizeof(Node));
		  temp->data = Cnum;
		  temp->next = NULL;
		
		// Makes element 
		
		  ptr->next = temp;
		  break;
		}
	    
	    }
	    
	    prev = ptr;
	  }
	}
	
      }else{
	/* If current item is d */
	
	if(hasElement(head, Cnum)){
	  /* If the LL has the element */
	  
	  Node* prev = NULL;
	  //printf("%d", head->data);
	  for(Node* ptr = head; ptr != NULL; ptr = ptr->next){
	    
	    if(ptr->data == Cnum){
	      /* Finds the element */
	      if(prev == NULL){
		/* If the ptr is the head */
		
		if(ptr->next == NULL){
		  /* If the head is the only element */
		  Node* temp = head;
		  free(temp);
		  head = NULL;
		  break;
		}else{
		  /* If there is an element after the head */
		  
		  Node* temp = head;
		  head = head->next;
		  free(temp);
		  break;
		  
		}		
	      }else{
		/* If the ptr is not the head */
		
		Node* temp = ptr;
		prev->next = ptr->next;
		free(temp);
		break;
	      }
	      
	    }
	  
	    prev = ptr;
	  }
  
	}

      }
    }
    //printLL(head);
  }
 
  if(head == NULL){
    printf("%d\n", 0);
    exit(0);
  }
  fclose(fp);
  printLL(head);
  return 0;
}
