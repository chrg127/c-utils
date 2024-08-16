#ifndef UTILITY_H_INCLUDED
#define UTILITY_H_INCLUDED

#define MIN(a, b)           (((a) < (b)) ? (a) : (b))
#define MAX(a, b)           (((a) > (b)) ? (a) : (b))
#define CLAMP(a, min, max)  (MIN(MAX(a, min), max))

#define SWAP(a, b) { \
        typeof(a) tmp = a; \
        a = b; \
        b = tmp; \
    }

#endif
