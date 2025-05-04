#include "GroupedStock.h"
#include <algorithm>
#include <cmath>
#include <iostream>

using namespace std;
namespace fre {

    GroupedStock::GroupedStock(): Stock() {}

    void GroupedStock::addAARSample(const vector<double>& aar) {
        AAR_samples.push_back(aar);
    }

    void GroupedStock::addCAARSample(const vector<double>& caar) {
        CAAR_samples.push_back(caar);
    }

    const vector<vector<double>>& GroupedStock::getAARsamples() const {
        return AAR_samples;
    }

    const vector<vector<double>>& GroupedStock::getCAARsamples() const {
        return CAAR_samples;
    }

    void GroupedStock::clearSamples() {
        AAR_samples.clear();
        CAAR_samples.clear();
    }

    void GroupedStock::setGroupName(const string& name) {
        groupName = name;
    }

    string GroupedStock::getGroupName() const {
        return groupName;
    }

    void GroupedStock::addSymbol(const string& symbol) {
        symbolList.push_back(symbol);
    }

    bool GroupedStock::hasSymbol(const string& symbol) const {
        return find(symbolList.begin(), symbolList.end(), symbol) != symbolList.end();
    }

}
