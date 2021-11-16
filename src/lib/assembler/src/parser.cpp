#include <asm/parser.h>
#include <asm/context.h>

#include <fmt/core.h>
#include <fmt/color.h>

#include <boost/assert.hpp>
#include <boost/algorithm/string.hpp>

namespace docasm {

	parser::parser(context& ctx)
		: state_()
		, ctx_(ctx)
		, prg_(ctx)
	{
		state_.push(nonterminal_type::BODY);
	}

	bool parser::put(symbol const& sym)
	{
		//std::cout << ctx_.get_position() << ": " << sym << " " << state_ << std::endl;

		BOOST_ASSERT_MSG(!state_.empty(), "state stack is empty");

		switch (state_.top().nttype_) {
		case nonterminal_type::BODY:
			return state_body(sym);
		case nonterminal_type::PROGRAM:
			return state_program(sym);
		case nonterminal_type::LINE:
			return state_line(sym);
		case nonterminal_type::EOL:
			return state_eol(sym);
		case nonterminal_type::UUID:
			return state_uuid(sym);
		case nonterminal_type::IDENT:
			return state_ident(sym);
		case nonterminal_type::VALUE:
			return state_value(sym);
		case nonterminal_type::TERMINAL:
			return state_terminal(sym);
		default:
			BOOST_ASSERT_MSG(false, "internal error: unknown state");
			break;
		}
		return true;
	}

	bool parser::state_body(symbol const& sym) {
		BOOST_ASSERT(state_.top().nttype_ == nonterminal_type::BODY);

		state_.pop();
		state_.push(nonterminal_type::PROGRAM);
		return false;   // still nonterminal
	}

	bool parser::state_program(symbol const& sym) {
		BOOST_ASSERT(state_.top().nttype_ == nonterminal_type::PROGRAM);

		if (sym.equals(symbol_type::EOD)) {
			//
			//  complete
			//
			eod();
			return true;
		}
		state_.push(nonterminal_type::LINE);
		return false;   // still nonterminal
	}

	bool parser::state_line(symbol const& sym) {

		state_.pop();

		switch (sym.type_) {
		case symbol_type::DIR:
			state_.push(nonterminal_type::EOL);
			return true;
		case symbol_type::EOL:
			//state_.push(nonterminal_type::EOL);
			return true;	//	skip
		case symbol_type::LBL:
			state_.push(nonterminal_type::EOL);
			prg_.init_label(sym.value_);
			return true;
		case symbol_type::INS:
			statement(sym);
			return true;
		default:
			//  error
			fmt::print(
				stdout,
				fg(fmt::color::crimson) | fmt::emphasis::bold,
				"{}: error: label or instruction expected but found [{}]\n", ctx_.get_position(), sym.value_);
			break;
		}
		return true;
	}

	bool parser::state_eol(symbol const& sym) {
		state_.pop();
		if (sym.type_ == symbol_type::EOD) {
			//
			//  complete
			//
			prg_.finalize_statement();
			eod();
			return true;
		}
		if (sym.type_ != symbol_type::EOL) {
			fmt::print(
				stdout,
				fg(fmt::color::crimson) | fmt::emphasis::bold,
				"{}: error: EOL expected but found [{}]\n", ctx_.get_position(), sym.value_);

		}
		else {
			prg_.finalize_statement();
		}
		return true;
	}

	bool parser::state_uuid(symbol const& sym) {
		state_.pop();
		if (sym.type_ != symbol_type::LIT) {
			fmt::print(
				stdout,
				fg(fmt::color::crimson) | fmt::emphasis::bold,
				"{}: error: LIT expected but found [{}]\n", ctx_.get_position(), sym.value_);
		}

		//ctx_.emit(cyng::make_object(uuidgen_(sym.value_)));
		prg_.init_literal(sym.value_);

		state_.push(make_symbol(symbol_type::TYP, std::string("uuid")));
		return true;
	}

