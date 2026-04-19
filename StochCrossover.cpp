// StochCrossover.cpp
// Sierra Chart ACSIL Custom Indicator
//
// Places a configurable dot below the bar when the Stochastic Fast %K
// (period 10, D period 1) rises above 20 on a bar close.
//
// "K10, D1" means Fast K period = 10 with D smoothing period = 1 (i.e. D = K,
// no additional smoothing).  Fast %K is therefore computed directly as:
//   %K = (Close - LowestLow[10]) / (HighestHigh[10] - LowestLow[10]) * 100
//
// Subgraph 0 (Cross Dot) – the visible marker – is fully configurable
// through the Sierra Chart study settings dialog (color, draw style,
// line/point width, etc.).
//
// Inputs:
//   Input 0  – Fast K Period        (default: 10)
//   Input 1  – Crossover Level      (default: 20.0)
//   Input 2  – Dot Offset in Ticks  (default: 3)

#include "sierrachart.h"

SCDLLName("StochCrossover")

SCSFExport scsf_StochCrossover(SCStudyInterfaceRef sc)
{
    // -----------------------------------------------------------------
    // Subgraph references
    // -----------------------------------------------------------------
    SCSubgraphRef CrossDot = sc.Subgraph[0];   // Visible blue dot
    SCSubgraphRef FastK    = sc.Subgraph[1];   // Internal Fast %K storage

    // -----------------------------------------------------------------
    // Input references
    // -----------------------------------------------------------------
    SCInputRef FastKPeriod    = sc.Input[0];
    SCInputRef CrossoverLevel = sc.Input[1];
    SCInputRef DotOffsetTicks = sc.Input[2];

    // -----------------------------------------------------------------
    // SetDefaults block
    // -----------------------------------------------------------------
    if (sc.SetDefaults)
    {
        sc.GraphName    = "Stochastic Crossover Dot";
        sc.StudyVersion = 1;
        sc.AutoLoop     = 1;   // Sierra Chart calls this function once per bar

        // --- Visible dot subgraph ---
        CrossDot.Name         = "Cross Dot";
        CrossDot.DrawStyle    = DRAWSTYLE_POINT;
        CrossDot.PrimaryColor = RGB(0, 0, 255);   // Blue
        CrossDot.LineWidth    = 5;                // Point size (configurable)
        CrossDot.DrawZeros    = 0;                // Hide bars with no signal

        // --- Internal subgraph for Fast %K values (not drawn) ---
        FastK.Name      = "Fast %K (internal)";
        FastK.DrawStyle = DRAWSTYLE_IGNORE;
        FastK.DrawZeros = 0;

        // --- User-configurable inputs ---
        FastKPeriod.Name = "Fast K Period";
        FastKPeriod.SetInt(10);

        CrossoverLevel.Name = "Crossover Level";
        CrossoverLevel.SetFloat(20.0f);

        DotOffsetTicks.Name = "Dot Offset (Ticks below Low)";
        DotOffsetTicks.SetInt(3);

        return;
    }

    // -----------------------------------------------------------------
    // Calculation
    // -----------------------------------------------------------------

    const int period = FastKPeriod.GetInt();

    // Need enough bars to fill the lookback window, plus one previous bar
    // to detect a crossover.
    if (sc.Index < period)
    {
        FastK[sc.Index]    = 50.0f;   // Neutral midpoint of the 0-100 stochastic range
        CrossDot[sc.Index] = 0.0f;
        return;
    }

    // Compute Fast %K = (Close - LowestLow[period]) /
    //                   (HighestHigh[period] - LowestLow[period]) * 100
    // Scan the full lookback window [Index-period+1 .. Index] inclusive,
    // initializing with the oldest bar in the window and iterating forward
    // (same pattern used in HiLo.cpp).
    const int windowStart = sc.Index - period + 1;
    float highestHigh = sc.High[windowStart];
    float lowestLow   = sc.Low[windowStart];

    for (int i = windowStart + 1; i <= sc.Index; ++i)
    {
        if (sc.High[i] > highestHigh) highestHigh = sc.High[i];
        if (sc.Low[i]  < lowestLow)  lowestLow  = sc.Low[i];
    }

    const float range = highestHigh - lowestLow;
    if (range > 0.0f)
        FastK[sc.Index] = (sc.Close[sc.Index] - lowestLow) / range * 100.0f;
    else
        FastK[sc.Index] = FastK[sc.Index - 1];   // Flat bar – carry previous K

    // Crossover condition: K was at or below the level and is now above it.
    const float level  = CrossoverLevel.GetFloat();
    const float prevKv = FastK[sc.Index - 1];
    const float currKv = FastK[sc.Index];

    if (prevKv <= level && currKv > level)
    {
        // Place the dot a configurable number of ticks below the bar low.
        const float offset = static_cast<float>(DotOffsetTicks.GetInt()) * sc.TickSize;
        CrossDot[sc.Index] = sc.Low[sc.Index] - offset;
    }
    else
    {
        CrossDot[sc.Index] = 0.0f;
    }
}
