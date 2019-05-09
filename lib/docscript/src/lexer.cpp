/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Sylko Olzscher
 *
 */

#include <docscript/lexer.h>

namespace docscript
{
	lexer::lexer(emit_value_f f, std::function<void(cyng::logging::severity, std::string)> err)
		: emit_(f)
		, err_(err)
		, state_stack_()
	{
		state_stack_.push(STATE_START_);
	}

	void lexer::next(symbol sym)
	{
		switch (top()) {
		case STATE_START_:
			state_start(sym);
			break;
		case STATE_ARG_:
			state_arg(sym);
			break;
		case STATE_FUNCTION_:
			state_function(sym);
			break;
		default:
			//
			//	unknown symbol type
			//
			//err_(cyng::logging::severity::LEVEL_FATAL, name(sym.type_));
			break;
		}
	}

	void lexer::state_start(symbol sym)
	{
		switch (sym.type_) {
		case SYM_EOF:
			break;
		case SYM_UNKNOWN:
			err_(cyng::logging::severity::LEVEL_ERROR, "unknown symbol");
			break;
		case SYM_ENTITY:
		case SYM_TEXT:
		case SYM_NUMBER:
			break;
		case SYM_TOKEN:
			emit(VAL_CALL, sym);
			save(STATE_ARG_);
			break;
		case SYM_PAR:
			emit(VAL_CALL, sym);
			save(STATE_FUNCTION_);
			break;
		default:
			//
			//	unknown symbol type
			//
			err_(cyng::logging::severity::LEVEL_FATAL, name(sym.type_));
			break;
		}
	}

	void lexer::state_arg(symbol sym)
	{
		switch (sym.type_) {

		case SYM_EOF:
			err_(cyng::logging::severity::LEVEL_ERROR, "incomplete function");
			break;

		case SYM_UNKNOWN:
			err_(cyng::logging::severity::LEVEL_ERROR, "unknown symbol");
			break;

		case SYM_TEXT:
			if (!sym.is_open()) {
				emit(VAL_OPEN, symbol(SYM_TEXT, '('));
				emit(VAL_PARAM, sym);
				emit(VAL_CLOSE, symbol(SYM_TEXT, ')'));
			}
			else {
				emit(VAL_OPEN, sym);
				substitute(STATE_FUNCTION_);
			}
			break;

		case SYM_ENTITY:
		case SYM_NUMBER:
			emit(VAL_OPEN, symbol(SYM_TEXT, '('));
			emit(VAL_PARAM, sym);
			emit(VAL_CLOSE, symbol(SYM_TEXT, ')'));
			break;

		case SYM_TOKEN:
			//emit(VAL_CALL, sym);
			//return STATE_ARG_;
			break;

		case SYM_PAR:
			err_(cyng::logging::severity::LEVEL_ERROR, "function expected");
			break;

		default:
			//
			//	unknown symbol type
			//
			err_(cyng::logging::severity::LEVEL_FATAL, name(sym.type_));
			break;
		}
	}

	void lexer::state_function(symbol sym)
	{
		switch (sym.type_) {

		case SYM_EOF:
			break;

		case SYM_UNKNOWN:
			err_(cyng::logging::severity::LEVEL_ERROR, "unknown symbol");
			break;

		case SYM_ENTITY:
		case SYM_TEXT:
		case SYM_NUMBER:
			break;
		case SYM_TOKEN:
			break;
		case SYM_PAR:
			break;
		default:
			//
			//	unknown symbol type
			//
			err_(cyng::logging::severity::LEVEL_FATAL, name(sym.type_));
			break;
		}
	}


	void lexer::emit(value_type vt, symbol sym)
	{
		emit_(value(vt, sym));
	}

	lexer::state lexer::save(state st)
	{
		state_stack_.push(st);
		return st;
	}

	lexer::state lexer::substitute(state st)
	{
		pop();
		return save(st);
	}

	lexer::state lexer::pop()
	{
		if (!state_stack_.empty()) {
			auto const st = top();
			state_stack_.pop();
			return st;
		}
		err_(cyng::logging::severity::LEVEL_FATAL, "stack is empty");
		return STATE_ERROR_;
	}

	lexer::state lexer::top() const
	{
		return (state_stack_.empty())
			? STATE_ERROR_
			: state_stack_.top()
			;
	}

	std::string get_state_name(lexer::state state)
	{
		switch (state) {
		case lexer::STATE_ERROR_:		return "ERROR";
		case lexer::STATE_START_:		return "START";
		case lexer::STATE_ARG_:			return "ARG";
		case lexer::STATE_FUNCTION_:	return "FUNCTION";
		default:
			break;
		}
		return "unknown lexer state";
	}

}
