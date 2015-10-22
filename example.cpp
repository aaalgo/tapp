/*
 * TA++ Copyright (c) 2008-2009, Wei Dong  wdong.pku@gmail.com
 * All rights reserved.
 *
 * FOR PERSONAL AND NON-COMMERCIAL USE ONLY.  REDISTRIBUTION IS NOT PERMITTED.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 */

/**
 * \file example.cpp
 *
 * \brief Example program.
 */

#include "ta++.h"
#include "ta++-plot.h"

// All TA++ symbols are under the namespace tapp.
using namespace tapp;

int main ()
{
    // Initialize the TA-Lib.  This function is from TA-lib.
    TA_Initialize();

    // Load stock data from the file "C".
    // Records before the date 2008-05-01 are not loaded.
    Candles candles("C", str2time("2008-05-01"));

    // Calculate MACD using the close prices with default parameters.
    TA macd("MACD", candles.getClose());
    // Moving average of 5, 10, 30, 60 days.
    // The constructor of TA accepts an optional third parameter, which
    // is a list of optional parameters to the TA indicator.  This list
    // can be constructed by
    //      TA::getDefault().add(name, value).add(name, value)...;
    // Too see a list of optional parameters of various TA indicators,
    // see the file ta-list in the package.
    TA ma5("EMA", candles.getClose(), TA::getDefault().add("optInTimePeriod", 5));
    TA ma10("EMA", candles.getClose(), TA::getDefault().add("optInTimePeriod", 10));
    TA ma30("EMA", candles.getClose(), TA::getDefault().add("optInTimePeriod", 30));
    TA ma60("EMA", candles.getClose(), TA::getDefault().add("optInTimePeriod", 60));

    // GnuplotChart generate gnuplot scripts.  The output will be stored in "C.gp".
    // The constructor automatically generate two panes: pane0 for the candles and
    // pane1 for volume.
    GnuplotChart chart("C", candles);
    // Draw the moving averages on pane 0.
    chart.getPane(0)->draw(ma5, "MA5");
    chart.getPane(0)->draw(ma10, "MA10");
    chart.getPane(0)->draw(ma30, "MA30");
    chart.getPane(0)->draw(ma60, "MA60");
    // Add a MACD pane.
    chart.addPane("MACD")->draw(macd);
    // Generate the output.
    chart.render();

    TA_Shutdown();
    return 0;
}

