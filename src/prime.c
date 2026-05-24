#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include "prime.h"
#include "utils.h"

#define SEG_SIZE (1 << 16)  // We assume L1 cache to be 64 KiB

uint64_t approximate_upper_bound(uint64_t n) {
    uint64_t upper_bound;
    if (n >= 355991) {
        // Upper bound by Pierre Dusart (1998)
        double log_n = log(n);
        upper_bound = (uint64_t)(n / (log_n) * (1.0 + 1.0/log_n+2.51/(log_n*log_n)))+1; // Upper bound for number of primes up to n
    } else {
        upper_bound = 100000; // For small n, we can just use a fixed size
    }
    return upper_bound;
}

/**
 * This computes the sieve and primes up to n
 */
void prime_sieve(uint8_t *sieve, uint64_t *primes, uint64_t *num_primes, uint64_t n) {
    // If sieve can fit in L1 cache we can just run the sieve
    if(n < SEG_SIZE) {
        for (uint32_t i = 2; i * i <= n; i++) {
            if (!sieve[i]) {
                for (uint32_t j = i * i; j <= n; j += i)
                    sieve[j] = 1;
            }
        }
        // Fill prime array
        for(uint32_t i = 2; i <= n; i++) {
            if (!sieve[i]) {
                primes[(*num_primes)++] = i;
            }
        }
        return;
    }

    // Sieve is too large so we compute primes up to sqrt(n) and then we can use segmented sieve to fill the rest
    uint64_t sqrt_n = (uint64_t)sqrt((double)n);
    prime_sieve(sieve, primes, num_primes, sqrt_n);
    // Now we have primes and a sieve up to sqrt(n)

    for (uint64_t i = 0; i < (*num_primes); i++) {
        printf("%llu ", primes[i]);
    }

    // Fill the rest of the sieve in segments
    for (uint64_t segment_start = sqrt_n + 1; segment_start <= n; segment_start+=SEG_SIZE) {
        uint64_t segment_end = prime_min(segment_start + SEG_SIZE - 1, n);
        // Mark multiples of primes in the current segment
        for (uint64_t i = 0; i < (uint64_t)(*num_primes); i++) {
            uint64_t p = primes[i];
            uint64_t start = (segment_start + p - 1) / p * p; // First multiple of p in the segment
            for (uint64_t j = start; j <= segment_end; j += p) {
                sieve[j] = 1;
            }
        }
        // Populate the remaining primes
        for(uint64_t i = segment_start; i <= segment_end; i++) {
            if (!sieve[i]) {
                primes[(*num_primes)++] = i;
            }
        }
    }
}


uint64_t prime_count(uint64_t n) {
    
    uint64_t sqrt_n = (uint64_t)sqrt((double)n);

    uint64_t upper_bound = approximate_upper_bound(sqrt_n);
    uint64_t *primes = malloc(upper_bound * sizeof(uint64_t));

    uint8_t *sieve = calloc(sqrt_n + 1, 1);

    // Compute primes up to sqrt(n)
    uint64_t small_num_primes = 0;
    prime_sieve(sieve, primes, &small_num_primes, sqrt_n);
    free(sieve);

    uint64_t num_primes = small_num_primes;

    // Segmented sieve to count the rest
    uint8_t *segment_sieve = malloc(SEG_SIZE * sizeof(uint8_t));
    
    for (uint64_t segment_start = sqrt_n + 1; segment_start <= n; segment_start += SEG_SIZE) {
        uint64_t segment_end = prime_min(segment_start + SEG_SIZE - 1, n);
        memset(segment_sieve, 0, SEG_SIZE);

        // Mark multiples of primes in the current segment
        for (uint64_t i = 0; i < small_num_primes; i++) {
            uint64_t p = primes[i];
            uint64_t start = (segment_start + p - 1) / p * p; // First multiple of p in the segment
            for (uint64_t j = start; j <= segment_end; j += p) {
                segment_sieve[j - segment_start] = 1;
            }
        }
        for (uint64_t i = segment_start; i <= segment_end; i++) {
            if (!segment_sieve[i - segment_start]) {
                num_primes++;
            }
        }
    }

    free(primes);
    free(segment_sieve);
    return num_primes;
}