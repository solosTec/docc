#include <boost/assert.hpp>
#include <docc/symbol.h>

namespace docscript {

    symbol::symbol()
        : type_(symbol_type::EOD)
        , value_() {}

    symbol::symbol(symbol_type type, std::string &&str)
        : type_(type)
        , value_(std::move(str)) {}

    bool symbol::equals(char c) const {
        if (value_.size() == 1) {
            return value_.at(0) == c;
        }
        return false;
    }

    bool symbol::equals(symbol_type type, char c) const { return equals(type) && equals(c); }

    bool symbol::equals(symbol_type type) const { return (type_ == type); }

    bool operator==(symbol sym, symbol_type type) { return sym.equals(type); }

    bool operator!=(symbol sym, symbol_type type) { return !(sym == type); }

    std::ostream &operator<<(std::ostream &os, const symbol &sym) {
        os << '<' << to_string(sym.type_) << ':' << sym.value_ << '>';
        return os;
    }

    symbol make_symbol(symbol_type type, std::string &&str) { return symbol(type, std::move(str)); }

    symbol make_symbol(std::filesystem::path const &p) { return symbol(symbol_type::FIL, p.string()); }

    symbol make_symbol(std::size_t line) { return symbol(symbol_type::LIN, std::to_string(line)); }

    symbol make_symbol() { return symbol(symbol_type::EOD, ""); }

    std::string to_string(symbol_type st) {
        switch (st) {
        case symbol_type::EOD:
            return "EOD";
        case symbol_type::FUN:
            return "FUN";
        case symbol_type::TXT:
            return "TXT";
        case symbol_type::SYM:
            return "SYM";
        case symbol_type::TST:
            return "TST";
        case symbol_type::COL:
            return "COL";      //	color
        case symbol_type::BOL: //	boolean
            return "BOL";
        case symbol_type::NUM: //	number (unsigned integer)
            return "NUM";
        case symbol_type::FLT: //	floating point
            return "FLT";
        case symbol_type::EXP: //	floating point with exponent
            return "EXP";
        case symbol_type::TYP:
            return "TYP";
        case symbol_type::PAR:
            return "PAR";
        case symbol_type::DQU:
            return "DQU";
        case symbol_type::INC:
            return "INC";
        case symbol_type::FIL:
            return "FIL";
        case symbol_type::LIN:
            return "LIN";
        case symbol_type::NOT:
            return "NOT";
        default:
            break;
        }
        return "ERR";
    }

} // namespace docscript
