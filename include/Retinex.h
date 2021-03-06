#ifndef RETINEX_H_
#define RETINEX_H_


#include "Filter.h"
#include "Image_Type.h"
#include "Histogram.h"


const struct Retinex_Para
{
    ldbl sigma = 80.0L;
    std::vector<ldbl> sigmaVector;

    double lower_thr = 0.01;
    double upper_thr = 0.01;
    Histogram<FLType>::BinType HistBins = 1024;

    double chroma_protect = 1.2;

    FLType restore = FLType(125.0);

    FLType dynamic = FLType(10.0);

    Retinex_Para() : sigmaVector({ 15.0L, sizeof(FLType) ? 80.0L : 250.0L }) {}
} Retinex_Default;


class Retinex
    : public FilterIF
{
public:
    typedef Retinex _Myt;
    typedef FilterIF _Mybase;

protected:
    Retinex_Para para;

public:
    Retinex(const Retinex_Para _para = Retinex_Default)
        : para(_para)
    {}

protected:
    virtual Frame &process_Frame(Frame &dst, const Frame &src)
    {
        if (src.isRGB())
        {
            for (Frame::PlaneCountType i = 0; i < src.PlaneCount(); i++)
            {
                process_Plane(dst.P(i), src.P(i));
            }
        }
        else
        {
            DEBUG_FAIL("Retinex::process: invalid PixelType of Frame \"src\", should be RGB.");
        }

        return dst;
    }

    Plane_FL &Kernel(Plane_FL &dst, const Plane_FL &src);

    Plane_FL Kernel(const Plane_FL &src)
    {
        Plane_FL dst(src, false);
        return Kernel(dst, src);
    }
};


class Retinex_SSR
    : public Retinex
{
public:
    typedef Retinex_SSR _Myt;
    typedef Retinex _Mybase;

public:
    Retinex_SSR(const Retinex_Para _para = Retinex_Default)
        : _Mybase(_para)
    {
        para.sigmaVector.clear();
        para.sigmaVector.push_back(para.sigma);
    }

protected:
    virtual Plane_FL &process_Plane_FL(Plane_FL &dst, const Plane_FL &src);

    virtual Plane &process_Plane(Plane &dst, const Plane &src);
};


class Retinex_MSR
    : public Retinex
{
public:
    typedef Retinex_MSR _Myt;
    typedef Retinex _Mybase;

public:
    Retinex_MSR(const Retinex_Para _para = Retinex_Default)
        : _Mybase(_para)
    {}

protected:
    virtual Plane_FL &process_Plane_FL(Plane_FL &dst, const Plane_FL &src);

    virtual Plane &process_Plane(Plane &dst, const Plane &src);
};


class Retinex_MSRCR_GIMP
    : public Retinex_MSR
{
public:
    typedef Retinex_MSRCR_GIMP _Myt;
    typedef Retinex_MSR _Mybase;

public:
    Retinex_MSRCR_GIMP(const Retinex_Para _para = Retinex_Default)
        : _Mybase(_para)
    {}

protected:
    virtual Plane_FL &process_Plane_FL(Plane_FL &dst, const Plane_FL &src);

    virtual Plane &process_Plane(Plane &dst, const Plane &src);
};


class Retinex_MSRCP
    : public Retinex_MSR
{
public:
    typedef Retinex_MSRCP _Myt;
    typedef Retinex_MSR _Mybase;

public:
    Retinex_MSRCP(const Retinex_Para _para = Retinex_Default)
        : _Mybase(_para)
    {}

protected:
    virtual Frame &process_Frame(Frame &dst, const Frame &src);
};


class Retinex_MSRCR
    : public Retinex_MSR
{
public:
    typedef Retinex_MSRCR _Myt;
    typedef Retinex_MSR _Mybase;

public:
    Retinex_MSRCR(const Retinex_Para _para = Retinex_Default)
        : _Mybase(_para)
    {}

protected:
    virtual Frame &process_Frame(Frame &dst, const Frame &src);
};


