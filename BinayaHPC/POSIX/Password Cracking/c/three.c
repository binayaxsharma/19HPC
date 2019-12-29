#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <crypt.h>
#include <time.h>

/***********************************************************************
*******

  Compile with:
    cc -o three three.c -lcrypt

    ./three > threeinitials.txt
************************************************************************
******/
int n_passwords = 4;

char *encrypted_passwords[] = {
  "$6$KB$TZoFFZy.ei9t0vlhI8dq5.1dtwQ.FZi2psDUBeLBjAqvkKO5cCgzy0DQrsG3Jtx5n1ksgLk7nq5Ne2ITm9H481",
  "$6$KB$1qi.Hjp7CiXq.XOQKQZRckFlcjWQgnVZk3pmqYJSekRO3YJtDsEWc0e9SDx/AKJ9/meFi0EgJ7/sG2GWuIhYV1",
  "$6$KB$/laUxNxR3NuVUUxQMgYJXoeFbOcwWBu.h04mydEsE3QHF43xGSd4gIj4ZcDeS19LmjvqJQ3OzOMOb4YEVx6rZ0",
  "$6$KB$pyp2iANpAgU2H0ku7P1FHqyWbAO0/i856yyPhH72DiLHd.6TCWugtcMt2bFxAgnsbKXf2VfiC9OxlgFHAlEIO1"
};

/**
 Required by lack of standard function in C.
*/

void substr(char *dest, char *src, int start, int length){
  memcpy(dest, src + start, length);
  *(dest + length) = '\0';
}

/**
 This function can crack the kind of password explained above. All
combinations
 that are tried are displayed and when the password is found, #, is put
at the
 start of the line. Note that one of the most time consuming operations
that
 it performs is the output of intermediate results, so performance
experiments
 for this kind of program should not include this. i.e. comment out the
printfs.
*/

void crack(char *salt_and_encrypted){
  int m, n, o,a;     // Loop counters
  char salt[7];    // String used in hashing the password. Need space for \0
  char plain[7];   // The combination of letters currently being checked
  char *enc;       // Pointer to the encrypted password
  int count = 0;   // The number of combinations explored so far

  substr(salt, salt_and_encrypted, 0, 6);

  for(m='A'; m<='Z'; m++){
    for(n='A'; n<='Z'; n++){
      for(o='A'; o<='Z'; o++){
	for(a=0; a<=99; a++){
        sprintf(plain, "%c%c%c%02d", m, n, o,a);
        enc = (char *) crypt(plain, salt);
        count++;
        if(strcmp(salt_and_encrypted, enc) == 0){
          printf("#%-8d%s %s\n", count, plain, enc);
        } else {
          printf(" %-8d%s %s\n", count, plain, enc);
        }
      }
    }
  }
}
  printf("%d solutions explored\n", count);
}


//Calculating time

int time_difference(struct timespec *start, struct timespec *finish, long long int *difference)
 {
	  long long int ds =  finish->tv_sec - start->tv_sec;
	  long long int dn =  finish->tv_nsec - start->tv_nsec;

	  if(dn < 0 ) {
	    ds--;
	    dn += 1000000000;
  }
	  *difference = ds * 1000000000 + dn;
	  return !(*difference > 0);
}
int main(int argc, char *argv[])
{
  	int i;
	struct timespec start, finish;
  	long long int time_elapsed;

  	clock_gettime(CLOCK_MONOTONIC, &start);

  	for(i=0;i<n_passwords;i<i++)
	{
    		crack(encrypted_passwords[i]);
  	}
	clock_gettime(CLOCK_MONOTONIC, &finish);
	  time_difference(&start, &finish, &time_elapsed);
	  printf("Time elapsed was %lldns or %0.9lfs\n", time_elapsed,
		                                 (time_elapsed/1.0e9));
  return 0;
}
