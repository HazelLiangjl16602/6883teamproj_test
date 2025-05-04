#include "RetrieveData.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <cstring>
#include "curl/curl.h"
#include <algorithm>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <cmath> // 用于 std::nan("")


using namespace std;

// Struct constructor
StockEarnings::StockEarnings(string t, string d, double s)
    : ticker(t), date(d), surprisePercent(s) {}

// Constructor for DataRetriever
DataRetriever::DataRetriever(const string& filename)
    : earningsFile(filename) {}


/*    void DataRetriever::populateStockPrice(vector<StockEarnings>& stockPriceList) {
    ifstream fin(earningsFile);
    if (!fin) {
        cerr << "Error opening earnings announcement file." << endl;
        return;
    }

    string line;
    getline(fin, line); // skip header

    while (getline(fin, line)) {
        stringstream ss(line);
        string ticker, date, period, estimate, reported, surprise, surprise_pct;

        getline(ss, ticker, ',');
        getline(ss, date, ',');
        getline(ss, period, ',');
        getline(ss, estimate, ',');
        getline(ss, reported, ',');
        getline(ss, surprise, ',');
        getline(ss, surprise_pct, ',');

        try {
            double pct = stod(surprise_pct);
            stockPriceList.emplace_back(ticker, date, pct);
        } catch (...) {
            cerr << "[Warning] Invalid surprise_pct in line: " << line << endl;
        }
    }

    fin.close();
}
*/
void DataRetriever::populateStockPrice(vector<StockEarnings>& stockPriceList) {
    ifstream fin(earningsFile);
    if (!fin) {
        cerr << "Error opening earnings announcement file." << endl;
        return;
    }

    string line;
    getline(fin, line); // skip header

    while (getline(fin, line)) {
        stringstream ss(line);
        string ticker, date, period, estimate, reported, surprise, surprise_pct;
    
        getline(ss, ticker, ',');
        getline(ss, date, ',');
        getline(ss, period, ',');
        getline(ss, estimate, ',');
        getline(ss, reported, ',');
        getline(ss, surprise, ',');
        getline(ss, surprise_pct, ',');
    
        try {
            if (ticker.empty()) {
                cerr << "[Warning] Empty ticker in line: " << line << endl;
                continue;
            }
            date = ConvertDate_MMDDYYYY_to_YYYYMMDD(date); // 🔥 加这一行！
            double pct = stod(surprise_pct);
            stockPriceList.emplace_back(ticker, date, pct);
        } catch (...) {
            cerr << "[Warning] Invalid surprise_pct in line: " << line << endl;
        }
    }
    

    fin.close(); // 原本就有的

    
}


bool compareSurprise(const StockEarnings& a, const StockEarnings& b) {
    return a.surprisePercent < b.surprisePercent;
}

void DataRetriever::sortAndDivideGroups(const vector<StockEarnings>& StockPrice,
                                        vector<StockEarnings>& Miss,
                                        vector<StockEarnings>& Meet,
                                        vector<StockEarnings>& Beat) {
    vector<StockEarnings> sorted = StockPrice;
    sort(sorted.begin(), sorted.end(), compareSurprise);

    size_t size = sorted.size();
    size_t third = size / 3;

    Miss.assign(sorted.begin(), sorted.begin() + third);
    Meet.assign(sorted.begin() + third, sorted.begin() + 2 * third);
    Beat.assign(sorted.begin() + 2 * third, sorted.end());
}

// Internal libcurl helper
struct MemoryStruct {
    char* memory;
    size_t size;
};

void* myrealloc(void* ptr, size_t size) {
    if (ptr)
        return realloc(ptr, size);
    else
        return malloc(size);
}

int write_data2(void* ptr, size_t size, size_t nmemb, void* data) {
    size_t realsize = size * nmemb;
    struct MemoryStruct* mem = (struct MemoryStruct*)data;
    mem->memory = (char*)myrealloc(mem->memory, mem->size + realsize + 1);
    if (mem->memory) {
        memcpy(&(mem->memory[mem->size]), ptr, realsize);
        mem->size += realsize;
        mem->memory[mem->size] = 0;
    }
    return realsize;
}

bool DataRetriever::fetchHistoricalPrices(void* handle, const string& symbol,
                                          const string& start_date,
                                          const string& end_date,
                                          const string& api_token,
                                          StockMap& stockMap) {
    struct MemoryStruct data;
    data.memory = NULL;
    data.size = 0;

    string url_common = "https://eodhistoricaldata.com/api/eod/";
    string url_request = url_common + symbol + ".US?" +
                          "from=" + start_date +
                          "&to=" + end_date +
                          "&api_token=" + api_token +
                          "&period=d";


    // cout << "Fetching: " << symbol << endl;  // can be removed fecthing log -> I want to check which stock is being fetched
    static int counter = 0;
    if (++counter % 100 == 0) cout << counter << " stocks fetched..." << endl; // can be removed -> print every 100 stocks instead of every 1



    curl_easy_setopt(handle, CURLOPT_URL, url_request.c_str());
    curl_easy_setopt(handle, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows NT 10.0; Win64; x64)");
    curl_easy_setopt(handle, CURLOPT_SSL_VERIFYPEER, 0);
    curl_easy_setopt(handle, CURLOPT_SSL_VERIFYHOST, 0);
    curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, write_data2);
    curl_easy_setopt(handle, CURLOPT_WRITEDATA, (void*)&data);

    CURLcode status = curl_easy_perform((CURL*)handle);
    if (status != CURLE_OK) {
        cerr << "curl_easy_perform() failed: " << curl_easy_strerror(status) << endl;
        return false;
    }

    stringstream sData;
    sData.str(data.memory);
    string line, sDate, sValue;

    while (getline(sData, line)) {
        if (line.find('-') == string::npos) continue;

        sDate = line.substr(0, line.find_first_of(','));
        line.erase(line.find_last_of(','));
        sValue = line.substr(line.find_last_of(',') + 1);

        try {
            double dPrice = stod(sValue);
            stockMap[symbol][sDate] = dPrice;
        } catch (...) {
            cerr << "Invalid price data: " << line << endl;
        }
    }

    free(data.memory);
    return true;
}

