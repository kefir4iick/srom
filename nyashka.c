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
    
    return 0;
}
