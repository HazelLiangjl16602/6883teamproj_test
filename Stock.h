#pragma once
#include <vector>
#include <map>
#include <string>


using namespace std;

namespace fre {

    typedef map<string, map<string, double>> StockMap; // Symbol -> (date -> adj_close)

    struct EarningsInfo {
        string ticker;
        string announcementDate;
        string periodEnding;
        double estimatedEPS;
        double reportedEPS;
        double surprise;
        double surprisePercent;

        EarningsInfo() : estimatedEPS(0), reportedEPS(0), surprise(0), surprisePercent(0) {}
    };

    class Stock {
    protected:
        StockMap stockData;
        map<string, EarningsInfo> earningsData;
        map<string, map<int, double>> alignedPrices;

    public:
        Stock();
        Stock(const StockMap& data, const map<string, EarningsInfo>& earnings);

        void addPriceData(const string& symbol, const map<string, double>& prices);
        void addEarningsInfo(const string& symbol, const EarningsInfo& info);

        void setAlignedPrices(const map<string, map<int, double>>& newAlignedPrices);


        // ========== Daily Prices and Returns ==========
        vector<double> getDailyPrices(const string& symbol, int N) const;
        vector<double> getDailyReturns(const string& symbol, int N) const;
        vector<double> getCumulativeReturns(const string& symbol, int N) const;

        EarningsInfo getEarningsInfo(const string& symbol) const;
        void printStockSummary(const string& symbol) const;

        // ========== New Added for Bootstrapping ==========
        vector<double> computeLogReturns(const map<int, double>& alignedPrice, int N);
        vector<double> computeAbnormalReturns(const vector<double>& stockLogReturns, const vector<double>& benchmarkLogReturns, int N);
        map<string, vector<double>> computeAllAbnormalReturns(const map<string, map<int, double>>& stockAlignedPrices, const map<int, double>& benchmarkAlignedPrice, int N);
    };

    map<string, map<int, double>> ConvertStockMap(const StockMap& originalMap);
    map<int, double> ConvertBenchmark(const map<string, double>& originalMap);

}

	
