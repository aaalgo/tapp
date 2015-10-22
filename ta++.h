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

#ifndef WDONG_TAPP
#define WDONG_TAPP

/** \mainpage
 *
 *  \section intro_sec Introduction
 *
 *  TA++ is a C++ wrapper of TA-lib.  I design this library for those who
 *  (mainly me) want to do quick and dirty technical analysis in C++.  My goal
 *  is simplicity rather than exporting all the functionality of TA-lib.  The
 *  typical use case is when you have a TA indicator in mind and want to screen
 *  stock data downloaded from Yahoo.
 *
 *  \section req_sec Prerequisites
 *  <ul>
 *  <li> Boost </li>
 *  <li> TA-lib </li>
 *  </ul>
 *  The library generates Gnuplot scripts as figures, though it doesn't depend
 *  on Gnuplot.
 *
 *  \section install_sec Installation 
 *  There are only two files in the library: ta++.h and ta++-plot.h.  Just drop
 *  these files in somewhere your C++ compiler is aware of and you are done.
 *
 *  \section link_sec Compile and Link
 *  TA++ depends on two libraries: TA-lib and boost date & time.  If you use g++
 *  and the latest version of TA-lib, you need to add "-lboost_date_time -lta_lib"
 *  to your g++ commandline.
 *
 *  \section start_sec Quick Start
 *
 *  See the sample program <a href="example_8cpp.html">example.cpp</a> in the package.
 */

/**
 * \file ta++.h
 *
 * A C++ wrapper of Lib TA.
 */

#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <string>
#include <fstream>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/foreach.hpp>
#include <ta-lib/ta_libc.h>

