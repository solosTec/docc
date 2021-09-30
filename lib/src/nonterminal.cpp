#include <nonterminal.h>
#include <boost/assert.hpp>

namespace docscript {

	nonterminal::nonterminal(symbol sym) 
		: sym_(sym)
		, nttype_(nonterminal_type::TERMINAL)
	{}

	nonterminal::nonterminal(nonterminal_type type)
		: sym_(make_symbol(symbol_type::NOT, std::string()))
		, nttype_(type)
	{}

	nonterminal::nonterminal(symbol sym, nonterminal_type type) 
		: sym_(sym)
		, nttype_(type)
	{}

	nonterminal::operator nonterminal_type() const
	{
		return nttype_;
	}


	std::ostream& operator<<(std::ostream& os, nonterminal_type nt) {
		switch (nt) {
		case nonterminal_type::BODY:
			os << "BODY";
			break;
		case nonterminal_type::LIST:
			os << "LIST";
			break;
		case nonterminal_type::TERM:
			os << "TERM";
			break;
		case nonterminal_type::VECTOR:
			os << "VECTOR";
			break;
		case nonterminal_type::MAP:
			os << "MAP";
			break;
		case nonterminal_type::TAIL:
			os << "TAIL";
			break;
		case nonterminal_type::SVM:
			os << "SVM";
			break;
		case nonterminal_type::VALUE:
			os << "VALUE";
			break;
		case nonterminal_type::PARAMS:
			os << "PARAMS";
			break;
		case nonterminal_type::TERMINAL:
			os << "TERMINAL";
			break;
		default:
			os << "unknown NT";
			break;
		}
		return os;
	}

	std::ostream& operator<<(std::ostream& os, nonterminal nt) {
		os << '{' << nt.nttype_ << '}';
		return os;
	}

}
