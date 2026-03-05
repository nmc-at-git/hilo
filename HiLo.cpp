// HiLo.cpp
// Sierra Chart ACSIL Custom Indicator
//
// Draws a gold/yellow line at the chart High and a light purple/pink line at
// the chart Low for every bar on the chart.  Each line is defined as its own
// Subgraph so users can customize colors, line width, and draw style
// independently through the Sierra Chart study settings dialog.

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
        sc.StudyVersion = 1;
        sc.AutoLoop     = 1;  // Sierra Chart calls this function once per bar

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
    // Per-bar calculation
    // -----------------------------------------------------------------
    HighLine[sc.Index] = sc.High[sc.Index];
    LowLine[sc.Index]  = sc.Low[sc.Index];
}