#ifndef panic
#if defined(WIN32)
#define panic(_fmt, ...) \
    do { \
        panic_intern("%s: %s: %d: "_fmt, \
                        __FILE__, \
                        __FUNCTION__, \
                        __LINE__ , \
                        ## __VA_ARGS__); \
    } while (0)
#else
#define panic(_fmt, _args...) \
    do { \
        panic_intern("%s: %s: %d: "_fmt, \
                        __FILE__, \
                        __FUNCTION__, \
                        __LINE__ , \
                        ## _args); \
    } while (0)
#endif
#endif

#ifndef verify
#define verify(_x) \
    do { \
        if (_x) { \
            /* noop */ \
        } else { \
            panic("!(%s)\n", #_x); \
        } \
    } while (0)
#endif

namespace tapp {

static inline void panic_intern(const char *fmt, ...)
{
        va_list args;
        va_start(args, fmt);
        vfprintf(stderr, fmt, args);
        va_end(args);
        exit(-1);
}

/// Time of a candle stick.
/**
 *  Time is used as an atomic type to represent the time of a candle stick.
 *  For now, we save the date part of time only. 
 */
typedef boost::gregorian::date Time;

/// Convert string to time.
/**
 *  The string should be formated like "2008-01-01",
 *  "2008/01/01", etc.
 */
static inline Time str2time (const std::string &str) {
    return boost::gregorian::from_string(str);
}

/// The smallest time.
static const Time BEGINNING(boost::gregorian::neg_infin);

/// The largest time.
static const Time ENDING(boost::gregorian::pos_infin);

/// The candle stick type.
struct Candle
{
	TA_Real open;
	TA_Real high;
	TA_Real low;
	TA_Real close;
	TA_Real volume;
    TA_Real openInterest;
	Time time;

	Candle () {}

    /// Initialize all fields with given values.
	Candle (TA_Real _open, TA_Real _high, TA_Real _low, TA_Real _close, TA_Real _volume, TA_Real _openInterest, Time _time)
		: open(_open), high(_high), low(_low), close(_close), volume(_volume), openInterest(_openInterest), time(_time) {}

    /// Merge two candles.
    /** invoking this method will update close, high, low and volume.
     */
	Candle operator += (const Candle &candle)
	{
		if (candle.high > high) high = candle.high;
		if (candle.low < low) low = candle.low;
		close = candle.close;
		volume += candle.volume;
	}
};

/// The base class of all series.
/**
 * Series for TA analysis have some common properties and this class
 * is used to represent those properties.  This class is inherited by all
 * series classes.
 *
 * Currently, two properties are supported.
 * <ul>
 * <li>
 * First:  Some TA functions have an unstable period.  For example, if one
 * 30-day MACD to a series, then the first 29 results are not meaningful.
 * The property first is the offset of the first meaningful entry in the series.
 * 
 * When applying TA functions, the first property of the input stream will
 * be considered and reflected in the output.  For example, if the input
 * series has first == 5, by applying 30-day MACD, the result will have
 * first == 5 + 29 == 34.
 * </li>
 * <li>
 * Flags:  this is used to control the drawing style of the series and is
 * automatically set by TA indicators.
 * </li>
 * </ul>
 *
 * The member functions of this class are self-envident.
 */
class BaseSeries
{
    TA_Integer first;       // first valid member
    TA_OutputFlags flags;   // for visualization
public:
    BaseSeries () : first(0), flags(0) {
    }

    void setFirst (TA_Integer f) {
        first = f;
    }

    TA_Integer getFirst () const {
        return first;
    }

    void setFlags (TA_OutputFlags f) {
        flags = f;
    }

    TA_OutputFlags getFlags () const {
        return flags;
    }
};

/// The template for series type.
/**
 * A series of atomic type like TA_Integer and TA_Real is just a STL vector
 * combined with the properties of BaseSeries.
 */
template <typename T>
class Series: public BaseSeries, public std::vector<T>
{
};

/// Real series.
typedef Series<TA_Real> RealSeries;

/// Integer Series.
typedef Series<TA_Integer> IntegerSeries;

/// Time series.
typedef Series<Time> TimeSeries;

/// The candle series type.
/**
 * Candles is different from other series types in that it is not
 * inherited from the STL vector.  Instead, it is implemented as
 * a bundle of RealSeries of open, high, low, close, volume, openInterest
 * and a TimeSeries.  The member series can be accessed by invoking
 * corresponding method.
 *
 * The properties of the member series are automatically updated.
 */
class Candles: public BaseSeries
{
    RealSeries open;
    RealSeries high;
    RealSeries low;
    RealSeries close;
    RealSeries volume;
    RealSeries openInterest;
    TimeSeries time;
public:
    /**
     * Initialize from a file.  This is the same as invoking
     * LoadFromFile with the same parameters immediately after
     * connstructing the object.
     */
    Candles (const std::string &path, Time begin = BEGINNING, Time end = ENDING)
    {
        LoadFromFile(path, begin, end);
    }

    /// Load candles from a file.
    /**
     * \param path  The input file's path.
     * \param begin The first date after which candles shall be added.
     * \param end Stop reading when this time is seen.
     *
     * Following the common practice, begin is inclusive and end is exclusive.
     * That is, a record with time equals begin is added and a record with time
     * equals end is not added.
     */
    void LoadFromFile (const std::string &path, Time begin = BEGINNING, Time end = ENDING)
    {
        Candle candle;

        std::ifstream fin(path.c_str());
        verify(fin);

        std::string buf;

        while (fin >> buf >> candle.open >> candle.high >> candle.low >> candle.close >> candle.volume >> candle.openInterest) {
            candle.time = str2time(buf);
            if (candle.time < begin) continue;
            if (candle.time >= end) break;
            time.push_back(candle.time);
            open.push_back(candle.open);
            high.push_back(candle.high);
            low.push_back(candle.low);
            close.push_back(candle.close);
            volume.push_back(candle.volume);
            openInterest.push_back(candle.openInterest);
        }
    }

    /// Access a candle in the series as a whole.
    Candle operator [] (unsigned i) {
        return Candle(open[i], high[i], low[i], close[i], volume[i], openInterest[i], time[i]);
    }

    /// Size of the series.
    size_t size() const {
        return open.size();
    }

    const RealSeries &getOpen () const {
        return open;
    }
    const RealSeries &getHigh () const {
        return high;
    }
    const RealSeries &getLow () const {
        return low;
    }
    const RealSeries &getClose () const {
        return close;
    }
    const RealSeries &getVolume () const {
        return volume;
    }
    const RealSeries &getOpenInterest () const {
        return openInterest;
    }
    const TimeSeries &getTime () const {
        return time;
    }

    /**
     * Set the first property, simultaneously update those of the member series.
     */
    void setFirst (TA_Integer f) {
        BaseSeries::setFirst(f);
        open.setFirst(f);
        high.setFirst(f);
        low.setFirst(f);
        close.setFirst(f);
        volume.setFirst(f);
        openInterest.setFirst(f);
        time.setFirst(f);
    }

    /**
     * Set the flags property, simultaneously update those of the member series.
     */
    void setFlags (TA_OutputFlags f) {
        BaseSeries::setFlags(f);
        open.setFlags(f);
        high.setFlags(f);
        low.setFlags(f);
        close.setFlags(f);
        volume.setFlags(f);
        openInterest.setFlags(f);
        time.setFlags(f);
    }
};


/// The TA indicator class.
class TA
{
    enum OptionType {
        REAL, INTEGER
    };

    class Option {
        std::string name;
        OptionType type;
        TA_Integer integer;
        TA_Real real;
    public:
        Option (const std::string &_name, TA_Integer value)
            : name(_name), type(INTEGER), integer(value) {
        }
        Option (const std::string &_name, TA_Real value)
            : name(_name), type(REAL), real(value) {
        }
        const std::string &getName () const {
            return name;
        }
        TA_Integer getInteger () const {
            verify(type == INTEGER);
            return integer;
        }
        TA_Real getReal () const {
            verify(type == REAL);
            return real;
        }
    };

public:
    /// Output series.
    /**
     * Though the struct contains two series real and integer, only one
     * is valid depeding on the type field.
     */
    struct Output {
        /// Type of the series.
        /** can be TA_Output_Real or TA_Output_Integer.
         */
        TA_OutputParameterType type;
        /// Name of the series.
        std::string name;
        /// Real series.
        RealSeries real;
        /// Integer series.
        IntegerSeries integer;
    };

    /// Options to a TA indicator.
    class Options: public std::vector<Option> {
    public:
        Options () {
        }
        /// Add a integer option.
        Options &add (const std::string &name, TA_Integer value) {
            push_back(Option(name, value));
            return *this;
        }
        /// Add a float option.
        Options &add (const std::string &name, TA_Real value) {
            push_back(Option(name, value));
            return *this;
        }
    };

    /// Get the default options.
    static Options getDefault() {
        return Options();
    }

    /// List of output series.
    typedef std::vector<Output> Outputs;
private:
    typedef std::map<std::string, unsigned> OptionMap;

	const TA_FuncHandle *funcHandle;
	const TA_FuncInfo *funcInfo;
	TA_ParamHolder *params;

    OptionMap optionMap;
    Outputs outputs;

    unsigned inputFirst;
    unsigned inputSize;

    void Init (const std::string &name) {
		if (TA_GetFuncHandle(name.c_str(), &funcHandle) != TA_SUCCESS) panic();
		if (TA_GetFuncInfo(funcHandle, &funcInfo) != TA_SUCCESS) panic();
		if (TA_ParamHolderAlloc(funcHandle, &params) != TA_SUCCESS) panic();

        // opt input parameter info
        for (unsigned i = 0; i < funcInfo->nbOptInput; ++i) {
            const TA_OptInputParameterInfo *info;
		    if (TA_GetOptInputParameterInfo(funcHandle, i, &info) != TA_SUCCESS) panic();
            optionMap[std::string(info->paramName)] = i;
        }

        outputs.resize(funcInfo->nbOutput);
        for (unsigned i = 0; i < funcInfo->nbOutput; ++i) {
            const TA_OutputParameterInfo *info;
		    if (TA_GetOutputParameterInfo(funcHandle, i, &info) != TA_SUCCESS) panic();
            outputs[i].type = info->type;
            outputs[i].name = info->paramName;
            outputs[i].real.setFlags(info->flags);
            outputs[i].integer.setFlags(info->flags);
        }
    }

    void setInputHelper (unsigned idx, const RealSeries &input) {
        const TA_InputParameterInfo *info;
		if (TA_GetInputParameterInfo(funcHandle, idx, &info) != TA_SUCCESS) panic();
        verify(info->type == TA_Input_Real);
        if (TA_SetInputParamRealPtr(params, idx, &input[inputFirst]) != TA_SUCCESS) panic();
    };

    void setInputHelper (unsigned idx, const IntegerSeries &input) {
        const TA_InputParameterInfo *info;
		if (TA_GetInputParameterInfo(funcHandle, idx, &info) != TA_SUCCESS) panic();
        verify(info->type == TA_Input_Integer);
        if (TA_SetInputParamIntegerPtr(params, idx, &input[inputFirst]) != TA_SUCCESS) panic();
    };

    void setInputHelper (unsigned idx, const Candles &input) {
        const TA_InputParameterInfo *info;
		if (TA_GetInputParameterInfo(funcHandle, idx, &info) != TA_SUCCESS) panic();
        verify(info->type == TA_Input_Price);
        if (TA_SetInputParamPricePtr(params, idx,
                    &input.getOpen()[inputFirst],
                    &input.getHigh()[inputFirst],
                    &input.getLow()[inputFirst],
                    &input.getClose()[inputFirst],
                    &input.getVolume()[inputFirst],
                    &input.getOpenInterest()[inputFirst]) != TA_SUCCESS) panic();
    };

    template <typename T>
    void setInput (const T &input) {
        verify(funcInfo->nbInput == 1);
        inputFirst = input.getFirst();
        inputSize = input.size();
        setInputHelper(0, input);
    };

    template <typename T1, typename T2>
    void setInput (const T1 &input1, const T2 &input2) {
        verify(funcInfo->nbInput == 2);
        inputFirst = std::max(input1.getFirst(), input2.getFirst());
        inputSize = std::min(input1.size(), input2.size());
        setInputHelper(0, input1);
        setInputHelper(1, input2);
        return *this;
    };


    void setOption (const Option &option) {
        OptionMap::const_iterator it = optionMap.find(option.getName());
        verify(it != optionMap.end());

        const TA_OptInputParameterInfo *info;
		if (TA_GetOptInputParameterInfo(funcHandle, it->second, &info) != TA_SUCCESS) panic();
        if (info->type == TA_OptInput_RealRange
            || info->type == TA_OptInput_RealList) {
            if (TA_SetOptInputParamReal(params, it->second, option.getReal()) != TA_SUCCESS) panic();
        }
        else if (info->type == TA_OptInput_IntegerRange
            || info->type == TA_OptInput_IntegerList) {
            if (TA_SetOptInputParamInteger(params, it->second, option.getInteger()) != TA_SUCCESS) panic();
        }
    };

    void update ()
    {
        TA_Integer lookback = 0;
        if (TA_GetLookback(params, &lookback) != TA_SUCCESS) panic();
        TA_Integer outputFirst = inputFirst + lookback;
        for (unsigned i = 0; i < outputs.size(); ++i) {
            switch (outputs[i].type) {
            case TA_Output_Real:
                outputs[i].real.setFirst(outputFirst);
                outputs[i].real.resize(inputSize);
                if (TA_SetOutputParamRealPtr(params, i, &outputs[i].real[outputFirst]) != TA_SUCCESS) panic();
                break;
            case TA_Output_Integer:
                outputs[i].integer.setFirst(outputFirst);
                outputs[i].integer.resize(inputSize);
                if (TA_SetOutputParamIntegerPtr(params, i, &outputs[i].integer[outputFirst]) != TA_SUCCESS) panic();
                break;
                default:
                panic();
            }
        }
        TA_Integer outBegIdx, outNbElement;
        if (TA_CallFunc(params, 0, inputSize - inputFirst - 1, &outBegIdx, &outNbElement) != TA_SUCCESS) panic();
        verify(outBegIdx == lookback);
        verify(outNbElement == inputSize - outputFirst);
    }

public:
    /**
     * Initialize a TA indicator with one input series.
     */
    template <typename T>
	TA (const std::string &name, const T& input, const Options &options = Options())
	{
        Init(name);
        BOOST_FOREACH(const Option &option, options) {
            setOption(option);
        }
        setInput(input);
        update();
    };

    /**
     * Initialize a TA indicator with two input series.
     */
    template <typename T1, typename T2>
	TA (const std::string &name, const T1& input1, const T2 &input2, const Options &options = Options())
	{
        Init(name);
        BOOST_FOREACH(const Option &option, options) {
            setOption(option);
        }
        setInput(input1, input2);
        update();
    };

    /// get the name of the indicator.
    std::string getName () const {
        return std::string(funcInfo->name);
    }

    /// get all outputs.
    const Outputs &getOutputs () const {
        return outputs;
    }

    /// get number of outputs
    size_t size () {
        return outputs.size();
    }

    /// get one output series.
    const Output &operator [] (unsigned i) const {
        return outputs[i];
    }
};

}

#endif

