#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <openssl/md2.h>
#include <openssl/sha.h>

#define arrlen(x) (sizeof(x) / sizeof (*x))
#define ARRAY_MAX 8000000

/* Function definitions */
char **populateList(FILE *fp, char *name, size_t *listLen);
int checkList(char *toCheck, char **list, size_t listLength);
char *stripNewLine(char *toModify, size_t length);
void freeList(char **list);
void hashList(char **toHash, size_t length, unsigned char bloomFilter[], size_t filterLength);
void addStringToBloomFilter(char *toAdd, unsigned char *filter);
int checkStringInBloomFilter(char *toCheck, unsigned char *filter);
int getBitPosition(unsigned char *digest, int length);
void setBitPosition(size_t bitPosition, unsigned char *filter);
int isBitFlipped(const unsigned char *filter, int bitPosition);
unsigned char *hashMD2(const unsigned char *toHash, size_t length);
unsigned char *hashSHA256(const unsigned char *toHash, size_t length);
unsigned char *hashSHA512(const unsigned char *toHash, size_t length);
int binarySearch (char **in, int n, char *toFind);
int cmpString(const void *a, const void *b);



int main(void)
{
  FILE *rockYou = NULL;
  FILE *passwordsToTry = NULL;

  unsigned char bloomArray[ARRAY_MAX] = {0};
  char *toRead;

  ssize_t numRead = 0;
  size_t length = 0;
  int inList = 0;
  int inFilter = 0;

  int truePositive = 0;
  int falsePositive = 0;
  int trueNegative = 0;
  int falseNegative = 0;
  
  printf("Populating our common password list\n");

  /* Populate our list using the rockYou pointer */
  char **rockList = NULL;
  size_t rockLength = 0;
  rockList = populateList(rockYou, "rockyou.ISO-8859-1.txt", &rockLength);

  /* Now we assert that our list has been created and that is has more than 10 million elements */
  printf("Done, unit tests on rockList next\n");
  assert(rockList != NULL);
  assert(rockLength == 14344391);
  printf("Unit tests passed\n");

  /* Now we populate the bloom filter array using the strings from rockList */
  printf("Populating the bloom filter array\n");
  hashList(rockList, rockLength, bloomArray, ARRAY_MAX);

  /* Now we use qsort to sort our strings in the array */
  printf("using qsort on our rockList\n");
  qsort(rockList, rockLength, sizeof(char *), cmpString);
  printf("qsort() Completed\n");

  /* Now we perform our unit tests and print the bloom array */
  printf("Done, unit tests on bloomArray next\n");
  assert(bloomArray != NULL);
  printf("Unit Tests passed\n");
  printf("Bloom array is:%s\n", bloomArray);
  
  /* Now lets check each string in dictionary.txt against our list and print if it is in rockList */
  printf("Checking dictionary.txt against rockList\n");
  passwordsToTry = fopen("dictionary.txt", "r, ccs=ISO-8859-1");
  while((numRead = getline(&toRead, &length, passwordsToTry)) != -1)
  {
    toRead = stripNewLine(toRead, numRead);
    inList = checkList(toRead, rockList, rockLength);
    inFilter = checkStringInBloomFilter(toRead, bloomArray);
    if(inList && inFilter)
    {
      truePositive ++;
    }
    else if(!(inList) && inFilter)
    {
      falsePositive++;
    }
    else if(inList && !(inFilter))
    {
      falseNegative++;
    }
    else if(!(inList) && !(inFilter))
    {
      trueNegative++;
    }
  }

  /* Now we do unit tests for our stats */
  printf("Done, unit tests on the stats next\n");
  assert(falseNegative == 0);
  assert(truePositive > 100000);
  assert(trueNegative > 400000);
  assert(falsePositive > 40000);
  printf("Unit tests passed, printing stats\n");

  /* Now we print out the number of positives and negatives we found */
  printf("True positive:%d\n", truePositive);
  printf("False positive:%d\n", falsePositive);
  printf("True negative:%d\n", trueNegative);
  printf("False negative:%d\n", falseNegative);

  free(toRead);
  freeList(rockList);
  fclose(passwordsToTry);
  return 0;
}

