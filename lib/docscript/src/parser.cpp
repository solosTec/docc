/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Sylko Olzscher
 *
 */

#include <docscript/parser.h>
#include <docscript/lookup.h>

#include <iomanip>
#include <algorithm>
#include <sstream>

namespace docscript
{
	parser::parser(typename symbol_reader::symbol_list_t& sl, int verbosity)
		: producer_(sl)
		, ast_(verbosity > 4)
	{}

	ast const& parser::get_ast() const
	{
		return ast_;
	}

	void parser::parse()
	{
		//
		//	main loop until EOF
		//
		loop(0, ast_.get_node_root());
	}

	void parser::loop(std::size_t depth, node::d_args* args)
	{
		while (true) {

			switch (producer_.get().type_) {

			case SYM_EOF:	return;
			case SYM_UNKNOWN:	return;

			case SYM_TOKEN:
				if (lookup::is_standalone(producer_.get().value_)) {
					args->push_back(generate_function(depth));
				}
				else {
					//	new paragraph
					print_error(cyng::logging::severity::LEVEL_WARNING, "missing paragraph");
					node::d_args dargs;
					dargs.push_back(generate_function(depth));
					args->push_back(generate_paragraph(depth, dargs));
				}
				break;

			case SYM_TEXT:
			case SYM_VERBATIM:
			case SYM_NUMBER:

			case SYM_DQUOTE:
			case SYM_SQUOTE:
			case SYM_OPEN:
			case SYM_CLOSE:
			case SYM_SEP:
			case SYM_KEY:
			case SYM_BEGIN:
			case SYM_END:
				//	missing paragraph
				print_error(cyng::logging::severity::LEVEL_WARNING, "missing paragraph");
				{
					node::d_args dargs;
					dargs.push_back(make_node_symbol(producer_.get()));
					match();
					args->push_back(generate_paragraph(depth, dargs));
				}
				break;

			case SYM_PAR:
				//
				//	There is a special case when after a NL the text starts with function call. 
				//	In some cases this could be part of the new paragraph (e.g. formatting). In other cases this could
				//	be a single function call (e.g. new header). It depends on the function name.
				//
				match(SYM_PAR);
				if (producer_.get().is_type(SYM_TOKEN) && lookup::is_standalone(producer_.get().value_)) {
					args->push_back(generate_function(depth));
				}
				else {
					node::d_args dargs;
					args->push_back(generate_paragraph(depth, dargs));
				}
				break;


			default:
				break;
			}
		}
	}

	node parser::generate_paragraph(std::size_t depth, node::d_args& args)
	{
		//
		//	generate paragraph
		//

		while (!producer_.is_eof() && !producer_.get().is_type(SYM_PAR)) {

#ifdef _DEBUG
			//std::cout << producer_.get() << std::endl;
#endif

			switch (producer_.get().type_) {

			case SYM_TEXT:
			case SYM_VERBATIM:
			case SYM_NUMBER:
			case SYM_DQUOTE:
			case SYM_SQUOTE:
			case SYM_OPEN:
			case SYM_CLOSE:
			case SYM_SEP:
			case SYM_KEY:
			case SYM_BEGIN:
			case SYM_END:
				args.push_back(make_node_symbol(producer_.get()));
				match();
				break;

			case SYM_TOKEN:
				if (lookup::is_standalone(producer_.get().value_)) {
					print_error(cyng::logging::severity::LEVEL_WARNING, "standalone function in paragraph");
					return make_node_paragraph(std::move(args));
				}
				args.push_back(generate_function(depth));
				break;

			case SYM_PAR:
				BOOST_ASSERT(false);
				break;

			default:
				//
				//	next token
				//
				match();
				break;
			}
		}
		return make_node_paragraph(std::move(args));
	}

