#ifndef __UTILS_H_
#define __UTILS_H_

#include <stddef.h>
#include <stdint.h>

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

#endif
