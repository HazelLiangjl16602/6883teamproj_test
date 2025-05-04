// ====== BootstrapAnalysis.h ======
#pragma once
#include "Stock.h"
#include "GroupedStock.h"
#include "Operations.h"
#include <vector>
#include <map>
#include <string>

using namespace std;

// ====== Function Declarations ======

vector<string> randomSample(const vector<string>& symbols);  

vector<double> calculateAAR(const vector<vector<double>>& abnormalReturns, int N);

vector<double> calculateCAAR(const vector<double>& aar);

void bootstrapGroup(
    const map<string, vector<double>>& abnormalReturnsMap,
    const vector<string>& groupSymbols,
    int N,
    int bootstrapTimes,
    vector<vector<double>>& AAR_samples,
    vector<vector<double>>& CAAR_samples
);

void computeMeanAndStd(
    const vector<vector<double>>& samples,
    vector<double>& mean,
    vector<double>& std
);

// ====== END ======