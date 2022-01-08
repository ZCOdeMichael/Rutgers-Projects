#include "first.h"

struct circuit* line;

struct INPUTVAR* input_Var;
struct OUTPUTVAR* output_Var;

int* curr_GrayComb;

int main(int argc, char** argv){
    
  FILE* fp = fopen(argv[1], "r");
  if(fp == NULL){
    return 0;
  }
  fillLine(fp);
  
  int* grayCode = gray_Code(input_Var->num_Inputs);
  curr_GrayComb = malloc(sizeof(int) * input_Var->num_Inputs);
 
  for(int i = 0; i < pow(2, input_Var->num_Inputs); i++){
    for(int j = 0; j < input_Var->num_Inputs; j++){
      curr_GrayComb[j] = grayCode[(i*input_Var->num_Inputs)+j];
    }
    
    for(int j = 0; j < input_Var->num_Inputs; j++){
      printf("%d ", curr_GrayComb[j]);
    }
    out_Result();
    for(struct circuit* ptr = line; ptr != NULL; ptr = ptr->next){
      free(ptr->result);
    }

    
    if((i+1) == pow(2, input_Var->num_Inputs)){
      break;
    }

    printf("\n");
  }
  

  free(curr_GrayComb);
  for(int i = 0; i < input_Var->num_Inputs; i++){
    free(input_Var->inputs[i]);
  }
  for(int i = 0; i < output_Var->num_Outputs; i++){
    free(output_Var->outputs[i]);
  }
  free(input_Var->inputs);
  free(output_Var->outputs);
  free(input_Var);
  free(output_Var);
  struct circuit* ptr = line;
  while(ptr != NULL){
    struct circuit* temp = ptr->next;
    free(ptr->type);
    if(ptr->result != NULL){
      //free(ptr->result);
    }
    if(ptr->para != NULL){
      for(int i = 0; i < ptr->num_Para; i++){
	free(ptr->para[i]);
      }
      free(ptr->para);
    }
    for(int i = 0; i < ptr->num_Input; i++){
      free(ptr->inputs[i]);
    }
    free(ptr->inputs);
    for(int i = 0; i < ptr->num_Output; i++){
      free(ptr->outputs[i]);
    }
    free(ptr->outputs);
    free(ptr);
    ptr = temp;
  }
  free(grayCode);
  fclose(fp);
  //out_Result();
  //debug();
  return 0;
}

void out_Result(){
  for(struct circuit* ptr = line; ptr != NULL; ptr = ptr->next){
    if(strcmp(ptr->type, "NOT") == 0){
      ptr->result = malloc(sizeof(int));
      ptr->result[0] = NOT(getVar(ptr->inputs[0]));
    }else if(strcmp(ptr->type, "AND") == 0){
      ptr->result = malloc(sizeof(int));
      ptr->result[0] = AND(getVar(ptr->inputs[0]), getVar(ptr->inputs[1]));
    }else if(strcmp(ptr->type, "OR") == 0){
      ptr->result = malloc(sizeof(int));
      ptr->result[0] = OR(getVar(ptr->inputs[0]), getVar(ptr->inputs[1]));
    }else if(strcmp(ptr->type, "NAND") == 0){
      //printf("HELLO");
      ptr->result = malloc(sizeof(int));
      ptr->result[0] = NAND(getVar(ptr->inputs[0]), getVar(ptr->inputs[1]));
    }else if(strcmp(ptr->type, "NOR") == 0){
      //printf("HELLO");
      ptr->result = malloc(sizeof(int));
      ptr->result[0] = NOR(getVar(ptr->inputs[0]), getVar(ptr->inputs[1]));
    }else if(strcmp(ptr->type, "XOR") == 0){
      ptr->result = malloc(sizeof(int));
      ptr->result[0] = XOR(getVar(ptr->inputs[0]), getVar(ptr->inputs[1]));
    }else if(strcmp(ptr->type, "XNOR") == 0){
      ptr->result = malloc(sizeof(int));
      ptr->result[0] = XNOR(getVar(ptr->inputs[0]), getVar(ptr->inputs[1]));
    }else if(strcmp(ptr->type, "DECODER") == 0){
      int* temp = malloc(sizeof(int) * ptr->num_Input);
      for(int i = 0; i < ptr->num_Input; i++){
	temp[i] = getVar(ptr->inputs[i]);
      }
      ptr->result = DECODER(temp, ptr->num_Input);
      free(temp);
    }else{
      int* temp_Para = malloc(sizeof(int) * ptr->num_Para);
      int* temp_Input = malloc(sizeof(int) * ptr->num_Input);
      //printf("\n");
      for(int i = 0; i < ptr->num_Para; i++){
	temp_Para[i] = getVar(ptr->para[i]);
	//printf("[%d]", temp_Para[i]);
      }
      //printf("\n");
      //printf("!!%d!!", ptr->num_Input);
      for(int i = 0; i < ptr->num_Input; i++){
	temp_Input[i] = getVar(ptr->inputs[i]);
	//printf("[%d]", temp_Input[i]);
      }
      ptr->result = malloc(sizeof(int));
      ptr->result[0] = MULTIPLEXER(temp_Para, temp_Input, ptr->num_Input);
      free(temp_Para);
      free(temp_Input);
    }
    
  }
  /* 
  printf("[%d] ", getVar("t1")); 
  printf("[%d] ", getVar("t2"));
  printf("[%d] ", getVar("t3"));
  printf("[%d] ", getVar("t4"));
  printf("[%d] ", getVar("t5"));
  printf("[%d] ", getVar("t6"));
  */
  for(int i = 0; i < output_Var->num_Outputs; i++){
    printf("%d ", getVar(output_Var->outputs[i]));
  }
}

