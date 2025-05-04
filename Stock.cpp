// ================= Stock.cpp =================
#include "Stock.h"
#include <algorithm>
#include <cmath>
#include <iostream>

using namespace std;

namespace fre {

// ---------- constructors ----------
Stock::Stock() {}

Stock::Stock(const StockMap& data,
             const map<string,EarningsInfo>& earnings)
        : stockData(data), earningsData(earnings) {}

// ---------- basic setters ----------
void Stock::addPriceData(const string& sym,
                         const map<string,double>& px)
{
    stockData[sym] = px;
}

void Stock::addEarningsInfo(const string& sym,
                            const EarningsInfo& info)
{
    earningsData[sym] = info;
}

void Stock::setAlignedPrices(
        const map<string,map<int,double>>& ap)
{
    alignedPrices = ap;          // keys are −N…+N
}

// ---------- helpers ----------
vector<double>
Stock::getDailyPrices(const string& sym,int N) const
{
    vector<double> v;
    auto it = alignedPrices.find(sym);
    if (it != alignedPrices.end())
        for (int off=-N; off<=N; ++off)
            v.push_back( it->second.count(off)
                          ? it->second.at(off) : std::nan("") );
    return v;
}

vector<double>
Stock::getDailyReturns(const string& sym,int N) const
{
    vector<double> p = getDailyPrices(sym,N), r;
    for (size_t i=1;i<p.size();++i)
        r.push_back( (!isnan(p[i-1]) && !isnan(p[i]) &&
                      p[i-1]>0 && p[i]>0)
                     ? log(p[i]/p[i-1]) : std::nan("") );
    return r;
}

vector<double>
Stock::getCumulativeReturns(const string& sym,int N) const
{
    vector<double> ret = getDailyReturns(sym,N),
                   cum(ret.size(),0.0);
    if (!ret.empty())
        for (size_t i=0;i<ret.size();++i)
            cum[i] = (i==0) ? ret[0]
                            : (!isnan(cum[i-1])&&!isnan(ret[i])
                               ? cum[i-1]+ret[i] : std::nan(""));
    return cum;
}

// ---------- earnings ----------
EarningsInfo Stock::getEarningsInfo(const string& sym) const
{
    auto it = earningsData.find(sym);
    return it!=earningsData.end() ? it->second : EarningsInfo();
}

void Stock::printStockSummary(const string& sym) const
{
    cout << "===== Stock Summary: " << sym << " =====\n";
    auto ei = getEarningsInfo(sym);
    if (ei.ticker.empty()) { cout << "No earnings data.\n"; return; }
    cout << "Announcement Date: " << ei.announcementDate << '\n'
         << "Period Ending   : " << ei.periodEnding    << '\n'
         << "Estimated EPS   : " << ei.estimatedEPS    << '\n'
         << "Reported EPS    : " << ei.reportedEPS     << '\n'
         << "Surprise        : " << ei.surprise        << '\n'
         << "Surprise %      : " << ei.surprisePercent << "%\n";
}

// ---------- log & abnormal returns ----------
vector<double>
Stock::computeLogReturns(const map<int,double>& px,int N)
{
    vector<double> lr;
    for (int d=-N; d<=N-1; ++d)
        lr.push_back( px.count(d)&&px.count(d+1)&&
                      px.at(d)>0&&px.at(d+1)>0
                      ? log(px.at(d+1)/px.at(d)) : std::nan(""));
    return lr;
}

vector<double>
Stock::computeAbnormalReturns(const vector<double>& st,
                              const vector<double>& bm,int N)
{
    vector<double> ar;
    for (int i=0;i<2*N;++i)
        ar.push_back( i<st.size()&&i<bm.size() &&
                      !isnan(st[i])&&!isnan(bm[i])
                      ? st[i]-bm[i] : std::nan(""));
    return ar;
}

map<string,vector<double>>
Stock::computeAllAbnormalReturns(
        const map<string,map<int,double>>& aligned,
        const map<int,double>& bench,int N)
{
    map<string,vector<double>> out;
    vector<double> bmLog = computeLogReturns(bench,N);
    for (const auto& kv : aligned)
    {
        auto stLog = computeLogReturns(kv.second,N);
        out[kv.first] = computeAbnormalReturns(stLog,bmLog,N);
    }
    return out;
}

// ---------- helper for benchmark only ----------
map<int,double>
ConvertBenchmark(const map<string,double>& datePx)
{
    map<int,double> out;
    int idx=0;
    for (const auto& kv : datePx) out[idx++] = kv.second;
    return out;
}

} // namespace fre
// =============== end Stock.cpp ===============