	node parser::generate_content(std::size_t depth)
	{
		match(SYM_OPEN);
		std::size_t open_counter{ 1 };

		//
		//	generate paragraph
		//
		node::d_args args;

		while (!producer_.is_eof() && (open_counter != 0u)) {

#ifdef _DEBUG
			//std::cout << producer_.get() << std::endl;
#endif

			switch (producer_.get().type_) {

			case SYM_DQUOTE:
			case SYM_SQUOTE:
			case SYM_SEP:
			case SYM_KEY:
			case SYM_BEGIN:
			case SYM_END:
			case SYM_TEXT:
			case SYM_VERBATIM:
			case SYM_NUMBER:
				args.push_back(make_node_symbol(producer_.get()));
				match();
				break;

			case SYM_TOKEN:
				if (lookup::is_standalone(producer_.get().value_)) {
					print_error(cyng::logging::severity::LEVEL_WARNING, "paragraph ended by standalone function");
				}
				args.push_back(generate_function(depth));
				break;

			case SYM_PAR:
				print_error(cyng::logging::severity::LEVEL_WARNING, "nested paragraphs are bot allowed");
				match(SYM_PAR);
				break;

			case SYM_OPEN:
				++open_counter;
				args.push_back(make_node_symbol(producer_.get()));
				match(SYM_OPEN);
				break;
			case SYM_CLOSE:
				--open_counter;
				if (open_counter != 0u) {
					args.push_back(make_node_symbol(producer_.get()));
					match(SYM_CLOSE);
				}
				break;

			default:
				//
				//	next token
				//
				match();
				break;
			}
		}
		match(SYM_CLOSE);
		std::reverse(args.begin(), args.end());	//	preserve original ordering for generator
		return make_node_content(std::move(args));
	}


	node parser::generate_vector(std::size_t depth)
	{
		match(SYM_BEGIN);

		node::v_args args;

		while (!producer_.is_eof() && !producer_.get().is_type(SYM_END)) {
			//
			//	functions allowed
			//
			switch (producer_.get().type_) {

			case SYM_TOKEN:
				args.push_back(generate_function(depth + 1));
				break;

			case SYM_BEGIN:
				args.push_back(generate_vector(depth));
				break;

			case SYM_END:
				BOOST_ASSERT_MSG(false, "internal error");
				break;

			case SYM_OPEN:
				args.push_back(generate_content(depth + 1));
				break;

			default:
				args.push_back(make_node_symbol(producer_.get()));
				match();
				break;
			}

			if (!producer_.get().is_type(SYM_END)) {
				match(SYM_SEP);
			}
		}

		match(SYM_END);
		std::reverse(args.begin(), args.end());	//	preserve original ordering for generator
		return make_node_vector(std::move(args));
	}

	node parser::generate_function(std::size_t depth)
	{
		//
		//	preserve function name
		//
		auto const sym = producer_.get();

		//
		//	next token
		//
		match(SYM_TOKEN);

		switch (producer_.get().type_) {
		case SYM_EOF:	
		case SYM_UNKNOWN:
			break;

		case SYM_TEXT:
		case SYM_VERBATIM:
		case SYM_NUMBER:

		case SYM_DQUOTE:
		case SYM_SQUOTE:
		case SYM_CLOSE:
		case SYM_SEP:
		case SYM_KEY:
		case SYM_BEGIN:
		case SYM_END:
			//	single argument function
			if (producer_.look_ahead().is_type(SYM_KEY)) {
				auto n = make_node_function_par(sym.value_);
				generate_arg(depth, access_function_params(n));
				return n;
			}
			else {
				auto n = make_node_function_vec(sym.value_);
				generate_arg(depth, access_function_vector(n));
				return n;
			}
			break;

		case SYM_TOKEN:
		{
			//	nested function call
			auto n = make_node_function_vec(sym.value_);
			auto args = access_function_vector(n);
			args->push_back(generate_function(depth + 1));
			return n;
		}

		case SYM_PAR:
			break;

		case SYM_OPEN:
			match(SYM_OPEN);
			if (producer_.look_ahead().is_type(SYM_KEY)) {

				//
				//	expecting a key:value list
				//
				auto n = make_node_function_par(sym.value_);
				auto const size = generate_list(depth, access_function_params(n));
				return n;
			}
			else {

				//
				//	expection a vector list
				//
				auto n = make_node_function_vec(sym.value_);
				auto const size = generate_list(depth, access_function_vector(n));
				return n;
			}
			break;

		default:
			break;
		}

		//
		//	empty node
		//
		return make_node();
	}

