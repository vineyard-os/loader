#pragma once

#include <stddef.h>
#include <stdint.h>

void *memcpy(void * restrict s1, const void * restrict s2, size_t n) __attribute__((nonnull(1, 2)));
int memcmp(const void *str1, const void *str2, size_t count) __attribute__((nonnull(1, 2)));
void *memset(void *s, int c, size_t n) __attribute__((nonnull(1)));
size_t strlen(const char *str) __attribute__((nonnull(1)));
int strncmp(const char *s1, const char *s2, size_t n) __attribute__((nonnull(1, 2)));
