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

Mew barrett_reduce(const Mew *x, const Mew *m, const Mew *mu) {
    if (cmp(x, m) < 0)
        return copy(x);

    int k = digit_len(m);

    Mew q1 = shift_digits_low(x, k - 1);
    Mew q2 = mul(&q1, mu);
    if (q2.chozabretto) return q2;

    Mew q3 = shift_digits_low(&q2, k + 1);
    Mew q3m = mul(&q3, m);
    if (q3m.chozabretto) return q3m;

    Mew r = sub(x, &q3m);

    if (cmp(&r, m) >= 0)
        r = sub(&r, m);
    if (cmp(&r, m) >= 0)
        r = sub(&r, m);

    return r;
}

Mew modm(const Mew *a, const Mew *m) {
    if (cmp(a, m) < 0)
        return copy(a);

    Mew mu = compute_barrett_mu(m);
    if (mu.chozabretto) return mu;

    return barrett_reduce(a, m, &mu);
}



Mew mul_mod(const Mew *a, const Mew *b, const Mew *m) {
    Mew p = mul(a, b);
    if (p.chozabretto) return p;
    return modm(&p, m);
}

Mew sqr_mod(const Mew *a, const Mew *m) {
    return mul_mod(a, a, m);
}




static int mont_k(const Mew *n) {
    return bit_len(n);
}


static Mew mont_compute_ninv(const Mew *n, int k) {
    Mew r = shift_left(&(Mew){ .numberArray = {1}, .chozabretto = false }, k);
    Mew t = modm(n, &r);
    Mew ninv = sub(&r, &t);
    return ninv;
}

static Mew mont_reduce(const Mew *t, const Mew *n, const Mew *n_inv, int k) {
    Mew m = mul(t, n_inv);
    m = shift_right(&m, k);

    Mew mn = mul(&m, n);
    Mew u = sub(t, &mn);

    if (cmp(&u, n) >= 0)
        u = sub(&u, n);

    return u;
}

static Mew mont_mul(const Mew *a, const Mew *b,
                    const Mew *n, const Mew *n_inv, int k) {
    Mew t = mul(a, b);
    if (t.chozabretto) return t;
    return mont_reduce(&t, n, n_inv, k);
}



Mew pow_mod(const Mew *base, const Mew *exp, const Mew *mod) {
    Mew err = zero();
    if (is_zero(mod)) { err.chozabretto = true; return err; }

    int k = mont_k(mod);
    Mew n_inv = mont_compute_ninv(mod, k);

    Mew one = from_u32(1);

    Mew r = shift_left(&one, k);
    Mew a = mul_mod(base, &r, mod);
    if (a.chozabretto) return a;

    Mew x = modm(&r, mod);
    if (x.chozabretto) return x;

    int nbits = bit_len(exp);
    for (int i = nbits - 1; i >= 0; --i) {
        x = mont_mul(&x, &x, mod, &n_inv, k);
        if (x.chozabretto) return x;

        if (bit_at(exp, i)) {
            x = mont_mul(&x, &a, mod, &n_inv, k);
            if (x.chozabretto) return x;
        }
    }

    return mont_mul(&x, &one, mod, &n_inv, k);
}

Mew add_mod(const Mew *a, const Mew *b, const Mew *m) {
    Mew err = zero();
    if (is_zero(m)) { err.chozabretto = true; return err; }

    Mew s = add(a, b);
    if (s.chozabretto) return s;

    return modm(&s, m);
}

Mew sub_mod(const Mew *a, const Mew *b, const Mew *m) {
    Mew err = zero();
    if (is_zero(m)) { err.chozabretto = true; return err; }

    Mew ar = modm(a, m);
    if (ar.chozabretto) return ar;

    Mew br = modm(b, m);
    if (br.chozabretto) return br;

    Mew r;
    if (cmp(&ar, &br) >= 0) {
        r = sub(&ar, &br);
    } else {
        Mew t = sub(&br, &ar);
        r = sub(m, &t);
    }
    return r;
}
