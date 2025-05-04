// ===== Menu.h =====
#pragma once
#include "RetrieveData.h"
#include "Stock.h"
#include "GroupedStock.h"
#include "BootStrap.h"
#include <vector>
#include <map>
#include <string>

using namespace std;
using namespace fre;

class Menu {
    private:
    int choice;
    int N;
    DataRetriever retriever;

    std::vector<StockEarnings> MissGroup, MeetGroup, BeatGroup;
    GroupedStock MissStock, MeetStock, BeatStock;

    std::vector<std::vector<double>> Miss_AAR, Miss_CAAR;
    std::vector<std::vector<double>> Meet_AAR, Meet_CAAR;
    std::vector<std::vector<double>> Beat_AAR, Beat_CAAR;

    std::vector<double> Miss_AAR_mean, Miss_AAR_std, Miss_CAAR_mean, Miss_CAAR_std;
    std::vector<double> Meet_AAR_mean, Meet_AAR_std, Meet_CAAR_mean, Meet_CAAR_std;
    std::vector<double> Beat_AAR_mean, Beat_AAR_std, Beat_CAAR_mean, Beat_CAAR_std;

public:
    Menu();
    void DisplayMenu();
    void ProcessChoice();
    void HandleOption1();
    void HandleOption2();
    void HandleOption3();
    void HandleOption4();
    void HandleOption5();
};
