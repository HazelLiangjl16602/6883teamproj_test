#include "RetrieveData.h"
#include "curl/curl.h"
#include <iostream>

using namespace std;

int main() {
    DataRetriever retriever("Russell3000EarningsAnnouncements.csv");

    vector<StockEarnings> StockPrice; // StockPrice Vector can later be linked to stock class 
    vector<StockEarnings> Miss, Meet, Beat;

    retriever.populateStockPrice(StockPrice); // reads all earnings announcement into StockPrice
    retriever.sortAndDivideGroups(StockPrice, Miss, Meet, Beat); // sort StockPrice by surprise% and splits into thre egroups
 
      //  Convert all announcement dates to YYYY-MM-DD format
    for (auto& stock : Miss) {
        stock.date = ConvertDate_MMDDYYYY_to_YYYYMMDD(stock.date);
    }
    for (auto& stock : Meet) {
        stock.date = ConvertDate_MMDDYYYY_to_YYYYMMDD(stock.date);
    }
    for (auto& stock : Beat) {
        stock.date = ConvertDate_MMDDYYYY_to_YYYYMMDD(stock.date);
    }
    StockMap MissStockMap, MeetStockMap, BeatStockMap; // StockMap Map can later be linked to stock class 
    map<string, double> BenchmarkData; 

    string start_date = "2024-08-01";
    string end_date = "2025-04-16";
    string api_token = "680439755dd504.07501587";


    // directly from t11 for initializing CURL 
    CURL* handle;
    curl_global_init(CURL_GLOBAL_ALL);
    handle = curl_easy_init();

    if (handle) {
        retriever.fetchGroupPrices(handle, Miss, start_date, end_date, api_token, MissStockMap);
        retriever.fetchGroupPrices(handle, Meet, start_date, end_date, api_token, MeetStockMap);
        retriever.fetchGroupPrices(handle, Beat, start_date, end_date, api_token, BeatStockMap);

        StockMap TempBenchmarkMap;
        retriever.fetchHistoricalPrices(handle, "IWV", start_date, end_date, api_token, TempBenchmarkMap);

        for (const auto& [date, price] : TempBenchmarkMap["IWV"]) {
            BenchmarkData[date] = price;
        }

        curl_easy_cleanup(handle);
    } else {
        cerr << "CURL init failed" << endl;
    }

    curl_global_cleanup();


    // can be removed, csv are just for checking
    retriever.saveStockDataToCSV(MissStockMap, "MissStockData.csv");
    retriever.saveStockDataToCSV(MeetStockMap, "MeetStockData.csv");
    retriever.saveStockDataToCSV(BeatStockMap, "BeatStockData.csv");

    retriever.saveBenchmarkToCSV(BenchmarkData, "Benchmark_IWV.csv");
    retriever.saveEarningsGroupToCSV(Miss, "Group_Miss.csv");
    retriever.saveEarningsGroupToCSV(Meet, "Group_Meet.csv");
    retriever.saveEarningsGroupToCSV(Beat, "Group_Beat.csv");

    return 0;
}
