#ifndef PARBUFFER_H_
#define PARBUFFER_H_

#include <cstddef>
#include <string>
#include <sstream>

class ParBuffer {
public:
    ParBuffer()
        : SEP(':')
    {
    }
    ParBuffer(std::string buf)
        : SEP(':')
    {
        inBuffer = buf;
    }

    template <typename T>
    ParBuffer& operator<<(const T& v)
    {
        if (outBuffer.str().length() == 0)
            outBuffer << v;
        else
            outBuffer << SEP << v;
        return *this;
    }

    std::string next()
    {
        std::string value;
        size_t sep;

        if (inBuffer.size() == 0) return "";

        sep = inBuffer.find(SEP);
        if (sep == std::string::npos) {
            value = inBuffer;
            inBuffer = "";
        }
        else {
            value = inBuffer.substr(0, sep);
            inBuffer = inBuffer.substr(sep + 1);
        }
        return value;
    }

    ParBuffer& operator>>(double& v)
    {
        std::string value = next();
        sscanf(value.c_str(), "%lf", &v);
        return *this;
    }

    ParBuffer& operator>>(int& v)
    {
        std::string value = next();
        sscanf(value.c_str(), "%d", &v);
        return *this;
    }

    ParBuffer& operator>>(std::string& v)
    {
        v = next();
        return *this;
    }

    void set(std::string buf)
    {
        inBuffer = buf;
    }
    void clear()
    {
        outBuffer.clear();
    }
    std::string str() const
    {
        return outBuffer.str();
    }

private:
    const char SEP;
    std::stringstream outBuffer;
    std::string inBuffer;
};

#endif
