#include <string>
#include <iostream>
#include <stdlib.h>
#include <time.h>

#include "BlockHeader.hpp"

using namespace std;

BlockHeader::BlockHeader()
{
  this->version = 0;  this->timestamp = 0; this->nonce = 0; this->bits = 0; this->index = 1;
  memset(this->prev_hash_value, NULL, sizeof(BYTE)*SHA256_BLOCK_SIZE);
  memset(this->curt_hash_value, NULL, sizeof(BYTE)*SHA256_BLOCK_SIZE);
  memset(this->buff, NULL, sizeof(BYTE)*MAX_BUFF_SIZE);
  memset((void*)&this->ctx, NULL, sizeof(SHA256_CTX));
  this->tmp = NULL;
}
BlockHeader::BlockHeader(int version, int timestamp, int bits, int nonce, int index, char prev_hash_value[SHA256_BLOCK_SIZE])
{
  this->version = version; this->timestamp = timestamp; this->nonce = nonce; this->bits = bits; this->index = index;
  memcpy(this->prev_hash_value, prev_hash_value, sizeof (BYTE) *SHA256_BLOCK_SIZE);
  memset(this->curt_hash_value, NULL, sizeof(BYTE)*SHA256_BLOCK_SIZE);
  memset(this->buff, NULL, sizeof(BYTE)*MAX_BUFF_SIZE);
  memset((void*)&this->ctx, NULL, sizeof(SHA256_CTX));
  this->tmp = NULL;
}
BlockHeader::~BlockHeader()
{
  if (this->tmp != NULL)
    delete tmp;
}
void  BlockHeader::Set_Merkle_Tree_Value(char MKT[SHA256_BLOCK_SIZE])
{
  memcpy(merkle_tree_value, MKT, sizeof(BYTE)*SHA256_BLOCK_SIZE);
}
void  BlockHeader::Set_Hash_Value()
{
  BYTE buff[MAX_BUFF_SIZE], input[MAX_BUFF_SIZE];
  memset( buff, NULL, sizeof(BYTE)*MAX_BUFF_SIZE);
  memset(input, NULL, sizeof(BYTE)*MAX_BUFF_SIZE);
  memset( &ctx, NULL, sizeof(SHA256_CTX));
  
  sprintf((char*)buff, "%d", this->nonce);
  strncpy((char*)input, (char*)buff, MAX_BUFF_SIZE);
  //_itoa_s(this->nonce, (char*)buff, MAX_BUFF_SIZE, 10);
  //strcpy_s((char*)input, MAX_BUFF_SIZE, (char*)this->buff);
  //strcat_s((char*)input, MAX_BUFF_SIZE, (char*)buff);
  
  sha256_init(&ctx);
  sha256_update(&ctx, input, strlen((char*)input));
  sha256_final(&ctx, this->curt_hash_value);
}
void  BlockHeader::Set_Buff()
{
  // "version" + "timestmp" + "bits" + merkle_tree_value"
  BYTE buff[4][MAX_BUFF_SIZE];
  sprintf((char*)buff[0], "%d", this->version);
  sprintf((char*)buff[1], "%d", this->timestamp);
  sprintf((char*)buff[2], "%d", this->bits);
  memset(buff   , NULL                   , sizeof(BYTE) * 4 * MAX_BUFF_SIZE);
  memcpy(buff[3], this->merkle_tree_value, sizeof(BYTE) * SHA256_BLOCK_SIZE);
  for (int i = 0; i < 4; i++)
    strncat((char*)this->buff, (char*)buff[i], MAX_BUFF_SIZE);
}

bool BlockHeader::Mining(LDPC *decoder)
{
  bool is_success = false;
  this->Set_Buff();
  this->nonce = 0;
  this->timestamp = static_cast<unsigned int>(time(NULL));
  int iter = 0;
  printf("Starting ECCPoW Mining\n");
  printf("Mining");
  while (1)
  {
    /*
    for (int i = 0; i <= 10; i++) {
      printf("...");
      if (i == 10 && is_success == false) {
        system("clear");
      }
    }
     */
    this->Set_Hash_Value();
    decoder->Generate_Input_Word(this->curt_hash_value);
    if (decoder->LDPC_Decoding() == true)
    {
      printf("\nSuccess!!!!\n");
      printf("\nIter of LDPC decoding: %d\n", iter);
      this->Print_Header_Info();
      decoder->Print_Word(INPUT_WORD);
      decoder->Print_Word(OUTPUT_WORD);
      is_success = true;
      return true;
    }
    
    
     if (this->nonce % 1000 == 0)
     {
     printf("\n");
     this->Print_Header_Info();
     decoder->Print_Word(INPUT_WORD);
     decoder->Print_Word(OUTPUT_WORD);
    }
    
    this->nonce++;
    is_success = false;
  }
  return false;
}
char* BlockHeader::Get_Hash_Value(int type)
{
  if (type == Curt_Hash_Value)
    return (char*)this->curt_hash_value;
  else if (type == Prev_Hash_Value)
    return (char*)this->prev_hash_value;
  else
    return NULL;
}
unsigned int BlockHeader::Get_Param(int type) {
  if (type == VERSION)
    return this->version;
  else if (type == TIMESTAMP)
    return this->timestamp;
  else if (type == BITS)
    return this->bits;
  else if (type == NONCE)
    return this->nonce;
  else if (type == SEED)
  {
    srand(static_cast<unsigned int>(time(NULL)));
    for (int i = 0; i < SHA256_BLOCK_SIZE; i++)
      this->seed = this->seed + (int)this->prev_hash_value[i]*rand();
    return this->seed;
  }
  else if (type == INDEX)
    return this->index;
  else
    return NULL;
}
void BlockHeader::Print_Header_Info()
{
  printf("Version : %d, Timestamp : %d, BlockIndex : %d, Bits : %d, Nonce : %d, Seed : %d\n", this->version, this->timestamp, this->index,this->bits, this->nonce,this->seed);
  printf("Previous Hash Value: ");
  for (int i = 0;i < SHA256_BLOCK_SIZE; i++)
    printf("%02X ", this->prev_hash_value[i]);
  printf("\n Current Hash Value: ");
  for (int i = 0; i < SHA256_BLOCK_SIZE; i++)
    printf("%02X ", this->curt_hash_value[i]);
  printf("\n");
  
}
