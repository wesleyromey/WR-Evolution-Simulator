// Contains custom functions which require no external functions aside from the Cpp standard libraries
#ifndef MAIN_INCLUDES_H
#include "../mainIncludes/mainIncludes.h"
#define MAIN_INCLUDES_H
#endif

// Macros
#define ln(x) std::log(x)

// Variadic Functions

// Set the values of all valPtr to the new specified values
void init_sim_gnd_energy(int initGndEnergy);
void set_vals(){}
template <typename T, typename... Types>
void set_vals(T* pVal, T newVal, Types... vals){
    *pVal = newVal;
    set_vals(vals...);
}
void print_scalar_vals(){cout << endl;}
template<typename T, typename... Types>
void print_scalar_vals(std::string valName, T val, Types... vals){
    cout << valName << ": " << val << " ";
    print_scalar_vals(vals...);
}
void set_sim_params(std::vector<SimParamInt*> vals, std::vector<int> newVals){
    // e.g. set_sim_params({&ubX, &ubY, ...}, {30, 20, ...});
    if(vals.size() != newVals.size()){
        cout << "ERROR! The size of the input vectors should be equal!\n";
        cout << "  Currently, the sizes are " << vals.size() << " and " << newVals.size() << endl;
        return;
    }
    for(int i = 0; i < vals.size(); i++){
        vals[i]->set_val(newVals[i]);
    }
    init_sim_gnd_energy(maxGndEnergy.val / 2);
}





// Modified <cmath> functions
float sin_deg(int input){
    return sin(input * PI / 180);
}
float cos_deg(int input){
    return cos(input * PI / 180);
}
float tan_deg(int input){
    return tan(input * PI / 180);
}
float arc_tan_deg(int dy, int dx){
    //if(dx == 0) return INFINITY;
    return atan((double)dy / (double)dx) * 180 / PI;
}
float arc_sin_deg(double dy, double distance){
    return asin(dy / distance) * 180 / PI;
}
float arc_cos_deg(double dx, double distance){
    return acos(dx / distance) * 180 / PI;
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
// The answer should have a non-negative exponent
int pow_int(int root, int exponent){
    assert(exponent >= 0);
    assert(root != 0 || exponent != 0);
    int ans = 1;
    // Multiplication
    for(int i = 0; i < exponent; i++){
        if(abs(ans) >= MAX_INT_DEFAULT / abs(root)){
            cout << "Error! The answer has too large of magnitude to fit into an int!\n";
            return -1;
        }
        ans *= root;
    }
    return ans;
}
float saturate_float(float num, float lb, float ub){
    if(num < lb) return lb;
    if(num > ub) return ub;
    return num;
}
float abs_float(float num){
    return (num > 0 ? num : -num);
}
int spline_int(int x, int xLb, int xUb, int fLb, int fUb, float fxLb, float fxUb){
    assert(xLb <= x && x <= xUb);
    // x: The value to use as an interpolation metric
    // xLb, xUb: The bounds for the variable x
    // fLb, fUb: The function values at xLb and xUb
    // fxLb, fxUb: The derivative of the function at values xLb and xUb

    if(x == xLb) return fLb;
    if(x == xUb) return fUb;
    float t = (float)(x - xLb) / (xUb - xLb);
    float a = fxLb*(xUb-xLb) - (fUb - fLb);
    float b = -fxUb*(xUb-xLb) + (fUb - fLb);
    float ansFloat = (1-t)*fLb + t*fUb + t*(1-t)*( (1-t)*a + t*b );
    return (int)(ansFloat + 0.5);
}
// x: input, xLb and xUb: bounds on x, fLb and fUb: function value at xLb and xUb
int linear_interp_x_int(int x, std::vector<int> xVec, std::vector<int> fcnValVec){
    assert(xVec.size() >= 2 && xVec.size() == fcnValVec.size());
    if(x <= xVec[0]) return fcnValVec[0];
    if(x >= xVec[xVec.size()-1]) return fcnValVec[xVec.size()-1];
    int iLb = 0; // Index of Lower Bound
    while(iLb < xVec.size() - 1 && x >= xVec[iLb+1]) iLb++;
    int xLb = xVec[iLb], xUb = xVec[iLb+1];
    int fLb = fcnValVec[iLb], fUb = fcnValVec[iLb+1];

    //cout << "  iLb: " << iLb << endl;
    int numerator = (x - xLb) * (fUb - fLb);
    int denominator = xUb - xLb;
    //cout << "  f(" << x << "): " << fLb << " + " << numerator << " / " << denominator << endl;
    return fLb + (float)numerator / denominator;
}
// Takes into consideration screen wrapping
float calc_distance_between_points(int posX, int posY, int targetX, int targetY){
    int dx = targetX - posX;
    if(WRAP_AROUND_X) dx = min_int(dx, ubX.val - dx);
    int dy = targetY - posY;
    if(WRAP_AROUND_Y) dy = min_int(dy, ubY.val - dy);
    return sqrt(dx*dx + dy*dy);
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

// Other calculations
int find_closest_value(int target, std::vector<int> vec){
    assert(vec.size() > 0);
    int ans = vec[0], diff = abs(target - ans);
    for(auto ele : vec){
        if(abs(target - ele) < diff){
            diff = abs(target - ele);
            ans = ele;
        }
    }
    return ans;
}