#ifndef _CORRELATOR_H_
#define _CORRELATOR_H_

#include <iostream>
#include <vector>
#include <complex>
#include <stdint>

class Correlator
{
private: // functions

// Calculate correlation
//! [in]  data_a        - First signal to correlate
//! [in]  data_b        - Second signal to correlate
//! [out] corr_out      - Output correlation samples
static void findCorrelation(const std::vector<complex<double>>& data_a,
                            const std::vector<complex<double>>& data_b,
                                  std::vector<double>&          corr_out);

public: // functions

// Process correlation with ouput data
//! [in]  data_a        - First signal to correlate
//! [in]  data_b        - Second signal to correlate
//! [out] corr_out      - Output correlation samples
//! [out] max_metric_id - Index of maximum metric of the correlation
static void correlate(const std::vector<complex<double>>& data_a,
                      const std::vector<complex<double>>& data_b,
                            std::vector<double>&          corr_out,
                            uint32_t&                     max_metric_id);

// Process correlation without ouput data
//! [in]  data_a        - Output generated AWGN
//! [in]  data_b        - Size of the output data
//! [out] max_metric_id - Index of maximum metric of the correlation
static void correlate(const std::vector<complex<double>>& data_a,
                      const std::vector<complex<double>>& data_b,
                            uint32_t&                     max_metric_id);

};

#endif //_CORRELATOR_H_