	void parser::generate_arg(std::size_t depth, node::v_args* args)
	{
		//
		//	function with a vector list
		//
		BOOST_ASSERT(args != nullptr);

		switch (producer_.get().type_) {
		case SYM_EOF:	
		case SYM_UNKNOWN:
			break;

		case SYM_TEXT:
		case SYM_VERBATIM:
		case SYM_NUMBER:

		case SYM_DQUOTE:
		case SYM_SQUOTE:
		case SYM_OPEN:
		case SYM_CLOSE:
		case SYM_SEP:
		case SYM_KEY:
		case SYM_BEGIN:
		case SYM_END:
			args->push_back(make_node_symbol(producer_.get()));
			match();
			break;

		case SYM_TOKEN:
			args->push_back(generate_function(depth + 1));
			break;

		case SYM_PAR:
			break;

		default:
			BOOST_ASSERT_MSG(false, "internal error");
			break;
		}
	}

	void parser::generate_arg(std::size_t depth, node::p_args* args)
	{
		//
		//	function with a parameter list
		//
		BOOST_ASSERT(args != nullptr);

		switch (producer_.get().type_) {
		case SYM_EOF:
		case SYM_UNKNOWN:
			break;

		case SYM_TEXT:
		case SYM_VERBATIM:
		case SYM_NUMBER:

		case SYM_DQUOTE:
		case SYM_SQUOTE:
		case SYM_OPEN:
		case SYM_CLOSE:
		case SYM_SEP:
		case SYM_KEY:
		case SYM_BEGIN:
		case SYM_END:
			//	key:value
			args->insert(generate_parameter(depth));
			break;

		case SYM_TOKEN:
			break;

		case SYM_PAR:
			break;
		default:
			break;
		}

	}

	std::size_t parser::generate_list(std::size_t depth, node::p_args* args)
	{
		BOOST_ASSERT(args != nullptr);

		std::size_t counter{ 0 };
		while (!producer_.is_eof() && !producer_.get().is_type(SYM_CLOSE)) {

			switch (producer_.get().type_) {
			case SYM_EOF:	return counter;
			case SYM_UNKNOWN:	return counter;

			case SYM_TEXT:
			case SYM_VERBATIM:
			case SYM_NUMBER:

			case SYM_DQUOTE:
			case SYM_SQUOTE:
			case SYM_OPEN:
			case SYM_CLOSE:
			case SYM_SEP:
			case SYM_KEY:
			case SYM_BEGIN:
			case SYM_END:
				//	key:value
				args->insert(generate_parameter(depth));
				break;

			case SYM_TOKEN:
				print_error(cyng::logging::severity::LEVEL_ERROR, "key name cannot be calculated");
				match(SYM_TOKEN);
				break;
			case SYM_PAR:
				print_error(cyng::logging::severity::LEVEL_WARNING, "key name expected");
				match(SYM_PAR);
				break;
			default:
				break;
			}

			//
			//	parameters are separated by ","
			//
			if (!producer_.is_eof() && !producer_.get().is_type(SYM_CLOSE)) {
				if (!producer_.get().is_type(SYM_SEP)) {
					print_error(cyng::logging::severity::LEVEL_ERROR, "parameters have to be separated by commas, but get ");

					//
					//	try to recover from error
					//
					while (!producer_.is_eof() && !producer_.get().is_type(SYM_CLOSE) && !producer_.get().is_type(SYM_SEP)) {
						print_error(cyng::logging::severity::LEVEL_INFO, "try to recover from error");
						match();
					}
					if (!producer_.is_eof() && !producer_.get().is_type(SYM_CLOSE)) {
						match(SYM_SEP);
					}

				}
				else {
					match(SYM_SEP);
				}
			}

			//
			//	update parameter counter
			//
			++counter;
		}

		match(SYM_CLOSE);
		return counter;
	}

	std::size_t parser::generate_list(std::size_t depth, node::v_args* args)
	{
		std::size_t counter{ 0 };
		while (!producer_.is_eof() && !producer_.get().is_type(SYM_CLOSE)) {

			//
			//	update vector size
			//
			++counter;

			switch (producer_.get().type_) {
			case SYM_EOF:	return counter;
			case SYM_UNKNOWN:	return counter;

			case SYM_TEXT:
			case SYM_VERBATIM:
			case SYM_NUMBER:

			case SYM_DQUOTE:
			case SYM_SQUOTE:
			case SYM_OPEN:
			case SYM_CLOSE:
			case SYM_KEY:
			case SYM_BEGIN:
			case SYM_END:
			case SYM_SEP:	//	in a vector the "," separator is treated like normal character

				args->push_back(make_node_symbol(producer_.get()));
				match();
				break;

			case SYM_TOKEN:
				//
				//	function calls in vector list are possible
				//
				args->push_back(generate_function(depth + 1));
				break;

			case SYM_PAR:
				print_error(cyng::logging::severity::LEVEL_WARNING, "key:value expected - but get a new paragraph");
				match(SYM_PAR);
				break;
			default:
				break;
			}
		}

		match(SYM_CLOSE);
		return counter;
	}

