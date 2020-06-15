/*
 * The MIT License (MIT)
 * 
 * Copyright (c) 2019 Sylko Olzscher 
 * 
 */ 

#include <docscript/ast.h>
#include <docscript/lookup.h>

#include <cyng/object.h>
#include <cyng/vm/generator.h>
#include <cyng/parser/chrono_parser.h>

#include <boost/algorithm/string.hpp>

namespace docscript
{
	/**
	 * Only visible in this compilation unit
	 */
	void print_ast_n(std::size_t, char);

	ast::ast(bool log)
		: log_(log)
		, root_(make_node_root())
	{}

	node::d_args* ast::get_node_root()
	{
		return access_node_root(root_);
	}

	node::d_args const* ast::get_node_root() const
	{
		return access_node_root(root_);
	}

	cyng::vector_t ast::generate(cyng::filesystem::path out, bool meta, bool index) const
	{
		return generate(get_node_root(), out, meta, index);
	}

	cyng::vector_t ast::generate(node::d_args const* args
		, cyng::filesystem::path out
		, bool meta
		, bool index) const
	{
		cyng::vector_t prg;

		//
		//	build a call frame generate function
		//
		prg
			<< cyng::code::ESBA
			<< out
			;

		if (args != nullptr && !args->empty())	{

			if (log_) {
				std::cout 
					<< "generate_root(" 
					<< args->size() 
					<< ")" 
					<< std::endl;
			}
			//
			//	generate code
			//
			for (auto const& n : *args) {
				prg << cyng::unwinder(generate(0u, n));
			}
		}

		//
		//	generate output file
		//
		prg
			<< cyng::invoke("generate.file")
			<< cyng::code::REBA
			;

		if (meta) {

			//
			//	generate meta 
			//
			out.replace_extension(".json");
			prg << cyng::generate_invoke_unwinded("generate.meta", out);
		}

		if (index) {

			//
			//	generate meta 
			//
			//out.replace_extension(".json");
			prg << cyng::generate_invoke_unwinded("generate.index", out.parent_path());
		}
		return prg;
	}

	cyng::vector_t ast::generate(std::size_t depth, node const& n) const
	{
		switch (n.get_type()) {
		case node::NODE_FUNCTION_PAR:
			return generate_function_par(depth + 1, access_function_params(n), get_function_par_name(n));
		case node::NODE_FUNCTION_VEC:
			return generate_function_vec(depth + 1, access_function_vector(n), get_function_vec_name(n));
		case node::NODE_PARAGRAPH:
			return generate_paragraph(depth + 1, access_node_paragraph(n));
		case node::NODE_CONTENT:
			return generate_content(depth + 1, access_node_content(n));
		case node::NODE_SYMBOL:
			return generate_symbol(depth + 1, access_node_symbol(n));
		case node::NODE_LIST:
			return generate_list(depth + 1, access_node_list(n));
		case node::NODE_VECTOR:
			return generate_vector(depth + 1, access_node_vector(n));
			 
		default:
			//
			//	unknown type
			//
			std::cerr << "*** unknown node type: " << n.get_type() << std::endl;
			break;
		}
		return cyng::vector_t();
	}

	cyng::vector_t ast::generate_paragraph(std::size_t depth, node::d_args const* args) const
	{
		if (log_) {
			print_ast_n(depth, '.');
			std::cout 
				<< "generate_paragraph(" 
				<< args->size() 
				<< ")" 
				<< std::endl;
		}
		cyng::vector_t prg;

		if (!args->empty()) {
			//
			//	reserve for return values
			//	and open function call
			//
			prg
				<< cyng::code::ASP	//	1 return value
				<< cyng::code::ESBA
				;

			//
			//	walktrouh the paragraph
			//
			for (auto const& n : *args) {
				prg << cyng::unwind(generate(depth + 1, n));
			}

			//
			//	trailer
			//
			prg
				<< cyng::invoke("paragraph")
				<< cyng::pr_n(1)	// code::PR
				<< cyng::code::REBA
				;
		}
		return prg;
	}

	cyng::vector_t ast::generate_content(std::size_t depth, node::d_args const* args) const
	{
		if (log_) {
			print_ast_n(depth, '.');
			std::cout
				<< "generate_content("
				<< args->size()
				<< ")"
				<< std::endl;
		}
		cyng::vector_t prg;

		if (!args->empty()) {
			for (auto const& n : *args) {
				prg << cyng::unwind(generate(depth + 1, n));
			}

			prg
				<< args->size()
				<< cyng::code::ASSEMBLE_VECTOR
				;
		}

		return prg;
	}

