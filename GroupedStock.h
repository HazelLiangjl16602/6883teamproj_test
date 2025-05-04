#pragma once
#include <vector>
#include <map>
#include <string>
#include "Stock.h" // 注意是大写S！

using namespace std;

namespace fre {

    class GroupedStock: public Stock {
    private:
        vector<vector<double>> AAR_samples;
        vector<vector<double>> CAAR_samples;
        string groupName;
        vector<string> symbolList;

    public:
        GroupedStock();

        void addAARSample(const vector<double>& aar);
        void addCAARSample(const vector<double>& caar);
        const vector<vector<double>>& getAARsamples() const;
        const vector<vector<double>>& getCAARsamples() const;
        void clearSamples();

        void setGroupName(const string& name);
        string getGroupName() const;
        void addSymbol(const string& symbol);
        bool hasSymbol(const string& symbol) const;
    };

}