class Retinex_MSR_IO
    : public FilterIO
{
public:
    typedef Retinex_MSR_IO _Myt;
    typedef FilterIO _Mybase;

protected:
    Retinex_Para para;

    virtual void arguments_process()
    {
        _Mybase::arguments_process();

        Args ArgsObj(argc, args);
        ldbl sigma;
        para.sigmaVector.clear();

        for (int i = 0; i < argc; i++)
        {
            if (args[i] == "-S" || args[i] == "--sigma")
            {
                ArgsObj.GetPara(i, sigma);
                para.sigmaVector.push_back(sigma);
                continue;
            }
            if (args[i] == "-L" || args[i] == "--lower_thr")
            {
                ArgsObj.GetPara(i, para.lower_thr);
                continue;
            }
            if (args[i] == "-U" || args[i] == "--upper_thr")
            {
                ArgsObj.GetPara(i, para.upper_thr);
                continue;
            }
            if (args[i][0] == '-')
            {
                i++;
                continue;
            }
        }

        ArgsObj.Check();

        if (para.sigmaVector.size() == 0)
        {
            para.sigmaVector = Retinex_Default.sigmaVector;
        }
    }

    virtual Frame process(const Frame &src) = 0;

public:
    Retinex_MSR_IO(std::string _Tag = ".MSR")
        : _Mybase(std::move(_Tag)) {}
};


class Retinex_MSRCP_IO
    : public Retinex_MSR_IO
{
public:
    typedef Retinex_MSRCP_IO _Myt;
    typedef Retinex_MSR_IO _Mybase;

protected:
    virtual void arguments_process()
    {
        _Mybase::arguments_process();

        Args ArgsObj(argc, args);

        for (int i = 0; i < argc; i++)
        {
            if (args[i] == "-CP" || args[i] == "--chroma_protect")
            {
                ArgsObj.GetPara(i, para.chroma_protect);
                continue;
            }
            if (args[i][0] == '-')
            {
                i++;
                continue;
            }
        }

        ArgsObj.Check();
    }

    virtual Frame process(const Frame &src)
    {
        Retinex_MSRCP filter(para);
        return filter(src);
    }

public:
    Retinex_MSRCP_IO(std::string _Tag = ".MSRCP")
        : _Mybase(std::move(_Tag)) {}
};


class Retinex_MSRCR_IO
    : public Retinex_MSR_IO
{
public:
    typedef Retinex_MSRCR_IO _Myt;
    typedef Retinex_MSR_IO _Mybase;

protected:
    virtual void arguments_process()
    {
        _Mybase::arguments_process();

        Args ArgsObj(argc, args);

        for (int i = 0; i < argc; i++)
        {
            if (args[i] == "-R" || args[i] == "--restore")
            {
                ArgsObj.GetPara(i, para.restore);
                continue;
            }
            if (args[i][0] == '-')
            {
                i++;
                continue;
            }
        }

        ArgsObj.Check();
    }

    virtual Frame process(const Frame &src)
    {
        Retinex_MSRCR filter(para);
        return filter(src);
    }

public:
    Retinex_MSRCR_IO(std::string _Tag = ".MSRCR")
        : _Mybase(std::move(_Tag)) {}
};


class Retinex_MSRCR_GIMP_IO
    : public Retinex_MSR_IO
{
public:
    typedef Retinex_MSRCR_GIMP_IO _Myt;
    typedef Retinex_MSR_IO _Mybase;

protected:
    virtual void arguments_process()
    {
        _Mybase::arguments_process();

        Args ArgsObj(argc, args);

        for (int i = 0; i < argc; i++)
        {
            if (args[i] == "-D" || args[i] == "--dynamic")
            {
                ArgsObj.GetPara(i, para.dynamic);
                continue;
            }
            if (args[i][0] == '-')
            {
                i++;
                continue;
            }
        }

        ArgsObj.Check();
    }

    virtual Frame process(const Frame &src)
    {
        Retinex_MSRCR_GIMP filter(para);
        return filter(src);
    }

public:
    Retinex_MSRCR_GIMP_IO(std::string _Tag = ".MSRCR_GIMP")
        : _Mybase(std::move(_Tag)) {}
};


#endif
