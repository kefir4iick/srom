#include "mew.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int hex_to_digit(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

Mew zero(void) {
    Mew z;
    memset(&z, 0, sizeof(Mew));
    return z;
}

Mew newm(void) { return zero(); }

Mew from_u32(uint32_t n) {
    Mew r = zero();
    r.numberArray[0] = n;
    return r;
}

Mew from_hex(const char *hex) {
    Mew res = zero();
    if (!hex) { res.chozabretto = true; return res; }

    int len = (int)strlen(hex);
    for (int i = 0; i < len; ++i) {
        int d = hex_to_digit(hex[len - 1 - i]);
        if (d < 0) { res.chozabretto = true; return res; }

        int shift = (i % 8) * 4;
        int idx = i / 8;
        if (idx >= NUM_LEN) { res.chozabretto = true; return res; }

        res.numberArray[idx] |= ((uint32_t)d << shift);
    }
    return res;
}

char *to_hex(const Mew *a) {
    if (a == NULL) {
        return strdup("error");
    }
    if (a->chozabretto) {
        return strdup("error");
    }

    bool allZero = true;
    for (int i = 0; i < NUM_LEN; ++i) {
        if (a->numberArray[i] != 0) {
            allZero = false;
            break;
        }
    }
    if (allZero) {
        return strdup("0");
    }

    size_t bufSize = NUM_LEN * 8 + 1;
    char *buf = calloc(bufSize, 1);
    if (!buf) {
        return strdup("error");
    }

    char *p = buf;
    for (int i = NUM_LEN - 1; i >= 0; --i) {
        p += sprintf(p, "%08x", a->numberArray[i]);
    }

    char *start = buf;
    while (*start == '0' && *(start + 1)) start++;

    char *result = strdup(start);
    free(buf);

    if (!result) {
        return strdup("error");
    }
    return result;
}



Mew copy(const Mew *a) {
    Mew r = zero();
    memcpy(r.numberArray, a->numberArray, sizeof(a->numberArray));
    r.negative = a->negative;
    r.chozabretto = a->chozabretto;
    return r;
}


bool is_zero(const Mew *a) {
    for (int i = 0; i < NUM_LEN; ++i)
        if (a->numberArray[i]) return false;
    return true;
}

int digit_len(const Mew *a) {
    for (int i = NUM_LEN - 1; i >= 0; --i)
        if (a->numberArray[i]) return i + 1;
    return 0;
}

int bit_len(const Mew *a) {
    for (int i = NUM_LEN - 1; i >= 0; --i) {
        if (a->numberArray[i]) {
            for (int bit = 31; bit >= 0; --bit)
                if (a->numberArray[i] & (1u << bit))
                    return i * 32 + bit + 1;
        }
    }
    return 0;
}

uint32_t bit_at(const Mew *a, int i) {
    if (i < 0) return 0;
    int idx = i / 32;
    int sh = i % 32;
    if (idx >= NUM_LEN) return 0;
    return (a->numberArray[idx] >> sh) & 1u;
}

bool is_even(const Mew *a) {
    return !(a->numberArray[0] & 1u);
}

int cmp(const Mew *a, const Mew *b) {
    for (int i = NUM_LEN - 1; i >= 0; --i) {
        if (a->numberArray[i] > b->numberArray[i]) return 1;
        if (a->numberArray[i] < b->numberArray[i]) return -1;
    }
    return 0;
}

void print_hex(const Mew *a) {
    char *s = to_hex(a);
    printf("%s", s);
    free(s);
}


Mew shift_left(const Mew *a, int bits) {
    Mew r = zero();
    if (bits <= 0) return copy(a);

    int ds = bits / 32;
    int bs = bits % 32;
    if (ds >= NUM_LEN) return r;

    uint32_t carry = 0;
    for (int i = 0; i < NUM_LEN - ds; ++i) {
        uint64_t cur = ((uint64_t)a->numberArray[i] << bs) | carry;
        r.numberArray[i + ds] = (uint32_t)cur;
        carry = (bs == 0) ? 0 : (a->numberArray[i] >> (32 - bs));
    }
    if (carry && ds + NUM_LEN < NUM_LEN) r.chozabretto = true;
    return r;
}