/*******************************************************************************
 * populateList:
 *
 *  This function will take a pointer to a file and the name of the file and
 *  return an array of strings by reading each line and dynamicallt allocating
 *  enough space for that array. 
 ********************************************************************************/
char **populateList(FILE *fp, char *name, size_t *listLen)
{
  int continueReadingFile = 1;
  ssize_t numRead = 0;
  size_t length = 0;
  size_t arrLength = 0;

  char *toRead = NULL;
  char **toReturn = NULL;

  fp = fopen(name, "r, ccs=ISO-8859-1");
  if(fp == NULL)
  {
    printf("fopen() on %s\n", name);
    exit(1);
  }

  /* Read the first line of our file into our array and if we already reach EOF, set continueReadingFile to false */
  if(toRead == NULL)
  {
    numRead = getline(&toRead, &length, fp);
    if(numRead == -1)
    {
      fclose(fp);
      return NULL;
    }
    toRead = stripNewLine(toRead, numRead);
    toReturn = (char **)malloc(sizeof(char *));

    toReturn[arrLength] = (char *)malloc(numRead+1);
    strcpy(toReturn[arrLength], toRead);
    arrLength++;
  } 

  /* Now while continueReadingFile is true, we continue reading lines from our file */
  while(continueReadingFile)
  {
    numRead = getline(&toRead, &length, fp);
    if(numRead == -1)
    {
      continueReadingFile = 0;
      continue;
    }
    toRead = stripNewLine(toRead, numRead);
    toReturn = realloc(toReturn, (arrLength+ 1) * sizeof(char *));

    toReturn[arrLength] = (char *)malloc(numRead);
    strcpy(toReturn[arrLength], toRead);
    arrLength++;
  }
  (*listLen) = arrLength;
  /* Free toRead and return our new list */
  free(toRead);
  return toReturn;
}

/*******************************************************************************
 * checkList:
 *
 *  This function will take in a file pointer, its name, and a list to check against
 *  where it will perform a binary search to see how many strings are in a 
 *  given list.
 ********************************************************************************/
int checkList(char *toCheck, char **list, size_t listLength)
{
  int inList = 0;
  inList = binarySearch(list, listLength, toCheck);
  assert(inList == 1 || inList == 0);
  return inList;
}

/*******************************************************************************
 * stripNewLine:
 *
 *  This function will take in a string and its length and trunicate the
 *  newline off of it or return the passed in string if there is no newline. 
 ********************************************************************************/
char *stripNewLine(char *toModify, size_t length)
{
  if(toModify[length - 1] == '\n')
  { 
    toModify[length - 1] = '\0'; 
  }
  else 
  { 
    return toModify; 
  }
  return toModify;
}


/*******************************************************************************
 * freeList:
 *
 *  This function will take in an array of allocated strings and free them before
 *  freeing the list itself or it will silently return if the list is NULL.
 ********************************************************************************/
void freeList(char **list)
{
  if(list == NULL)
  {
    return;
  }
  for(int i = 0; i < arrlen(list); i++)
  {
    free(list[i]);
  }
  free(list);
}

/*******************************************************************************
 * hashList:
 *
 *  This function will take in an array of allocated strings, a length of the list,
 *  a bloom filter to populate, and its length to add each string to the passed
 *  bloom filter.
 ********************************************************************************/
void hashList(char **list, size_t length, unsigned char bloomFilter[], size_t filterLength)
{
  for(int i = 0; i < length; i++)
  {
    addStringToBloomFilter(list[i], bloomFilter);
  }
  assert(bloomFilter != NULL);
}

