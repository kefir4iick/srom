#include "mew.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define NUM_TESTS 1000

uint32_t rand_u32(uint32_t min, uint32_t max) {
    return min + rand() % (max - min + 1);
}

Mew random_mew(int digits) {
    Mew result = zero();
    for (int i = 0; i < digits && i < NUM_LEN; i++) {
        result.numberArray[i] = rand_u32(1, 0xFFFFFFFF);
    }
    if (is_zero(&result)) result.numberArray[0] = 1;
    return result;
}

double timer(Mew (*op)(const Mew*, const Mew*), int digits, int num_tests) {
    double total_time = 0.0;
    int successful_tests = 0;
    
    for (int i = 0; i < num_tests; i++) {
        Mew a = random_mew(digits);
        Mew b = random_mew(digits);
        
        clock_t start = clock();
        Mew result = op(&a, &b);
        clock_t end = clock();
        
        if (!result.chozabretto) {
            total_time += ((double)(end - start)) / CLOCKS_PER_SEC * 1e6;
            successful_tests++;
        }
    }
    
    return (successful_tests > 0) ? (total_time / successful_tests) : -1.0;
}

double timer_div(int digits, int num_tests) {
    double total_time = 0.0;
    int successful_tests = 0;
    
    for (int i = 0; i < num_tests; i++) {
        Mew a = random_mew(digits);
        Mew b = random_mew(digits / 2 + 1);
        if (is_zero(&b)) b.numberArray[0] = 1;
        
        clock_t start = clock();
        Mew result = divm(&a, &b);
        clock_t end = clock();
        
        if (!result.chozabretto) {
            total_time += ((double)(end - start)) / CLOCKS_PER_SEC * 1e6;
            successful_tests++;
        }
    }
    
    return (successful_tests > 0) ? (total_time / successful_tests) : -1.0;
}

int main() {
    printf("Time\n");
    
    srand((unsigned int)time(NULL));
    
    int digits_list[] = {3, 16, 32, 48, 64};
    int num_digits = sizeof(digits_list) / sizeof(digits_list[0]);
    
    printf("bit |  add  |  sub  |  mul  |  div\n");
    
    for (int i = 0; i < num_digits; i++) {
        int digits = digits_list[i];
        if (digits > NUM_LEN) continue;
        
        double add_time = timer(add, digits, NUM_TESTS);
        double sub_time = timer(sub, digits, NUM_TESTS);
        double mul_time = timer(mul, digits, NUM_TESTS);
        double div_time = timer_div(digits, NUM_TESTS);
        
        printf("%4d | %8.2f  | %8.2f  | %8.2f | %8.2f \n",
               digits, add_time, sub_time, mul_time, div_time);
    }
    
    
    printf("=======================================================\n");
    
    Mew a = from_hex("26210d961e7b8b6b8dc3a7504d8c59b0b4f5df0451fd4f7c2d7ff5a050e8f7e02c9b0f5a4aebf1b4c2c9f0a8e7f06c1d5f0d7f2a1b3c7e9d8a4b6c2d1e0f9a87b6c5d4e3f2b1a0c9d8e7f6a5b4c3d2e1f0ab12cd34ef56a7890bcdef1234567890abcdef0fedcba0987654321ff00ee11dd22cc33bb44aa5599887766554433221100aa55aa55cc33cc33");
    
    Mew b = from_hex("16ab41a3c4f0b7e2d9a8c7b6e5f4d3c2b1a0f9e8d7c6b5a4938271605f4e3d2c1b0a9f8e7d6c5b4a39281706f5e4d3c2b1a0f9e8d7c6b5a4938271605f4e3d2c1b0a9f8e7d6c5b4a39281706f5e4d3c2b1a0f9e8d7c6b5a4938271605f4e3d2c1b0a9f8e7d6c5b4a39281706f5e4d3c2b1a0f9e8d7c6b5a4938271605f4e3d2c1b0a9");
    
    Mew l = from_hex("b8f729c85e0d4b671f92c8a134e6d0f5b82a7c9e16d4b380c5f9a2e1374d6c891b0f2a5c8e3197d4b6a0283c5f9e174d6b0a2");
    
    Mew c = add(&a, &b);
    Mew d = sub(&a, &b);
    Mew e = mul(&a, &b);
    Mew f = divm(&a, &b);
    
    Mew w1 = add_mod(&a, &b, &l);
    Mew w2 = sub_mod(&a, &b, &l);
    Mew w3 = mul_mod(&a, &b, &l);
    //Mew w4 = add_mod(&a, &b, &l);
    
    
    print_hex(&c);
    printf("\n\n");
    print_hex(&d);
    printf("\n\n");
    print_hex(&e);
    printf("\n\n");
    print_hex(&f);
    printf("\n\n");
    print_hex(&w1);
    printf("\n\n");
    print_hex(&w2);
    printf("\n\n");
    print_hex(&w3);
    printf("\n\n");
    //print_hex(&w4);
    printf("\n\n");
    
    return 0;
}
