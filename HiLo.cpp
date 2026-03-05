// HiLo.cpp
// Sierra Chart ACSIL Custom Indicator
//
// Scans all bars on the chart to find the single highest High and the single
// lowest Low, then draws a flat horizontal gold/yellow line at the chart High
// and a flat horizontal light purple/pink line at the chart Low.
// Each line is defined as its own Subgraph so users can customize colors,
// line width, and draw style independently through the Sierra Chart study
// settings dialog.

#include "SierraChartACSIL.h"

SCSFExport scsf_HiLo(SCStudyInterfaceRef sc)
{
    // -----------------------------------------------------------------
    // Subgraph references
    // -----------------------------------------------------------------
    SCSubgraphRef HighLine = sc.Subgraph[0];
    SCSubgraphRef LowLine  = sc.Subgraph[1];

    // -----------------------------------------------------------------
    // SetDefaults block – runs once when the study is first added or
    // when Sierra Chart needs to read the study settings.
    // -----------------------------------------------------------------
    if (sc.SetDefaults)
    {
        sc.GraphName    = "Hi-Lo Lines";
        sc.StudyVersion = 2;
        sc.AutoLoop     = 0;  // Manual loop – we scan all bars ourselves

        // High subgraph – gold / yellow line
        HighLine.Name         = "High Line";
        HighLine.DrawStyle    = DRAWSTYLE_LINE;
        HighLine.PrimaryColor = RGB(255, 215, 0);   // Gold
        HighLine.LineWidth    = 2;
        HighLine.DrawZeros    = 0;

        // Low subgraph – light purple / plum line
        LowLine.Name         = "Low Line";
        LowLine.DrawStyle    = DRAWSTYLE_LINE;
        LowLine.PrimaryColor = RGB(221, 160, 221);  // Plum (light purple)
        LowLine.LineWidth    = 2;
        LowLine.DrawZeros    = 0;

        return;
    }

    // -----------------------------------------------------------------
    // Chart-wide calculation
    // When sc.UpdateStartIndex == 0 a full rescan is required (e.g. study
    // first added, settings changed, or chart replay).  For incremental
    // updates (new bar / real-time tick), we seed from the already-computed
    // extrema stored in the subgraph and only examine bars from
    // sc.UpdateStartIndex onward, avoiding a full O(n) rescan each tick.
    // Pass 2 stamps every bar with the global extrema so both subgraphs
    // appear as flat horizontal lines across the whole chart.
    // -----------------------------------------------------------------
    if (sc.ArraySize < 1)
        return;

    float chartHigh;
    float chartLow;

    if (sc.UpdateStartIndex == 0)
    {
        // Full rescan
        chartHigh = sc.High[0];
        chartLow  = sc.Low[0];

        for (int i = 1; i < sc.ArraySize; ++i)
        {
            if (sc.High[i] > chartHigh)
                chartHigh = sc.High[i];
            if (sc.Low[i] < chartLow)
                chartLow = sc.Low[i];
        }
    }
    else
    {
        // Incremental update: seed from the previously computed extrema
        // (all prior bars were stamped with the same value, so index 0 holds it)
        chartHigh = HighLine[0];
        chartLow  = LowLine[0];

        for (int i = sc.UpdateStartIndex; i < sc.ArraySize; ++i)
        {
            if (sc.High[i] > chartHigh)
                chartHigh = sc.High[i];
            if (sc.Low[i] < chartLow)
                chartLow = sc.Low[i];
        }
    }

    for (int i = 0; i < sc.ArraySize; ++i)
    {
        HighLine[i] = chartHigh;
        LowLine[i]  = chartLow;
    }
}
