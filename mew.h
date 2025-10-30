#ifndef MEW_H
#define MEW_H

#include <stdint.h>
#include <stdbool.h>

#define NUM_LEN 256

typedef struct {
    uint32_t numberArray[NUM_LEN];
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
Mew shift_digits_high(const Mew *a, int shift);
Mew shift_digits_low(const Mew *a, int shift);


Mew add(const Mew *a, const Mew *b);
Mew sub(const Mew *a, const Mew *b);
Mew mul_one(const Mew *a, uint32_t b);
Mew mul(const Mew *a, const Mew *b);
Mew sqr(const Mew *a);
Mew divm(const Mew *num, const Mew *den);
Mew powm(const Mew *base, const Mew *exp);

Mew modm(const Mew *num, const Mew *den);
Mew gcd(const Mew *a, const Mew *b);
Mew lcm(const Mew *a, const Mew *b);

Mew add_mod(const Mew *a, const Mew *b, const Mew *m);
Mew sub_mod(const Mew *a, const Mew *b, const Mew *m);
Mew mul_mod(const Mew *a, const Mew *b, const Mew *m);
Mew sqr_mod(const Mew *a, const Mew *m);
Mew pow_mod(const Mew *base, const Mew *exp, const Mew *mod);

Mew compute_barrett_mu(const Mew *m);
Mew barrett_reduce(const Mew *x, const Mew *m, const Mew *mu);


#endif 
