// HiLo.cpp
// Sierra Chart ACSIL Custom Indicator
//
// Scans the currently visible bars in the chart viewport to find the highest
// High and lowest Low, then draws a flat horizontal gold/yellow line at the
// visible High and a flat horizontal light purple/pink line at the visible Low.
// Lines update automatically as the user scrolls or zooms the chart.
// Each line is defined as its own Subgraph so users can customize colors,
// line width, and draw style independently through the Sierra Chart study
// settings dialog.

#include "sierrachart.h"

SCDLLName("HiLo")

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
        sc.AutoLoop     = 0;  // Manual loop – we scan visible bars ourselves

        // High subgraph – gold / yellow line
        HighLine.Name         = "High Line";
        HighLine.DrawStyle    = DRAWSTYLE_LINE;
        HighLine.PrimaryColor = RGB(255, 215, 0);   // Gold
        HighLine.LineWidth    = 2;
        HighLine.DrawZeros    = 0;  // Hide bars outside the visible range (value 0)

        // Low subgraph – light purple / plum line
        LowLine.Name         = "Low Line";
        LowLine.DrawStyle    = DRAWSTYLE_LINE;
        LowLine.PrimaryColor = RGB(221, 160, 221);  // Plum (light purple)
        LowLine.LineWidth    = 2;
        LowLine.DrawZeros    = 0;  // Hide bars outside the visible range (value 0)

        return;
    }

    // -----------------------------------------------------------------
    // Visible-range calculation
    // Determine the visible bar range using sc.IndexOfFirstVisibleBar and
    // sc.IndexOfLastVisibleBar, then compute extrema only over those bars.
    // Lines update whenever the user scrolls or zooms the chart.
    //
    // sc.PersistVars->i1 / i2 remember the previous visible range so that
    // only the previously-stamped bars need to be cleared on each update
    // (avoids an O(n) clear of the entire bar history).
    //
    // Non-visible bars are set to 0 and hidden via DrawZeros = 0, so the
    // flat horizontal lines appear only across the visible viewport.
    //
    // On a full reset (sc.UpdateStartIndex == 0, e.g. study first added,
    // settings changed, or chart replay) the entire array is cleared once.
    // -----------------------------------------------------------------
    if (sc.ArraySize < 1)
        return;

    // Retrieve the previous visible range stored in persistent variables.
    // sc.PersistVars is zero-initialized when the study loads; i3 acts as an
    // "initialized" flag so the clear step is skipped on the very first call
    // (when no bars have been stamped yet).
    int prevFirst      = sc.PersistVars->i1;
    int prevLast       = sc.PersistVars->i2;
    bool prevRangeValid = (sc.PersistVars->i3 == 1);

    // Clamp persisted range to current array bounds in case the chart was
    // reloaded with fewer bars since the values were last written.
    if (prevFirst < 0)                prevFirst = 0;
    if (prevFirst >= sc.ArraySize)    prevFirst = sc.ArraySize - 1;
    if (prevLast  < 0)                prevLast  = 0;
    if (prevLast  >= sc.ArraySize)    prevLast  = sc.ArraySize - 1;

    // Determine the current visible bar range and clamp to valid bounds.
    int firstVisible = sc.IndexOfFirstVisibleBar;
    int lastVisible  = sc.IndexOfLastVisibleBar;

    if (firstVisible < 0)
        firstVisible = 0;
    if (lastVisible >= sc.ArraySize)
        lastVisible = sc.ArraySize - 1;

    // Fall back to the entire array if the visible range is unavailable
    // or empty (e.g. chart not yet rendered).
    if (firstVisible > lastVisible)
    {
        firstVisible = 0;
        lastVisible  = sc.ArraySize - 1;
    }

    // Clear subgraph values so that only visible bars carry a non-zero
    // value (and therefore get drawn).
    if (sc.UpdateStartIndex == 0)
    {
        // Full reset – clear the entire array once.
        for (int i = 0; i < sc.ArraySize; ++i)
        {
            HighLine[i] = 0.0f;
            LowLine[i]  = 0.0f;
        }
    }
    else if (prevRangeValid)
    {
        // Incremental update – only erase the previously stamped range.
        for (int i = prevFirst; i <= prevLast; ++i)
        {
            HighLine[i] = 0.0f;
            LowLine[i]  = 0.0f;
        }
    }

    // Scan visible bars to find the extrema (O(visible range), not O(n)).
    float visibleHigh = sc.High[firstVisible];
    float visibleLow  = sc.Low[firstVisible];

    for (int i = firstVisible + 1; i <= lastVisible; ++i)
    {
        if (sc.High[i] > visibleHigh)
            visibleHigh = sc.High[i];
        if (sc.Low[i] < visibleLow)
            visibleLow = sc.Low[i];
    }

    // Stamp visible bars with the extrema so the lines appear flat and
    // horizontal across the entire visible viewport.
    for (int i = firstVisible; i <= lastVisible; ++i)
    {
        HighLine[i] = visibleHigh;
        LowLine[i]  = visibleLow;
    }

    // Persist the current visible range for the next call.
    sc.PersistVars->i1 = firstVisible;
    sc.PersistVars->i2 = lastVisible;
    sc.PersistVars->i3 = 1;  // Mark range as valid for subsequent calls
}
