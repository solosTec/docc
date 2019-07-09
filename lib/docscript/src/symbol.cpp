/*
 * The MIT License (MIT)
 * 
 * Copyright (c) 2019 Sylko Olzscher 
 * 
 */ 

#include <docscript/symbol.h>
#include <iomanip>
#include <boost/regex/pending/unicode_iterator.hpp>
#include <boost/algorithm/string.hpp>

namespace docscript
{
	symbol::symbol(symbol_type t, std::string const& s)
		: type_(t)
		, value_(s)
	{}

	symbol::symbol(symbol_type t, std::u32string const& s)
		: type_(t)
		, value_(boost::u32_to_u8_iterator<std::u32string::const_iterator>(s.begin()), boost::u32_to_u8_iterator<std::u32string::const_iterator>(s.end()))
	{}

	symbol::symbol(symbol_type t, std::uint32_t c)
		: type_(t)
		, value_(1, c)
	{}

	symbol::symbol(symbol const& other)
		: type_(other.type_)
		, value_(other.value_)
	{}

	symbol::symbol(symbol&& other)
		: type_(other.type_)
		, value_(std::move(other.value_))
	{
		const_cast<symbol_type&>(other.type_) = SYM_UNKNOWN;
	}

	bool symbol::is_equal(std::string test) const
	{
		return boost::algorithm::equals(value_, test);
	}

	bool symbol::is_type(symbol_type st) const
	{
		return st == type_;
	}

	bool symbol::is_meta() const
	{
		return is_type(SYM_FILE) 
			|| is_type(SYM_LINE)
			|| is_type(SYM_EOF)
			;
	}

	void symbol::swap(symbol& other)
	{
		std::swap(const_cast<symbol_type&>(type_), const_cast<symbol_type&>(other.type_));
		std::swap(const_cast<std::string&>(value_), const_cast<std::string&>(other.value_));
	}


	void swap(symbol& s1, symbol& s2)
	{
		s1.swap(s2);
	}

	std::ostream& operator<<(std::ostream& os, const symbol& sym)
	{
		os
			<< '{'
			<< name(sym.type_)
			<< ' '
			;
		switch (sym.type_)
		{
		case SYM_TEXT:
			os
				<< '"'
				<< sym.value_
				<< '"'
				;
			break;

		default:
			os
				<< sym.value_
				;
			break;
		}
		os
			<< '}'
			;
		return os;
	}

	std::string name(symbol_type st)
	{
		switch (st)
		{
		case SYM_EOF:		return "EOF";
		case SYM_UNKNOWN:	return "???";
		case SYM_TEXT:		return "TXT";
		case SYM_VERBATIM:	return "LIT";	//	literal
		case SYM_NUMBER:	return "NUM";
		case SYM_DATETIME:	return "DAT";
		case SYM_TOKEN:		return "TOK";
		case SYM_PAR:		return "PAR";

		case SYM_DQUOTE:	return "DQT";	// double quote
		case SYM_SQUOTE:	return "SQT";	// single quote
		case SYM_OPEN:		return "OPN";	//	"("
		case SYM_CLOSE:		return "CLS";	//	")"
		case SYM_SEP:		return "SEP";	//	separator ","
		case SYM_KEY:		return "KEY";	//	":"
		case SYM_BEGIN:		return "BEG";	//	"["
		case SYM_END:		return "END";	//	"]"

		case SYM_FILE:		return "FIL";
		case SYM_LINE:		return "LIN";

		default:
			break;
		}
		return "ERR";

	}

	//
	//	helper class to read the linearized symbol input
	//
	symbol_reader::symbol_reader(symbol_list_t const& sl)
		: pos_(sl.begin())
		, look_ahead_(pos_)
		, end_(sl.end())
		, back_(SYM_EOF, "EOF")
		, current_file_()
		, current_line_(0)
	{
		//
		//	don't point to meta data
		//
		skip_meta();

		//
		//	position look ahead
		//
		adjust_look_ahead();
	}

	bool symbol_reader::next()
	{
		if (!is_eof()) {

			//
			//	proceed
			//
			++pos_;
			if (pos_ == end_)	return false;

			//
			//	don't point to meta data
			//
			skip_meta();

			//
			//	keep look ahead right
			//
			adjust_look_ahead();

			return true;
		}
		return false;
	}

	symbol const& symbol_reader::get() const
	{
//		std::cout << "producer => " << (is_eof() ? back_ : *pos_) << std::endl;
		return (is_eof())
			? back_
			: *pos_
			;
	}

	symbol const& symbol_reader::look_ahead() const
	{
		return (look_ahead_ != end_)
			? *look_ahead_
			: back_
			;
	}

	void symbol_reader::skip_meta()
	{
		while (pos_->is_meta()) {

			switch (pos_->type_)
			{
			case SYM_EOF:
				return;

			case SYM_FILE:
				current_file_ = pos_->value_;
				break;

			case SYM_LINE:
				current_line_ = std::stoull(pos_->value_);
				break;

			default:
				BOOST_ASSERT_MSG(false, "internal error");
				return;
			}

			//
			//	next symbol
			//
			++pos_;
			if (pos_ == end_)	break;
		}
	}

	void symbol_reader::adjust_look_ahead()
	{
		if (is_eof()) {
			look_ahead_ = end_;
		}
		else {

			look_ahead_ = pos_;

			do {
				++look_ahead_;
				if (look_ahead_ == end_)	break;
				if (look_ahead_->is_type(SYM_EOF))	break;

			} while (look_ahead_->is_meta());
		}
	}

	bool symbol_reader::is_eof() const
	{
		return pos_ == end_;
	}

	std::string const& symbol_reader::get_current_file() const
	{
		return current_file_;
	}

	std::size_t symbol_reader::get_current_line() const
	{
		return current_line_;
	}

}


