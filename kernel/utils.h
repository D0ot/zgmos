#ifndef __UTILS_H_
#define __UTILS_H_

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

// if match, return 1
// if not match, return 0
uint8_t strmatch(const char *s1, const char *s2, uint64_t count);

char *reverse(char *buffer, size_t i, size_t j);
uint8_t checksum(uint8_t *p, uint64_t count);
// if match, return 1
// if not match, return 0
uint8_t strmatch(const char *s1, const char *s2, uint64_t count);

// find the next number which is times of n
uint64_t nextntimes(uint64_t num, uint64_t n);

uint64_t max(uint64_t a, uint64_t b);
uint64_t min(uint64_t a, uint64_t b);

char *itoa(long value, char *str, int base);
char *utoa(unsigned long value, char *str, unsigned base);
void swap(char *x, char *y);
int abs(int value);

uint64_t align_to(uint64_t x, uint64_t a);

bool is_power_of_2(uint64_t n);

#define POWER_OF_2(p) (((uint64_t)1 << (p)))

// n is number of '1' in binary number
// ALL_ONE_MASK(12) = POWER_OF_2(12) - 1
//                  = 4096 - 1
//                  = 4095 = 1111_1111_1111b
#define ALL_ONE_MASK(n) ( POWER_OF_2(n) - 1 )

#define ALIGN_XBITS(a, n) ( ((uint64_t)(a)) & ( ~ALL_ONE_MASK(n) ) )

#define ALIGN_4K(a) (ALIGN_XBITS(a, 12))
#define ALIGN_2M(a) (ALIGN_XBITS(a, 21))
#define ALIGN_1G(a) (ALIGN_XBITS(a, 30))


#endif
