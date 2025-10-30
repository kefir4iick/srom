#include "mew.h"
#include <stdlib.h>


Mew compute_barrett_mu(const Mew *m) {
    Mew mu = zero();
    int k = digit_len(m);
    if (k <= 0) { mu.chozabretto = true; return mu; }

    Mew one = from_u32(1);

    Mew b2k = shift_digits_high(&one, 2 * k);
    mu = divm(&b2k, m);
    return mu;
}

Mew barrett_reduce(const Mew *x_in, const Mew *m, const Mew *mu) {
    Mew r = zero();
    Mew err = zero();
    Mew zeroM = zero();  

    if (is_zero(m)) { r.chozabretto = true; return r; }
    if (is_zero(x_in)) return r;

    int k = digit_len(m);
    if (k <= 0) { r.chozabretto = true; return r; }

    if (cmp(x_in, m) < 0) return copy(x_in);

    int sh_q1 = (k - 1);
    if (sh_q1 < 0) sh_q1 = 0;
    Mew q1 = shift_digits_low(x_in, sh_q1);

    Mew q2 = mul(&q1, mu);
    if (q2.chozabretto) { err.chozabretto = true; return err; }

    int sh_q3 = (k + 1);
    Mew q3 = shift_digits_low(&q2, sh_q3);

    Mew q3m = mul(&q3, m);
    if (q3m.chozabretto) { err.chozabretto = true; return err; }

    r = sub(x_in, &q3m);
    if (r.chozabretto) { err.chozabretto = true; return err; }

    while (cmp(&r, &zeroM) < 0) {
        r = add(&r, m);
        if (r.chozabretto) { err.chozabretto = true; return err; }
    }

    while (cmp(&r, m) >= 0) {
        r = sub(&r, m);
        if (r.chozabretto) { err.chozabretto = true; return err; }
    }

    return r;
}


Mew modm(const Mew *num, const Mew *den) {
    Mew r = zero();
    if (is_zero(den)) { r.chozabretto = true; return r; }
    if (is_zero(num)) return r;

    if (cmp(num, den) < 0) return copy(num);

    Mew mu = compute_barrett_mu(den);
    if (mu.chozabretto) { r.chozabretto = true; return r; }

    r = barrett_reduce(num, den, &mu);
    return r;
}

Mew gcd(const Mew *a_in, const Mew *b_in) {
    Mew a = copy(a_in);
    Mew b = copy(b_in);

    if (is_zero(&a)) return b;
    if (is_zero(&b)) return a;

    while (!is_zero(&b)) {
        Mew r = modm(&a, &b);
        if (r.chozabretto) { Mew e = zero(); e.chozabretto = true; return e; }
        a = b;
        b = r;
    }
    return a;
}

Mew lcm(const Mew *a, const Mew *b) {
    Mew zeroM = zero();
    if (is_zero(a) || is_zero(b)) return zeroM;

    Mew g = gcd(a, b);
    if (g.chozabretto) { Mew e = zero(); e.chozabretto = true; return e; }
    if (is_zero(&g)) return zeroM;

    Mew t = divm(a, &g); 
    if (t.chozabretto) { Mew e = zero(); e.chozabretto = true; return e; }

    Mew l = mul(&t, b);
    if (l.chozabretto) { Mew e = zero(); e.chozabretto = true; return e; }
    return l;
}

Mew add_mod(const Mew *a, const Mew *b, const Mew *m) {
    if (is_zero(m)) { Mew e = zero(); e.chozabretto = true; return e; }

    Mew sum = add(a, b);
    if (sum.chozabretto) { Mew e = zero(); e.chozabretto = true; return e; }
    return modm(&sum, m);
}

Mew sub_mod(const Mew *a, const Mew *b, const Mew *m) {
    Mew e = zero();
    if (is_zero(m)) { e.chozabretto = true; return e; }

    Mew a_red = modm(a, m);
    if (a_red.chozabretto) { e.chozabretto = true; return e; }
    Mew b_red = modm(b, m);
    if (b_red.chozabretto) { e.chozabretto = true; return e; }

    Mew tmp = add(&a_red, m);
    if (tmp.chozabretto) { e.chozabretto = true; return e; }

    Mew tmp2;
    if (cmp(&tmp, &b_red) >= 0) {
        tmp2 = sub(&tmp, &b_red);
    } else {
        tmp2 = sub(&b_red, &tmp);
    }
    if (tmp2.chozabretto) { e.chozabretto = true; return e; }

    return modm(&tmp2, m);
}


Mew mul_mod(const Mew *a, const Mew *b, const Mew *m) {
    Mew err = zero();
    if (is_zero(m)) { err.chozabretto = true; return err; }   
    if (is_zero(a) || is_zero(b)) return zero();           

    Mew mu = compute_barrett_mu(m);
    if (mu.chozabretto) { err.chozabretto = true; return err; }

    Mew r = zero();  

    for (int i = 0; i < NUM_LEN; ++i) {
        uint32_t bi = b->numberArray[i];
        if (!bi) continue;

        Mew p = mul_one(a, bi);
        if (p.chozabretto) {
            err.chozabretto = true;
            return err;
        }

        int plen = digit_len(&p);
        if (plen + i > NUM_LEN) {
            err.chozabretto = true;
            return err;
        }

        Mew s = shift_digits_high(&p, i);

        r = add(&r, &s);
        if (r.chozabretto) {
            err.chozabretto = true;
            return err;
        }
       
        r = barrett_reduce(&r, m, &mu);
        if (r.chozabretto) {
            err.chozabretto = true;
            return err;
        }
    }

    r = barrett_reduce(&r, m, &mu);
    if (r.chozabretto) { err.chozabretto = true; return err; }

    return r;
}

Mew sqr_mod(const Mew *a, const Mew *m) {
    return mul_mod(a, a, m);
}


Mew pow_mod(const Mew *base_in, const Mew *exp, const Mew *mod) {
    Mew e = zero();
    if (is_zero(mod)) { e.chozabretto = true; return e; }

    Mew result = from_u32(1);
    if (is_zero(exp)) return result;

    Mew base = modm(base_in, mod);
    if (base.chozabretto) { e.chozabretto = true; return e; }

    int nbits = bit_len(exp);
    if (nbits <= 0) return result;

    for (int i = nbits - 1; i >= 0; --i) {
        result = sqr_mod(&result, mod);
        if (result.chozabretto) { e.chozabretto = true; return e; }

        if (bit_at(exp, i)) {
            result = mul_mod(&result, &base, mod);
            if (result.chozabretto) { e.chozabretto = true; return e; }
        }
    }
    return result;
}