int getVar(char* curr){
  //printf("[%d]", strcmp(curr, "1"));
  
  if(strcmp(curr, "1") == 0){
    return 1;
  }
  if(strcmp(curr, "0") == 0){
    return 0;
  }
  
  for(int i = 0; i < input_Var->num_Inputs; i++){
    if(strcmp(curr, input_Var->inputs[i]) == 0){
      return curr_GrayComb[i];
    }
  }
  
  for(struct circuit* ptr = line; ptr != NULL; ptr = ptr->next){
    for(int i = 0; i < ptr->num_Output; i++){
      if(strcmp(curr, ptr->outputs[i]) == 0){
	  return ptr->result[i];
      }
    }
  }
  
  return -1;
}

void debug(){
  int temp = 0;
  
  printf("INPUTVAR: ");
  for(int i = 0; i < input_Var->num_Inputs; i++){
    printf("%s ", input_Var->inputs[i]);
  }
  printf("\n");
  printf("OUTPUTVAR: ");
  for(int i = 0; i < output_Var->num_Outputs; i++){
    printf("%s ", output_Var->outputs[i]);
  }
  printf("\n");
  for(struct circuit* ptr = line; ptr != NULL; ptr = ptr->next){
    temp++;
    printf("%s\n", ptr->type);
    for(int i = 0; i < ptr->num_Input; i++){
      printf("[%s]", ptr->inputs[i]);
    }
    printf(" [Size]: %d", ptr->num_Input);
    printf("\n");
    for(int i = 0; i < ptr->num_Output; i++){
      printf("[%s]", ptr->outputs[i]);
    }
    printf("\n");
    if(ptr->para != NULL){
      for(int i = 0; i < ptr->num_Para; i++){
        printf("[%s]", ptr->para[i]);
      }
      printf("\n");
    }

  }
  printf("%d", temp);

}

