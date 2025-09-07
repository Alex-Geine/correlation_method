#ifndef _GENERATOR_H_
#define _GENERATOR_H_

#include <iostream>
#include <vector>
#include <random>

class RandomGenerator
{
private: // variables

std::mt19937                      gen{};    // Random generator
std::normal_distribution<double>  dist{};   // Normal distribution

public: // functions

// Default constructor
RandomGenerator(){};

// Configure function
//! [in] std  - Standart noise deviation
//! [in] mean - Mean value 
void config(double std = 1., double mean = 0.);

// Generate random bits
//! [out] data_out - Output generated Bits
//! [in]  size     - Size of the output data
template <typename Data_t>
void generateBits(std::vector<Data_t>& data_out, uint32_t size);

};

// Generate random bits
//! [out] data_out - Output generated Bits
//! [in]  size     - Size of the output data
template <typename Data_t>
void RandomGenerator::generateBits(std::vector<Data_t>& data_out, uint32_t size)
{
    if (size == 0)
    {
        std::cerr << "Error in generateBits function! Unsupported data size: " << size << std::endl;
        return;
    }

    data_out.resize(size);
    for (auto& it:data_out)
        it = ((dist(gen) > 0) ? static_cast<Data_t>(1) : static_cast<Data_t>(0));

    return;
}

#endif //_GENERATOR_H_
