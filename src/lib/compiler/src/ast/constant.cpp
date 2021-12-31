#include <boost/algorithm/string.hpp>
#include <boost/assert.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <cyng/io/ostream.h>
#include <cyng/parse/color.h>
#include <cyng/parse/timestamp.h>
#include <docc/ast/constant.h>
#include <docc/context.h>
#include <sstream>

namespace docscript {
    namespace ast {

	template < class... Ts > struct overloaded : Ts... {
		using Ts::operator()...;
	};
	// explicit deduction guide (not needed as of C++20)
	template < class... Ts > overloaded(Ts...)->overloaded< Ts... >;

	//
	//	----------------------------------------------------------*
	//	-- constant
	//	----------------------------------------------------------*
	//
	constant constant::factory(symbol const& sym)
	{

	    switch (sym.type_) {
	    case symbol_type::TST:
		//	parse string with timestamp info
		//	format: "YYYY-MM-DD[THH:MM:SS[Z|[+|-]hh::mm]]
		return constant{ sym.value_, cyng::to_timestamp(sym.value_) };
	    case symbol_type::BOL: return constant{ sym.value_, boost::algorithm::equals(sym.value_, "true") };
	    case symbol_type::NUM:
		if (!sym.value_.empty()) {
		    try {
			return (sym.value_.at(0) == '-')
			           ? constant{ sym.value_, static_cast< std::int64_t >(std::stoll(sym.value_, nullptr, 10)) }
			           : constant{ sym.value_, static_cast< std::uint64_t >(std::stoull(sym.value_, nullptr, 10)) };
		    }
		    catch (...) {
			return constant{ sym.value_, static_cast< std::uint64_t >(0) };
		    }
		}
		break;
	    case symbol_type::FLT:
	    case symbol_type::EXP: try { return constant{ sym.value_, std::stod(sym.value_, nullptr) };
		}
		catch (...) {
		    return constant{ sym.value_, static_cast< double >(0.0) };
		}
	    case symbol_type::COL:
		//	color
		return constant{ sym.value_, cyng::to_color< std::uint8_t >(sym.value_) };
	    default: break;
	    }
	    return constant::factory(sym.value_);
	}

	constant constant::factory(std::string const& s) { return constant{ s, s }; }

	constant constant::factory(cyng::raw const& r) { return constant{ r.get_literal(), r }; }

	void constant::compile(std::function< void(std::string const&) > emit, std::size_t depth, std::size_t index) const
	{
	    // std::cout << "constant::compile()" << std::endl;
		emit("push ");
		std::stringstream ss;
	    // ss << *this;

	    std::visit(
	        overloaded{
	            [&](double arg) { ss << std::scientific << arg; }, //	1.000000e-02
	            [&](std::int64_t arg) { ss << std::dec << arg << "i64"; },
	            [&](std::uint64_t arg) { ss << std::dec << arg << "u64"; },
	            [&](std::string const& arg) {
		        // represent certain special characters in the target language (HTML, LaTeX, ...)
					ss << std::quoted(arg);
		   //     ss 
					//<< "push " << std::quoted(arg) << "\n"
					//<< "push 1\n"	//	vector with one elemenet
					//<< "make_vector\n"
					//<< "push 1\n"
					//<< "invoke_r esc\n"
					//;
	            },
	            [&](std::chrono::system_clock::time_point const& arg) {
					const std::time_t t_c = std::chrono::system_clock::to_time_t(arg);
					ss << std::put_time(std::localtime(&t_c), "@%FT%T");
	            },
	            [&](bool arg) { ss << (arg ? "true" : "false"); },
	            [&](cyng::raw const& arg) {
					//	'type:literal'
					ss << '\'' << arg.get_code_name() << ':' << arg.get_literal() << '\'';
	            },
	            [&](cyng::color_8 const& arg) { ss << arg << "\t; color"; },
	        },
	        node_);

	    emit(ss.str());
	    emit("\n");
	}
	void constant::transform(context const&) { }


	std::ostream& operator<<(std::ostream& os, constant const& c)
	{
	    std::visit(
	        overloaded{
	            [&](double arg) { os << std::scientific << arg; }, //	1.000000e-02
	            [&](std::int64_t arg) { os << std::dec << (arg > 0 ? "+" : "") << arg << "i64"; },
	            [&](std::uint64_t arg) { os << std::dec << arg << "u64"; },
	            [&](std::string const& arg) {
		        // os << std::quoted(arg) << "\t; " << arg.size() << " bytes";
		        os << std::quoted(arg);
	            },
	            [&](std::chrono::system_clock::time_point const& arg) {
		        const std::time_t t_c = std::chrono::system_clock::to_time_t(arg);
		        os << std::put_time(std::localtime(&t_c), "@%FT%T");
	            },
	            [&](bool arg) { os << (arg ? "true" : "false"); },
	            [&](cyng::raw const& arg) {
		        //	'type:literal'
		        os << '\'' << arg.get_code_name() << ':' << arg.get_literal() << '\'';
	            },
	            [&](cyng::color_8 const& arg) { os << arg; },
	        },
	        c.node_);
	    return os;
	}

    }
}
