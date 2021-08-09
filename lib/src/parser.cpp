#include <parser.h>
#include <context.h>

#ifdef _DEBUG
#include <iostream>
#endif

#include <boost/algorithm/string.hpp>

namespace docscript {

    parser::parser(context const& ctx)
        : ctx_(ctx)
    {}

    void parser::put(symbol const& sym)
    {
        std::cout << ctx_.get_position() << ": " << sym << std::endl;
    }


}