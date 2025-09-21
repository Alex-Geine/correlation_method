#include <iostream>
#include <fstream>
#include <string>

#include "Generator.h"
#include "Correlator.h"


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

void parceCfg(cfg& cfg, char* argv[])
{
    cfg.fd   = std::stod(argv[1]);
    cfg.f    = std::stod(argv[2]);
    cfg.n    = std::stoi(argv[3]);
    cfg.vel  = std::stod(argv[4]);
    cfg.dt   = std::stod(argv[5]);
    cfg.snr1 = std::stod(argv[6]);
    cfg.snr2 = std::stod(argv[7]);
    cfg.type = static_cast<SignalType>(std::stoi(argv[8]));
}

int main(int argc, char* argv[])
{
    std::cout << "argc: " << argc << std::endl;
    std::cout << " argv" << std::endl;
    for (int i = 0; i < argc; ++i)
        std::cout << argv[i] << std::endl;

    if (argc != 9)
    {
        std::cerr << "Incorrect input number of parameters: " << argc << std::endl;
        abort();
    }

    cfg config;
    parceCfg(config, argv);

    BaseGenerator gen;
    NoiseInjector noiseInjector;

    std::vector<std::complex<double>> firstSignal;
    std::vector<std::complex<double>> secondSignal;
    std::vector<double>               correlation;

    double   d_t              = config.dt;
    double   snr_static       = config.snr1; // 10 dB
    double   snr_variable     = config.snr2; // 10 dB
    double shifted_size_per   = 0.3; // 30 %

    uint32_t max_metric_id = 0;

    gen.configure(config);

    // Generate first sample
    gen.generate(firstSignal);

    SignalGenerator::generateShiftedSignal(config.fd, d_t, shifted_size_per * firstSignal.size(), firstSignal, secondSignal);

    noiseInjector.addNoise(firstSignal, snr_variable);
    noiseInjector.addNoise(secondSignal, snr_static);

    Correlator::correlate(firstSignal, secondSignal, correlation, max_metric_id);

    write(firstSignal, std::string("../data/first_data.txt"));
    write(secondSignal, std::string("../data/second_data.txt"));
    write(correlation, std::string("../data/correlation.txt"));

    std::cout << "shift: " << d_t << ", detected_shift: " << max_metric_id << std::endl;

    return 0;
}