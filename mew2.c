#include "mew.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>



static Mew abs_mew(const Mew *a) {
    Mew r = copy(a);
    r.negative = false;
    return r;
}

Mew modm(const Mew *a, const Mew *mod) {
    Mew r = zero();
    if (!a || !mod) { r.chozabretto = true; return r; }
    if (a->chozabretto || mod->chozabretto) { r.chozabretto = true; return r; }
    if (is_zero(mod)) { r.chozabretto = true; return r; }

    Mew aa = abs_mew(a);
    Mew mm = abs_mew(mod);

    Mew q = divm(&aa, &mm);
    if (q.chozabretto) { r.chozabretto = true; return r; }

    Mew prod = mul(&q, &mm);
    Mew rem  = sub(&aa, &prod);

    rem.negative = false;

    if (a->negative && !is_zero(&rem)) {
        rem = sub(&mm, &rem);
        rem.negative = false;
    }

    return rem;
}


Mew mod_add(const Mew *a, const Mew *b, const Mew *mod) {
    Mew r = zero();
    if (!a || !b || !mod) { r.chozabretto = true; return r; }
    if (a->chozabretto || b->chozabretto || mod->chozabretto) { r.chozabretto = true; return r; }
    if (is_zero(mod)) { r.chozabretto = true; return r; }

    Mew sum = add(a, b);
    if (sum.chozabretto) { r.chozabretto = true; return r; }
    return modm(&sum, mod);
}


Mew mod_subtract(const Mew *a, const Mew *b, const Mew *mod) {
    Mew r = zero();
    if (!a || !b || !mod) { r.chozabretto = true; return r; }
    if (a->chozabretto || b->chozabretto || mod->chozabretto) { r.chozabretto = true; return r; }
    if (is_zero(mod)) { r.chozabretto = true; return r; }

    Mew am = modm(a, mod);
    Mew bm = modm(b, mod);
    if (am.chozabretto || bm.chozabretto) { r.chozabretto = true; return r; }

    Mew diff = sub(&am, &bm);
    Mew mm = abs_mew(mod);

    if (diff.negative) {
        diff = sub(&mm, &diff);
        diff.negative = false;
    }

    return modm(&diff, &mm);
}



Mew barrett_mu(const Mew *mod) {
    Mew r = zero();
    if (!mod) { r.chozabretto = true; return r; }
    if (mod->chozabretto) { r.chozabretto = true; return r; }
    if (is_zero(mod)) { r.chozabretto = true; return r; }

    Mew mm = abs_mew(mod);
    int k = digit_len(&mm);
    if (k <= 0) { r.chozabretto = true; return r; }

    Mew beta = from_u32(1);
    beta = shift_digits_high(&beta, 2 * k);

    Mew mu = divm(&beta, &mm);
    if (mu.chozabretto || is_zero(&mu)) {
        mu.chozabretto = true;
        return mu;
    }
    return mu;
}



Mew barrett_reduction(const Mew *x, const Mew *mod, const Mew *mu) {
    Mew r = zero();
    if (!x || !mod || !mu) { r.chozabretto = true; return r; }
    if (x->chozabretto || mod->chozabretto || mu->chozabretto) { r.chozabretto = true; return r; }
    if (is_zero(mod)) { r.chozabretto = true; return r; }

    Mew mm = abs_mew(mod);
    Mew xx = abs_mew(x);

    if (cmp(&xx, &mm) < 0) return xx;

    int k = digit_len(&mm);
    if (k <= 0) { r.chozabretto = true; return r; }

    Mew Q = shift_digits_low(&xx, k - 1);

    Q = mul(&Q, mu);
    if (Q.chozabretto) { r.chozabretto = true; return r; }

    Q = shift_digits_low(&Q, k + 1);

    Q = mul(&Q, &mm);
    if (Q.chozabretto) { r.chozabretto = true; return r; }

    Mew R = sub(&xx, &Q);

    if (R.negative) {
        return modm(&xx, &mm);
    }

    if (cmp(&R, &mm) >= 0) {
        R = modm(&R, &mm);
    }

    return R;
}


Mew mod_multiply(const Mew *a, const Mew *b, const Mew *mod) {
    Mew r = zero();
    if (!a || !b || !mod) { r.chozabretto = true; return r; }
    if (a->chozabretto || b->chozabretto || mod->chozabretto) { r.chozabretto = true; return r; }
    if (is_zero(mod)) { r.chozabretto = true; return r; }

    Mew mu = barrett_mu(mod);
    if (mu.chozabretto) { r.chozabretto = true; return r; }

    Mew prod = mul(a, b);
    if (prod.chozabretto) { r.chozabretto = true; return r; }

    return barrett_reduction(&prod, mod, &mu);
}


