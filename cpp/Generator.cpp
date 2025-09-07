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

