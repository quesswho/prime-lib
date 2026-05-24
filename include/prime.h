#pragma once
#include <stddef.h>
#include <stdint.h>

/**
 * Returns an upper bound on the number of primes up to n, used for pre-allocating the primes array.
 * 
 * For n >= 355991, we use the upper bound by Pierre Dusart (1998)
 * 
 * For smaller n, we can just use a fixed size (for now)
 */
uint64_t approximate_upper_bound(uint64_t n);

/**
 * Computes all primes up to n using a segmented sieve of Eratosthenes.
 *
 * The sieve works in 65536-byte segments to stay within L1 cache.
 *
 * @param sieve      Needs to be pre-allocated and zero-initialized array of at least
 *                   n + 1 bytes. Index i is set to 1 if i is composite.
 * @param primes     Caller-allocated array that receives each prime found.
 *                   Must have capacity for all primes <= n; use
 *                   approximate_upper_bound(n) to get an upper bound on the number of primes.
 * @param num_primes Pointer to a counter initialised to 0 by the caller.
 *                   Incremented once for every prime appended to primes[].
 * @param n          Upper bound for prime generation.
 */
void prime_sieve(uint8_t *sieve, uint64_t *primes, uint64_t *num_primes, uint64_t n);

/**
 * Counts all primes in [2, n].
 *
 * Internally allocates a sieve up to sqrt(n), then counts primes in the
 * remaining range [sqrt(n)+1, n] with a segmented sieve. All memory is
 * managed internally and freed before returning.
 *
 * @param n  Upper bound (inclusive).
 * @return   Number of primes in [2, n].
 */
uint64_t prime_count(uint64_t n);
