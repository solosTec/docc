#include <parser.h>
#include <context.h>

#ifdef _DEBUG
#include <iostream>
#endif

#include <fmt/core.h>
#include <fmt/color.h>

#include <boost/algorithm/string.hpp>

namespace docscript {

    parser::parser(context& ctx)
        : state_()
        , ctx_(ctx)
        , prev_(make_symbol())
    {
        //state_.push(parameter(state::INITIAL_));
    }

    void parser::put(symbol const& sym)
    {
        std::cout << ctx_.get_position() << ": " << sym << " $" << (state_.empty() ? make_symbol() : state_.top()) << std::endl;

        switch (sym.type_) {
        case symbol_type::EOD:	//	end-of-data
            eod();
            break;
        case symbol_type::FUN:	//	function name
        [[fallthrough]];    //  since C++17
        case symbol_type::PAR:	//	paragraph - multiple NL
            next_function(sym);
            break;
        case symbol_type::TXT:	//	text
            next_text(sym);
            break;
        case symbol_type::SYM:	//	special symbol like "(", ")", ",", ":"
            next_symbol(sym);
            break;
        case symbol_type::TST:	//	timestamp
            state_.push(sym);
            break;
        case symbol_type::DQU:	//	double quotes: "
            state_.push(sym);
            break;
        default:
            BOOST_ASSERT_MSG(false, "invalid symbol");
            break;
        }

        //
        //  update last symbol
        //
        new (&prev_) symbol(sym);

    }

    void parser::next_symbol(symbol const& sym) {
        BOOST_ASSERT_MSG(sym.value_.size() == 1, "symbol with wrong length");
        switch (sym.value_.front()) {
        case '(':
            state_.push(sym);
            //state_.push(make_symbol(0));    //  parameter count
            break;
        case ')':
            if (state_.top().equals(symbol_type::SYM) && (state_.top().value_.front() == '(')) {
                //  parameter list complete
                state_.pop();
                if (state_.top().equals(symbol_type::FUN)) {
                    //  generate function call
                    ctx_.emit("CALL ");
                    ctx_.emit(state_.top().value_);
                    ctx_.emit("\n");
                    state_.pop();
                }
            }
            else {
                state_.push(sym);
            }
            break;
        case ':':
            BOOST_ASSERT(!state_.empty());
            //  build key
            std::cout << ctx_.get_position() << ": reduce key [" << state_.top().value_ << "]" << std::endl;
            {
                auto const next_sym = make_symbol(symbol_type::KEY, std::string(sym.value_));
                state_.push(next_sym);
            }
            break;
        case ',':
            BOOST_ASSERT(!state_.empty());
            if (state_.top().equals(symbol_type::SYM) && (state_.top().value_.front() == '(')) {
                //  build parameter list
                std::cout << ctx_.get_position() << ": reduce list [" << state_.top().value_ << "]" << std::endl;
                //state_.pop();
            }
            else {
                state_.push(sym);
            }
            break;
        default:
            break;
        } 
    }

    void parser::next_function(symbol const& sym) {
        //
        //  generate a function call
        //
        state_.push(sym);
    }

    void parser::next_text(symbol const& sym) {
        //
        //  look at stack
        //
        if (state_.top().type_ == symbol_type::KEY) {
            std::cout << ctx_.get_position() << ": reduce list [" << state_.top().value_ << ":" << sym.value_ << "]" << std::endl;
            ctx_.emit("PUSH ");
            ctx_.emit(sym.value_);
            ctx_.emit("\n");

            state_.pop();

            ctx_.emit("PARAM");
            ctx_.emit("\n");
        }
        else if (state_.top().type_ == symbol_type::FUN) {
            //
            //  single parameter function
            //
            ctx_.emit("PUSH ");
            ctx_.emit(sym.value_);
            ctx_.emit("\n");

            ctx_.emit("CALL ");
            ctx_.emit(state_.top().value_);
            ctx_.emit(" #1\n");
            state_.pop();
        }
        else {
            ctx_.emit("PUSH ");
            ctx_.emit(sym.value_);
            ctx_.emit("\n");
        }
    }

    void parser::eod() {
        if (state_.top().type_ == symbol_type::PAR) {
            ctx_.emit("CALL ");
            ctx_.emit("\xc2\xb6");    //  pilgrow (¶)
            ctx_.emit("\n");
        }
        if (state_.top().type_ == symbol_type::FUN) {
            ctx_.emit("CALL ");
            ctx_.emit(state_.top().value_);
            ctx_.emit("\n");

            state_.pop();
        }
        ctx_.emit("HALT");
        ctx_.emit("\n");

    }

}