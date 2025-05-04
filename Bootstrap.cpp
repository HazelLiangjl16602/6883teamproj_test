#include "Stock.h"
#include "GroupedStock.h"
#include "Operations.h"
#include "BootStrap.h"
#include <vector>
#include <map>
#include <random>
#include <numeric>
#include <iostream>
#include <algorithm>
#include <cmath>

using namespace std;
using namespace fre;

// ====== Randomly Sample 30 Stocks ======
vector<string> randomSample(const vector<string>& symbols) {
    int sampleSize = 30;

    if (symbols.size() < sampleSize) {
        return symbols;
    }

    vector<string> shuffled = symbols;
    random_device rd;
    mt19937 g(rd());
    shuffle(shuffled.begin(), shuffled.end(), g);

    vector<string> sample(shuffled.begin(), shuffled.begin() + sampleSize);
    return sample;
}

// ====== Calculate AAR (Average Abnormal Return) for a sample ======
vector<double> calculateAAR(const vector<vector<double>>& abnormalReturns, int N) {
    vector<double> aar(2*N + 1, 0.0);  // 👈注意是2N+1
    for (int t = 0; t < 2*N + 1; ++t) {
        double sum = 0.0;
        int count = 0;
        for (const auto& ar : abnormalReturns) {
            if (t < ar.size() && !isnan(ar[t])) {
                sum += ar[t];
                ++count;
            }
        }
        aar[t] = (count > 0) ? (sum / count) : 0.0;
    }
    return aar;
}

// ====== Calculate CAAR (Cumulative AAR) from AAR ======
vector<double> calculateCAAR(const vector<double>& aar) {
    vector<double> caar(aar.size(), 0.0);
    if (!aar.empty()) {
        caar[0] = aar[0];
        for (size_t i = 1; i < aar.size(); ++i) {
            caar[i] = caar[i-1] + aar[i];
        }
    }
    return caar;
}

// ====== Core Bootstrapping Function ======
void bootstrapGroup(
    const map<string, vector<double>>& abnormalReturnsMap,
    const vector<string>& groupSymbols,
    int N,
    int bootstrapTimes,
    vector<vector<double>>& AAR_samples,
    vector<vector<double>>& CAAR_samples
) {
    for (int b = 0; b < bootstrapTimes; ++b) {
        vector<string> sampleSymbols = randomSample(groupSymbols);
        vector<vector<double>> sampledAR;

        for (const auto& symbol : sampleSymbols) {
            auto it = abnormalReturnsMap.find(symbol);
            if (it != abnormalReturnsMap.end()) {
                sampledAR.push_back(it->second);
            }
        }

        vector<double> aar = calculateAAR(sampledAR, N);
        vector<double> caar = calculateCAAR(aar);

        AAR_samples.push_back(aar);
        CAAR_samples.push_back(caar);
    }
}

// ====== Compute Mean and Standard Deviation for AAR or CAAR ======
void computeMeanAndStd(
    const vector<vector<double>>& samples,
    vector<double>& mean,
    vector<double>& std
) {
    if (samples.empty()) return;

    int T = samples[0].size();
    int B = samples.size();

    mean.assign(T, 0.0);
    std.assign(T, 0.0);

    for (const auto& sample : samples) {
        for (int t = 0; t < T; ++t) {
            mean[t] += sample[t];
        }
    }
    for (int t = 0; t < T; ++t) {
        mean[t] /= B;
    }

    for (const auto& sample : samples) {
        for (int t = 0; t < T; ++t) {
            std[t] += (sample[t] - mean[t]) * (sample[t] - mean[t]);
        }
    }
    for (int t = 0; t < T; ++t) {
        std[t] = sqrt(std[t] / (B-1));
    }
}