	bool parser::state_ident(symbol const& sym) {
		state_.pop();
		switch (sym.type_) {
		case symbol_type::TXT:
		case symbol_type::LIT:
			prg_.finalize_statement(sym);
			break;
		default:
			fmt::print(
				stdout,
				fg(fmt::color::crimson) | fmt::emphasis::bold,
				"{}: error: IDENTIFIER expected but found [{}]\n", ctx_.get_position(), sym.value_);
			break;
		}
		return true;
	}
	bool parser::state_value(symbol const& sym) {
		state_.pop();

		//ctx_.emit("\t");

		switch (sym.type_) {
		case symbol_type::TXT:
		case symbol_type::TST:
		case symbol_type::COL:
		case symbol_type::BOL:
		case symbol_type::NUM:
		case symbol_type::INT:
		case symbol_type::EXP:
			prg_.finalize_statement(sym);
			break;
		case symbol_type::LIT:
			state_.push(make_symbol(symbol_type::TYP, std::string("uuid")));
			break;
		default:
			fmt::print(
				stdout,
				fg(fmt::color::crimson) | fmt::emphasis::bold,
				"{}: error: [{}] is not a value\n", ctx_.get_position(), sym.value_);
			break;
		}
		return true;
	}

	bool parser::state_terminal(symbol const& sym) {
		//	same type and same value
		if (sym == state_.top().sym_) {
			state_.pop();
			prg_.finalize_statement(sym);
		}
		else {
			//  error
			fmt::print(
				stdout,
				fg(fmt::color::crimson) | fmt::emphasis::bold,
				"{}: error: expected [{}] but found [{}]\n", ctx_.get_position(), state_.top().sym_.value_, sym.value_);
		}
		return true;
	}

	void parser::statement(symbol const& sym) {

		auto const pos = ops_.find(sym.value_);
		if (pos != ops_.end()) {

			state_.push(nonterminal_type::EOL);

			switch (pos->second) {
			case cyng::op::PUSH:
				state_.push(nonterminal_type::VALUE);
				break;
			case cyng::op::INVOKE:
			case cyng::op::INVOKE_R:
				state_.push(nonterminal_type::IDENT);
				break;
			case cyng::op::FORWARD:
				state_.push(nonterminal_type::UUID);
				break;
			case cyng::op::JA:
			case cyng::op::JE:	//!< 	jump if error register is set
			case cyng::op::JNE:	//!< 	jump if error register is not set
				state_.push(nonterminal_type::IDENT);
				break;
			default:
				break;
			}
			prg_.init_op(pos->second);
		}
		else {
			fmt::print(
				stdout,
				fg(fmt::color::orange) | fmt::emphasis::bold,
				"{}: warning: unknown instruction [{}]\n", ctx_.get_position(), sym.value_);
		}
	}

	void parser::eod() {

		//
		//	resolve references and emit code
		// 
		auto const ll = prg_.build_label_list();
		prg_.generate(ll);

		//
		//  close programm
		//

		//fmt::print(
		//	stdout,
		//	fg(fmt::color::green_yellow) | fmt::emphasis::bold,
		//	"***info {}: EOD\n", ctx_.get_position());

	}