	std::pair<std::string, node> parser::generate_parameter(std::size_t depth)
	{
		symbol const key = producer_.get();

		//
		//	next token
		//
		match(key.type_);	//	match parameter name
		match(SYM_KEY);		//	match ":" symbol

		symbol const value = producer_.get();

		switch (producer_.get().type_) {
		case SYM_EOF:	
		case SYM_UNKNOWN:
			break;

		case SYM_TEXT:
		case SYM_VERBATIM:
		case SYM_NUMBER:

		case SYM_SQUOTE:
			//
			//	value
			//
			match(value.type_);
			return std::make_pair(key.value_, make_node_symbol(value));

		case SYM_DQUOTE:
			//
			//	"quote" vector
			//
			return std::make_pair(key.value_, generate_quote());

		case SYM_TOKEN:
			//
			//	function
			//
			if (lookup::is_standalone(key.value_)) {
				print_error(cyng::logging::severity::LEVEL_ERROR, "standalone function as parameter");
			}
			return std::make_pair(key.value_, generate_function(depth + 1));

		case SYM_SEP:
			print_error(cyng::logging::severity::LEVEL_ERROR, "missing value");
			match(SYM_SEP);
			break;

		case SYM_PAR:
			print_error(cyng::logging::severity::LEVEL_ERROR, "paragraph is not a valid parameter");
			match(SYM_PAR);
			break;

		case SYM_KEY:
			print_error(cyng::logging::severity::LEVEL_ERROR, "nested \"key:\" not allowed");
			match(SYM_KEY);
			break;
		case SYM_OPEN:
			return std::make_pair(key.value_, generate_content(depth + 1));
			break;
		case SYM_CLOSE:
			print_error(cyng::logging::severity::LEVEL_ERROR, "\")\" is not a valid parameter");
			match(SYM_CLOSE);
			break;
		case SYM_BEGIN:
			return std::make_pair(key.value_, generate_vector(depth));
		case SYM_END:
			print_error(cyng::logging::severity::LEVEL_ERROR, "\"]\" is not a valid parameter");
			match(SYM_END);
			break;
		default:
			break;
		}

		//
		//	empty node
		//
		match(value.type_);
		return std::make_pair(key.value_, make_node());
	}

	node parser::generate_quote()
	{
		match(SYM_DQUOTE);

		node::s_args args;

		while (!producer_.get().is_type(SYM_DQUOTE) && !producer_.is_eof()) {
			//
			//	no functions allowed / ignored
			//
			if (producer_.get().is_type(SYM_TOKEN)) {
				print_error(cyng::logging::severity::LEVEL_WARNING, "function will be ignored");
			}
			args.push_back(producer_.get());
			match();
		}

		match(SYM_DQUOTE);
		std::reverse(args.begin(), args.end());	//	preserve original ordering for generator
		return make_node_list(std::move(args));
	}

	bool parser::match(symbol_type st)
	{
		if (producer_.get().type_ != st) {
			std::stringstream ss;
			ss
				<< "wrong lookahead - "
				<< name(st)
				<< " expected"
				;

			print_error(cyng::logging::severity::LEVEL_WARNING, ss.str());
			producer_.next();
			return false;
		}

		producer_.next();
		return true;
	}

	void parser::match()
	{
		match(producer_.get().type_);
	}

	void parser::print_error(cyng::logging::severity level, std::string msg)
	{
		std::cout
			<< "***"
			<< cyng::logging::to_string(level)
			<< ": "
			<< msg
			<< " <"
			<< producer_.get()
			<< " > - #"
			<< std::setfill('0')
			<< std::setw(4)
			<< producer_.get_current_line()
			<< '@'
			<< producer_.get_current_file()
			<< std::endl
			;
	}

}
