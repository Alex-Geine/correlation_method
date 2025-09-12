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
void RandomGenerator::generateAwgn(std::vector<std::complex<double>>& data_out, uint32_t size)
{
    if (size == 0)
        throw std::runtime_error("Error in generateAwgn function! Unsupported data size: " + size);

    data_out.resize(size);

    for (uint32_t i = 0; i < size; ++i)
        data_out[i] = std::complex<double>(dist(gen), dist(gen));

    return;
}

// Function for generating shifted in TD signal
//! [in]  sample_freq      - Sample frequency of the signal
//! [in]  d_t              - Time offset in sec
//! [in]  shifted_size     - Size of shifted signal
//! [in]  data_in          - Input data samples
//! [out] data_out         - Output shifted signal
void SignalGenerator::generateShiftedSignal(double sample_freq, double d_t, uint32_t shifted_size,
                                            const std::vector<std::complex<double>>& data_in,
                                                  std::vector<std::complex<double>>& data_out)
{
    if (sample_freq <= 0)
        throw std::runtime_error("Error in SignalGenerator::generateShiftedSignal."
                                 " Invalid sample_freq: " + std::to_string(sample_freq));
    if (d_t < 0)
        throw std::runtime_error("Error in SignalGenerator::generateShiftedSignal."
                                 " Invalid d_t: " + std::to_string(d_t));

    uint32_t size = data_in.size();

    if (size < shifted_size)
        throw std::runtime_error("Error in SignalGenerator::generateShiftedSignal."
                                 " Invalid shifted_size: " + std::to_string(shifted_size) +
                                 std::string("while size of data_in: ") + std::to_string(size));

    // Calculating n_shift
    uint32_t n_shift = d_t / sample_freq;

    std::cout << "n_shift: " << n_shift << std::endl;

    if (n_shift + shifted_size > size)
        throw std::runtime_error("Error in SignalGenerator::generateShiftedSignal." +
                                 std::string(" Invalid n_shift: ") + std::to_string(n_shift) +
                                 std::string(", n_shift is size / fd / d_t, where size: ") +
                                 std::to_string(size) + std::string(", fd: ") + std::to_string(sample_freq) +
                                 std::string(", d_t: ") + std::to_string(d_t));

    data_out.clear();
    data_out.resize(shifted_size);

    auto iter_data_in = data_in.begin() + n_shift;

    std::copy(iter_data_in, iter_data_in + shifted_size, data_out.begin());

    return;
}

// Add noise in data
//! [in/out] data     - Input/Output data
//! [in]     snr      - Signal to Noise Ratio
void NoiseInjector::addNoise(std::vector<std::complex<double>>& data, double snr)
{
    double energy = 0;

    for (auto& it: data)
        energy += std::abs(it * std::conj(it));

    // snr = log(energy / noise)
    // 10^snr = enegry / noise
    // noise = energy / 10^snr
    double lin_snr = std::pow(10, snr);
    double noise_power = energy / lin_snr;

    // need to process mean energy
    // Very big questions
    NoiseInjector::gen.config(std::sqrt(noise_power / 2.));

    uint32_t size = data.size();

    std::vector<std::complex<double>> noise;
    gen.generateAwgn(noise, size);

    for (uint32_t i = 0; i < size; ++i)
        data[i] += noise[i];

    return;
}
