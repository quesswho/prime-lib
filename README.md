# prime-lib

A fast segmented prime sieve written in C.

## Features

- **`prime_sieve`** — generates all primes up to `n`, storing them in a caller-supplied array
- **`prime_count`** — counts all primes up to `n` with fully managed memory

The sieve processes numbers in 64 KiB segments to stay within L1 cache. Even numbers are skipped entirely, halving the work.

## Build

Requires GCC.

```sh
make
```

This produces `example/main.exe`, which prints the number of primes up to 1,000,000,000.

```sh
./example/main.exe
# 50847534
```

## API

```c
#include "prime.h"

// Upper bound on the count of primes <= n
uint64_t approximate_upper_bound(uint64_t n);

// Fill primes[] with all primes <= n; increment *num_primes for each one found
// sieve must be a zeroed array of at least n+1 bytes
void prime_sieve(uint8_t *sieve, uint64_t *primes, uint64_t *num_primes, uint64_t n);

// Count all primes in [2, n]; allocates and frees its own memory
uint64_t prime_count(uint64_t n);
```

## Project structure

```
include/   public headers (prime.h, utils.h)
src/       implementation (prime.c)
example/   usage example (main.c)
```
