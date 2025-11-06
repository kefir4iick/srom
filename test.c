#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mew.h"

static void expect(const char *label, const char *got, const char *want) {
    if (strcmp(got, want)) {
        fprintf(stderr, "ne ok %s: got %s expected %s\n", label, got, want);
        exit(1);
    } else printf("ok   %s = %s\n", label, got);
}

static void expect_mew(const char *label, const Mew *got, const Mew *want) {
    char *got_hex = to_hex(got);
    char *want_hex = to_hex(want);
    expect(label, got_hex, want_hex);
    free(got_hex);
    free(want_hex);
}

int main(void) {
    Mew a = from_hex("ff");
    Mew b = from_hex("1");
    Mew s = add(&a, &b);
    char *hs = to_hex(&s);
    expect("add 0xff+1", hs, "100");
    free(hs);

    Mew x = from_hex("1234");
    Mew y = from_hex("10");
    Mew q = divm(&x, &y);
    char *hq = to_hex(&q);
    expect("div 0x1234 / 0x10", hq, "123");
    free(hq);

    Mew m1 = from_hex("abcdef");
    Mew m2 = from_hex("2");
    Mew p = mul(&m1, &m2);
    char *hp = to_hex(&p);
    expect("mul", hp, "1579bde");
    free(hp);

    printf("\n=== distributivity ===\n");
    Mew a1 = from_hex("123456789");
    Mew b1 = from_hex("abcdef");
    Mew c1 = from_hex("1000");
    
    Mew sum_ab = add(&a1, &b1);
    Mew left_side = mul(&sum_ab, &c1);
    
    Mew right_side1 = mul(&c1, &sum_ab);
    
    Mew ac = mul(&a1, &c1);
    Mew bc_val = mul(&b1, &c1);
    Mew right_side2 = add(&ac, &bc_val);
    
    expect_mew("(a+b)*c == c*(a+b)", &left_side, &right_side1);
    expect_mew("(a+b)*c == a*c + b*c", &left_side, &right_side2);
    expect_mew("c*(a+b) == a*c + b*c", &right_side1, &right_side2);

    printf("\n=== n*a == a+a+...+a (n times) ===\n");
    Mew base = from_hex("5");
    Mew n_times = from_u32(100);
    
    Mew mul_result = mul(&n_times, &base);
    
    Mew sum_result = zero();
    for (int i = 0; i < 100; i++) {
        Mew temp = add(&sum_result, &base);
        sum_result = temp;
    }
    
    expect_mew("100 * 5 == 5+5+...+5 (100 times)", &mul_result, &sum_result);

    printf("\n=== associativity ===\n");
    Mew a3 = from_hex("123456789abcdef");
    Mew b3 = from_hex("fedcba987654321");
    Mew c3 = from_hex("13579bdf2468ace");
    
    Mew ab = add(&a3, &b3);
    Mew left_assoc = add(&ab, &c3);
    
    Mew bc_sum = add(&b3, &c3);
    Mew right_assoc = add(&a3, &bc_sum);
    
    expect_mew("(a+b)+c == a+(b+c)", &left_assoc, &right_assoc);

    printf("\n=== commutativity ===\n");
    Mew a4 = from_hex("123456789");
    Mew b4 = from_hex("abcdef");
    
    Mew ab_mul = mul(&a4, &b4);
    Mew ba_mul = mul(&b4, &a4);
    
    expect_mew("a*b == b*a", &ab_mul, &ba_mul);

    printf("\n=== square ===\n");
    Mew a5 = from_hex("12345");
    
    Mew sqr_result = sqr(&a5);
    Mew mul_sqr = mul(&a5, &a5);
    
    expect_mew("a² == a*a", &sqr_result, &mul_sqr);

    printf("\n=== n multiplication ===\n");
    Mew base6 = from_hex("2");
    Mew n_large = from_u32(500);
    
    Mew mul_large = mul(&n_large, &base6);
    
    char *large_result = to_hex(&mul_large);
    expect("500 * 2 = 1000 (0x3e8)", large_result, "3e8");
    free(large_result);

    printf("\n=== distributivity ===\n");
    Mew a7 = from_hex("ffffffff");
    Mew b7 = from_hex("aaaaaaaa");
    Mew c7 = from_hex("55555555");
    
    Mew sum7 = add(&a7, &b7);
    Mew left7 = mul(&sum7, &c7);
    
    Mew ac7 = mul(&a7, &c7);
    Mew bc7_val = mul(&b7, &c7);
    Mew right7 = add(&ac7, &bc7_val);
    
    expect_mew("distributive law with large numbers", &left7, &right7);



    printf("\n=== modular arithmetic ===\n");
    
    Mew mod_base = from_hex("d");
    Mew a8 = from_hex("10");
    Mew b8 = from_hex("7");
    
    Mew mod_result = modm(&a8, &mod_base);
    char *mod_str = to_hex(&mod_result);
    expect("16 mod 13 = 3", mod_str, "3");
    free(mod_str);
    
    Mew add_mod_result = add_mod(&a8, &b8, &mod_base);
    char *add_mod_str = to_hex(&add_mod_result);
    expect("(16 + 7) mod 13 = 10", add_mod_str, "a");
    free(add_mod_str);
    
    Mew sub_mod_result = sub_mod(&a8, &b8, &mod_base);
    char *sub_mod_str = to_hex(&sub_mod_result);
    expect("(16 - 7) mod 13 = 9", sub_mod_str, "9");
    free(sub_mod_str);
    
    Mew mul_mod_result = mul_mod(&a8, &b8, &mod_base);
    char *mul_mod_str = to_hex(&mul_mod_result);
    expect("(16 * 7) mod 13 = 8", mul_mod_str, "8");
    free(mul_mod_str);
    
    Mew base9 = from_hex("3");
    Mew exp9 = from_hex("4");
    Mew mod9 = from_hex("b");
    
    Mew pow_mod_result = pow_mod(&base9, &exp9, &mod9);
    char *pow_mod_str = to_hex(&pow_mod_result);
    expect("3^4 mod 11 = 4", pow_mod_str, "4");
    free(pow_mod_str);
    
    Mew a10 = from_hex("5");
    Mew mod10 = from_hex("13");
    
    Mew sqr_mod_result = sqr_mod(&a10, &mod10);
    char *sqr_mod_str = to_hex(&sqr_mod_result);
    expect("5^2 mod 19 = 6", sqr_mod_str, "6");
    free(sqr_mod_str);
    
    printf("\n=== gcd lcm ===\n");
    
    Mew a11 = from_hex("24");
    Mew b11 = from_hex("12");
    
    Mew gcd_result = gcd(&a11, &b11);
    char *gcd_str = to_hex(&gcd_result);
    expect("GCD(36, 18) = 18", gcd_str, "12");
    free(gcd_str);
    
    Mew lcm_result = lcm(&a11, &b11);
    char *lcm_str = to_hex(&lcm_result);
    expect("LCM(36, 18) = 36", lcm_str, "24");
    free(lcm_str);
    
    printf("\n=== once more ===\n");
    
    Mew large_mod = from_hex("100000000");
    Mew large_a = from_hex("123456789");
    Mew large_b = from_hex("abcdef12");
    
    Mew large_add_mod = add_mod(&large_a, &large_b, &large_mod);
    Mew large_mul_mod = mul_mod(&large_a, &large_b, &large_mod);
    
    if (cmp(&large_add_mod, &large_mod) < 0 && cmp(&large_mul_mod, &large_mod) < 0) {
        printf("ok\n");
    } else {
        printf("ne ok\n");
        exit(1);
    }
    
    printf("\n=== mod prop ===\n");
    
    Mew mod13 = from_hex("1f");
    Mew a13 = from_hex("a");
    Mew b13 = from_hex("b");

    Mew ab_sum = add(&a13, &b13);
    Mew left13 = modm(&ab_sum, &mod13);
    
    Mew a_mod = modm(&a13, &mod13);
    Mew b_mod = modm(&b13, &mod13);
    Mew right13 = add_mod(&a_mod, &b_mod, &mod13);
    
    expect_mew("(a+b) mod m = [(a mod m)+(b mod m)] mod m", &left13, &right13);
    
    Mew ab_mul13 = mul(&a13, &b13);
    Mew left_mul13 = modm(&ab_mul13, &mod13);
    
    Mew right_mul13 = mul_mod(&a_mod, &b_mod, &mod13);
    
    expect_mew("(a*b) mod m = [(a mod m)*(b mod m)] mod m", &left_mul13, &right_mul13);
    
    printf("\n=== negative sub ===\n");
    
    Mew mod14 = from_hex("a");
    Mew small = from_hex("2");
    Mew large = from_hex("8");
    
    Mew sub_neg_result = sub_mod(&small, &large, &mod14);
    char *sub_neg_str = to_hex(&sub_neg_result);
    expect("(2 - 8) mod 10 = 4", sub_neg_str, "4");
    free(sub_neg_str);
    
    printf("\n=== once more ===\n");
    
    Mew base15 = from_hex("2");
    Mew exp15 = from_hex("a");
    Mew mod15 = from_hex("1f");
    
    Mew pow_large_result = pow_mod(&base15, &exp15, &mod15);
    char *pow_large_str = to_hex(&pow_large_result);
    expect("2^10 mod 31 = 1", pow_large_str, "1");
    free(pow_large_str);
    
    printf("\n=== zero ===\n");
    
    Mew zero_val = zero();
    Mew mod16 = from_hex("d");
    
    Mew zero_mod = modm(&zero_val, &mod16);
    expect_mew("0 mod m = 0", &zero_val, &zero_mod);
    
    Mew add_zero = add_mod(&a8, &zero_val, &mod16);
    expect_mew("a + 0 mod m = a mod m", &add_zero, &mod_result);
    
    printf("\n=== barett ===\n");
    
    Mew large_num = from_hex("123456789abcdef");
    Mew barrett_mod = from_hex("100000000");
    Mew mu = compute_barrett_mu(&barrett_mod);
    
    Mew barrett_result = barrett_reduce(&large_num, &barrett_mod, &mu);
    Mew normal_mod = modm(&large_num, &barrett_mod);
    
    expect_mew("barrett reduction == normal mod", &barrett_result, &normal_mod);
    
    printf("\n=== Testing exponentiation property ===\n");
    
    Mew base18 = from_hex("1a");
    Mew exp18 = from_hex("3");
    Mew mod18 = from_hex("d");
    
    Mew direct_pow = pow_mod(&base18, &exp18, &mod18);
    
    Mew base_mod = modm(&base18, &mod18);
    Mew indirect_pow = pow_mod(&base_mod, &exp18, &mod18);
    
    expect_mew("a^b mod m = (a mod m)^b mod m", &direct_pow, &indirect_pow);
    
    printf("\n ok\n");

    return 0;
}
