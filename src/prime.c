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
        upper_bound = (uint64_t)(n / (log_n) * (1.0 + 1.0/log_n+2.51/(log_n*log_n)))+1;
    } else {
        upper_bound = 100000;
    }
    return upper_bound;
}

/**
 * This computes the sieve and primes up to n
 */
void prime_sieve(uint8_t *sieve, uint64_t *primes, uint64_t *num_primes, uint64_t n) {
    // If sieve can fit in L1 cache we can just run the sieve
    if(n < SEG_SIZE) {
        // 2 is the only even prime; sieve only odd numbers
        if (n >= 2) primes[(*num_primes)++] = 2;
        for (uint32_t i = 3; (uint64_t)i * i <= n; i += 2) {
            if (!sieve[i]) {
                for (uint32_t j = i * i; j <= n; j += 2 * i)
                    sieve[j] = 1;
            }
        }
        for (uint32_t i = 3; i <= n; i += 2) {
            if (!sieve[i]) {
                primes[(*num_primes)++] = i;
            }
        }
        return;
    }

    // Sieve is too large so we compute primes up to sqrt(n) and then we can use segmented sieve to fill the rest
    uint64_t sqrt_n = (uint64_t)sqrt((double)n);
    prime_sieve(sieve, primes, num_primes, sqrt_n);

    // First odd number > sqrt_n; skip p=2 since all its multiples are even
    uint64_t seg_start = (sqrt_n % 2 == 0) ? sqrt_n + 1 : sqrt_n + 2;
    uint64_t prime_idx_start = (*num_primes > 0 && primes[0] == 2) ? 1 : 0;

    // Fill the rest of the sieve in segments (odd numbers only)
    for (uint64_t segment_start = seg_start; segment_start <= n; segment_start += SEG_SIZE) {
        uint64_t segment_end = prime_min(segment_start + SEG_SIZE - 1, n);
        // Mark odd multiples of odd primes in the current segment
        for (uint64_t i = prime_idx_start; i < (uint64_t)(*num_primes); i++) {
            uint64_t p = primes[i];
            uint64_t start = (segment_start + p - 1) / p * p;
            if (start % 2 == 0) start += p;  // ensure first marked multiple is odd
            for (uint64_t j = start; j <= segment_end; j += 2 * p) {
                sieve[j] = 1;
            }
        }
        // Populate the remaining primes (odd only)
        for (uint64_t i = segment_start; i <= segment_end; i += 2) {
            if (!sieve[i]) {
                primes[(*num_primes)++] = i;
            }
        }
    }
}


uint64_t prime_count(uint64_t n) {
    if (n < 2) return 0;

    uint64_t sqrt_n = (uint64_t)sqrt((double)n);

    uint64_t upper_bound = approximate_upper_bound(sqrt_n);
    uint64_t *primes = malloc(upper_bound * sizeof(uint64_t));

    uint8_t *sieve = calloc(sqrt_n + 1, 1);

    // Compute primes up to sqrt(n)
    uint64_t small_num_primes = 0;
    prime_sieve(sieve, primes, &small_num_primes, sqrt_n);
    free(sieve);

    // prime_sieve only covers [2, sqrt_n]; if sqrt_n < 2, count prime 2 manually
    uint64_t num_primes = small_num_primes;
    if (sqrt_n < 2) num_primes++;

    // Segmented sieve to count odds in (sqrt_n, n] using a packed odd-only buffer.
    // segment_sieve[k] corresponds to the number segment_start + 2*k.
    // Each SEG_SIZE-byte buffer covers SEG_SIZE consecutive odd numbers (a range of 2*SEG_SIZE integers).
    uint8_t *segment_sieve = malloc(SEG_SIZE);

    uint64_t n_odd = (n % 2 == 0) ? n - 1 : n;
    uint64_t seg_start = (sqrt_n % 2 == 0) ? sqrt_n + 1 : sqrt_n + 2;
    uint64_t prime_idx_start = (small_num_primes > 0 && primes[0] == 2) ? 1 : 0;

    for (uint64_t segment_start = seg_start; segment_start <= n_odd; segment_start += 2 * SEG_SIZE) {
        uint64_t seg_end = prime_min(segment_start + 2 * (SEG_SIZE - 1), n_odd);
        uint64_t seg_len = (seg_end - segment_start) / 2 + 1;

        memset(segment_sieve, 0, seg_len);

        // Mark odd multiples of odd primes; skip p=2 (no odd multiples exist)
        for (uint64_t i = prime_idx_start; i < small_num_primes; i++) {
            uint64_t p = primes[i];
            uint64_t start = (segment_start + p - 1) / p * p;
            if (start % 2 == 0) start += p;  // ensure first marked multiple is odd
            if (start > seg_end) continue;
            for (uint64_t k = (start - segment_start) / 2; k < seg_len; k += p) {
                segment_sieve[k] = 1;
            }
        }
        for (uint64_t k = 0; k < seg_len; k++) {
            if (!segment_sieve[k]) num_primes++;
        }
    }

    free(primes);
    free(segment_sieve);
    return num_primes;
}