Mew mod_square(const Mew *a, const Mew *mod) {
    return mod_multiply(a, a, mod);
}


Mew mod_pow_barrett(const Mew *base, const Mew *exp, const Mew *mod) {
    Mew r = zero();
    if (!base || !exp || !mod) { r.chozabretto = true; return r; }
    if (base->chozabretto || exp->chozabretto || mod->chozabretto) { r.chozabretto = true; return r; }
    if (is_zero(mod)) { r.chozabretto = true; return r; }

    Mew mu = barrett_mu(mod);
    if (mu.chozabretto) { r.chozabretto = true; return r; }

    Mew b = barrett_reduction(base, mod, &mu);

    Mew result = from_u32(1);

    int nbits = bit_len(exp);
    for (int i = nbits - 1; i >= 0; --i) {
        result = mul(&result, &result);
        if (result.chozabretto) { r.chozabretto = true; return r; }
        result = barrett_reduction(&result, mod, &mu);

        if (bit_at(exp, i)) {
            result = mul(&result, &b);
            if (result.chozabretto) { r.chozabretto = true; return r; }
            result = barrett_reduction(&result, mod, &mu);
        }
    }

    return result;
}







Mew gcd(const Mew *a, const Mew *b) {
    Mew x = copy(a);
    Mew y = copy(b);

    if (is_zero(&x)) return y;
    if (is_zero(&y)) return x;

    while (!is_zero(&y)) {
        Mew r = modm(&x, &y);
        if (r.chozabretto) return r;
        x = y;
        y = r;
    }
    return x;
}

Mew lcm(const Mew *a, const Mew *b) {
    Mew err = zero();
    if (is_zero(a) || is_zero(b)) {
        err.chozabretto = true;
        return err;
    }

    Mew g = gcd(a, b);
    if (g.chozabretto) return g;

    Mew t = divm(a, &g);
    if (t.chozabretto) return t;

    Mew r = mul(&t, b);
    return r;
}





static Mew random_below(const Mew *n) {
    Mew r = zero();
    int bits = bit_len(n);

    do {
        for (int i = 0; i < NUM_LEN; ++i)
            r.numberArray[i] =
                ((uint32_t)rand() << 16) ^ (uint32_t)rand();

        int excess = NUM_LEN * 32 - bits;
        if (excess > 0) {
            uint32_t mask = (bits % 32 == 0)
                ? 0xFFFFFFFFu
                : ((1u << (bits % 32)) - 1);
            r.numberArray[bits / 32] &= mask;
            for (int i = bits / 32 + 1; i < NUM_LEN; ++i)
                r.numberArray[i] = 0;
        }
    } while (cmp(&r, n) >= 0 || is_zero(&r));

    return r;
}

static Mew random_base(const Mew *n) {
    Mew three = from_u32(3);
    Mew nm3 = sub(n, &three);
    Mew r = random_below(&nm3);
    Mew two = from_u32(2);
    return add(&r, &two);
}

bool miller_rabin(const Mew *n, int rounds) {
    if (!n || n->chozabretto) return false;

    Mew zero_m = zero();
    Mew one  = from_u32(1);
    Mew two  = from_u32(2);
    Mew four = from_u32(4);

    if (cmp(n, &two) < 0) return false;
    if (cmp(n, &four) < 0) return true;
    if (is_even(n)) return false;

    Mew n_minus_1 = sub(n, &one);
    Mew d = copy(&n_minus_1);
    int s = 0;

    while (is_even(&d)) {
        d = shift_right(&d, 1);
        s++;
    }

    for (int i = 0; i < rounds; ++i) {
        Mew a = random_base(n);

        Mew x = mod_pow_barrett(&a, &d, n);
        if (x.chozabretto) return false;

        if (cmp(&x, &one) == 0 || cmp(&x, &n_minus_1) == 0)
            continue;

        bool composite = true;
        for (int r = 1; r < s; ++r) {
            x = mod_square(&x, n);
            if (x.chozabretto) return false;

            if (cmp(&x, &n_minus_1) == 0) {
                composite = false;
                break;
            }
        }

        if (composite) return false;
    }

    return true;
}
