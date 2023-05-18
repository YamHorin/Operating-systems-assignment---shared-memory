#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <fcntl.h>
#define _BSD_SOURCE /* Get MAP_ANONYMOUS definition */

void parseargs(char *argv[], int argc, int *lval, int *uval, int *nval, int *childrenNumber);
int isprime(int n);

int main(int argc, char **argv)
{
  int lval = 1;
  int uval = 100;
  int nval = 10;
  char *flagarr = NULL;
  int number;
  int count = 0;
  int printed_count = 0;
  int childrenNumber = 4;

  // Parse arguments
  parseargs(argv, argc, &lval, &uval, &nval, &childrenNumber);
  if (uval < lval)
  {
    fprintf(stderr, "Upper bound should not be smaller than lower bound\n");
    exit(1);
  }
  if (lval < 2)
  {
    lval = 2;
    uval = (uval > 1) ? uval : 1;
  }
  // Allocate flags
  // to show with the sons
  flagarr = (char *)malloc(sizeof(char) * (uval - lval + 1));
  if (flagarr == NULL)
    exit(1);

  flagarr = mmap(NULL, (sizeof(char) * (uval - lval + 1)), PROT_READ | PROT_WRITE,
                 MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  if (flagarr == MAP_FAILED)
    exit(1);
  // make the childrens
  printf("created %d  childrens\n",childrenNumber);
  for (int i = 0; i < childrenNumber; i++)
  {
    int counter = 0;
    switch (fork())
    {
    case -1:
      exit(1);

    case 0:
      printf("child created!\n");
      while ((childrenNumber + i) * count < (uval - lval + 1))
      {
        if (isprime((childrenNumber + i)))
          flagarr[(childrenNumber + i) * count] = 1;
        else
          flagarr[(childrenNumber + i) * count] = 0;
        count++;
      }
      if (munmap(flagarr, sizeof(char) * (uval - lval + 1)) == -1)
      {
        printf("error in update the array");
        perror("munmap");
      }

      exit(EXIT_SUCCESS);
    default:
      break;
    }
  }
  for (int i = 0; i < childrenNumber; i++)
  {
    if (wait(NULL) == -1)
      perror("wait");
  }
  for (number = lval; number <= uval; number++)
  {
    if (flagarr[number - lval] == 1)
      count++;
  }
  printf("Found %d primes%c\n", count, count ? ':' : '.');
  for (number = lval; number <= uval; number++)
  {
    if (flagarr[number - lval])
    {
      printed_count++;
      printf("%d%c", number, (printed_count < nval && printed_count < count) ? ',' : '\n');
      if (printed_count >= nval || printed_count >= count)
        break;
    }
  }
  return 0;
}

// NOTE : use 'man 3 getopt' to learn about getopt(), opterr, optarg and optopt
void parseargs(char *argv[], int argc, int *lval, int *uval, int *nval, int *childrenNumber)
{
  int ch;

  opterr = 0;
  while ((ch = getopt(argc, argv, "l:u:n:p:")) != -1)
  {
    switch (ch)
    {
    case 'l': // Lower bound flag
      *lval = atoi(optarg);
      break;
    case 'u': // Upper bound flag
      *uval = atoi(optarg);
      break;
    case 'n': // Maximum number of primes to print
      *nval = atoi(optarg);
      break;
    case 'p': // Maximum number of primes to print
      *childrenNumber = atoi(optarg);
      break;
    case '?':
      if (optopt == 'l' || optopt == 'u' || optopt == 'n' || optopt == 'p')
        fprintf(stderr, "Option -%c requires an argument.\n", optopt);
      else if (isprint(optopt))
        fprintf(stderr, "Unknown option `-%c'.\n", optopt);
      else
        fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
      exit(1);
    default:
      exit(1);
    }
  }
}

int isprime(int n)
{
  static int *primes = NULL; // NOTE: static !
  static int size = 0;       // NOTE: static !
  static int maxprime;       // NOTE: static !
  int root;
  int i;

  // Init primes array (executed on first call)
  if (primes == NULL)
  {
    primes = (int *)malloc(2 * sizeof(int));
    if (primes == NULL)
      exit(1);
    size = 2;
    primes[0] = 2;
    primes[1] = 3;
    maxprime = 3;
  }

  root = (int)(sqrt(n));

  // Update primes array, if needed
  while (root > maxprime)
    for (i = maxprime + 2;; i += 2)
      if (isprime(i))
      {
        size++;
        primes = (int *)realloc(primes, size * sizeof(int));
        if (primes == NULL)
          exit(1);
        primes[size - 1] = i;
        maxprime = i;
        break;
      }

  // Check 'special' cases
  if (n <= 0)
    return -1;
  if (n == 1)
    return 0;

  // Check prime
  for (i = 0; ((i < size) && (root >= primes[i])); i++)
    if ((n % primes[i]) == 0)
      return 0;
  return 1;
}