#include <iostream>

#include "Generator.h"
#include "Correlator.h"
#include <fstream>

template <typename T>
static void print(std::vector<T> data, std::string name)
{
    std::cout << name << ", size: " << data.size() << std::endl;
    for (auto& it: data)
        std::cout << it << " ";
    std::cout << std::endl;
}

template <typename T>
static void write(std::vector<T> data, std::string name)
{
    std::ofstream outFile;
    outFile.open(name);

    if (!outFile.is_open())
        throw std::runtime_error("Cannot open file: " + name);

    for (uint32_t i = 0; i < data.size() - 1; ++i)
       outFile << data[i] << ", ";

    outFile << data[data.size() - 1] << std::endl;
}

int main()
{
    RandomGenerator gen;
    NoiseInjector noiseInjector;
    gen.config();

    std::vector<std::complex<double>> firstSignal;
    std::vector<std::complex<double>> secondSignal;
    std::vector<double>               correlation;

    // 1 sample - 1 second
    uint32_t size         = 100;
    double   fd           = 1;
    double   d_t          = 30;
    double   snr_static   = 10; // 10 dB
    double   snr_variable = 0; // 10 dB
    uint32_t shifted_size = 30;

    uint32_t max_metric_id = 0;

    // Generate first sample
    gen.generateAwgn(firstSignal, size);

    SignalGenerator::generateShiftedSignal(fd, d_t, shifted_size, firstSignal, secondSignal);

    noiseInjector.addNoise(firstSignal, snr_variable);
    noiseInjector.addNoise(secondSignal, snr_static);

    Correlator::correlate(firstSignal, secondSignal, correlation, max_metric_id);

    print(firstSignal, std::string("first_data"));
    print(secondSignal, std::string("second_data"));
    print(correlation, std::string("correlation"));

    write(firstSignal, std::string("../data/first_data.txt"));
    write(secondSignal, std::string("../data/second_data.txt"));
    write(correlation, std::string("../data/correlation.txt"));

    std::cout << "shift: " << d_t << ", detected_shift: " << max_metric_id << std::endl;

    return 0;
}