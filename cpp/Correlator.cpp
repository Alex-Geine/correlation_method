#include "Correlator.h"

#include <algorithm>

// Calculate correlation
//! [in]  data_a        - First signal to correlate
//! [in]  data_b        - Second signal to correlate
//! [out] corr_out      - Output correlation samples
void Correlator::findCorrelation(const std::vector<std::complex<double>>& data_a,
                                 const std::vector<std::complex<double>>& data_b,
                                 std::vector<double>&                     corr_out)
{
    uint32_t size_a = data_a.size();
    uint32_t size_b = data_b.size();

    corr_out.clear();
    corr_out.resize(size_a);

    std::complex<double> temp;

    for (uint32_t i = 0; i < size_a; ++i)
    {
        for (uint32_t j = 0; j < size_b; ++j)
            temp += std::conj(data_a[i + j]) * data_b[j];
        corr_out[i] = std::abs(temp);
        temp = std::complex<double>(0, 0);
    }

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