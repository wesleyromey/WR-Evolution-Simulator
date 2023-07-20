// This file controls the AI of each cell. It is essentially a simple, relatively small neural network
//  with a large number of inputs and several hidden layer nodes + outputs
#ifndef PRIMARY_INCLUDES_H
#include "../primary/primaryIncludes.h"
#define PRIMARY_INCLUDES_H
#endif


struct Cell;

// First, define a node
static const int ACT_FCN_IDENTITY = 0;
static const std::vector<int> VALID_ACT_FCNS = {ACT_FCN_IDENTITY};
static const int NUM_HIDDEN_LAYER_NODES = 10;
struct aiNode {
    // Essentially a node in an artificial neural network (ANN)

    // Identity
    int id = -1;

    // Parameters
    bool isInput = false;
    //bool isOutput = false;
    int activationFcn = ACT_FCN_IDENTITY;
    float stddevWB = 0.2; // The standard deviation of the bias and weights when they mutate

    // Mutateable parameters (controlled by 'genome')
    float bias = 0;     // Add this to the weighted sum
    std::vector<float> inputWeights; // Multiply each previous node output by a weight
    //std::vector<float> prevNodeOutputs; // The outputs of each previous node (keep empty if isInput == true)
    
    // Outputs and intermediate values
    float weightedSum = 0; // equal to the sum of bias + inputWeights[i]*prevNodeOutputs[i]
    float outputVal = 0;

    // Calculated values
    int numWeights = -1;     // equal to (# of previous layer nodes)

    // Struct-specific functions
    bool activation_fcn_is_valid(){
        for(auto validFcn : VALID_ACT_FCNS) {
            if(activationFcn == validFcn) return true;
        }
        std::cout << "The activation function, represented by the number " << activationFcn;
        std::cout << ", is either invalid or unaccounted for!!!" << std::endl;
        return false;
    }
    bool verify_node_validity(){
        assert(id >= 0);
        //assert(pCell != NULL);
        if(isInput) {
            std::cout << "Input nodes do NOT need to be defined!!!" << std::endl;
            assert(false);
        }
        assert(numWeights >= 0);
        assert(activation_fcn_is_valid());
        assert(inputWeights.size() == numWeights);
        assert(stddevWB >= 0);
        return true;
    }
    void init_node(int _id, bool _isInput, int numNodesInPrevLayer){
        id = _id;
        isInput = _isInput;
        numWeights = numNodesInPrevLayer;
        for(int i = 0; i < numWeights; i++) {
            inputWeights.push_back(gen_normal_dist(rng, 0, 1));
        }
        verify_node_validity();
    }
    float calc_act_fcn(float actFcnInput, int activationFcn){
        switch(activationFcn){
            case ACT_FCN_IDENTITY:
            return actFcnInput;
            default:
            assert(activation_fcn_is_valid());
            std::cout << "The activation function is valid, but NOT accounted for in this function!!!";
            assert(false);
        }
        return 0;
    }
    float calc_weighted_sum(std::vector<float> prevNodeOutputs, std::vector<float> inputWeights, float bias){
        float ans = bias;
        assert(prevNodeOutputs.size() == inputWeights.size());
        for(int i = 0; i < inputWeights.size(); i++){
            ans += prevNodeOutputs[i] * inputWeights[i];
        }
        return ans;
    }
    void do_forward_propagation(std::vector<float> _prevLayerOutputs){
        // Update the intermediate value(s) and output(s) of the node
        //  based on the assumption that the node's 'input' values
        //  like weights, bias, and previous node outputs are held constant
        weightedSum = calc_weighted_sum(_prevLayerOutputs, inputWeights, bias);
        outputVal = calc_act_fcn(weightedSum, activationFcn);
    }
    void mutate_node(int mutationRate, float prob){
        if (std_uniform_dist(rng) < prob) bias += gen_normal_dist(rng, 0, stddevWB);
        for(int i = 0; i < inputWeights.size(); i++){
            if (std_uniform_dist(rng) < prob) {
                inputWeights[i] += gen_normal_dist(rng, 0, stddevWB);
            }
        }
    }
    void set_node_weights_and_biases(std::vector<int> weightsAndBiases){
        // Set the weights and biases to {bias, w1, w2, ...}
        if(weightsAndBiases.size() != inputWeights.size()){
            cout << "WARNING! Size mismatch in setting the weights and biases of a node\n";
            return;
        }
        for(int i = 0; i < inputWeights.size(); i++){
            inputWeights[i] = weightsAndBiases[i];
        }
    }
    void print_node_weights_and_biases(){
        cout << "wb: ";
        for(auto item : inputWeights) cout << item << " ";
        cout << endl;
    }
};