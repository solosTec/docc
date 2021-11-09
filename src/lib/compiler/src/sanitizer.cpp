#include <docc/sanitizer.h>
#ifdef _DEBUG
#include <iostream>
#endif
namespace docscript {

    sanitizer::sanitizer(callback cb)
        : cb_(cb)
    {}

    bool sanitizer::put(std::uint32_t c)
    {
        switch (c) {
        case '\n':  
            cb_(make_nl());
            return true;
        case '\r':  
            return false;
        default:
            //
            //  emit token
            //
            cb_(c);
            break;
        }
        return false;
    }

    void sanitizer::eof()
    {
        cb_(make_eof());
    }

}