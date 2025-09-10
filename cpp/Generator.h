#ifndef _GENERATOR_H_
#define _GENERATOR_H_

#include <iostream>
#include <vector>
#include <random>
#include <complex>
#include <stdint>

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

// Generate AWGN
//! [out] data_out  - Output generated AWGN
//! [in]  size      - Size of the output data
void generateAwgn(std::vector<complex<double>>& data_out, uint32_t size);
 
};

// Generate random bits
//! [out] data_out - Output generated Bits
//! [in]  size     - Size of the output data
template <typename Data_t>
void RandomGenerator::generateBits(std::vector<Data_t>& data_out, uint32_t size)
{
    if (size == 0)
        throw std::runtime_error("Error in generateBits function! Unsupported data size: " + size);

    data_out.resize(size);
    for (auto& it:data_out)
        it = ((dist(gen) > 0) ? static_cast<Data_t>(1) : static_cast<Data_t>(0));

    return;
}

class SignalGenerator
{
    public:

    // Function for generating shifted in TD signal
    //! [in]  sample_freq      - Sample frequency of the signal
    //! [in]  d_t              - Time offset in sec
    //! [in]  shifted_size     - Size of shifted signal
    //! [in]  data_in          - Input data samples
    //! [out] data_out         - Output shifted signal
    static void generateShiftedSignal(double sample_freq, double d_t, uint32_t shifted_size,
                                      const std::vector<complex<double>>& data_in,
                                            std::vector<complex<double>>& data_out);
}

#endif //_GENERATOR_H_