	cyng::vector_t ast::generate_function_par(std::size_t depth, node::p_args const* args, std::string name) const
	{
		if (log_) {
			print_ast_n(depth, '.');
			std::cout << "generate_function_par(" << name << ": " << args->size() << ")" << std::endl;
		}
		auto const rcount = lookup::rcount(name);
		cyng::vector_t prg;

		//
		//	reserve for return values
		//	and open function call
		//
		prg 
			<< cyng::times(rcount, cyng::code::ASP)
			<< cyng::code::ESBA
			;

		//
		//	build a parameter map
		//
		for (auto const& arg : *args) {

			if (log_) {
				print_ast_n(depth, '.');
				std::cout << "param(" << name << ": " << arg.first << ")" << std::endl;
			}
			verify_param_range(name, arg.first, arg.second);
			prg
				<< cyng::unwind(generate(depth + 1, arg.second))
				<< arg.first
				<< cyng::code::ASSEMBLE_PARAM
				;
		}

		prg
			<< args->size()
			<< cyng::code::ASSEMBLE_PARAM_MAP
			;

		//
		//	trailer
		//
		return prg 
			<< cyng::invoke(name)
			<< cyng::pr_n(rcount)	// code::PR
			<< cyng::code::REBA
			;
	}

	cyng::vector_t ast::generate_function_vec(std::size_t depth, node::v_args const* args, std::string name) const
	{
		if (log_) {
			print_ast_n(depth, '.');
			std::cout << "generate_function_vec(" << name << ": " << args->size() << ")" << std::endl;
		}
		auto const rcount = lookup::rcount(name);
		cyng::vector_t prg;

		//
		//	reserve for return values
		//	and open function call
		//
		prg
			<< cyng::times(rcount, cyng::code::ASP)
			<< cyng::code::ESBA
			;

		//
		//	build a vector
		//
		for (auto const& n : *args) {
			prg << cyng::unwind(generate(depth + 1, n));
		}

		//
		//	trailer
		//
		prg
			<< cyng::invoke(name)
			<< cyng::pr_n(rcount)
			<< cyng::code::REBA
			;

		return prg;
	}

	cyng::vector_t ast::generate_symbol(std::size_t depth, symbol const* sp) const
	{
		if (log_) {
			print_ast_n(depth, '.');
			std::cout << "generate_symbol(" << *sp << ")" << std::endl;
		}
		//
		//	check type to build the correct data type
		//	and handle entities with a special conversion function.
		//
		if (sp->is_type(SYM_NUMBER)) {
			try {
				//
				//	convert to numeric data type
				//
				auto const dots = std::count(sp->value_.begin(), sp->value_.end(), '.');
				switch (dots) {
				case 0:
					//	integer
					return cyng::vector_factory({ static_cast<std::uint64_t>(std::stoull(sp->value_)) });
				case 1:
					//	double
					return cyng::vector_factory({ std::stod(sp->value_, 0) });
				default:
					//	error: use the string
					return cyng::vector_factory({ sp->value_ });
				}
			}
			catch (std::exception const& ex) {
				std::cerr
					<< "*** error: symbol ["
					<< sp->value_
					<< "] is not numeric: "
					<< ex.what()
					<< std::endl;

				cyng::vector_t prg;
				prg
					<< cyng::code::ASP
					<< cyng::code::ESBA
					<< sp->value_
					<< cyng::invoke("convert.numeric")
					<< cyng::pr_n(1)	// code::PR
					<< cyng::code::REBA
					;
				return prg;
			}
		}
		else if (sp->is_type(SYM_DATETIME)) {
			//
			//	convert to datetime object
			//
			auto const r = cyng::parse_rfc3339_timestamp(sp->value_);
			
			if (r.second) {
				return cyng::vector_factory({ r.first });
			}
			else {
				std::cerr << "*** conversion to date-time failed: " << sp->value_ << std::endl;
				return cyng::vector_factory({ sp->value_ });
			}
		}
		else if (sp->is_type(SYM_TEXT)) {
			if (boost::algorithm::equals("true", sp->value_)) {
				return cyng::vector_factory({ true });
			}
			else if (boost::algorithm::equals("false", sp->value_)) {
				return cyng::vector_factory({ false });
			}
			else {
				//
				//	target platform may have special requirements for escaping
				//
				cyng::vector_t prg;
				prg
					<< cyng::code::ASP
					<< cyng::code::ESBA
					<< sp->value_
					<< cyng::invoke("convert.alpha")
					<< cyng::pr_n(1)	// code::PR
					<< cyng::code::REBA
					;
				return prg;
			}
		}
		else {
			return cyng::vector_factory({ sp->value_ });
		}
		return cyng::vector_t{};
	}