/*******************************************************************************
 * checkStringInBloomFilter:
 *
 *  This function will take in a string to check and a filter to check against
 *  where it will calculate the hash and bit positions of the string to see
 *  if the given string is in the filter or not.
 ********************************************************************************/
int checkStringInBloomFilter(char *toCheck, unsigned char *filter)
{
  size_t checkLen = strlen(toCheck);
  int numSet = 0;

 /* Calculate and set the digest of MD2 then calculate and set the bit position from that */
  unsigned char *md2Digest = hashMD2((unsigned char *)toCheck, checkLen);
  int bitPosition = getBitPosition(md2Digest, MD2_DIGEST_LENGTH);
  assert(bitPosition >= 0);
  numSet += isBitFlipped(filter, bitPosition);

  /* Calculate and set the digest of SHA256 then calculate and set the bit position from that */
  unsigned char *sha256Digest = hashSHA256((unsigned char *)toCheck, checkLen);
  bitPosition = getBitPosition(sha256Digest, SHA256_DIGEST_LENGTH);
  assert(bitPosition >= 0);
   numSet += isBitFlipped(filter, bitPosition);
  

  /* Calculate and set the digest of SHA512 then calculate and set the bit position from that */
  unsigned char *sha512Digest = hashSHA512((unsigned char *)toCheck, checkLen);
  bitPosition = getBitPosition(sha512Digest, SHA512_DIGEST_LENGTH);
  assert(bitPosition >= 0);
  numSet += isBitFlipped(filter, bitPosition);

  /* Now free each digest */
  free(md2Digest);
  free(sha256Digest);
  free(sha512Digest);

  /* If the number of flipped bits so happens to be 3, it is possibly in the filter */
  if(numSet == 3)
  {
    return 1;
  }
  return 0;
}

/*******************************************************************************
 * addStringToBloomFilter:
 *
 *  This function will take in a string to add and the filter that needs to be
 *  populated and it will call helper functions to use the digest of a hash
 *  to calculate and flip a bit at a certain position.
 ********************************************************************************/
void addStringToBloomFilter(char *toAdd, unsigned char *filter)
{
  size_t addLen = strlen(toAdd);

  /* Calculate and set the digest of MD2 then calculate and set the bit position from that */
  unsigned char *md2Digest = hashMD2((unsigned char *)toAdd, addLen);
  int bitPosition = getBitPosition(md2Digest, MD2_DIGEST_LENGTH);
  assert(bitPosition >= 0);
  setBitPosition(bitPosition, filter);

  /* Calculate and set the digest of SHA256 then calculate and set the bit position from that */
  unsigned char *sha256Digest = hashSHA256((unsigned char *)toAdd, addLen);
  bitPosition = getBitPosition(sha256Digest, SHA256_DIGEST_LENGTH);
  assert(bitPosition >= 0);
  setBitPosition(bitPosition, filter);

  /* Calculate and set the digest of SHA512 then calculate and set the bit position from that */
  unsigned char *sha512Digest = hashSHA512((unsigned char *)toAdd, addLen);
  bitPosition = getBitPosition(sha512Digest, SHA512_DIGEST_LENGTH);
  assert(bitPosition >= 0);
  setBitPosition(bitPosition, filter);

  /* Now free each digest */
  free(md2Digest);
  free(sha256Digest);
  free(sha512Digest);
}

/*******************************************************************************
 * getBitPosition:
 *
 *  This function will take in a digest and its length to calculate the bit position
 *  that needs to be flipped based on the digest value modulo our array bit size.
 *  Then it will return the index that needs to be flipped.
 *********************************************************************************/
int getBitPosition(unsigned char *digest, int length)
{
  /* First we convert the digest into an array of bytes and then copy it to the array */
  unsigned char bytes[length];
  memset(bytes, 0, sizeof(bytes));
  memcpy(bytes, digest, length);

  unsigned long long bitIndex = 0;

  /* Loop through our digest and calculate the bit position that needs to be flipped */
  for(int i = 0; i < length; i++)
  {
    bitIndex = (bitIndex << 8) | bytes[i];
    bitIndex %= (ARRAY_MAX * 8);
  }

  return bitIndex;
}

