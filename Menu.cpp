// ================= Menu.cpp =================
#include "Menu.h"
#include "GNUPlot.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <thread>
#include <mutex>
#include <curl/curl.h>

using namespace std;
using namespace fre;

static std::mutex stockMapMutex;          // protects shared maps

// ───────── constructor & menu ─────────
Menu::Menu() : choice(0), N(30),
               retriever("Russell3000EarningsAnnouncements.csv")
{
    DisplayMenu();
    ProcessChoice();
}

void Menu::DisplayMenu()
{
    cout << "\n===== Financial Event Study Menu =====\n"
         << "1. Enter N to retrieve historical data (N 30‑60)\n"
         << "2. Pull information for one stock\n"
         << "3. Show AAR / AAR‑STD / CAAR / CAAR‑STD for one group\n"
         << "4. Show CAAR gnuplot graph for all groups\n"
         << "5. Exit\n"
         << "=======================================\n";
}

void Menu::ProcessChoice()
{
    while (true)
    {
        cout << "Please enter your choice: ";
        cin  >> choice;

        switch (choice)
        {
            case 1: HandleOption1(); break;
            case 2: HandleOption2(); break;
            case 3: HandleOption3(); break;
            case 4: HandleOption4(); break;
            case 5: HandleOption5(); return;
            default: cout << "Invalid choice. Try again.\n";
        }
    }
}

// ───────── Option‑1 : fetch & compute ─────────
void Menu::HandleOption1()
{
    cout << "Enter N (30‑60): ";  cin >> N;
    if (N < 30 || N > 60) { cout << "Invalid N.\n"; return; }

    cout << "Fetching 2N+1 days with N = " << N << " …\n";

    /* 1) read earnings csv and split into 3 groups */
    vector<StockEarnings> allEarnings;
    retriever.populateStockPrice(allEarnings);
    retriever.sortAndDivideGroups(allEarnings,
                                  MissGroup, MeetGroup, BeatGroup);

    curl_global_init(CURL_GLOBAL_ALL);

    string startDate = "2024-08-01",
           endDate   = "2025-04-20",
           apiToken  = "YOUR_API_TOKEN";   // <= set yours

    StockMap MissRaw, MeetRaw, BeatRaw;     // date‑keyed prices
    StockMap BenchRaw;                      // IWV raw

    /* 2) multithreaded fetch: 3 threads at a time */
    vector<pair<StockEarnings,StockMap*>> jobs;
    for (auto& s : MissGroup) jobs.emplace_back(s,&MissRaw);
    for (auto& s : MeetGroup) jobs.emplace_back(s,&MeetRaw);
    for (auto& s : BeatGroup) jobs.emplace_back(s,&BeatRaw);

    size_t idx = 0;
    while (idx < jobs.size())
    {
        vector<thread> batch;
        for (int k=0; k<3 && idx<jobs.size(); ++k, ++idx)
        {
            batch.emplace_back([&, j = idx]()
            {
                CURL* h = curl_easy_init();
                if (!h) return;
                StockMap tmp;
                auto& [se, target] = jobs[j];
                if (retriever.fetchHistoricalPrices(
                        h, se.ticker, startDate, endDate,
                        apiToken, tmp))
                {
                    lock_guard<mutex> lg(stockMapMutex);
                    (*target)[se.ticker] = tmp[se.ticker];
                }
                curl_easy_cleanup(h);
            });
        }
        for (auto& t : batch) t.join();
    }
    cout << "Stock data fetched (3 threads cap).\n";

    /* 3) fetch IWV benchmark (single thread) */
    CURL* hBench = curl_easy_init();
    retriever.fetchHistoricalPrices(hBench,"IWV",
                                    startDate,endDate,apiToken,
                                    BenchRaw);
    curl_easy_cleanup(hBench);
    curl_global_cleanup();

    /* 4) build aligned maps keyed by −N…+N */
    map<string,map<int,double>> MissAligned, MeetAligned, BeatAligned;

    auto buildAligned = [&](const vector<StockEarnings>& grp,
                            const StockMap& raw,
                            map<string,map<int,double>>& aligned)
    {
        for (const auto& se : grp)
        {
            auto vec = retriever.alignPriceToDay0(
                           se.date, raw.at(se.ticker), N);
            for (const auto& [off, px] : vec)
                aligned[se.ticker][off] = px;
        }
    };

    buildAligned(MissGroup, MissRaw, MissAligned);
    buildAligned(MeetGroup, MeetRaw, MeetAligned);
    buildAligned(BeatGroup, BeatRaw, BeatAligned);

    /* 5) load into GroupedStock objects */
    MissStock.setAlignedPrices(MissAligned);
    MeetStock.setAlignedPrices(MeetAligned);
    BeatStock.setAlignedPrices(BeatAligned);

    for (auto& s : MissGroup) MissStock.addSymbol(s.ticker);
    for (auto& s : MeetGroup) MeetStock.addSymbol(s.ticker);
    for (auto& s : BeatGroup) BeatStock.addSymbol(s.ticker);

    /* 6) compute AR / AAR / CAAR */
    auto benchAligned = ConvertBenchmark(BenchRaw["IWV"]);

    auto MissAR = MissStock.computeAllAbnormalReturns(
                      MissAligned, benchAligned, N);
    auto MeetAR = MeetStock.computeAllAbnormalReturns(
                      MeetAligned, benchAligned, N);
    auto BeatAR = BeatStock.computeAllAbnormalReturns(
                      BeatAligned, benchAligned, N);

    vector<string> MissSymbols, MeetSymbols, BeatSymbols;
    for (const auto& kv : MissAR) MissSymbols.push_back(kv.first);
    for (const auto& kv : MeetAR) MeetSymbols.push_back(kv.first);
    for (const auto& kv : BeatAR) BeatSymbols.push_back(kv.first);

    bootstrapGroup(MissAR, MissSymbols, N, 40, Miss_AAR, Miss_CAAR);
    bootstrapGroup(MeetAR, MeetSymbols, N, 40, Meet_AAR, Meet_CAAR);
    bootstrapGroup(BeatAR, BeatSymbols, N, 40, Beat_AAR, Beat_CAAR);

    computeMeanAndStd(Miss_AAR,  Miss_AAR_mean,  Miss_AAR_std);
    computeMeanAndStd(Miss_CAAR, Miss_CAAR_mean, Miss_CAAR_std);
    computeMeanAndStd(Meet_AAR,  Meet_AAR_mean,  Meet_AAR_std);
    computeMeanAndStd(Meet_CAAR, Meet_CAAR_mean, Meet_CAAR_std);
    computeMeanAndStd(Beat_AAR,  Beat_AAR_mean,  Beat_AAR_std);
    computeMeanAndStd(Beat_CAAR, Beat_CAAR_mean, Beat_CAAR_std);

    cout << "\n============= Group Summary (Day 0) =============\n"
         << "Group     AAR        AAR‑STD     CAAR       CAAR‑STD\n"
         << fixed << setprecision(6)
         << "Miss   " << Miss_AAR_mean[N]  << ' ' << Miss_AAR_std[N]
         << "   "    << Miss_CAAR_mean[N] << ' ' << Miss_CAAR_std[N] << '\n'
         << "Meet   " << Meet_AAR_mean[N]  << ' ' << Meet_AAR_std[N]
         << "   "    << Meet_CAAR_mean[N] << ' ' << Meet_CAAR_std[N] << '\n'
         << "Beat   " << Beat_AAR_mean[N]  << ' ' << Beat_AAR_std[N]
         << "   "    << Beat_CAAR_mean[N] << ' ' << Beat_CAAR_std[N] << '\n'
         << "==================================================\n";
}

