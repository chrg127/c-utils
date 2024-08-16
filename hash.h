#include <stdint.h>

#define FNV64_OFFSET 14695981039346656037UL
#define FNV64_PRIME 1099511628211UL

#define FNV32_OFFSET 2166136261u
#define FNV32_PRIME 16777619

static uint32_t fnv_1a_32(const char *str, size_t len)
{
    uint32_t hash = FNV32_OFFSET;
    for (size_t i = 0; i < len; i++) {
        hash ^= (uint32_t) (unsigned char) str[i];
        hash *= FNV32_PRIME;
    }
    return hash;
}

static uint32_t fnv_1a_32_cstring(const char *str)
{
    uint32_t hash = FNV32_OFFSET;
    for (const char *p = str; *p; p++) {
        hash ^= (uint32_t) (unsigned char) (*p);
        hash *= FNV32_PRIME;
    }
    return hash;
}

static uint64_t fnv_1a_64(const char *str, size_t len)
{
    uint64_t hash = FNV64_OFFSET;
    for (size_t i = 0; i < len; i++) {
        hash ^= (uint64_t) (unsigned char) str[i];
        hash *= FNV64_PRIME;
    }
    return hash;
}

uint64_t fnv_1a_64_cstring(const char *str)
{
    uint64_t hash = FNV64_OFFSET;
    for (const char *p = str; *p; p++) {
        hash ^= (uint64_t)(unsigned char)(*p);
        hash *= FNV64_PRIME;
    }
    return hash;
}

