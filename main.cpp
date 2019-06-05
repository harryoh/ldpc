// Blockchain.cpp : 이 파일에는 'main' 함수가 포함됩니다. 거기서 프로그램 실행이 시작되고 종료됩니다.
//
#include <iostream>
#include <algorithm>
#include <stdio.h>
#include <time.h>
#include <math.h>

#include "pch.hpp"
#include "utility_function/Memory_Manage.hpp"
#include "decoder/LDPC.hpp"
#include "utility_function/Def_List.h"
#include "block/BlockHeader.hpp"


static int Iter;
int ex(LDPC *decoder, int error_weight, int num)
{
  int succ = 0;
  Iter = 0;
  for (int i = 0; i < num; i++)
  {
    decoder->Generate_Code_Word(0);
    decoder->Generate_Error_Word(error_weight);
    //decoder->Print_Word(INPUT_WORD);
    if (decoder->LDPC_Decoding() == true)
    {
      succ = succ + 1;
      Iter = Iter + (int)decoder->Get_Param(ITER);
    }
  }
  return succ;
}

int Make_Set_of_Crossover_Prob(int N, int Expected_d_min, double *arr)
{
  for (int i = 1; i <= floor( (Expected_d_min - 1) / 2); i++)
    arr[i] = (double) i / N;
  return floor((Expected_d_min - 1) / 2);
}

