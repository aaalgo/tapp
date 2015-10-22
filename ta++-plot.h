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

#ifndef ISIGNAL_GNUPLOT
#define ISIGNAL_GNUPLOT

/**
 * \file ta++-plot.h
 * \brief Generate figures.
 */

#include <sstream>

namespace tapp {

/// Subfigure of a chart.
class Pane
{
    std::string name;
    bool log;
public:
    Pane(): log(false) {};
    const std::string &getName() const { return name; }
    void setName (const std::string &_name) { name = _name; }

    /// Draw a integer series.
    virtual Pane *draw (const std::string &name, const IntegerSeries &series) = 0;
    /// Draw a real series.
    virtual Pane *draw (const std::string &name, const RealSeries &series) = 0;
    /// Draw candles.
    /**
     * the parameter bars controls whether financial bars or candle sticks to be used.
     */
    virtual Pane *drawCandles (const Candles &series, bool bars = true) = 0;
    /// Draw volumes.
    virtual Pane *drawVolumes (const Candles &series) = 0;

    /// Draw the output of a TA indicator.
    Pane *draw (const TA &indicator, std::string name = "")
    {
        BOOST_FOREACH(const TA::Output &output, indicator.getOutputs()) {
            std::string full_name;
            if (indicator.getOutputs().size() > 1) {
                if (name.empty()) {
                    full_name = output.name;
                }
                else {
                    full_name = name + ':' + output.name;
                }
            }
            else {
                if (name.empty()) {
                    full_name = indicator.getName();
                }
                else {
                    full_name = name;
                }
            }
            switch (output.type) {
                case TA_Output_Real: draw(full_name, output.real); break;
                case TA_Output_Integer: draw(full_name, output.integer); break;
            }
        }
        return this;
    }

    void setLogScale (bool _log) { log = _log; }
    bool logScale() const { return log; }
};

class Chart
{
    std::string name;
public:
    Chart () {};
    Chart (const std::string &_name): name(_name) {}
    const std::string &getName() const { return name; }
    void setName (const std::string &_name) { name = _name; }

    virtual ~Chart() {};
    /// Add a subfigure.
    virtual Pane *addPane (const std::string &name) = 0;
    /// Get a subfigure.
    virtual Pane *getPane (size_t index) = 0;
    /// Get the number of subfigures.
    virtual size_t size () const = 0;
    /// Generate the output.
    virtual void render () = 0;
};


class GnuplotChart: public Chart
{
    class GnuplotPane: public Pane
    {
        std::stringstream head,cmd,data;
        bool first;
        double ratio;
        double offset;

        const char *input ()
        {
            if (first)
            {
                first = false;
                return "plot \"-\" ";
            }
            else
            {
                return ", \"-\" ";
            }
        }

        std::string titleClause (const std::string &name)
        {
            if (name.size() == 0) return "notitle ";
            return "title \"" + name + "\" ";
        }
        std::string styleClause (TA_OutputFlags flags)
        {
            if (flags & TA_OUT_LINE)
            {
                return "with lines ";
            }
            else if (flags & TA_OUT_DOT_LINE)
            {
                return "with lines ";
            }
            else if (flags & TA_OUT_DASH_LINE)
            {
                return "with lines ";
            }
            else if (flags & TA_OUT_DOT)
            {
                return "with dots ";
            }
            else if (flags & (TA_OUT_HISTO
                | TA_OUT_PATTERN_BOOL | TA_OUT_PATTERN_BULL_BEAR | TA_OUT_PATTERN_STRENGTH))
            {
                return "with impulses ";
            }
            else return "with lines ";
        }

        std::string styleClause (bool bars)
        {
            return bars ? "with financebars ":"with candlesticks ";
        }

        std::string color (const std::string &c) { return "lc rgb \"" + c + "\" "; }

    public:
        GnuplotPane (): ratio(1), offset(0), head(std::ios_base::out), cmd(std::ios_base::out), data(std::ios_base::out), first(true) {}
        ~ GnuplotPane () {}

