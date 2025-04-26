#pragma once

#include <string>
#include <vector>
#include <map>

using namespace std;

// Alias for clarity
typedef map<string, map<string, double>> StockMap; // Here can use or refer to Stock.cpp/Stock.h 

struct StockEarnings {
    string ticker;
    string date;
    double surprisePercent;

    StockEarnings(string t, string d, double s);
};

class DataRetriever {
private:
    const string earningsFile;

public:
    DataRetriever(const string& filename);

    void populateStockPrice(vector<StockEarnings>& stockPriceList);
    void sortAndDivideGroups(const vector<StockEarnings>& StockPrice,
                              vector<StockEarnings>& Miss,
                              vector<StockEarnings>& Meet,
                              vector<StockEarnings>& Beat);

    bool fetchHistoricalPrices(void* handle,
                                const string& symbol,
                                const string& start_date,
                                const string& end_date,
                                const string& api_token,
                                StockMap& stockMap);

    void fetchGroupPrices(void* handle,
                           const vector<StockEarnings>& group,
                           const string& start_date,
                           const string& end_date,
                           const string& api_token,
                           StockMap& groupStockMap);
                           

    void saveStockDataToCSV(const StockMap& stockData, const string& filename);
    void saveBenchmarkToCSV(const map<string, double>& benchmarkData, const string& filename);
    void saveEarningsGroupToCSV(const vector<StockEarnings>& group, const string& filename);
};

string ConvertDate_MMDDYYYY_to_YYYYMMDD(const string& date);
string AddDaysToDate(const string& date, int days);

