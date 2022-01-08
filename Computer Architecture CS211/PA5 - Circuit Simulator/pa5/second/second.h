#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

///////////////////////////////////////////////////////////////////////////////////////
/*
Gray Code Generator
*/
///////////////////////////////////////////////////////////////////////////////////////

int* gray_Code(int);

struct block** init(int);

void mirror(struct block**, int);

struct block{
    int* arr;
    int valid;
};

///////////////////////////////////////////////////////////////////////////////////////
/*
Logic Gate
*/
///////////////////////////////////////////////////////////////////////////////////////

/*
NOT AND OR NAND NOR XOR XNOR DECODER MULTIPLEXER
*/
int NOT(int); //Input: 1 || Output: 1 (Size)

int AND(int, int);//Input: 2 || Output: 1 (Size)

int OR(int, int);//Input: 2 || Output: 1 (Size)

int NAND(int, int);//Input: 2 || Output: 1 (Size)

int NOR(int, int);//Input: 2 || Output: 1 (Size)

int XOR(int, int);//Input: 2 || Output: 1 (Size)

int XNOR(int, int);//Input: 2 || Output: 1 (Size)

//Input: The input to Decoder and number of inputs 
int* DECODER(int*, int);//Input: n, 1 || Output: 2^n (Size)

//Input: The input to Multi, input variables, and number of input variables
int MULTIPLEXER(int*, int*, int);//Input: 2^n, n, 1 || Output: 1 (Size)
/*
Logic Gates
*/


struct circuit{
    char* type;

    int* result;
    
    char** para;
    char** inputs;
    char** outputs;

    int num_Para;
    int num_Input;
    int num_Output;
    struct circuit* next;

};

struct INPUTVAR{
   char** inputs;
   int num_Inputs;
};

struct OUTPUTVAR{ 
   char** outputs;
   int num_Outputs;
};


void fillLine(FILE*);

void debug();

void out_Result();

int getVar(char*);


struct list{
   char* data;
   struct list* next;
};

void add_Sort(struct circuit*, struct circuit*);

struct circuit* getLast();

int check_List(char*, struct list*);

void transfer();

void add_Main(char*);
void add_Sec(char*);