	cyng::vector_t ast::generate_list(std::size_t depth, node::s_args const* args) const
	{
		cyng::vector_t prg;

		if (log_) {
			print_ast_n(depth, '.');
			std::cout << "generate_list(" << args->size() << ")" << std::endl;
		}

		for (auto const& sym : *args) {
			prg << cyng::unwind(generate_symbol(depth + 1, &sym));
		}

		prg
			<< args->size()
			<< cyng::code::ASSEMBLE_TUPLE
			;

		return prg;
	}

	cyng::vector_t ast::generate_vector(std::size_t depth, node::v_args const* args) const
	{
		cyng::vector_t prg;

		if (log_) {
			print_ast_n(depth, '.');
			std::cout << "generate_vector(" << args->size() << ")" << std::endl;
		}

		for (auto const& arg : *args) {
			prg << cyng::unwind(generate(depth + 1, arg));
		}

		prg
			<< args->size()
			<< cyng::code::ASSEMBLE_VECTOR
			;

		return prg;
	}

	void ast::generate_html(std::ostream& os, bool linenumbers) const
	{
		//os << "DOSCRIPT" << std::endl;
		generate_html(get_node_root(), os, linenumbers);
	}

	void ast::generate_html(node::d_args const* args, std::ostream& os, bool linenumber) const
	{
		if (args != nullptr && !args->empty()) {

			//
			//	generate code
			//
			bool initialized{ false };
			for (auto const& n : *args) {
				if (initialized) {
					os << std::endl;
				}
				else {
					initialized = true;
				}
				generate_html(0u, n, os, linenumber);
			}
		}
	}

	void ast::generate_html(std::size_t depth, node const& n, std::ostream& os, bool linenumber) const
	{
		switch (n.get_type()) {
		case node::NODE_FUNCTION_PAR:
			return generate_html_function_par(depth + 1, access_function_params(n), get_function_par_name(n), os, linenumber);
		case node::NODE_FUNCTION_VEC:
			return generate_html_function_vec(depth + 1, access_function_vector(n), get_function_vec_name(n), os, linenumber);
		case node::NODE_PARAGRAPH:
			return generate_html_paragraph(depth + 1, access_node_paragraph(n), os, linenumber);
		case node::NODE_CONTENT:
			return generate_html_content(depth + 1, access_node_content(n), os, linenumber);
		case node::NODE_SYMBOL: {
			auto const sp = access_node_symbol(n);
			if (sp != nullptr) generate_html_symbol(depth + 1, *access_node_symbol(n), os, linenumber);
		}
			break;
		case node::NODE_LIST:
			return generate_html_list(depth + 1, access_node_list(n), os, linenumber);
		case node::NODE_VECTOR:
			return generate_html_vector(depth + 1, access_node_vector(n), os, linenumber);

		default:
			//
			//	unknown type
			//
			std::cerr << "*** unknown node type: " << n.get_type() << std::endl;
			break;
		}
	}

	void ast::generate_html_function_par(std::size_t depth, node::p_args const* args, std::string name, std::ostream& os, bool linenumber) const
	{
		if (log_) {
			print_ast_n(depth, '.');
			std::cout << "generate_function_par(" << name << ": " << args->size() << ")" << std::endl;
		}


		if (lookup::is_standalone(name)) {

			//
			//	new line
			//
			os 
				<< std::endl
				<< std::string((depth - 1) * 2, ' ')
				<< '.'
				<< color_blue_
				<< name
				<< end_
				<< '('
				;

		}
		else {
			os
				<< '.'
				<< color_grey_
				<< name
				<< end_
				<< '('
				;
		}

		//
		//	build a parameter map
		//
		std::size_t counter{ 0 };
		for (auto const& arg : *args) {

			if (log_) {
				print_ast_n(depth, '.');
				std::cout << "param(" << name << ": " << arg.first << ")" << std::endl;
			}

			if (counter != 0) {
				os 
					<< std::endl
					<< std::string(depth * 2, ' ')
					<< ", ";
			}

			os
				<< color_red_
				<< arg.first
				<< end_
				<< ':'
				<< ' '
				;

			generate_html(depth + 1, arg.second, os, linenumber);
			++counter;
		}

		//
		//	trailer
		//
		os
			<< ')'
			//<< std::endl
			;
	}