	const std::map<std::string, cyng::op> parser::ops_ = {

		{"NOW", cyng::op::NOW},
		{"PUSH", cyng::op::PUSH},
		{"PC", cyng::op::PC},	//!< 	push constant, mem[--sp] = x
		{"PR", cyng::op::PR},		//!< 	push relative, mem[--sp] = mem[bp + s]
		{"CORA", cyng::op::CORA},	//!<	convert rel addr, mem[--sp] = (bp + s)
		{"ASP", cyng::op::ASP}, 	//!< 	add to sp, sp = (sp + s)
		{"CALL", cyng::op::CALL},	//!< 	call, mem[--sp] = pc; pc = x

		{"JA", cyng::op::JA},		//!< 	jump always and absolute, pc = x
		// 			JCT = 7,	//!< 	jump count, if (--ct) pc = x
		// 			JP = 8,		//!< 	jump positive, if (mem[sp++] > 0) pc = x
		// 			JN = 9,		//!< 	jump negative, if (mem[sp++] < 0) pc = x
		// 			JZ = 0xA,	//!< 	jump zero, if (mem[sp++] == 0) pc = x
		// 			JNZ = 0xB,	//!< 	jump nonzero, if (mem[sp++] != 0) pc = x
		// 			JODD = 0xC,	//!< 	jump odd, if (mem[sp++] % 2 == 1) pc = x
		// 			JZON = 0xD,	//!< 	jump zero or neg, if (mem[sp++] <= 0) pc = x
		// 			JZOP = 0xE,	//!< 	jump zero or pos, if (mem[sp++] >= 0) pc = x
		{"JE", cyng::op::JE},		//!< 	jump if error register is set
		{"JNE", cyng::op::JNE},	//!< 	jump if error register is not set

		{"RET", cyng::op::RET},		//!< 	return, pc = mem[sp++]

		{"ESBA", cyng::op::ESBA},	//!< 	establish base address, mem[--sp] = bp; bp = sp;
		{"REBA", cyng::op::REBA},	//!< 	restore base address, sp = bp; bp = mem[sp++];
		{"PULL", cyng::op::PULL},	//!< 	restore base address but keep the content above
		{"FRM", cyng::op::FRM},		//!< 	push current frame size to stack

		{"ADD", cyng::op::ADD},	//!< 	add, temp = mem[sp++]; mem[sp] = mem[sp] + temp; cy = carry
		{"SUB", cyng::op::SUB},	//!< 	subtract, temp = mem[sp++]; mem[sp] = mem[sp] - temp

		{"INVOKE", cyng::op::INVOKE},		//!< 	call a library function
		{"INVOKE_R", cyng::op::INVOKE_R},			//!< 	call a library function
		{"FORWARD", cyng::op::FORWARD},			//!<	forward function call to other VM
		{"RESOLVE", cyng::op::RESOLVE},			//!<	substitute function name by id

		{"IDENT", cyng::op::IDENT},			//!< 	push VM tag onto stack (if available)
		{"NOW", cyng::op::NOW},			//!<	push current timestamp on stack
		{"PID", cyng::op::PID},			//!<	push current process id on stack
		{"TID", cyng::op::TID},			//!<	push current thread id on stack

		//	assembly
		{"MAKE_ATTR", cyng::op::MAKE_ATTR},			//!< 	build an attribute
		{"MAKE_PARAM", cyng::op::MAKE_PARAM},			//!< 	build a parameter
		{"MAKE_ATTR_MAP", cyng::op::MAKE_ATTR_MAP},		//!< 	build an attribute map
		{"MAKE_PARAM_MAP", cyng::op::MAKE_PARAM_MAP},		//!< 	build a parameter map
		{"MAKE_TUPLE", cyng::op::MAKE_TUPLE},			//!< 	build a tuple (std::list<object>)
		{"MAKE_VECTOR", cyng::op::MAKE_VECTOR},		//!< 	build a vector (std::vector<object>)
		{"MAKE_DEQUE", cyng::op::MAKE_DEQUE},			//!< 	build a deque (std::deque<object>)

														//	deassembly
		{"SPLIT", cyng::op::SPLIT},			//!<	push all elements of a container onto stack

		//	debugging
		//DUMP_DATA,		//!< 	dump all elements from data stack to standard output
		//DUMP_CODE,		//!< 	dump memory info
		//DBG_ON,			//!< 	switch debug mode on
		//DBG_OFF,		//!< 	switch debug mode off

		//	error register
		{"LERR", cyng::op::LERR},			//!< 	load error register
		{"TSTERR", cyng::op::TSTERR},			//!< 	test error register and set cmp/jump register (true if no error occured)
		{"RESERR", cyng::op::RESERR},			//!< 	reset error register

		//REMOVE,			//!<	remove an embedded VM
		{"TIDY", cyng::op::TIDY},			//!<	free unused memory 

		{"ASSERT_TYPE", cyng::op::ASSERT_TYPE},	//!<	asserts a type on stack (only for testing)
		{"ASSERT_VALUE", cyng::op::ASSERT_VALUE},	//!<	asserts a value on stack (only for testing)

		{"HALT", cyng::op::HALT}, //!< 	trigger halt
		{"NOOP", cyng::op::NOOP} //!< 	no operation

	};

	void parser::nt_stack::print(std::ostream& os) const {
		bool initialized = false;
		for (auto const& e : this->c) {
			if (initialized) {
				os << ',';
			}
			else {
				initialized = true;
			}
			os << e.nttype_;
		}
	}

	std::ostream& operator<<(std::ostream& os, const parser::nt_stack& s) {
		os << '[';
		os << '$';
		s.print(os);
		os << "]";
		return os;
	}


}
