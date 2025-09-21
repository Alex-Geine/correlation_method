#include "Correlator.h"

#include <algorithm>
#include <numeric>
#include <fftw3.h>

// Calculate correlation
//! [in]  data_a        - First signal to correlate
//! [in]  data_b        - Second signal to correlate
//! [out] corr_out      - Output correlation samples
void Correlator::findCorrelation(const std::vector<std::complex<double>>& data_a,
                                 const std::vector<std::complex<double>>& data_b,
                                 std::vector<double>&                     corr_out)
{
    // NCC(k) = sum( [x(n) - mean(x)] * [y(n+k) - mean(y)] ) / ( std(x) * std(y) )

    uint32_t size_a = data_a.size();
    uint32_t size_b = data_b.size();
    uint32_t size_out = size_a - size_b;

    corr_out.clear();
    corr_out.resize(size_out);

   // 1. Вычисляем средние значения
    std::complex<double> mean_a = std::accumulate(
        data_a.begin(), data_a.end(), std::complex<double>(0, 0)) / static_cast<double>(size_a);
    
    std::complex<double> mean_b = std::accumulate(
        data_b.begin(), data_b.end(), std::complex<double>(0, 0)) / static_cast<double>(size_b);

    // 2. Создаем копии сигналов с вычитанием среднего
    std::vector<std::complex<double>> a_centered(size_a);
    std::vector<std::complex<double>> b_centered(size_b);
    
    for (uint32_t i = 0; i < size_a; ++i) {
        a_centered[i] = data_a[i] - mean_a;
    }
    
    for (uint32_t i = 0; i < size_b; ++i) {
        b_centered[i] = data_b[i] - mean_b;
    }

    // 3. Вычисляем энергии сигналов для нормализации
    double energy_a = 0.0;
    double energy_b = 0.0;
    
    for (const auto& val : a_centered) {
        energy_a += std::norm(val); // |x|² = x_real² + x_imag²
    }
    
    for (const auto& val : b_centered) {
        energy_b += std::norm(val);
    }
    
    double normalizer = std::sqrt(energy_a * energy_b);
    if (normalizer < 1e-12) {
        normalizer = 1e-12; // Защита от деления на ноль
    }
    // For FFT
    uint32_t n_fft = 1;
    while (n_fft < size_out)
        n_fft <<= 1;

    std::vector<std::complex<double>> a_fft(n_fft, 0.0);
    std::vector<std::complex<double>> b_fft(n_fft, 0.0);
    std::vector<std::complex<double>> corr_fft(n_fft);

    std::copy(a_centered.begin(), a_centered.end(), a_fft.begin());
    std::copy(b_centered.begin(), b_centered.end(), b_fft.begin());

    fftw_plan plan_forward_a = fftw_plan_dft_1d(n_fft,
                                                reinterpret_cast<fftw_complex*>(a_fft.data()),
                                                reinterpret_cast<fftw_complex*>(a_fft.data()),
                                                FFTW_FORWARD,
                                                FFTW_ESTIMATE);
                                                
    fftw_plan plan_forward_b = fftw_plan_dft_1d(n_fft,
                                                reinterpret_cast<fftw_complex*>(b_fft.data()),
                                                reinterpret_cast<fftw_complex*>(b_fft.data()),
                                                FFTW_FORWARD,
                                                FFTW_ESTIMATE);
                                                
    fftw_plan plan_backward = fftw_plan_dft_1d(n_fft,
                                               reinterpret_cast<fftw_complex*>(corr_fft.data()),
                                               reinterpret_cast<fftw_complex*>(corr_fft.data()),
                                               FFTW_BACKWARD,
                                               FFTW_ESTIMATE);

    fftw_execute(plan_forward_a);
    fftw_execute(plan_forward_b);

    for (uint32_t i = 0; i < n_fft; ++i)
        corr_fft[i] = a_fft[i] * std::conj(b_fft[i]);

    // Обратное FFT
    fftw_execute(plan_backward);

    // После получения сырой корреляции через FFT
for (uint32_t lag = 0; lag < size_out; ++lag) {
    // Для каждого лага вычисляем локальную энергию
    uint32_t start = std::max(0, static_cast<int>(lag) - static_cast<int>(size_b) + 1);
    uint32_t end = std::min(size_a, lag + 1);
    
    double local_energy = 0.0;
    for (uint32_t i = start; i < end; ++i) {
        uint32_t j = lag - i;
        if (j < size_b) {
            local_energy += std::norm(a_centered[i]) * std::norm(b_centered[j]);
        }
    }
    
    double local_normalizer = std::sqrt(local_energy);
    if (local_normalizer > 1e-12) {
        corr_out[lag] = std::abs(corr_fft[lag]) / (n_fft * local_normalizer);
    } else {
        corr_out[lag] = 0.0;
    }
}

    // for (uint32_t i = 0; i < size_out; ++i)
        // corr_out[i] = std::abs(corr_fft[i] / normalizer) / size_out;

    // Очистка ресурсов FFTW
    fftw_destroy_plan(plan_forward_a);
    fftw_destroy_plan(plan_forward_b);
    fftw_destroy_plan(plan_backward);

    return;
}

// Process correlation with ouput data
//! [in]  data_a        - First signal to correlate
//! [in]  data_b        - Second signal to correlate
//! [out] corr_out      - Output correlation samples
//! [out] max_metric_id - Index of maximum metric of the correlation
void Correlator::correlate(const std::vector<std::complex<double>>& data_a,
                           const std::vector<std::complex<double>>& data_b,
                                 std::vector<double>&               corr_out,
                                 uint32_t&                          max_metric_id)
{
    if (data_b.size() > data_a.size())
        throw std::runtime_error("Error in correlate function. Size of data_a less then the size of data_b");

    findCorrelation(data_a, data_b, corr_out);

    // Находим итератор на максимальный элемент
    auto max_it = std::max_element(corr_out.begin(), corr_out.end());
    
    std::cout << "max el: " << *max_it << std::endl;
    // Вычисляем индекс
    max_metric_id = std::distance(corr_out.begin(), max_it);

    return;
}

// Process correlation with ouput data
//! [in]  data_a        - First signal to correlate
//! [in]  data_b        - Second signal to correlate
//! [out] max_metric_id - Index of maximum metric of the correlation
void Correlator::correlate(const std::vector<std::complex<double>>& data_a,
                           const std::vector<std::complex<double>>& data_b,
                                 uint32_t&                          max_metric_id)
{
    if (data_b.size() > data_a.size())
        throw std::runtime_error("Error in correlate function. Size of data_a less then the size of data_b");

    std::vector<double> corr_out;

    findCorrelation(data_a, data_b, corr_out);

    // Находим итератор на максимальный элемент
    auto max_it = std::max_element(corr_out.begin(), corr_out.end());
    
    // Вычисляем индекс
    max_metric_id = std::distance(corr_out.begin(), max_it);

    return;
}