// ───────── Option‑2 : single‑stock info ─────────
void Menu::HandleOption2()
{
    string t;  cout << "Enter ticker: ";  cin >> t;

    if      (MissStock.hasSymbol(t)) cout << t << " in Miss group.\n";
    else if (MeetStock.hasSymbol(t)) cout << t << " in Meet group.\n";
    else if (BeatStock.hasSymbol(t)) cout << t << " in Beat group.\n";
    else { cout << "Ticker not found.\n"; return; }

    cout << "(Extend to show prices / returns / EPS…)\n";
}

// ───────── Option‑3 : print matrices ─────────
void Menu::HandleOption3()
{
    cout << "Select group (1‑Miss 2‑Meet 3‑Beat): ";
    int g; cin >> g;

    const vector<double> *aar=nullptr,*aarStd=nullptr,
                         *caar=nullptr,*caarStd=nullptr;
    string gName;
    if      (g==1){ aar=&Miss_AAR_mean; aarStd=&Miss_AAR_std;
                    caar=&Miss_CAAR_mean; caarStd=&Miss_CAAR_std; gName="Miss";}
    else if (g==2){ aar=&Meet_AAR_mean; aarStd=&Meet_AAR_std;
                    caar=&Meet_CAAR_mean; caarStd=&Meet_CAAR_std; gName="Meet";}
    else if (g==3){ aar=&Beat_AAR_mean; aarStd=&Beat_AAR_std;
                    caar=&Beat_CAAR_mean; caarStd=&Beat_CAAR_std; gName="Beat";}
    else { cout << "Bad choice.\n"; return; }

    int len = 2*N+1;
    auto printVec=[&](const vector<double>& v,string title)
    {
        cout << "\n=== " << title << " ("<<gName<<") ===\n";
        for(int i=0;i<len;++i){ cout<<fixed<<setprecision(6)<<v[i]<<' ';
            if((i+1)%10==0) cout<<'\n'; }
        cout<<'\n';
    };
    printVec(*aar,  "Avg AAR");
    printVec(*aarStd,"AAR STD");
    printVec(*caar, "Avg CAAR");
    printVec(*caarStd,"CAAR STD");
}

// ───────── Option‑4 : gnuplot ─────────
void Menu::HandleOption4()
{
    cout << "Plotting CAAR curves …\n";
    plotCAARCurves(Miss_CAAR_mean,
                   Meet_CAAR_mean,
                   Beat_CAAR_mean, N);
}

// ───────── Option‑5 : exit ─────────
void Menu::HandleOption5()
{
    cout << "Good‑bye!\n";
}