        virtual Pane *draw (const std::string &name, const IntegerSeries &series)
        {
            cmd << input() << "using 1:2 "
                << styleClause(series.getFlags())  << titleClause(name);
            for (size_t i = series.getFirst(); i < series.size(); i++)
            {
                data << i << '\t' << series[i] << std::endl;
            }
            data << 'e' << std::endl;
            return this;
        }
        virtual Pane *draw (const std::string &name, const RealSeries &series)
        {
            cmd << input() << "using 1:2 "
                << styleClause(series.getFlags())  << titleClause(name);
            for (size_t i = series.getFirst(); i < series.size(); i++)
            {
                data << i << '\t' << series[i] << std::endl;
            }
            data << 'e' << std::endl;
            return this;
        }
        virtual Pane *drawCandles (const Candles &series, bool bars)
        {
            cmd << input() << "using 1:2:3:4:5 notitle " << styleClause(bars) << color("green");
            cmd << input() << "using 1:2:3:4:5 notitle " << styleClause(bars) << color("red");
            for (size_t i = 0; i < series.size(); i++)
            {
                if (series.getOpen()[i] > series.getClose()[i]) continue;
                data << i
                    << '\t' << series.getOpen()[i]
                    << '\t' << series.getHigh()[i]
                    << '\t' << series.getLow()[i]
                    << '\t' << series.getClose()[i]
                    << std::endl;
            }
            data << 'e' << std::endl;
            for (size_t i = 0; i < series.size(); i++)
            {
                if (series.getOpen()[i] <= series.getClose()[i]) continue;
                data << i
                    << '\t' << series.getOpen()[i]
                    << '\t' << series.getHigh()[i]
                    << '\t' << series.getLow()[i]
                    << '\t' << series.getClose()[i]
                    << std::endl;
            }
            data << 'e' << std::endl;
            return this;
        }
        virtual Pane *drawVolumes (const Candles &series)
        {
            cmd << input() << "using 1:2 notitle " << styleClause(TA_OUT_HISTO) << color("green");
            cmd << input() << "using 1:2 notitle " << styleClause(TA_OUT_HISTO) << color("red");
            for (size_t i = 0; i < series.size(); i++)
            {
                if (series.getOpen()[i] > series.getClose()[i]) continue;
                data << i << '\t' << series.getVolume()[i] << std::endl;
            }
            data << 'e' << std::endl;
            for (size_t i = 0; i < series.size(); i++)
            {
                if (series.getOpen()[i] <= series.getClose()[i]) continue;
                data << i << '\t' << series.getVolume()[i] << std::endl;
            }
            data << 'e' << std::endl;
            return this;
        }

        void setRatio (double r) { ratio = r; }
        void setOffset (double r) { offset = r; }
        
        void dump (std::ostream &os)
        {
            os << "set xrange [0:]" << std::endl;
            os << "set size 1, " << ratio << std::endl;
            os << "set origin 0, " << offset << std::endl;
            if (logScale()) os << "set logscale y" <<std::endl;
            os << head.str() << std::endl;
            os << std::endl;
            os << cmd.str() << std::endl;
            os << data.str();
            if (logScale()) os << "unset logscale y" <<std::endl;
        }
    };

    std::vector<GnuplotPane*> panes;
    std::string script_path;
    std::string image_path;

    void addDefaultPanes (const Candles &candles)
    {
        addPane("")->drawCandles(candles);
        addPane("")->drawVolumes(candles);
    }
public:
    /**
     * \param name Name of the chart.
     * \param path Path of the output file.
     */
    GnuplotChart (const std::string &name, const std::string &_script_path = "", const std::string &_image_path = ""): Chart(name), script_path(_script_path), image_path(_image_path) {}

    GnuplotChart (const std::string &name, const Candles &candles, const std::string &_script_path = "", const std::string &_image_path = "")
        : Chart(name), script_path(_script_path), image_path(_image_path) 
    {
        addDefaultPanes(candles);
    }

    /// Add a subfigure.
    Pane *addPane (const std::string &name)
    {
        GnuplotPane *pane = new GnuplotPane();
        pane->setName(name);
        panes.push_back(pane);
        return pane;
    }

    virtual size_t size () const {
        return panes.size();
    }

    virtual Pane *getPane (size_t index)
    {
        return panes[index];
    }

    virtual ~GnuplotChart()
    {
        BOOST_FOREACH(GnuplotPane *p, panes) {
            delete p;
        }
    }

    virtual void render ()
    {
        if (script_path.size() == 0) {
            script_path = getName() + ".gp";
        }
        if (image_path.size() == 0) {
            image_path = getName() + ".png";
        }

        std::ofstream fout(script_path.c_str());
        
        int height = 480 * (2 + panes.size()) / 3;
        double ratio = 1.0 / (2 + panes.size());
        double acc = 0;
        
        fout << "set terminal png size 800, " << height << std::endl;
        fout << "set output \"" << image_path << "\"" << std::endl;
        fout << "set grid\n" << std::endl;
        fout << "set key tmargin left horizontal" << std::endl;
        fout << "set lmargin 10" << std::endl;
        fout << "set multiplot layout " << panes.size() << ",1" << std::endl;
        fout << std::endl;

        bool first = true;
        BOOST_FOREACH(GnuplotPane *pane, panes) {
            double r = ratio;
            if (first) r *= 3;
            pane->setRatio(r);
            acc += r;
            pane->setOffset(1.0 - acc);
            pane->dump(fout);
            first = false;
        }
        fout << "unset multiplot" << std::endl;
    }
};

}
#endif
