#ifndef _GENERATOR_H_
#define _GENERATOR_H_

#include <iostream>
#include <vector>
#include <random>
#include <complex>
#include <cstdint>

enum class SignalType : int
{
    ndf       = -1,  // Undefined
    amplitude = 0, // AM
    phase     = 1, // BPSK
    freq      = 2  // MFM
};

struct cfg
{
    double   fd = 0;      // Sample freq
    double   f  = 0;      // Carrier freq
    uint32_t n = 0;       // Num info bits
    double   vel = 0;     // Info velocity
    double   dt = 0;      // Time offset
    double   snr1 = 0;    // SNR for signal 1
    double   snr2 = 0;    // SNR for signal 1
    SignalType type       = SignalType::ndf; // Type of modulation
    bool     is_random_dt = false;
};

class RandomGenerator
{
private: // variables

std::mt19937                      gen{};    // Random generator
std::normal_distribution<double>  dist{};   // Normal distribution

public: // functions

// Default constructor
RandomGenerator() : gen(std::random_device{}()) {}

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
void generateAwgn(std::vector<std::complex<double>>& data_out, uint32_t size);

// Add noise in data
//! [in/out] data     - Input/Output data
//! [in]     snr      - Signal to Noise Ratio
void addNoise(std::vector<std::complex<double>>& data, double snr);
};

class RandomUniformGen
{
    private:

    std::mt19937                           gen  = {};
    std::uniform_real_distribution<double> dist = {};

    public:

    //! Default constructor
    RandomUniformGen() : gen(std::random_device{}()),
                         dist(std::uniform_real_distribution<double>(0.0, 1.0)){};

    //! Generate random number from 0 to 1
    double generate() {return dist(gen);};
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

class NoiseInjector
{
private:  // variables
RandomGenerator gen;    //! Random Generator

public:  // functions

// Add noise in data
//! [in/out] data     - Input/Output data
//! [in]     snr      - Signal to Noise Ratio
void addNoise(std::vector<std::complex<double>>& data, double snr);
};

struct GeneratorCfg
{
    double          fd       = 0.;    //! Sample frequency
    double          f        = 0.;    //! Carrier frequency
    uint32_t        numBits  = 0;     //! Number of bits in the signal
    double          infoVel  = 0.;    //! Information bits velocity
};

class BaseGenerator
{
    private: //! variables

    RandomGenerator  m_Gen         = {};    //! Random generator to create random bits
    RandomUniformGen m_UniGen      = {};    //! Random uniform generator to produce numbers from 0 to 1

    uint32_t                  m_NumBits              = 0;               //! Number of info bits
    uint32_t                  m_NumSampl             = 0;               //! Number of samples in output signal
    uint32_t                  m_SamplPerBit          = 0;               //! Number samples per bit
    double                    m_DPhase               = 0.;              //! Phase addition koeff
    std::pair<double, double> m_DPhaseFreqMod        = {0,0};           //! Freq for Freq Modulation
    SignalType                m_Type                 = SignalType::ndf; // Modulation type

    public: //! fucntions

    //! Configurate signal generator
    //! [in] params - Configuration parameters
    void configure(const cfg& params);

    // Generate data signal
    //! [out] data_out - Generated sample data
    void generate(std::vector<std::complex<double>>& data_out);

    // Get number of samples per bit
    uint32_t getNumSamplesPerBit();
};

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
                                      const std::vector<std::complex<double>>& data_in,
                                            std::vector<std::complex<double>>& data_out);

    // Function for generating shifted in TD signal
    //! [in]  sample_freq      - Sample frequency of the signal
    //! [in]  d_t              - Time offset in sec
    //! [in]  shifted_size     - Size of shifted signal
    //! [in]  data_in          - Input data samples
    //! [out] data_out         - Output shifted signal
    //! [return] generated dt
    static double generateShiftedSignal(double sample_freq, uint32_t shifted_size,
                                          const std::vector<std::complex<double>>& data_in,
                                          double seed,
                                                std::vector<std::complex<double>>& data_out);

};

class DataProcessor
{
    private:

    BaseGenerator    m_GenData;   // Generator of data
    NoiseInjector    m_Noise;     // Noise injector
    cfg              m_Cfg;       // Configuration params
    RandomUniformGen m_UniGen;    // Random uniform Generator

    std::string fileName;         // Filename to write ber data

    public: // functions

    // Configure Data Processor
    void config(const cfg& params);

    // Run Data Processing
    void run(uint32_t num_runs);

    // Run Data Processing with writing temp data 
    void run();
};

#endif //_GENERATOR_H_