	void ast::generate_html_paragraph(std::size_t depth, node::d_args const* args, std::ostream& os, bool linenumber) const
	{
		if (log_) {
			print_ast_n(depth, '.');
			std::cout
				<< "generate_paragraph("
				<< args->size()
				<< ")"
				<< std::endl;
		}

		if (!args->empty()) {
			//
			//	walktrouh the paragraph
			//
			for (auto const& n : *args) {
				generate_html(depth + 1, n, os, linenumber);
			}

		}
	}

	void ast::generate_html_content(std::size_t depth, node::d_args const* args, std::ostream& os, bool linenumber) const
	{
		if (log_) {
			print_ast_n(depth, '.');
			std::cout
				<< "generate_content("
				<< args->size()
				<< ")"
				<< std::endl;
		}

		os << '(';
		std::size_t counter{ 0 };
		//	reverse
		for (auto pos = args->crbegin(); pos != args->crend(); ++pos) {
			if (counter != 0) {
				os
					<< ' ';
			}
			generate_html(depth + 1, *pos, os, linenumber);
			++counter;
		}

		os << ')';
	}

	void ast::generate_html_function_vec(std::size_t depth, node::v_args const* args, std::string name, std::ostream& os, bool linenumber) const
	{
		if (log_) {
			print_ast_n(depth, '.');
			std::cout << "generate_function_vec(" << name << ": " << args->size() << ")" << std::endl;
		}

		if (lookup::is_standalone(name)) {

			//
			//	new line
			//
			os
				<< std::endl
				<< '.'
				<< color_blue_
				<< name
				<< end_
				;

		}
		else {
			os
				<< '.'
				<< color_grey_
				<< name
				<< end_
				;
		}

		if (args->size() == 1) {
			os << ' ';
			generate_html(depth + 1, args->at(0), os, linenumber);
			os << ' ';
		}
		else {
			//
			//	build a vector - reverse
			//
			os << '(';
			//	reverse
			for (auto pos = args->crbegin(); pos != args->crend(); ++pos) {
				generate_html(depth + 1, *pos, os, linenumber);

			}
			os << ')';
		}
	}

	void ast::generate_html_symbol(std::size_t depth, symbol const& sym, std::ostream& os, bool linenumber) const
	{

		if (log_) {
			print_ast_n(depth, '.');
			std::cout << "generate_symbol(" << sym << ")" << std::endl;
		}
		//
		//	check type to build the correct data type
		//	and handle entities with a special conversion function.
		//
		if (sym.is_type(SYM_NUMBER)) {
			//
			//	numeric data type
			//
			os
				<< color_brown_
				<< sym.value_
				<< end_
				;
		}
		else if (sym.is_type(SYM_TEXT)) {
			if (boost::algorithm::equals("true", sym.value_)) {
				os
					<< color_blue_
					<< sym.value_
					<< end_
					;
			}
			else if (boost::algorithm::equals("false", sym.value_)) {
				os
					<< color_blue_
					<< sym.value_
					<< end_
					;
			}
			else {
				os
					<< sym.value_
					;
			}
		}
		else if (sym.is_type(SYM_VERBATIM)) {
			os
				<< "'"
				<< color_green_
				<< sym.value_
				<< end_
				<< "'"
				;
		}
		else {
			os
				<< sym.value_
				;
		}
	}

	void ast::generate_html_list(std::size_t depth, node::s_args const* args, std::ostream& os, bool linenumber) const
	{
		if (log_) {
			print_ast_n(depth, '.');
			std::cout << "generate_list(" << args->size() << ")" << std::endl;
		}

		os << '"';
		bool initial{ true };
		//	reverse order
		for (auto pos = args->crbegin(); pos != args->crend(); ++pos) {
			if (initial) {
				initial = false;
			}
			else {
				os << ' ';
			}
			generate_html_symbol(depth + 1, *pos, os, linenumber);
		}
		os << '"';
	}

