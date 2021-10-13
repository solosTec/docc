#include <symbol.h>
#include <boost/assert.hpp>

namespace docscript {

	symbol::symbol()
		: type_(symbol_type::EOD)
		, value_()
	{}

	symbol::symbol(symbol_type type, std::string&& str)
		: type_(type)
		, value_(std::move(str))
	{}

	bool symbol::equals(char c) const
	{
		if (value_.size() == 1) {
			return value_.at(0) == c;
		}
		return false;
	}

	bool symbol::equals(symbol_type type, char c) const
	{
		return equals(type) && equals(c);
	}

	bool symbol::equals(symbol_type type) const
	{
		return (type_ == type);
	}

	bool operator==(symbol sym, symbol_type type)
	{
		return sym.equals(type);
	}

	bool operator!=(symbol sym, symbol_type type)
	{
		return !(sym == type);
	}

    std::ostream& operator<<(std::ostream& os, const symbol& sym)
	{
		os << '<';
		switch(sym.type_)	{
		case symbol_type::EOD:
			os << "EOD";
			break;
		case symbol_type::FUN:
			os << "FUN";
			break;
		case symbol_type::TXT:
			os << "TXT";
			break;
		case symbol_type::SYM:
			os << "SYM";
			break;
		case symbol_type::TST:
			os << "TST";
			break;
		case symbol_type::COL:
			os << "COL";	//	color
			break;
		case symbol_type::BOL:	//	boolean
			os << "BOL";
			break;
		case symbol_type::NUM:	//	number (unsigned integer)
			os << "NUM";
			break;
		case symbol_type::TYP:
			os << "TYP";
			break;
		case symbol_type::PAR:
			os << "PAR";
			break;
		case symbol_type::DQU:
			os << "DQU";
			break;
		case symbol_type::INC:
			os << "INC";
			break;
		case symbol_type::FIL:
			os << "FIL";
			break;
		case symbol_type::LIN:
			os << "LIN";
			break;
		case symbol_type::NOT:
			os << "NOT";
			break;
		default:
			os << "ERR";
			break;
		}
		os 
			<< ':'
			<< sym.value_
			<< '>';
		return os;
	}     

	symbol make_symbol(symbol_type type, std::string&& str)
	{
		return symbol(type, std::move(str));
	}

	symbol make_symbol(std::filesystem::path const& p)
	{
		return symbol(symbol_type::FIL, p.string());
	}

	symbol make_symbol(std::size_t line)
	{
		return symbol(symbol_type::LIN, std::to_string(line));
	}

	symbol make_symbol() {
		return symbol(symbol_type::EOD, "");
	}

}