int main(int argc, char *argv[])
{
  int block_length, message_length, num_of_ones_in_each_col, num_of_ones_in_each_row, total_num_of_H, total_experiment;
  
  if (argc >= 2)
  {
    block_length = atoi(argv[1]);
    message_length = atoi(argv[2]);
    num_of_ones_in_each_col = atoi(argv[3]);
    num_of_ones_in_each_row = atoi(argv[4]);
    total_num_of_H = atoi(argv[5]);
    total_experiment = atoi(argv[6]);
  }
  else
  {
    /* original length
     block_length = 256;
     message_length = 128;
     */
    
    block_length = 64;
    message_length = 32;
    num_of_ones_in_each_col = 3;
    num_of_ones_in_each_row = 6;
    total_num_of_H = 1;
    total_experiment = 4000;
  }

#if 1
  LDPC *decoder = new LDPC;
  char curt_hash_value[SHA256_BLOCK_SIZE];
  decoder->Set_Param(64, 32, 3, 6, 2, 0.001);
  decoder->Make_Gallager_Parity_Check_Matrix(10);
  decoder->Make_Sparse_Matrix_From_Parity_Check_Matrix();

  for (int x = 0; x < 10000; x++) {
    sprintf(curt_hash_value, "%02x", x);
    decoder->Generate_Input_Word((unsigned char*)curt_hash_value);
    if (decoder->LDPC_Decoding() == true)
    {
      printf("\nSuccess!!!!\n");
      printf("\nNonce is : %d\n", x);
      decoder->Print_Word(INPUT_WORD);
      decoder->Print_Word(OUTPUT_WORD);
      break;
    }
  }

#else
  LDPC *decoder = new LDPC;
  double cross = 0.01;
  decoder->Set_Param(block_length, message_length, num_of_ones_in_each_col, num_of_ones_in_each_row, 2, cross);
  decoder->Make_Gallager_Parity_Check_Matrix(1929);    // random construction of gallager ldpc code
  decoder->Make_Sparse_Matrix_From_Parity_Check_Matrix();  //row_in_col and col_in_row matrices
  ex(decoder, 2, 1000);
  
  double *Cross_List = new double[block_length];  //the set of crossover probabilites
  int d_min = floor(0.1*block_length*1.2);
  int last_hamming_weight = Make_Set_of_Crossover_Prob(block_length,d_min , Cross_List);
  BlockHeader *header = new BlockHeader(0, static_cast<unsigned int>(time(NULL)), 0, 0, 9, (char*)" ");
  decoder->Set_Param(block_length, message_length, num_of_ones_in_each_col, num_of_ones_in_each_row, 2, 0.01);  //block length 32, message length 16, # of ones in each row 8 , # of ones in each column 4, crossover probability 0.1
  
  header->Mining(decoder);
  
  FILE *fp = NULL;
  fp = fopen("out.txt", "w");
  
  fprintf(fp, "Nc = %d Nr = %d nc = %d nr = %d expected of d_min = %d\n", block_length, message_length, num_of_ones_in_each_col, num_of_ones_in_each_row, d_min);
  printf("Nc = %d Nr = %d nc = %d nr = %d expected of d_min = %d\n", block_length, message_length, num_of_ones_in_each_col, num_of_ones_in_each_row, d_min);
  
  printf("Set of crossover probabilites\n");
  fprintf(fp,"Set of crossover probabilites\n");
  for (int i = 1; i <= last_hamming_weight; i++)
  {
    printf("%f\t", Cross_List[i]);
    fprintf(fp, "%f\t", Cross_List[i]);
  }
  printf("\n\n");
  fprintf(fp,"\n\n");
  
  last_hamming_weight = last_hamming_weight + 3;
  int *Iter_Array = new int [last_hamming_weight];
  
  
  
  for (int cross_ind = 1; cross_ind < floor( (d_min-1)/2); cross_ind++)
  {
    srand(static_cast<unsigned int>(time(NULL)));
    double cross_over = Cross_List[cross_ind];
    decoder->cross_over_probability = cross_over;
    for (int H_ind = 0; H_ind < total_num_of_H; H_ind++)
    {
      int seed = rand();
      decoder->Make_Gallager_Parity_Check_Matrix(seed);    // random construction of gallager ldpc code
      decoder->Make_Sparse_Matrix_From_Parity_Check_Matrix();  //row_in_col and col_in_row matrices
      printf("\nCross over : %f\t Seed : %d \n", cross_over, seed);
      fprintf(fp, "\nCross over : %f\t Seed : %d \n",cross_over, seed);
      
      printf("Hamming weight\n"); printf("Num_of_Success\n");  printf("Num_of_Iteration\n");
      fprintf(fp,"Hamming weight\n");  fprintf(fp,"Num_of_Success\n");  fprintf(fp,"Num_of_Iteration\n");
      for (int ind = 1; ind <= last_hamming_weight; ind++)
      {
        printf("%d\t",ind);
        fprintf(fp,"%d\t", ind);
      }
      printf("\n");
      fprintf(fp,"\n");
      for (int ind = 1; ind <= last_hamming_weight; ind++)
      {
        int suc =  ex(decoder, ind, total_experiment);
        Iter_Array[ind] = Iter;
        fprintf(fp, "%d\t",suc);
        printf("%d\t",suc);
      }
      printf("\n");
      fprintf(fp, "\n");
      for (int ind = 1; ind <= last_hamming_weight; ind++)
      {
        fprintf(fp, "%d\t", Iter_Array[ind]);
        printf("%d\t", Iter_Array[ind]);
      }
      printf("\n");
      fprintf(fp,"\n");
    }
    printf("\n");
    fprintf(fp,"\n");
  }
  fclose(fp);
  
  delete decoder;
  return 0;
  /*
   BlockHeader *root = list;
   list = header;
   while (total_num_of_block--)
   {
   BlockHeader *header = new BlockHeader(list->Get_Param(VERSION), time(NULL), list->Get_Param(BITS),0, list->Get_Param(INDEX)+1, list->Get_Hash_Value(Curt_Hash_Value));
   decoder->Set_Param(128, 64, 3, 6, 2, 0.01);  //block length 32, message length 16, # of ones in each row 8 , # of ones in each column 4, crossover probability 0.1
   decoder->Make_Gallager_Parity_Check_Matrix(header->Get_Param(SEED));    // random construction of gallager ldpc code
   decoder->Make_Sparse_Matrix_From_Parity_Check_Matrix();  //row_in_col and col_in_row matrices
   header->Mining(decoder);
   list->next = header;
   list = header;
   }
   */
  
#endif
  
  return 0;
}

