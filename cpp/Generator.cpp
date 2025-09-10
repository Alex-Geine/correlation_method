#include "Generator.h"

// Configure function
//! [in] std  - Standart noise deviation
//! [in] mean - Mean value 
void RandomGenerator::config(double std, double mean)
{
    // Update generator
    gen = std::mt19937(std::random_device{}());
    dist = std::normal_distribution<double>(mean, std);
}

// Generate AWGN
//! [out] data_out  - Output generated AWGN
//! [in]  size      - Size of the output data
void RandomGenerator::generateAwgn(std::vector<complex<double>>& data_out, uint32_t size)
{
    if (size == 0)
        throw std::runtime_error("Error in generateAwgn function! Unsupported data size: " + size);

    data_out.resize(size);

    for (uint32_t i = 0; i < size; ++i)
        data_out[i] = std::complex<double>(dist(gen), dist(gen));

    return;
}