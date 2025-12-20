#ifndef MEW_H
#define MEW_H

#include <stdint.h>
#include <stdbool.h>

#define NUM_LEN 256

typedef struct {
    uint32_t numberArray[NUM_LEN];
    bool negative;
    bool chozabretto;
} Mew;


Mew      zero(void);
Mew      newm(void);
Mew      from_u32(uint32_t n);
Mew      from_hex(const char *hex);

char*    to_hex(const Mew *a);

Mew      copy(const Mew *a);
bool     is_zero(const Mew *a);
int      digit_len(const Mew *a);
int      bit_len(const Mew *a);
uint32_t bit_at(const Mew *a, int i);
bool     is_even(const Mew *a);
int      cmp(const Mew *a, const Mew *b);
void     print_hex(const Mew *a);

Mew shift_left(const Mew *a, int bits);
Mew shift_right(const Mew *a, int bits);
Mew shift_digits_high(const Mew *a, int shift_words);
Mew shift_digits_low(const Mew *a, int shift_words);

Mew add(const Mew *a, const Mew *b);
Mew sub(const Mew *a, const Mew *b);
Mew mul_one(const Mew *a, uint32_t b);
Mew mul(const Mew *a, const Mew *b);
Mew sqr(const Mew *a);

Mew divm(const Mew *num, const Mew *den);
Mew modm(const Mew *num, const Mew *den);

Mew powm(const Mew *base, const Mew *exp);

Mew gcd(const Mew *a, const Mew *b);
Mew lcm(const Mew *a, const Mew *b);

Mew modm(const Mew *a, const Mew *mod);
Mew mod_add(const Mew *a, const Mew *b, const Mew *mod);
Mew mod_subtract(const Mew *a, const Mew *b, const Mew *mod);

Mew barrett_mu(const Mew *mod);
Mew barrett_reduction(const Mew *x, const Mew *mod, const Mew *mu);

Mew mod_multiply(const Mew *a, const Mew *b, const Mew *mod);
Mew mod_square(const Mew *a, const Mew *mod);
Mew mod_pow_barrett(const Mew *base, const Mew *exp, const Mew *mod);

#endif
