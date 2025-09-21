#include "Generator.h"

constexpr double PI_2 = 6.28318530718;

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
    uint32_t n_shift = d_t;// * sample_freq;

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

//! Configurate signal generator
//! [in] params - Configuration parameters
void BaseGenerator::configure(const cfg& params)
{
    if (params.fd <= 0.)
        throw std::runtime_error("Error in BaseGenerator::configure function." + 
                                 std::string(" Invalid parameters fd: ") +
                                 std::to_string(params.fd));

    if (params.f <= 0.)
        throw std::runtime_error("Error in BaseGenerator::configure function." + 
                                 std::string(" Invalid parameters f: ") +
                                 std::to_string(params.f));

    if (params.n == 0)
        throw std::runtime_error("Error in BaseGenerator::configure function." + 
                                 std::string(" Invalid parameters numBits: ") +
                                 std::to_string(params.n));

    if (params.vel <= 0.)
        throw std::runtime_error("Error in BaseGenerator::configure function." + 
                                 std::string(" Invalid parameters infoVel: ") +
                                 std::to_string(params.vel));

    // Koef for normal trasmission (F_info / f << 1)
    double koeff = 1. / params.vel / params.f;
    std::cout << "config. koeff: " << koeff << std::endl;

    if (koeff >= 0.1)
        throw std::runtime_error("Error in BaseGenerator::configure function." + 
                                 std::string(" Invalid parameters infoVel. F_info / f_carrier =  ") +
                                 std::to_string(koeff) + std::string(", (koeff << 1)"));
    if (params.f * 2 > params.fd)
        throw std::runtime_error("Error in BaseGenerator::configure function." + 
                                 std::string(" Invalid parameters fd and f. fd >= 2 * f"));
 
    m_NumBits = params.n;
    m_Type    = params.type;
    
    // T = 1 / fd
    // t = numBits * infoVel
    // Num samples = t / T
    m_NumSampl = params.n * params.vel * params.fd;
    std::cout << "config. numSamples: " << m_NumSampl << std::endl;

    // Num samples / numBits
    m_SamplPerBit = params.vel  * params.fd;
    std::cout << "config. Samples per bit: " << m_SamplPerBit << std::endl;

    // T = 1 / fd
    // phase = ph0 + f * t, where t = n * T
    m_DPhase = params.f / params.fd;
    std::cout << "config. d Phase: " << m_DPhase << std::endl;

    // d_f = Fd / 2, whete Fd - Info freq
    double fMin = 1. / 2. / params.vel;
    std::cout << "d_f: " << fMin << std::endl;

    // m_DPhaseFreqMod = {(params.f + fMin) / params.fd, (params.f - fMin) / params.fd};
    m_DPhaseFreqMod = {(params.f * (1 + 0.5)) / params.fd, (params.f * (1 - 0.5)) / params.fd};

    std::cout << "dPhaseMax: " << m_DPhaseFreqMod.first << "dPhaseMin: " << m_DPhaseFreqMod.second << std::endl;

    return;
};

// Generate data signal
//! [out] data_out - Generated sample data
void BaseGenerator::generate(std::vector<std::complex<double>>& data_out)
{
    // Generate random bits
    std::vector<uint8_t> info_bits;
    m_Gen.generateBits(info_bits, m_NumBits);

    // std::cout << "generate. Bits size: " << info_bits.size() << std::endl;
    // for (auto&it:info_bits)
    //     std::cout << (uint32_t)it << " ";
    // std::cout << std::endl;

    // Initial phase of the signal
    double phase   = m_UniGen.generate() * PI_2;

    // Pointer to current info bit val
    auto   cur_bit = info_bits.begin();

    data_out.clear();
    data_out.resize(m_NumSampl);

    double I = 0;
    double Q = 0;

    uint32_t incrementBits = 1;
    if (m_Type == SignalType::phase)
        incrementBits = 2;

    for (uint32_t i = 0; i < m_NumSampl; ++i)
    {
        switch (m_Type)
        {
            case SignalType::amplitude: // AM
                data_out[i] = {(1. + *cur_bit) * cos(phase), 0.};
                phase += m_DPhase;
                break;
            case SignalType::phase: // BPSK
                I = (*cur_bit ? 1 : -1) * cos(phase);
                Q = (*(cur_bit + 1) ? 1 : -1) * sin(phase);
                data_out[i] = I + Q;
                phase += m_DPhase;
                break;
            case SignalType::freq: // MFM
                data_out[i] = cos(phase);
                phase += (*cur_bit ? m_DPhaseFreqMod.first : m_DPhaseFreqMod.second);
                break;
            default:
                throw std::runtime_error("Error in modulation type! Type: " + std::to_string(static_cast<int>(m_Type)));
        }

        // Phase correction
        if (phase > PI_2)
            phase -= PI_2;

        // We need to change bit val
        if (!(i % m_SamplPerBit) && (i != 0))
            cur_bit += incrementBits;
    }

    return;
}

// Configure Data Processor
void DataProcessor::config(const cfg& params)
{
    
    return;
}

// Run Data Processing
void DataProcessor::run(double& persent, uint32_t num_runs = 1);