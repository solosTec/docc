/*
 * The MIT License (MIT)
 * 
 * Copyright (c) 2019 Sylko Olzscher 
 * 
 */ 

#ifndef DOCSCRIPT_SYMBOL_H
#define DOCSCRIPT_SYMBOL_H

#include <iostream>
#include <string>
#include <functional>
#include <list>

namespace docscript
{
	enum symbol_type
	{
		SYM_EOF,		//!<	no more symbols
		SYM_UNKNOWN,	//!<	unknown or error state
		SYM_TEXT,		//!<	text and symbols
		SYM_VERBATIM,	//!<	keep spaces
		SYM_NUMBER,		//!<	number (sanitized)
		SYM_TOKEN,		//!<	functions at beginning of line are global
		SYM_PAR,		//!<	new paragraph

		SYM_DQUOTE,		//!<	'"' double quote
		SYM_SQUOTE,		//!<	' single quote
		SYM_OPEN,		//!<	'('
		SYM_CLOSE,		//!<	')'
		SYM_SEP,		//!<	','
		SYM_KEY,		//!<	':'
		SYM_BEGIN,		//!<	'['
		SYM_END,		//!<	']'
		//	'!'
		//	'?' 
		//	';' 
		//	'\''

		//
		//	support diagnostic and error message
		//
		SYM_FILE,	//!<	current source file
		SYM_LINE,	//!<	current line in source file
	};

	struct symbol
	{
		symbol(symbol_type, std::string const&);
		symbol(symbol_type, std::u32string const&);
#if defined(__CPP_SUPPORT_P0482R6)
		template <std::size_t N>
		symbol(symbol_type t, const char8_t(&sp)[N])
			: type_(t)
			, value_((const char*)&sp[0])
		{}
#else
		template <std::size_t N>
		symbol(symbol_type t, const char(&sp)[N])
			: type_(t)
			, value_(sp)
		{}
#endif
		explicit symbol(symbol_type, std::uint32_t);
		symbol(symbol const&);
		symbol(symbol&&);

		bool is_equal(std::string) const;
		bool is_type(symbol_type) const;
		bool is_meta() const;	//!<	SYM_FILE, SYM_LINE or SYM_EOF

		void swap(symbol&);

		symbol_type const type_;
		std::string const value_;
	};

	void swap(symbol& , symbol&);

	/**
	 * Define an emit function
	 */
	using emit_symbol_f = std::function<void(symbol&&)>;

	/**
	 * Streaming operator
	 */
	std::ostream& operator<<(std::ostream& os, const symbol& sym);

	std::string name(symbol_type);

	/**
	 *	helper class to read the linearized symbol input
	 */
	class symbol_reader
	{
	public:
		using symbol_list_t = std::list<symbol>;

	public:
		symbol_reader(symbol_list_t const& sl);

		/**
		 * produce next symbol
		 * @return false if no more symbols available
		 */
		bool next();
		symbol const& get() const;
		symbol const& look_ahead() const;

		bool is_eof() const;

		std::string const& get_current_file() const;
		std::size_t get_current_line() const;

	private:
		void adjust_look_ahead();
		void skip_meta();

	private:
		symbol_list_t::const_iterator pos_, look_ahead_, end_;
		const symbol back_;
		std::string current_file_;
		std::size_t current_line_;
	};

}

#endif