void DataRetriever::fetchGroupPrices(void* handle,
                                     const vector<StockEarnings>& group,
                                     const string& start_date,
                                     const string& end_date,
                                     const string& api_token,
                                     StockMap& groupStockMap) {
    for (const auto& stock : group) {
        fetchHistoricalPrices(handle, stock.ticker, start_date, end_date, api_token, groupStockMap);
    }
}


string ConvertDate_MMDDYYYY_to_YYYYMMDD(const string& date) {
    int firstSlash = date.find('/');
    int secondSlash = date.find('/', firstSlash + 1);

    if (firstSlash == string::npos || secondSlash == string::npos) {
        cerr << "Invalid MM/DD/YYYY date format: " << date << endl;
        return "";
    }

    string mm = date.substr(0, firstSlash);
    string dd = date.substr(firstSlash + 1, secondSlash - firstSlash - 1);
    string yyyy = date.substr(secondSlash + 1);

    // Add leading zeros if needed
    if (mm.length() == 1) mm = "0" + mm;
    if (dd.length() == 1) dd = "0" + dd;

    return yyyy + "-" + mm + "-" + dd;
}



string AddDaysToDate(const string& date, int days) {
    struct tm tm = {};
    istringstream ss(date);
    ss >> get_time(&tm, "%Y-%m-%d");
    if (ss.fail()) {
        cerr << "Parse failed for date: " << date << endl;
        return "";
    }

    tm.tm_mday += days; // Add days

    // Normalize the date (tm structure will handle month/year rollover)
    mktime(&tm);

    ostringstream out;
    out << put_time(&tm, "%Y-%m-%d");
    return out.str();
}


vector<pair<int, double>> DataRetriever::alignPriceToDay0(
    const string& announcementDate,
    const map<string, double>& priceHistory,
    int N
) {
    vector<pair<int, double>> alignedSeries;

    if (priceHistory.empty()) {
        cerr << "Price history is empty for announcement: " << announcementDate << endl;
        return alignedSeries;
    }

    // Step 1: 排序所有交易日
    vector<string> sortedDates;
    for (const auto& it : priceHistory) {
        sortedDates.push_back(it.first);
    }
    sort(sortedDates.begin(), sortedDates.end()); // 保证从早到晚

    // Step 2: 找到最接近的Day0（只往后找，最多7天）
    struct tm annDate = {};
    istringstream ss(announcementDate);
    ss >> get_time(&annDate, "%Y-%m-%d");
    if (ss.fail()) {
        cerr << "Parse announcement date failed: " << announcementDate << endl;
        return alignedSeries;
    }

    string day0Date = "";
    for (int shift = 0; shift <= 7; ++shift) {
        struct tm temp = annDate;
        temp.tm_mday += shift;
        mktime(&temp);
        ostringstream out;
        out << put_time(&temp, "%Y-%m-%d");
        string shiftedDate = out.str();

        if (binary_search(sortedDates.begin(), sortedDates.end(), shiftedDate)) {
            day0Date = shiftedDate;
            break;
        }
    }

    if (day0Date == "") {
        cerr << "Cannot align stock with announcement: " << announcementDate << endl;
        return alignedSeries;
    }

    // Step 3: 确定day0在sortedDates中的位置
    auto itDay0 = find(sortedDates.begin(), sortedDates.end(), day0Date);
    if (itDay0 == sortedDates.end()) {
        cerr << "Unexpected error: day0 not found after binary search" << endl;
        return alignedSeries;
    }
    int day0Index = distance(sortedDates.begin(), itDay0);

    // Step 4: 从day0往前N天+往后N天提取
    for (int offset = -N; offset <= N; ++offset) {
        int targetIndex = day0Index + offset;
        if (targetIndex >= 0 && targetIndex < (int)sortedDates.size()) {
            string targetDate = sortedDates[targetIndex];
            double price = priceHistory.at(targetDate);
            alignedSeries.push_back({offset, price});
        } else {
            alignedSeries.push_back({offset, std::nan("")}); // 超出范围时补nan
        }
    }

    return alignedSeries;
}








// void DataRetriever::saveStockDataToCSV(const StockMap& stockData, const string& filename) {
//     ofstream fout(filename);
//     fout << "ticker,date,adjusted_close\n";
//     for (const auto& [symbol, dailyData] : stockData) {
//         for (const auto& [date, price] : dailyData) {
//             fout << symbol << "," << date << "," << fixed << setprecision(6) << price << "\n";
//         }
//     }
//     fout.close();
// }

// void DataRetriever::saveBenchmarkToCSV(const map<string, double>& benchmarkData, const string& filename) {
//     ofstream fout(filename);
//     fout << "date,benchmark_IWV\n";
//     for (const auto& [date, price] : benchmarkData) {
//         fout << date << "," << fixed << setprecision(6) << price << "\n";
//     }
//     fout.close();
// }

// void DataRetriever::saveEarningsGroupToCSV(const vector<StockEarnings>& group, const string& filename) {
//     ofstream fout(filename);
//     fout << "ticker,date,surprise_percent\n";
//     for (const auto& entry : group) {
//         fout << entry.ticker << "," << entry.date << "," << fixed << setprecision(4) << entry.surprisePercent << "\n";
//     }
//     fout.close();
// }




