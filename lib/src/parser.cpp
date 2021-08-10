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
        state_.push(parameter(state::INITIAL_));
    }

    void parser::put(symbol const& sym)
    {
        std::cout << ctx_.get_position() << ": " << sym << std::endl;

        switch (state_.top().state_) {
        case state::INITIAL_:
            state_initial(sym);
            break;
        case state::FUNCTION_:
            state_function(sym);
            break;
        case state::OPEN_:
            state_open(sym);
            break;
        case state::PARAGRAPH_:
            state_paragraph(sym);
            break;

        default:
            break;
        }

        //
        //  update last symbol
        //
        new (&prev_) symbol(sym);

    }

    void parser::state_initial(symbol const& sym) {
        switch (sym.type_) {
           
        case symbol_type::EOD:	//	end-of-data
            break;
        case symbol_type::FUN:
            state_.push(parameter(state::FUNCTION_, sym));
            break;
        case symbol_type::PAR:	//	paragraph - multiple NL
            state_.push(parameter(state::PARAGRAPH_, make_symbol(symbol_type::FUN, "par")));
            break;

        //    TXT,	//	text
        //    SYM,	//	special symbol like "(", ")", ",", ":"
        //    TST,	//	timestamp
        //    DQU,	//	double quotes: "

        default:
            ctx_.emit("emit ");
            ctx_.emit(sym.value_);
            ctx_.emit("\n");
            break;
        }
    }

    void parser::state_function(symbol const& sym) {
        switch (sym.type_) {
        case symbol_type::FUN:
            state_.push(parameter(state::FUNCTION_, sym));
            break;
        case symbol_type::SYM:
            BOOST_ASSERT(sym.value_.size() == 1);
            if (sym.value_.size() == 1) {

                switch (sym.value_.front()) {
                case '(':
                    //
                    //  an opening bracket requires an closing bracket
                    //
                    state_.push(parameter(state::OPEN_, state_.top().symbols_.front()));
                    break;
                default:
                    //
                    //  "one parameter" function is complete
                    // 
                    emit_function();
                    break;
                }
            }
            break;

        default:
            if (state_.top().symbols_.size() == 1) {
                state_.top().symbols_.push_back(sym);
                //
                //  "one parameter" function is complete
                // 
                emit_function();
            }
            break;
        }
    }

    void parser::state_open(symbol const& sym) {
        switch (sym.type_) {
        case symbol_type::FUN:
            state_.push(parameter(state::FUNCTION_, sym));
            break;
        case symbol_type::SYM:
            BOOST_ASSERT(sym.value_.size() == 1);
            if (sym.value_.size() == 1) {

                switch (sym.value_.front()) {
                case ')':
                    //
                    //  complete: emit instructions
                    //
                    emit_function();
                    state_.pop();   //  function
                    break;
                case ':':
                case ',':
                    break;
                default:
                    fmt::print(
                        stdout,
                        fg(fmt::color::dark_orange) | fmt::emphasis::bold,
                        "***warn : unexpected symbol \"{}\" at {}\n", sym.value_, ctx_.get_position());
                    break;
                }
            }
            break;
        default:
            state_.top().symbols_.push_back(sym);
            break;
        }
    }

    void parser::state_paragraph(symbol const& sym) {
        switch (sym.type_) {
        case symbol_type::FUN:
            state_.push(parameter(state::FUNCTION_, sym));
            break;
        case symbol_type::PAR:
            //
            //  complete: emit instructions
            //
            emit_function();
            break;
        default:
            state_.top().symbols_.push_back(sym);
            break;
        }
    }

    void parser::emit_function() {
        for (auto pos = std::crbegin(state_.top().symbols_); pos != std::crend(state_.top().symbols_); pos++) {
            switch (pos->type_) {
            case symbol_type::FUN:
                ctx_.emit("call ");
                ctx_.emit(pos->value_);
                ctx_.emit("\n");
                break;
            default:
                ctx_.emit("push ");
                ctx_.emit(pos->value_);
                ctx_.emit("\n");
                break;
            }
        }
        state_.pop();
    }

    parser::parameter::parameter(state s) 
        : state_(s)
        , symbols_()
    { }

    parser::parameter::parameter(state s, symbol const& sym)
        : state_(s)
        , symbols_(1, sym)
    { }

}