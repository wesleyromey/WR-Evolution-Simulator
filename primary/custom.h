// Contains custom functions which require no external functions aside from the Cpp standard libraries
#ifndef MAIN_INCLUDES_H
#include "../mainIncludes/mainIncludes.h"
#define MAIN_INCLUDES_H
#endif

// Macros
#define ln(x) std::log(x)

// Modified <cmath> functions
float sin_deg(int input){
    return sin(input * PI / 180);
}
float cos_deg(int input){
    return cos(input * PI / 180);
}
int min_int(int num1, int num2){
    return (num1 <= num2 ? num1 : num2);
}
int max_int(int num1, int num2){
    return (num1 >= num2 ? num1 : num2);
}
int sign(int num) {
    if(num == 0) return 0;
    return (num > 0 ? 1 : -1);
}
int saturate_int(int num, int lb, int ub) {
    if (num < lb) return lb;
    if (num > ub) return ub;
    return num;
}
float saturate_float(float num, float lb, float ub){
    if(num < lb) return lb;
    if(num > ub) return ub;
    return num;
}
float abs_float(float num){
    return (num > 0 ? num : -num);
}

// Modified probability and statistic functions
float std_uniform_dist(std::mt19937& rng){
    std::uniform_real_distribution<float> distrib(0,1);
    return distrib(rng);
}
int gen_uniform_int_dist(std::mt19937& rng, int lb, int ub){
    std::uniform_int_distribution<int> distrib(lb, ub);
    return distrib(rng);
}
float gen_normal_dist(std::mt19937& rng, float mean, float stddev, bool enforceLbUb = false, float lb = 0, float ub = 0){
    std::normal_distribution<float> distrib(mean, stddev);
    float ans = distrib(rng);
    if(enforceLbUb){
        if(ans < lb) ans = lb;
        if(ans > ub) ans = ub;
    }
    return ans;
}
int gen_normal_int_dist_special(std::mt19937& rng, float prob, int mean, float stddev, int lb, int ub){
    // Convert the outcome of a normal distribution to an int
    // prob: The probability that the mutation occurs
    if(std_uniform_dist(rng) >= prob) return (int)mean;
    std::normal_distribution<float> distrib((float)mean, stddev);
    int ans = (int)(distrib(rng) + 0.5);
    if(ans < lb) ans = lb;
    if(ans > ub) ans = ub;
    return ans;
}

// Convert between various data types
int conv_str_to_int(std::string input){
    // Input can only contain digits
    return std::stoi(input);
}
std::string conv_int_to_str(int num){
    return std::to_string(num);
}