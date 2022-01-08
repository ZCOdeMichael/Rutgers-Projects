#include <stdio.h>
#include <stdlib.h>

int attr, ex, numTest;

double** trainingData;
double** testData;

double** weights;
double** prices;

double** multiMatrix(int, int, int, double**, double**);
double** invMatrix(int, int, double**);
double** transMatrix(int, int, double**);

double calc_num(double**, double**, int, int, int);
void initPrime(int, int, int);

/*
  TODO fix multiMatrix Finished??!?!?
  TODO implement invMatrix, transMatrix
  TODO calculate predicted prices using testData
*/

double** transMatrix(int row, int col, double** matrix){
  double** result = malloc(sizeof(double*) * col);
  for(int i = 0; i < col; i++){
    result[i] = malloc(sizeof(double) * row);
  }
  
  for(int i = 0; i < row; i++){
    for(int j = 0; j < col; j++){
      result[j][i] = matrix[i][j];
    }
  }
  
  return result;
}

double** invMatrix(int row, int col, double** matrix){
  double** temp = malloc(sizeof(double*) * row);
  double** result = malloc(sizeof(double*) * row);
  for(int i = 0; i < row; i++){
    result[i] = malloc(sizeof(double) * col);
    temp[i] = malloc(sizeof(double) * (2*col));
  }
  
  for(int i = 0; i < row; i++){
    for(int j = 0; j < 2*col; j++){
      temp[i][j] = 0;
    }
  }
  
  for(int i = 0; i < row; i++){
    for(int j = 0; j < col; j++){
      temp[i][j] = matrix[i][j];
    }
  }
  for(int i = 0; i < row; i++){
    temp[i][i+col] = 1;
  }


  //DEBUG CODE MUST COMMENT OUT
  /*
  for(int i = 0; i < row; i++){
    for(int j = 0; j < 2*col; j++){
      printf("%lf, ", temp[i][j]);
    }
    printf("\n");
  }
  */
  for(int i = 0; i < row; i++){
    double current = temp[i][i];

    if(current != 1){
      for(int j = 0; j < 2*col; j++){
	temp[i][j] = temp[i][j] / current;
      }
    }
    
    for(int j = i+1; j < row; j++){
      double curRow = temp[j][i];
      for(int k = 0; k < 2*col; k++){
	temp[j][k] = temp[j][k] - temp[i][k]*curRow;
      }
    }
  }
  
  for(int i = row-1; i >= 0; i--){
    
    for(int j = i-1; j >= 0; j--){
      //printf("%d %d\n", j, i);
      double curRow = temp[j][i];
      for(int k = i; k < 2*col; k++){
	//printf("%lf", curRow);
	temp[j][k] = temp[j][k] - temp[i][k]*curRow;
      }
    }
  }

  for(int i = 0; i < row; i++){
    for(int j = col; j < 2*col; j++){
      result[i][j-col] = temp[i][j];
    }
  }

  //printf("\n***********************************************************\n");
  
  //DEBUG CODE MUST COMMENT OUT
  /*
  for(int i = 0; i < row; i++){
    for(int j = 0; j < 2*col; j++){
      printf("%lf, ", temp[i][j]);
    }
    printf("\n\n");
  }
  */
  for(int i = 0; i < row; i++){
    free(temp[i]);
  }
  free(temp);
  return result;
  
}

double** multiMatrix(int row1, int col1, int col2, double** first, double** second){
  double** result = malloc(sizeof(double*) * row1);
  for(int i = 0; i < row1; i++){
    result[i] = malloc(sizeof(double) * col2);
  }
  
  for(int i = 0; i < row1; i++){
    for(int j = 0; j < col2; j++){
      result[i][j] = calc_num(first, second, i, j, col1);
    }
  }
  return result;
}

double calc_num(double** first, double** second, int fRows, int sCols, int cols_fMatrix){
  double sol_val = first[fRows][0] * second[0][sCols];

  for(int i = 1; i < cols_fMatrix; i++){
    sol_val += first[fRows][i] * second[i][sCols];
  }
  return sol_val;

}


void initPrime(int attr, int ex, int numTest){
  // There are attr+1 columns which have ex rows
  trainingData = malloc(sizeof(double*) * (ex));
  for(int i = 0; i < (ex); i++){
    trainingData[i] = malloc(sizeof(double) * (attr+1));
  }
  
  // There is 1 column which has attr+1 rows
  /*
  weights = malloc(sizeof(double*) * (attr+1));
  for(int i = 0; i < attr +1; i++){
    weights[i] = malloc(sizeof(double));
  }
  */
  // There is 1 column which has ex rows;
  prices = malloc(sizeof(double*) * ex);
  for(int i = 0; i < ex; i++){
    prices[i] = malloc(sizeof(double));
  }
  // There are attr columns which have numTest rows
  testData = malloc(sizeof(double*) * numTest);
  for(int i = 0; i < numTest; i++){
    testData[i] = malloc(sizeof(double) * attr);
  }
}

int main(int argc, char** argv){
  FILE* fTrain = fopen(argv[1], "r");
  FILE* fTest = fopen(argv[2], "r");

  if(fTrain == NULL || fTest == NULL){
    return 0;
  }
  fscanf(fTrain, "%d", &attr);
  fscanf(fTrain, "%d", &ex);
  fscanf(fTest, "%d", &numTest); // Test for 0 test examples
  
  initPrime(attr, ex, numTest);
  int count = 0;
  for(int i = 0; i < ex; i++){
    for(int j = 0; j < attr+2; j++){
      if(j == 0){
	trainingData[i][j] = 1;
      }else if(j == attr+1){
	double current;
	fscanf(fTrain, "%lf,", &current);
	prices[count][0] = current;
	count++;
      }else{
	double current;
	fscanf(fTrain, "%lf,", &current);
	trainingData[i][j] = current;
      }
    }
  }
  
  for(int i = 0; i < numTest; i++){
    for(int j = 0; j < attr; j++){
      double current;
      fscanf(fTest, "%lf,", &current);
      testData[i][j] = current;
    }
  }
  fclose(fTrain);
  fclose(fTest);
  // trainingData - X row = ex col = attr+1
  // prices - Y row = ex col = 1
  //printf("%d %d\n", ex, attr+1);
  double** transpose = transMatrix(ex, attr+1, trainingData); // Attr+1 X ex
  double** temp = multiMatrix((attr+1), ex, (attr+1), transpose, trainingData);
  double** inv = invMatrix((attr+1), (attr+1), temp); // Attr+1 X Attr+1
  double** invMult = multiMatrix((attr+1), (attr+1), ex, inv, transpose); // Attr+1 X ex
  double** weights = multiMatrix((attr+1), ex, 1, invMult, prices); // Attr+1 X 1

  for(int i = 0; i < numTest; i++){
    double result = weights[0][0];
    for(int j = 0; j < attr; j++){
      result += testData[i][j]*weights[j+1][0];
    }
    printf("%0.0lf\n", result);
  }
  

  for(int i = 0; i < attr+1; i++){
    free(transpose[i]);
    free(temp[i]);
    free(inv[i]);
    free(invMult[i]);
    free(weights[i]);
  }
  free(transpose);
  free(temp);
  free(inv);
  free(invMult);
  free(weights);
  for(int i = 0; i < ex; i++){
    free(trainingData[i]);
    free(prices[i]);
  }
  for(int i = 0; i < numTest; i++){
    free(testData[i]);
  }
  free(trainingData);
  free(prices);
  free(testData);
  
  return 0;
}
