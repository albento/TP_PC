#include <stdio.h>
#include <pthread.h>

#include "primes.h"
#include "hash.h"

/**
 * The size of the global hash map.
 */
#define HASH_SIZE 100

/**
 * The global mutex our functions will use to prevent
 * concurent access to the file we want to parse.
 */
static pthread_mutex_t mutexFile;

/**
 * The global mutex our functions will use to prevent
 * concurent access to the hashmap.
 */
static pthread_mutex_t mutexMap;

/**
 * The global hash table our functions will use.
 */
static hash_table* hashmap;

/**
 * The function which will initialize
 * the global hash table.
 */
static void initHashmap();

/**
 * The function which will destroy
 * the global hash table.
 */
static void destroyHashmap();

/**
 * The function our threads will use.
 */
void* th(FILE* file);

/**
 * Insert the number n in the hash table with its
 * decomposition. If it already exists, do nothing.
 */
int get_prime_factors_hash(uint64_t n, uint64_t* dest, hash_table* h);

/**
 * Print all the prime factors of each numbers in the
 * file numbers.txt, using the memoization optimization
 * and four worker threads.
 */
int main(void)
{
	pthread_mutex_init(&mutexFile, NULL);
	pthread_mutex_init(&mutexMap, NULL);
	
	initHashmap();
	parsePrimeFileThreaded("numbers.txt", 2, th);
	
	pthread_mutex_destroy(&mutexFile);
	pthread_mutex_destroy(&mutexMap);
	print_hash(hashmap);
	destroyHashmap();
	return 0;
}

void* th(FILE* file)
{
	uint64_t number = 0;
	
	while(!feof(file))
    {
		pthread_mutex_lock(&mutexFile);
			fscanf(file, "%ld", &number);
		pthread_mutex_unlock(&mutexFile);
    	
    	uint64_t factors[MAX_FACTORS];
    	int k = get_prime_factors_hash(number, factors, hashmap);
    }
	pthread_exit(0);
}

inline void initHashmap() {
	hashmap = create_hash(HASH_SIZE);
}

inline void destroyHashmap()
{
	delete_hash(hashmap);
}

int get_prime_factors_hash(uint64_t n, uint64_t* dest, hash_table* h)
{
	unsigned int cpt = 0;
	
	while( !(n%2) )
	{
		n/=2;
		dest[cpt++] = 2;
	}
	while( !(n%3) )
	{
		n/=3;
		dest[cpt++] = 3;
	}
	uint64_t pasi = 2;
    uint64_t i;
    for(i = 5; i<= n && cpt < MAX_FACTORS; i+=pasi, pasi= 6-pasi)
    {
		if(isfactor(n, i))
		{
			uint64_t* dec = (uint64_t*) malloc(sizeof(uint64_t) * MAX_FACTORS);
    	
			pthread_mutex_lock(&mutexMap);
			unsigned int nb_fac = get_decomposition(h, i, dec);
			pthread_mutex_unlock(&mutexMap);
			
			if (nb_fac > 0)
			{
				int j;
				
				for (j = 0; j < nb_fac; j++)
				{
					dest[cpt++] = dec[j];
				}
			}
			
			else
			{
				if(isPrime(i))
				{
					double tamp = n/i;
					while( tamp == floor(tamp) )
					{
						dest[cpt++] = i;
						tamp/=i;
					}
				}
			}
		}
    	
	}
		
	pthread_mutex_lock(&mutexMap);
	insert_hash(hashmap, n, dest, cpt);
	pthread_mutex_unlock(&mutexMap);
	
	return cpt;
}
