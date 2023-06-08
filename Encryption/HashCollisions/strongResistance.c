#define _POSIX_C_SOURCE 200809L
#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define arrlen(x) (sizeof(x) / sizeof *(x))

/* Function Declarations */
void digest(const unsigned char *message, size_t messageLength, unsigned char **digest, unsigned int *digestLength);


int main(void)
{
  int count = 0;

  /* Declare and initialize our initial input buffer and length*/
  unsigned char inputOne[3];
  size_t inputOneLength = arrlen(inputOne);

  /* Declare and initialize the input buffer and length we are using to test against the first */
  unsigned char inputTwo[3];
  size_t inputTwoLength = arrlen(inputTwo);

  /* Declare our hash value to act as a standard to test against */
  unsigned char *hashOutputOne;
  unsigned int hashLengthOne;

  /* Now declare and initialize our second set of hash values to test against the first */
  unsigned char *hashOutputTwo;
  unsigned int hashLengthTwo;

  /* Set our initial seed */
  srand(time(NULL));

  /* Now we randomize the bytes within both inputs on a 0-255 scale */
inputsEqualOuter:
  for(int i = 0; i< sizeof(inputOne); i++)
  {
    inputOne[i] = rand() % 256;
    inputTwo[i] = rand() % 256;
  }
  if(strcmp((char *)inputOne, (char *)inputTwo) == 0)
  {
    goto inputsEqualOuter;
  }

  /* Digest our initial messages */
  digest(inputOne, inputOneLength, &hashOutputOne, &hashLengthOne);
  digest(inputTwo, inputTwoLength, &hashOutputTwo, &hashLengthTwo);

  /* Now we search for a matching hash output */
  while(strcmp((char *)hashOutputOne, (char *)hashOutputTwo) != 0)
  {
    /* For strong resistance, we randomize one and two each time and figest them after */
inputsEqualInner:
    for(int i = 0; i< sizeof(inputTwo); i++)
    {
      inputOne[i] = rand() % 256;
      inputTwo[i] = rand() % 256;
    }
    if(strcmp((char *)inputOne, (char *)inputTwo) == 0)
    {
      goto inputsEqualInner;
    }

    digest(inputOne, inputOneLength, &hashOutputOne, &hashLengthOne);
    digest(inputTwo, inputTwoLength, &hashOutputTwo, &hashLengthTwo);
    count++;
  }
  printf("Trials before match was found: %d\n", count);
  return 0;
}

/**********************************************************************************************************
 * digest:
 *
 * This function will take a message, its length, a hash output, and its length and it will run the message
 * through the md5 hashing algorithm to product hashed cipherText.
 *
 ***********************************************************************************************************/
void digest(const unsigned char *message, size_t messageLength, unsigned char **digest, unsigned int *digestLength)
{
    EVP_MD_CTX *mdctx = EVP_MD_CTX_create();

    if(mdctx == NULL)
    {
      EVP_MD_CTX_destroy(mdctx);
      ERR_print_errors_fp(stderr);
      exit(EXIT_FAILURE);
    }

    /* Initialize our digest execution with the hashing algorithm */
    if((EVP_DigestInit_ex(mdctx, EVP_md5(), NULL)) != 1)
    {
      EVP_MD_CTX_destroy(mdctx);
      ERR_print_errors_fp(stderr);
      exit(EXIT_FAILURE);
    }

    /* Now we process our message up to messageLength bytes */
    if((EVP_DigestUpdate(mdctx, message, messageLength)) != 1)
    {
      EVP_MD_CTX_destroy(mdctx);
      ERR_print_errors_fp(stderr);
      exit(EXIT_FAILURE);
    }

    /* Now we malloc the appropriate size for our hash output */
    if((*digest = (unsigned char *) OPENSSL_malloc(EVP_MD_size(EVP_md5()))) == NULL)
    {
      EVP_MD_CTX_destroy(mdctx);
      ERR_print_errors_fp(stderr);
      exit(EXIT_FAILURE);
    }

    /* Now we put our hash text into our newly allocated digest */
    if((EVP_DigestFinal(mdctx, *digest, digestLength)) != 1)
    {
      EVP_MD_CTX_destroy(mdctx);
      ERR_print_errors_fp(stderr);
      exit(EXIT_FAILURE);
    }

    EVP_MD_CTX_destroy(mdctx);
}