/*******************************************************************************
 * setBitPosition:
 *
 *  This function will take in a bit position and the filter for it to look through
 *  and it will set the bit at the specified position. 
 *********************************************************************************/
void setBitPosition(size_t bitPosition, unsigned char *filter)
{
  unsigned int byteIndex = bitPosition / 8;
  unsigned bitOffset = bitPosition % 8;
  filter[byteIndex] |= (1 << bitOffset);
}

/*******************************************************************************
 * isBitFlipped:
 *
 *  This function will take in a bit position and the filter for it to look through
 *  and it will see if the bit at the specified location is flipped and will
 *  return 1 if it is, and 0 if it is not.
 *********************************************************************************/
int isBitFlipped(const unsigned char *filter, int bitPosition)
{
  unsigned int byteIndex = bitPosition / 8;
  unsigned bitIndex = bitPosition % 8;
  unsigned char mask = 1 << bitIndex;

  /* We check the byte at the byteIndex position in our filter to see if the bit we are looking at is flipped, if so, return 1 */
  if((filter[byteIndex] & mask) != 0)
  {
    return 1;
  }
  return 0;
}

/*******************************************************************************
 * hashMD2:
 *
 *  This function will hash toHash using MD2 and return the digested
 *  hash.
 ********************************************************************************/
unsigned char *hashMD2(const unsigned char *toHash, size_t length)
{
  unsigned char *digest = (unsigned char *)malloc(MD2_DIGEST_LENGTH);
  MD2(toHash, length, digest);
  assert(digest != NULL);
  return digest;
}

/*******************************************************************************
 * hashSHA256:
 *
 *  This function will hash toHash using SHA256 and return the digested
 *  hash.
 ********************************************************************************/
unsigned char *hashSHA256(const unsigned char *toHash, size_t length)
{
  unsigned char *digest = (unsigned char *)malloc(SHA256_DIGEST_LENGTH);
  SHA256(toHash, length, digest);
  assert(digest != NULL);
  return digest;
}

/*******************************************************************************
 * hashSHA512:
 *
 *  This function will hash toHash using SHA512 and return the digested
 *  hash.
 ********************************************************************************/
unsigned char *hashSHA512(const unsigned char *toHash, size_t length)
{
  unsigned char *digest = (unsigned char *)malloc(SHA512_DIGEST_LENGTH);
  SHA512(toHash, length, digest);
  assert(digest != NULL);
  return digest;
}


/******************************************************************************
 * binarySearch:
 *  
 *  This function will search for a given string within a given array using
 *  the binary search algorithm to cut down search time to O(log(n)) so that
 *  checking for false positives is faster than searching linearly.
 ******************************************************************************/
int binarySearch(char **in, int n, char *toFind)
{
  /* Initialize and declare our middle, high, and low indexes */
  int low = 0, high = n - 1, middle = 0;

  /* Continue searching for the string if we cannot find it and set our middle accordingly */
  while(low <= high)
  {
    middle = (high + low) / 2;

    if(strcmp(in[middle], toFind) == 0)
    {
      return 1;
    }
    else if(strcmp(in[middle], toFind) > 0)
    {
      high = middle - 1;
    }
    else
    {
      low = middle + 1;
    }
  }
  /* If we did not find it, return 0 */
  return 0;
}

/******************************************************************************
 * cmpString:
 *  
 *  This function will act as a comparison function used in qsort which will
 *  sort the strings from least to greatest.
 ******************************************************************************/
int cmpString(const void *a, const void *b)
{
  const char *strOne = *(const char **)a;
  const char *strTwo = *(const char **)b;
  return strcmp(strOne, strTwo);
}