	void ast::generate_html_vector(std::size_t depth, node::v_args const* args, std::ostream& os, bool linenumber) const
	{
		if (log_) {
			print_ast_n(depth, '.');
			std::cout << "generate_vector(" << args->size() << ")" << std::endl;
		}

		os << '[';
		std::size_t counter{ 0 };

		//	reverse
		for (auto pos = args->crbegin(); pos != args->crend(); pos++) {
			if (counter != 0) {
				os
					<< ','
					<< ' '
					;
			}
			generate_html(depth + 1, *pos, os, linenumber);
			++counter;
		}

		os << ']';
	}

	bool ast::verify_param_range(std::string const& cmd, std::string const& name, node const& n) const
	{
		if (boost::algorithm::equals(cmd, "list") && boost::algorithm::equals(name, "style")) {

			//
			//	valid list style types are
			//
			auto const style = get_name(n);
			if (!is_valid_list_style(style)) {
				std::cerr << "*** unknown list style: " << style << std::endl;
			}
		}
		return true;
	}

	bool is_valid_list_style(std::string style) {

		if (boost::algorithm::iequals(style, "disc"))	return true;	//	Default value.The marker is a filled circle
		if (boost::algorithm::iequals(style, "circle"))	return true;	//	The marker is a circle
		if (boost::algorithm::iequals(style, "none"))	return true;	//	No marker is shown
		if (boost::algorithm::iequals(style, "square"))	return true;	//	The marker is a square

		if (boost::algorithm::iequals(style, "armenian"))	return true;	//	The marker is traditional Armenian numbering
		if (boost::algorithm::iequals(style, "cjk"))	return true;	// - ideographic	The marker is plain ideographic numbers
		if (boost::algorithm::iequals(style, "decimal"))	return true;	//	The marker is a number
		if (boost::algorithm::iequals(style, "decimal-leading-zero"))	return true;	//	The marker is a number with leading zeros(01, 02, 03, etc.)
		if (boost::algorithm::iequals(style, "georgian"))	return true;	//	The marker is traditional Georgian numbering
		if (boost::algorithm::iequals(style, "hebrew"))	return true;	//	The marker is traditional Hebrew numbering
		if (boost::algorithm::iequals(style, "hiragana"))	return true;	//	The marker is traditional Hiragana numbering
		if (boost::algorithm::iequals(style, "hiragana"))	return true;	// - iroha	The marker is traditional Hiragana iroha numbering
		if (boost::algorithm::iequals(style, "katakana"))	return true;	//	The marker is traditional Katakana numbering
		if (boost::algorithm::iequals(style, "katakana-iroha"))	return true;	//	The marker is traditional Katakana iroha numbering
		if (boost::algorithm::iequals(style, "lower-alpha"))	return true;	//	The marker is lower - alpha(a, b, c, d, e, etc.)
		if (boost::algorithm::iequals(style, "lower-greek"))	return true;	//	The marker is lower - greek
		if (boost::algorithm::iequals(style, "lower-latin"))	return true;	//	The marker is lower - latin(a, b, c, d, e, etc.)
		if (boost::algorithm::iequals(style, "lower-roman"))	return true;	//	The marker is lower - roman(i, ii, iii, iv, v, etc.)
		if (boost::algorithm::iequals(style, "upper"))	return true;	// - alpha	The marker is upper - alpha(A, B, C, D, E, etc.)
		if (boost::algorithm::iequals(style, "upper-greek"))	return true;	//	The marker is upper - greek
		if (boost::algorithm::iequals(style, "upper-latin"))	return true;	//	The marker is upper - latin(A, B, C, D, E, etc.)
		if (boost::algorithm::iequals(style, "upper-roman"))	return true;	//	The marker is upper - roman(I, II, III, IV, V, etc.)
		return false;
	}

	void print_ast_n(std::size_t n, char c)
	{
		std::cout 
			<< std::string(n, c)
			<< '>';
	}

	std::string const ast::color_green_ = "<span style=\"color: green;\">";
	std::string const ast::color_blue_ = "<span style=\"color: blue;\">";
	std::string const ast::color_brown_ = "<span style=\"color: brown;\">";
	std::string const ast::color_red_ = "<span style=\"color: sienna;\">";
	std::string const ast::color_grey_ = "<span style=\"color: grey;\">";
	std::string const ast::end_ = "</span>";

}