void fillLine(FILE* fp){
  char* curr = malloc(sizeof(char) * 30);
  char ptr = '\0';
  int counter = 0;
  while(fscanf(fp, "%c", &ptr) > 0){
    if(ptr == ' ' || ptr == '\n'){
      //printf("%s\n", curr);
      if(strcmp(curr, "INPUTVAR") == 0){
	//Dedicated for the INPUTVAR array
	input_Var = malloc(sizeof(struct INPUTVAR));
	char* num_Temp = malloc(sizeof(char) * 20);
	fscanf(fp, "%c", &ptr);

	for(int tempC = 0; ptr != ' '; tempC++, fscanf(fp, "%c", &ptr)){
	  num_Temp[tempC] = ptr;
	 
	}
	
	input_Var->inputs = malloc(sizeof(char*) * atoi(num_Temp));
	input_Var->num_Inputs = atoi(num_Temp);
	
	for(int i = 0; i < atoi(num_Temp); i++){
	  input_Var->inputs[i] = malloc(sizeof(char) * 20);
	  fscanf(fp, "%c", &ptr);
	  for(int tempC = 0; ptr != ' '; fscanf(fp, "%c", &ptr), tempC++){
	    if(ptr == '\n'){
	      break;
	    }
	   
	    input_Var->inputs[i][tempC] = ptr;
	  }
	  
	  //input_Var->inputs[i] = currVar;
	  //free(currVar);
	}
	free(num_Temp);
	fscanf(fp, "%c", &ptr);
      }else if(strcmp(curr, "OUTPUTVAR") == 0){
	//Dedicated for OUTPUTVAR array
	output_Var = malloc(sizeof(struct OUTPUTVAR));
	char* num_Temp = malloc(sizeof(char) * 20);
	fscanf(fp, "%c", &ptr);
	
        for(int tempC = 0; ptr != ' '; tempC++, fscanf(fp, "%c", &ptr)){
          num_Temp[tempC] = ptr;

        }
	
	output_Var->outputs = malloc(sizeof(char*) * atoi(num_Temp));
	output_Var->num_Outputs = atoi(num_Temp);

        for(int i = 0; i < atoi(num_Temp); i++){
          char* currVar = malloc(sizeof(char) * 20);
          fscanf(fp, "%c", &ptr);
          for(int tempC = 0; ptr != ' '; fscanf(fp, "%c", &ptr), tempC++){
            if(ptr == '\n'){
              break;
            }

            currVar[tempC] = ptr;
          }

          output_Var->outputs[i] = currVar;
          //free(currVar);
        }
        free(num_Temp);
        fscanf(fp, "%c", &ptr);
       
     
      }else{
	
	struct circuit* curr_Gate = malloc(sizeof(struct circuit));
	curr_Gate->next = NULL;
	curr_Gate->para = NULL;
	curr_Gate->inputs = NULL;
	curr_Gate->outputs = NULL;
	curr_Gate->type =  malloc(sizeof(char)*30);
	for(int i = 0; i < 30; i++){
	  curr_Gate->type[i] = curr[i];
	}
			       
	if(strcmp(curr, "NOT") == 0){
	  //NOT gate
	  
	  curr_Gate->num_Input = 1;
	  curr_Gate->num_Output = 1;
	  char* input;
	  char* output;
	  for(int i = 0; i < 2; i++){
	    char* currVar = malloc(sizeof(char) * 20);
	    fscanf(fp, "%c", &ptr);
	    for(int tempC = 0; ptr != ' '; fscanf(fp, "%c", &ptr), tempC++){
	      if(ptr == '\n'){
		break;
	      }
	      currVar[tempC] = ptr;
	    }
	    if(i == 0){
	      input = currVar;
	    }else{
	      output = currVar;
	    }
	    //free(currVar);
	  }
	  curr_Gate->inputs = malloc(sizeof(char*));
	  curr_Gate->outputs = malloc(sizeof(char*));
	  curr_Gate->inputs[0] = input;
	  curr_Gate->outputs[0] = output;
	  
	  //fscanf(fp, "%c", &ptr);
	}else if(strcmp(curr, "AND") == 0 || strcmp(curr, "OR") == 0|| strcmp(curr, "NAND") == 0|| strcmp(curr, "NOR") == 0|| strcmp(curr, "XOR") == 0|| strcmp(curr, "XNOR") == 0){
	  //Not DECODER or MULTIPLEXER or NOT gates
	  curr_Gate->num_Input = 2;
	  curr_Gate->num_Output = 1;
	  char* input_1;
	  char* input_2;
	  char* output;
	  for(int i = 0; i < 3; i++){
	    char* currVar = malloc(sizeof(char) * 20);
	    fscanf(fp, "%c", &ptr);
	    for(int tempC = 0; ptr != ' '; fscanf(fp, "%c", &ptr), tempC++){
	      if(ptr == '\n'){
		break;
	      }
	      //printf("HELLO"); // BROKEN HERE
	      currVar[tempC] = ptr;
	    }
	    //printf("[%s] ", currVar);
	    //printf("\n");
	    if(i == 0){
	      input_1 = currVar;
	    }else if(i == 1){
	      input_2 = currVar;
	    }else{
	      output = currVar;
	    }
	    //free(currVar);
	  }
	  curr_Gate->inputs = malloc(sizeof(char*)*2);
	  curr_Gate->outputs = malloc(sizeof(char*));
	  curr_Gate->inputs[0] = input_1;
	  curr_Gate->inputs[1] = input_2;
	  curr_Gate->outputs[0] = output;
	}else if(strcmp(curr, "DECODER") == 0){
	  //DECODER gate TODO FIX!!!!!!!!!!!!!!!!!!!!
	  char* num_Temp = malloc(sizeof(char) * 20);
	  fscanf(fp, "%c", &ptr);
	  for(int tempC = 0; ptr != ' '; tempC++, fscanf(fp, "%c", &ptr)){
	    num_Temp[tempC] = ptr;
	  }
	  int num = atoi(num_Temp);
	  curr_Gate->num_Input = num;
	  curr_Gate->num_Output = pow(2, num);
	  curr_Gate->inputs = malloc(sizeof(char*) * num);
	  curr_Gate->outputs = malloc(sizeof(char*) * pow(2, num));
	  for(int i = 0; i < (num + pow(2, num)); i++){
	    char* currVar = malloc(sizeof(char) * 20);
	    fscanf(fp, "%c", &ptr);
	    for(int tempC = 0; ptr != ' '; fscanf(fp, "%c", &ptr), tempC++){
	      if(ptr == '\n'){
		break;
	      }
	      currVar[tempC] = ptr;
	    }
	    
	    if(i < num){
	      curr_Gate->inputs[i] = currVar;
	    }else{
	      curr_Gate->outputs[i-num] = currVar;
	    }
	    //free(currVar);
	  }
	  free(num_Temp);
	  //fscanf(fp, "%c", &ptr);
	}else{
	  //MULTIPLEXER gate
	  //printf("%s\n", curr);
	  char* num_Temp = malloc(sizeof(char) * 20);
	  fscanf(fp, "%c", &ptr);
	  for(int tempC = 0; ptr != ' '; tempC++, fscanf(fp, "%c", &ptr)){
	    num_Temp[tempC] = ptr;
	  }
	  //printf("%s", num_Temp);
	  int num = atoi(num_Temp);
	  curr_Gate->num_Para = num;
	  curr_Gate->num_Output = 1;
	  curr_Gate->num_Input = (int)log2(num);
	  curr_Gate->para = malloc(sizeof(char*) * num);
	  curr_Gate->outputs = malloc(sizeof(char*));
	  curr_Gate->inputs = malloc(sizeof(char*) * (int)log2(num));
	  for(int i = 0; i < (num + log2(num) + 1); i++){
	    char* currVar = malloc(sizeof(char) * 20);
	    fscanf(fp, "%c", &ptr);
	    for(int tempC = 0; ptr != ' '; fscanf(fp, "%c", &ptr), tempC++){
	      if(ptr == '\n'){
		break;
	      }
	      currVar[tempC] = ptr;
	    }
	    //printf("[|%s|]", currVar); 
	    if(i < num){
	      //printf(" %d\n", i);
	      curr_Gate->para[i] = currVar;
	    }else if(i >= num && i < (num+log2(num))){
	      //printf(" %d\n", (i-num));
	      curr_Gate->inputs[i-num] = currVar;
	    }else{
	      //printf(" %d\n", 0);
	      curr_Gate->outputs[0] = currVar;
	    }
	    //free(currVar);
	  }
	  free(num_Temp);
	  //fscanf(fp, "%c", &ptr);
	}
	
	if(line == NULL){
	  line = curr_Gate;
	}else{
	  struct circuit* ptr = line;
	  while(ptr->next != NULL){
	    ptr = ptr->next;
	  }
	  ptr->next = curr_Gate;
	}
	fscanf(fp, "%c", &ptr);
	
      }

      for(int i = 0; i < 30; i++){
	curr[i] = '\0';
      }
      counter = 0;
       
    }
    
    curr[counter] = ptr;
    counter++;
  }

  free(curr);
}

