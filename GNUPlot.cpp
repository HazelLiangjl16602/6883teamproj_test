// ===== GNUPlot.cpp =====
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>

using namespace std;

void plotCAARCurves(const vector<double>& miss, const vector<double>& meet, const vector<double>& beat, int N) {
    int dataSize = 2 * N + 1;

    // if (miss.size() < dataSize || meet.size() < dataSize || beat.size() < dataSize) {
    //     fprintf(stderr, "Error: One or more CAAR vectors are smaller than expected (%d).\n", dataSize);
    //     return;
    // }

    FILE *gnuplotPipe, *tempFile1, *tempFile2, *tempFile3;
    const char* file1 = "miss_temp.dat";
    const char* file2 = "meet_temp.dat";
    const char* file3 = "beat_temp.dat";

    gnuplotPipe = popen("gnuplot -persist", "w");

    if (gnuplotPipe) {
        // Create temporary files for each group
        tempFile1 = fopen(file1, "w");
        tempFile2 = fopen(file2, "w");
        tempFile3 = fopen(file3, "w");

        for (int i = 0; i < dataSize; ++i) {
            int day = i - N;
            fprintf(tempFile1, "%d %lf\n", day, miss[i]);
            fprintf(tempFile2, "%d %lf\n", day, meet[i]);
            fprintf(tempFile3, "%d %lf\n", day, beat[i]);
        }

        fclose(tempFile1);
        fclose(tempFile2);
        fclose(tempFile3);

        // Write gnuplot commands
        fprintf(gnuplotPipe, "set title 'CAAR Comparison Across Groups'\n");
        fprintf(gnuplotPipe, "set xlabel 'Event Window (Days from Earnings Announcement)'\n");
        fprintf(gnuplotPipe, "set ylabel 'CAAR'\n");
        fprintf(gnuplotPipe, "set grid\n");
        fprintf(gnuplotPipe, "set key outside\n");
        fprintf(gnuplotPipe, "plot '%s' with lines title 'Miss' lw 2, \\\n", file1);
        fprintf(gnuplotPipe, "     '%s' with lines title 'Meet' lw 2, \\\n", file2);
        fprintf(gnuplotPipe, "     '%s' with lines title 'Beat' lw 2\n", file3);

        fflush(gnuplotPipe);
        printf("✅ CAAR graph generated with gnuplot. Press Enter to continue...");
        getchar();

        // Cleanup
        remove(file1);
        remove(file2);
        remove(file3);
        fprintf(gnuplotPipe, "exit\n");
        pclose(gnuplotPipe);
    } else {
        printf("❌ gnuplot not found. Make sure it's installed and accessible from terminal.\n");
    }
}
