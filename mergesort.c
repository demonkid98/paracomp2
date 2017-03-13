#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include<omp.h>

#include <x86intrin.h>

#define NBEXPERIMENTS   7
#define MIN_SIZE_PER_TASK 64

static long long unsigned int experiments [NBEXPERIMENTS] ;

static   unsigned int N ;

typedef  int  *array_int ;

static array_int X ;

void init_array (array_int T)
{
  register int i ;
  
  for (i = 0 ; i < N ; i++)
  {
    T [i] = N - i ;
  }
}

void print_array (array_int T)
{
  register int i ;
  
  for (i = 0 ; i < N ; i++)
  {
    printf ("%d ", T[i]) ;
  }
  printf ("\n") ;
}

int is_sorted (array_int T)
{
  register int i ;
  
  for (i = 1 ; i < N ; i++)
  {
    if (T[i-1] > T [i])
    return 0 ;
  }
  return 1 ;
}

long long unsigned int average (long long unsigned int *exps)
{
  unsigned int i ;
  long long unsigned int s = 0 ;
  
  for (i = 2; i < (NBEXPERIMENTS-2); i++)
  {
    s = s + exps [i] ;
  }
  
  return s / (NBEXPERIMENTS-2) ;
}


void merge_sort (int *T, const int size)
{
  register unsigned int i;
  register unsigned int j;
  register unsigned int k;
  register unsigned int l;
  register unsigned int nb_parts;
  register unsigned int lo;
  register unsigned int hi;
  register unsigned int mi;
  int *X = malloc(size * sizeof(int));

  int sub_size = 1;
  while (sub_size < size) {
    nb_parts = size / sub_size / 2;
    for (l = 0; l < nb_parts; l++) {
      lo = l * sub_size * 2;
      hi = (l + 1) * sub_size * 2;
      mi = (lo + hi) / 2;
      i = lo;
      j = mi;
      k = lo;

      while (i < mi && j < hi) {
        if (T[i] < T[j]) {
          X[k] = T[i];
          i++;
        } else {
          X[k] = T[j];
          j++;
        }
        k++;
      }

      while (i < mi) {
        X[k] = T[i];
        i++;
        k++;
      }
      while (j < hi) {
        X[k] = T[j];
        j++;
        k++;
      }
    }

    memcpy(T, X, size * sizeof(int));
    sub_size = sub_size * 2;
  }
  free(X);
}


void parallel_merge_sort (int *T, const int size)
{
  /* TODO: sequential version of the merge sort algorithm */
  register unsigned int i;
  register unsigned int j;
  register unsigned int k;
  register unsigned int i1;
  register unsigned int i2;
  register unsigned int nb_parts;
  register unsigned int lo;
  register unsigned int hi;
  register unsigned int mi;
  int *X = malloc(size * sizeof(int));
  int max_roll = size > MIN_SIZE_PER_TASK ? MIN_SIZE_PER_TASK : size;
  register int roll_size;

  register int sub_size = 1;
  while (sub_size < size) {
    nb_parts = size / sub_size / 2;
    roll_size = sub_size > (max_roll / 2) ? 1 : (max_roll / sub_size / 2);

    #pragma omp task
    for (i1 = 0; i1 < nb_parts; i1 = i1 + roll_size) {
      #pragma omp parallel for schedule (runtime) private (i, j, k, i2, lo, hi, mi)
      for (i2 = i1; i2 < i1 + roll_size; i2++) {
        lo = i2 * sub_size * 2;
        hi = (i2 + 1) * sub_size * 2;
        mi = (lo + hi) / 2;
        i = lo;
        j = mi;
        k = lo;

        while (i < mi && j < hi) {
          if (T[i] < T[j]) {
            X[k] = T[i];
            i++;
          } else {
            X[k] = T[j];
            j++;
          }
          k++;
        }

        while (i < mi) {
          X[k] = T[i];
          i++;
          k++;
        }
        while (j < hi) {
          X[k] = T[j];
          j++;
          k++;
        }
      }
    }

    memcpy(T, X, size * sizeof(int));
    sub_size = sub_size * 2;
  }
  free(X);
}


int main (int argc, char **argv)
{
  unsigned long long int start, end, residu ;
  unsigned long long int av ;
  unsigned int exp ;
  
  if (argc != 2)
  {
    fprintf (stderr, "mergesort N \n") ;
    exit (-1) ;
  }
  
  N = 1 << (atoi(argv[1])) ;
  X = (int *) malloc (N * sizeof(int)) ;
  
  printf("--> Sorting an array of size %u\n",N);
  
  start = _rdtsc () ;
  end   = _rdtsc () ;
  residu = end - start ; 
  
  // print_array (X) ;
  
  printf("sequential sorting ...\n");
  
  
  for (exp = 0 ; exp < NBEXPERIMENTS; exp++)
  {
    init_array (X) ;
    
    start = _rdtsc () ;
    
    merge_sort (X, N) ;
    
    end = _rdtsc () ;
    experiments [exp] = end - start ;
    
    if (! is_sorted (X))
    {
      fprintf(stderr, "ERROR: the array is not properly sorted\n") ;
      exit (-1) ;
    }      
  }
  
  av = average (experiments) ;  
  printf ("\n merge sort serial\t\t %Ld cycles\n\n", av-residu) ;
  
  printf("parallel sorting ...\n");
  
  for (exp = 0 ; exp < NBEXPERIMENTS; exp++)
  {
    init_array (X) ;
    
    start = _rdtsc () ;
    
    parallel_merge_sort (X, N) ;
    
    end = _rdtsc () ;
    experiments [exp] = end - start ;
    
    if (! is_sorted (X))
    {
      fprintf(stderr, "ERROR: the array is not properly sorted\n") ;
      exit (-1) ;
    }      
  }
  
  av = average (experiments) ;
  printf ("\n merge sort parallel with tasks\t %Ld cycles\n\n", av-residu) ;
  
  
}
