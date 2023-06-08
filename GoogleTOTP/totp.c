#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <openssl/hmac.h>
/**********************************************************************
 * Citation for the following header and included C file: 
 * Date: 05/21/2023
 * Copied the header and C files from:
 * https://github.com/mjg59/tpmtotp/blob/master/base32.c
 **********************************************************************/
#include "base32.h"

#define TOTP_DIGITS 6
#define TIME_STEP 30

/* Function Declarations */
unsigned char *hmac(unsigned char *key, int keyLen, uint64_t timeInterval);
uint32_t codeGen(unsigned char *digest);
uint32_t modulus(uint32_t binary, int digits);
uint32_t generateTOTP(unsigned char *key, size_t keyLen, uint64_t interval, int digits);


/********************************************************************************
 * hmac:
 *
 *  This function will act as a glorified HMAC call using sha1 as its hash which 
 *  will return a hash of the passed in key.
 ********************************************************************************/
unsigned char *hmac(unsigned char *key, int keyLen, uint64_t timeInterval)
{
  return (unsigned char *)HMAC(EVP_sha1(), key, keyLen, (const unsigned char *)&timeInterval, sizeof(timeInterval), NULL, 0);
}

/**********************************************************************************
 * codeGen:
 *
 *  This function will perform the truncation of a specific digest to retrieve the
 *  OTP code displayed on an authenticator.
 **********************************************************************************/
uint32_t codeGen(unsigned char *digest)
{
  uint64_t offset;
  uint32_t binary;

  /* Now we truncate the hash and calculate the offset */
  offset = digest[19] & 0x0F;
  binary = (digest[offset] & 0x7F) << 24 |
           (digest[offset + 1] & 0xFF) << 16 |
           (digest[offset + 2] & 0xFF) << 8  |
           (digest[offset + 3] & 0xFF);

  /* Now we return our truncated digest */
  return binary;
}

/***************************************************************************************
 * modulus:
 *
 *  This function will perform a modulo operation on a given piece of binary data
 *  along with the desired digits of the resulting operation.
 ***************************************************************************************/
uint32_t modulus(uint32_t binary, int digits)
{
  int power = pow(10, digits);
  uint32_t toReturn = binary % power;
  return toReturn;
}

/*****************************************************************************************
 * generateTOTP:
 *
 *  This function will utilize the above helper functions to generate and print out a
 *  TOTP in accordance with RFC6238 and RFC4226 by proxy. 
 *****************************************************************************************/
uint32_t generateTOTP(unsigned char *key, size_t keyLen, uint64_t interval, int digits)
{
    unsigned char *digest;
    uint32_t toReturn;
    uint32_t endianness;

    /* Now we check to see if we are running in little endian and if we are, rearrange the interval as need be*/
    endianness = 0xDEADBEEF;
    if((*(const uint8_t *)&endianness) == 0xef)
    {
      /********************************************************************************************* 
       * Citation for the following code:
       * Date: 05/24/2023
       * Copied the style of swapping endianness from little to big
       * Source URL: https://www.geeksforgeeks.org/bit-manipulation-swap-endianness-of-a-number/
       **********************************************************************************************/
      interval = ((interval & 0x00000000ffffffff) << 32) | ((interval & 0xffffffff00000000) >> 32);
      interval = ((interval & 0x0000ffff0000ffff) << 16) | ((interval & 0xffff0000ffff0000) >> 16);
      interval = ((interval & 0x00ff00ff00ff00ff) <<  8) | ((interval & 0xff00ff00ff00ff00) >>  8);
    }

    /* Now we use the functions above to generate our TOTP and return it */
    digest = (unsigned char *)hmac(key, keyLen, interval);
    uint32_t code = codeGen(digest);
    toReturn = modulus(code, digits);
    return toReturn;
}


int main(void)
{
  /* Declare and initialize the necessary variables to generate our TOTP */
  char *key = "6gRQskb3j0ObS2tTvSgDRjOa2rr2l64xOdHqvYJWVP1mrlA9yasm6VwLChEM01AUa";
  size_t keyLen = strlen(key);
  time_t currTime = floor(time(NULL) / TIME_STEP);
  uint32_t totp = generateTOTP((unsigned char *)key, keyLen, currTime, 6);

  /* Now we print then return */
  printf("TOTP: %06u\n", totp);
  return 0;
}
