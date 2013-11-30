#ifndef __COMMON_UTIL_H__
#define __COMMON_UTIL_H__

template<typename T>
static inline std::string toString(T obj)
{
    std::ostringstream strm;
    strm << obj;
    return strm.str();
}

#endif /* COMMON_UTIL_H__ */