Mew shift_right(const Mew *a, int bits) {
    Mew r = zero();
    if (bits <= 0) return copy(a);

    int ds = bits / 32;
    int bs = bits % 32;
    if (ds >= NUM_LEN) return r;

    for (int i = ds; i < NUM_LEN; ++i) {
        uint64_t cur = a->numberArray[i];
        uint64_t part = cur >> bs;
        if (bs && i + 1 < NUM_LEN)
            part |= ((uint64_t)a->numberArray[i + 1] << (32 - bs));
        r.numberArray[i - ds] = (uint32_t)part;
    }
    return r;
}

Mew shift_digits_high(const Mew *a, int s) {
    Mew r = zero();
    if (s <= 0) return copy(a);
    if (s >= NUM_LEN) return r;
    for (int i = NUM_LEN - 1; i >= s; --i)
        r.numberArray[i] = a->numberArray[i - s];
    return r;
}

Mew shift_digits_low(const Mew *a, int s) {
    Mew r = zero();
    if (s <= 0) return copy(a);
    if (s >= NUM_LEN) return r;
    for (int i = 0; i < NUM_LEN - s; ++i)
        r.numberArray[i] = a->numberArray[i + s];
    return r;
}






Mew add(const Mew *a, const Mew *b) {
    Mew r = zero();
    uint64_t carry = 0;
    for (int i = 0; i < NUM_LEN; ++i) {
        uint64_t sum = (uint64_t)a->numberArray[i] + b->numberArray[i] + carry;
        r.numberArray[i] = (uint32_t)sum;
        carry = sum >> 32;
    }
    if (carry) r.chozabretto = true;
    return r;
}

Mew sub(const Mew *a, const Mew *b) {
    Mew r = zero();

    int c = cmp(a, b);
    if (c == 0) {
        return r;
    }

    const Mew *x = a;
    const Mew *y = b;

    if (c < 0) {
        r.negative = true;
        x = b;
        y = a;
    }

    uint64_t borrow = 0;
    for (int i = 0; i < NUM_LEN; ++i) {
        uint64_t ai = x->numberArray[i];
        uint64_t bi = y->numberArray[i];
        uint64_t diff;

        if (ai >= bi + borrow) {
            diff = ai - bi - borrow;
            borrow = 0;
        } else {
            diff = ((uint64_t)1 << 32) + ai - bi - borrow;
            borrow = 1;
        }
        r.numberArray[i] = (uint32_t)diff;
    }

    if (is_zero(&r)) r.negative = false;

    return r;
}


Mew mul_one(const Mew *a, uint32_t b) {
    Mew r = zero();
    uint64_t carry = 0;
    for (int i = 0; i < NUM_LEN; ++i) {
        uint64_t prod = (uint64_t)a->numberArray[i] * b + carry;
        r.numberArray[i] = (uint32_t)prod;
        carry = prod >> 32;
    }
    if (carry) r.chozabretto = true;
    return r;
}

Mew mul(const Mew *a, const Mew *b) {
    Mew r = zero();
    for (int i = 0; i < NUM_LEN; ++i) {
        if (!b->numberArray[i]) continue;
        Mew t = mul_one(a, b->numberArray[i]);
        Mew s = shift_digits_high(&t, i);
        r = add(&r, &s);
        if (r.chozabretto) break;
    }
    return r;
}

Mew sqr(const Mew *a) { return mul(a, a); }

static void set_bit(Mew *q, int bit) {
    if (bit < 0) return;
    int idx = bit / 32, sh = bit % 32;
    if (idx < NUM_LEN) q->numberArray[idx] |= (1u << sh);
}

Mew divm(const Mew *num, const Mew *den) {
    Mew q = zero();
    if (is_zero(den)) { q.chozabretto = true; return q; }

    Mew rem = copy(num);
    Mew d = copy(den);
    int s = bit_len(&rem) - bit_len(&d);
    if (s < 0) return q;

    Mew sh = shift_left(&d, s);
    for (int i = s; i >= 0; --i) {
        if (cmp(&rem, &sh) >= 0) {
            rem = sub(&rem, &sh);
            set_bit(&q, i);
        }
        sh = shift_right(&sh, 1);
    }
    return q;
}

Mew powm(const Mew *base, const Mew *exp) {
    Mew r = from_u32(1);
    Mew b = copy(base);
    int n = bit_len(exp);
    for (int i = 0; i < n; ++i) {
        if (bit_at(exp, i)) r = mul(&r, &b);
        b = mul(&b, &b);
    }
    return r;
}
