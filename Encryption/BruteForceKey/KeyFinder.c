#define _POSIX_C_SOURCE 200809L
#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define arrlen(x) (sizeof(x) / sizeof *(x))

/* Set up and declare our global givens */
unsigned char *plainText = (unsigned char *) "This is a top secret.";
unsigned char initialCipherText[65] = "8d20e5056a8d24d0462ce74e4904c1b513e10d1df4a2ef2ad4540fae1ca0aaf9";
unsigned char iv[16];
/* Function Declarations */
unsigned char* getKey(void);
int encrypt(unsigned char *plainText, int plainLength, unsigned char *keyToTry, unsigned char *iv, unsigned char *cipherText);

int main(void)
{
  /* Declare our key that we will be repeatedly setting with encrypt */
  unsigned char *key = NULL;
  /* Declare a ciphertext buffer long enough to fit our potential cipherText */
  unsigned char cipherTextToCompare[65] = {0};
  int cipherTextLength;

  /* Declare and initialize the rest of our variables */
  FILE *dictFile;
  size_t length = 0;
  ssize_t numRead = 0;
  int toAppend = 0;
  unsigned char toHex[65]; 

  dictFile = fopen("words.txt", "r");
  if(dictFile == NULL)
  {
    perror("fopen on words.txt");
    exit(EXIT_FAILURE);
  }

  
  while((numRead = getline((char **) &key, &length, dictFile)) != -1) 
  {
    if(numRead < 16)
    {
      /* Strip off the newline from our key while type casting the right side */
      key = (unsigned char *) strtok((char *) key, "\n");
      size_t keyLen = strlen((char *) key);

      /* Calculate the number of spaces we need to append on top of the key */
      toAppend = 16 - keyLen;

      /* Allocate enough room to append the new spaces and a null byte */
      unsigned char *newKey = malloc(keyLen + toAppend + 1);
      char appendChar = ' ';
      strncat((char *) newKey, (char *)key, keyLen);

      /* Now we start appending the spaces to our key */
      for(int i = toAppend; i > 0; i--)
      {
        strncat((char *) newKey, &appendChar, 1);
      }

      /* Now we encrypt the text with the new key we created and print it out*/
      cipherTextLength = encrypt(plainText, strlen((char *) plainText), newKey, iv, cipherTextToCompare);
      
      for(int i = 0, j = 0; i<strlen((char *) cipherTextToCompare); i++, j+=2)
      {
        if(cipherTextToCompare[i] > 0)
        {
          sprintf((char *) toHex + j, "%02x", cipherTextToCompare[i] & 0xff);
        }
        else
        {
          break;
        }
      }
      //printf("CipherText is:\n%s\n", toHex);
      if((strcmp((char *) initialCipherText, (char *) toHex)) == 0)
      {
        printf("Key found, key is: %s\n", key); 
      }
    }
  }
  return 0;
}
  

/****************************************************************************************************
 * encrypt:
 *
 * This function will encrypt a provided plaintext using the aes-128-cbc protocol and will put
 * the resulting encrypted text into the passed variable cipherText and will return the number of 
 * encrypted bytes written
 *
 *****************************************************************************************************/
int encrypt(unsigned char *plainText, int plainLength, unsigned char *keyToTry, unsigned char *iv, unsigned char *cipherText)
{
  EVP_CIPHER_CTX *ctx;

  int encryptedLength, cipherLength;

  /* Create and initialize our new context and error handle appropriately and print to stderr */
  if(!(ctx = EVP_CIPHER_CTX_new()))
  {
    EVP_CIPHER_CTX_free(ctx);
    ERR_print_errors_fp(stderr);
    exit(EXIT_FAILURE);
  }

  /* Initialize our encryption execution with aes-128-cbc as the cipher */
  if((EVP_EncryptInit_ex(ctx, EVP_aes_128_cbc(), NULL, keyToTry, NULL)) != 1)
  {
    EVP_CIPHER_CTX_free(ctx);
    ERR_print_errors_fp(stderr);
    exit(EXIT_FAILURE);
  }
    
  /* Now we feed in our plaintext to encrypt and the ciphertext pointer to put our encrypted text into */
  if((EVP_EncryptUpdate(ctx, cipherText, &encryptedLength, plainText, plainLength)) != 1)
  {
    EVP_CIPHER_CTX_free(ctx);
    ERR_print_errors_fp(stderr);
    exit(EXIT_FAILURE);
  }

  /* Now we set our cipherLength to our encryptedLength to eventually return later */
  cipherLength = encryptedLength;

  /* Finalize the encryption execution and update our cipherLength in case additional bytes are written */
  if((EVP_EncryptFinal(ctx, cipherText + encryptedLength, &encryptedLength)) != 1)
  {
    EVP_CIPHER_CTX_free(ctx);
    ERR_print_errors_fp(stderr);
    exit(EXIT_FAILURE);
  }
  cipherLength += encryptedLength;

  /* Now we free our context now that we are done */
  EVP_CIPHER_CTX_free(ctx);

  return cipherLength;
}