///////////////////////////////////////////////////////////////////////////////////////
/*
Log Gates
*/
///////////////////////////////////////////////////////////////////////////////////////

int NOT(int input){
  return !input;
}

int AND(int input1, int input2){
  return (input1 & input2);
}

int OR(int input1, int input2){
  return (input1 | input2);
}

int NAND(int input1, int input2){
  return !AND(input1, input2);
}

int NOR(int input1, int input2){
  return !OR(input1, input2);
}

int XOR(int input1, int input2){
  return OR((AND(input1, !(input2))), (AND(!(input1), input2)));
}

int XNOR(int input1, int input2){
  return OR(AND(!input1, !input2), AND(input1, input2));
}

int* DECODER(int* input, int n){
  int* grayCode = gray_Code(n);
  int* result = malloc(sizeof(int) * (int)pow(2, n));
  for(int i = 0; i < (int)pow(2, n); i++){
    result[i] = 0;
  }

  int counter = 0;
  for(int i = 0; i < (int)pow(2, n); i++){
    int isValid = 1;
    for(int j = 0; j < n; j++){
      if(input[j] != grayCode[j + counter]){
	isValid = 0;
      }
    }
        
    if(isValid == 1){
      result[counter/n] = 1;
    }
    counter += n;
  }

  free(grayCode);
  return result;
}

