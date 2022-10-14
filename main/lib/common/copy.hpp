#pragma once
#include <cstring>
#include <string>

template<class sourcetype, class desttype>
inline void copy(desttype& destination, sourcetype* source, size_t size = sizeof(desttype))
{
    memcpy(&destination, source, size);
}

//-----------------------------------------------------------------------------//

/**
 * @brief Memcpy source to destination and offset source-size
*/
template<class src, class dst, class sizetype>
inline void copynmov(dst& destination, src** source, sizetype& size)
{
    memcpy(&destination, *source, sizeof(dst));
    size += sizeof(dst);
    *source += sizeof(dst);
}

/**
 * @brief Memcpy source to destination and offset source
*/
template<class src, class dst>
inline void copynmov(dst& destination, src** source, size_t size = sizeof(dst))
{
    memcpy(&destination, *source, size);
    *source += sizeof(dst);
}

/**
 * @brief Memcpy source to destination and offset destination-size
*/
template<class src, class dst, class sizetype>
inline void copynmov(dst** destination, const src& source, sizetype& size)
{
    memcpy(*destination, &source, sizeof(src));
    size += sizeof(src);
    *destination += sizeof(src);
}

/**
 * @brief Memcpy source to destination and offset destination
*/
template<class src, class dst>
inline void copynmov(dst** destination, const src& source)
{
    memcpy(*destination, &source, sizeof(src));
    *destination += sizeof(src);
}

//-----------------------------------------------------------------------------//

/**
 * @brief Copy each elements of array while predicate(s : source) is true
*/
template<class copytype, class pred, class sizetype>
inline void copywhile(copytype** source, copytype* destination, pred predicate, sizetype& size)
{
    copytype* _source = *source;
    while(predicate(*_source))
    {
        *destination++ = *_source++;
        ++size;
    }
    *source = _source;
}

template<class copytype, class pred>
inline void copywhile(copytype** source, copytype* destination, pred predicate)
{
    copytype* _source = *source;
    while(predicate(*_source))
        *destination++ = *_source++;
    *source = _source;
}

template<class copytype, class pred>
inline void copydowhile(copytype** source, copytype* destination, pred predicate)
{
    copytype* _source = *source;
    do
        *destination++ = *_source++;
    while(predicate(*_source));
    *source = ++_source;
}

//-----------------------------------------------------------------------------//

/**
 * @brief Convert source -> char* -> std::string, offset source-i  
*/
template<class srctype, class sizetype>
inline void copyName(srctype** source, std::string& name, sizetype& i)
{
    name = std::string((char*)*source);
    sizetype len = name.size() + 1;

    *source += len;
    i += len;
}

/**
 * @brief Copy std::string::c_str() to destination, offset dest-i
*/
template<class desttype, class sizetype>
inline void copyNameTo(desttype** destination, const std::string& name, sizetype& i)
{
    sizetype len = name.size() + 1;
    memcpy(*destination, name.c_str(), len);

    *destination += len;
    i += len;
}

/**
 * @brief Copy std::string::c_str() to destination with x0
*/
template<class desttype>
inline void copyNameTo(desttype* destination, const std::string& name)
{
    memcpy(destination, name.c_str(), name.size() + 1);
}

/**
 * @brief Copy std::string::c_str() to destination, offset dest-i
*/
template<class desttype>
inline void copyNameTonmov(desttype** destination, const std::string& name)
{
    size_t len = name.size() + 1;
    memcpy(*destination, name.c_str(), len);
    *destination += len;
}

#define _ASSERT_(statement) if(statement) return;

//-----------------------------------------------------------------------------//

template<class type>
inline bool isEqual(const type* one, const type* two, size_t size)
{
    while(--size)
    {
        if(*one++ != *two++)
            return false;
    }
    return true;
}
