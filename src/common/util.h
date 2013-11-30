#ifndef __COMMON_UTIL_H__
#define __COMMON_UTIL_H__

#include <sstream>

template<typename T>
static inline std::string toString(T obj)
{
    std::ostringstream strm;
    strm << obj;
    return strm.str();
}

static inline int toInt(const std::string& str)
{
    std::stringstream strm(str);
    int result;
    strm >> result;
    return result;
}

#endif /* COMMON_UTIL_H__ */