int MULTIPLEXER(int* para, int* input, int n){
  int* grayCode = gray_Code(n);
  int result = 0;

  int counter = 0;
  for(int i = 0; i < (int)pow(2, n); i++){
    int isValid = 1;
    for(int j = 0; j < n; j++){
      if(input[j] != grayCode[j + counter]){
	isValid = 0;
      }
    }

    if(isValid == 1){
      result = para[i];
    }
    counter += n;
  }


  free(grayCode);
  return result;
}

///////////////////////////////////////////////////////////////////////////////////////
/*
Log Gates
*/
///////////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////////////
/*
Gray Code Generator
*/
///////////////////////////////////////////////////////////////////////////////////////

int* gray_Code(int n){
  struct block** temp = init(n);
  int* result = malloc(sizeof(int) * (n*(int)pow(2, n)));
  temp[0]->arr[0] = 0;
  temp[0]->valid = 1;
  temp[1]->arr[0] = 1;
  temp[1]->valid = 1;
    
  for(int i = 1; i < n; i++){
    mirror(temp, i);
  }

  int counter = 0;
  for(int i = 0; i < (int)pow(2, n); i++){
    for(int j = 0; j < n; j++){
      result[counter] = temp[i]->arr[j];
      counter++;
    }
  }
  for(int i = 0; i < (int)pow(2, n); i++){
    free(temp[i]->arr);
    free(temp[i]);
  }
  free(temp);

  return result;
}

void mirror(struct block** result, int n){
  int height = (int)pow(2, n);
   
  for(int i = height-1; i >= 0; i--){
        
    for(int j = 0; j < n; j++){
      result[height]->arr[j] = result[i]->arr[j];
    }
    height++;
  }
  height = (int)pow(2, n);

  for(int i = 0; i < height; i++){
    for(int j = n; j > 0; j--){
      result[i]->arr[j] = result[i]->arr[j-1];
    }
    result[i]->arr[0] = 0;
  }
    
  for(int i = height; i < height*2; i++){
    for(int j = n; j > 0; j--){
      result[i]->arr[j] = result[i]->arr[j-1];
    }
    result[i]->arr[0] = 1;
  }
    
}

struct block** init(int n){

  struct block** result = malloc(sizeof(struct block*) * (int)pow(2, n));
  for(int i = 0; i < (int)pow(2, n); i++){
    result[i] = malloc(sizeof(struct block));
    result[i]->valid = 0;
    result[i]->arr = malloc(sizeof(int) * n);
    for(int j = 0; j < n; j++){
      result[i]->arr[j] = -1;
    }
  }
  return result;
}

///////////////////////////////////////////////////////////////////////////////////////
/*
Gray Code Generator
*/
///////////////////////////////////////////////////////////////////////////////////////



