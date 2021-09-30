#include <parser.h>
#include <context.h>
#include <ast/root.h>

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
        , prg_(ctx)
    {
        state_.push(nonterminal_type::BODY);
    }

    bool parser::put(symbol const& sym)
    {
        std::cout << ctx_.get_position() << ": " << sym << " " << state_ << std::endl;

        BOOST_ASSERT_MSG(!state_.empty(), "state stack is empty");

        switch (state_.top().nttype_) {
        default:
        case nonterminal_type::BODY:
            return state_body(sym);
        case nonterminal_type::LIST:
            return state_list(sym);
        case nonterminal_type::TERM:
            return state_term(sym);
        case nonterminal_type::VECTOR:
            return state_vector(sym);
        case nonterminal_type::PARAMS:
            return state_params(sym);
        case nonterminal_type::MAP:
            return state_map(sym);
        case nonterminal_type::TAIL:
            return state_tail(sym);
        case nonterminal_type::VALUE:
            return state_value(sym);
        case nonterminal_type::SVM:
            return state_svm(sym);
        case nonterminal_type::TERMINAL:
            return state_terminal(sym);
        }

        return true;
    }

    bool parser::state_body(symbol const& sym) {
        BOOST_ASSERT(state_.top().nttype_ == nonterminal_type::BODY);

        state_.pop();
        state_.push(nonterminal_type::LIST);
        return false;   // still nonterminal
    }

    bool parser::state_list(symbol const& sym) {
        BOOST_ASSERT(state_.top().nttype_ == nonterminal_type::LIST);

        if (sym.equals(symbol_type::EOD)) {
            //
            //  complete
            //
            eod();
            return true;
        }
        state_.push(nonterminal_type::TERM);
        return false;   // still nonterminal
    }

    bool parser::state_term(symbol const& sym) {
        BOOST_ASSERT(state_.top().nttype_ == nonterminal_type::TERM);

        state_.pop();

        switch (sym.type_) {
        case symbol_type::FUN:
            state_.push({ sym, nonterminal_type::PARAMS });
            prg_.init_function(sym.value_);
            break;
        case symbol_type::PAR:
            state_.push(nonterminal_type::VECTOR);
            prg_.init_paragraph(sym.value_);
            break;
        default:
            //  error
            break;
        }
        return true;    //  advance
    }

    bool parser::assess_method_type(std::string const& name) {
        //
        //  lookup function table to determine if the function
        //  expects a VECTOR or a MAP.
        //
        auto const om = ctx_.lookup_method(name);
        if (!om.has_value()) {
            fmt::print(
                stdout,
                fg(fmt::color::orange) | fmt::emphasis::bold,
                "{}: warning: undefined method [{}]\n", ctx_.get_position(), name);
            return false; //  VECTOR
        }
        return om->is_parameter_type(parameter_type::MAP);
    }

    bool parser::is_display_inline(std::string const& name) {
        auto const om = ctx_.lookup_method(name);
        return (om)
            ? om->is_inline()
            : true;
    }


    bool parser::state_vector(symbol const& sym) {
        BOOST_ASSERT(state_.top().nttype_ == nonterminal_type::VECTOR);
        state_.pop();

        if ((state_.top().nttype_ == nonterminal_type::TERMINAL) && (sym == state_.top().sym_)) {
            state_.pop();
            fmt::print(
                stdout,
                fg(fmt::color::dim_gray),
                "{}: vector method complete\n", ctx_.get_position());
            //
            //  merge
            //
            prg_.merge();
            return true;
        }

        switch (sym.type_) {
        case symbol_type::EOD:
            //
            //  complete
            //
            eod();
            return true;

        case symbol_type::PAR:
            fmt::print(
                stdout,
                fg(fmt::color::dim_gray),
                "{}: vector complete\n", ctx_.get_position());
            return false;
        case symbol_type::FUN:
            if (!is_display_inline(sym.value_)) {
                //
                //  start new block
                //
                prg_.merge();
                return false;
            }

            state_.push({ sym, nonterminal_type::PARAMS });
            prg_.init_function(sym.value_);
            return true;
        case symbol_type::DQU:
            state_.push(nonterminal_type::VECTOR);
            state_.push(nonterminal_type::VALUE);
            state_.push(sym);
            state_.push({ sym, nonterminal_type::VECTOR });
            state_.push(nonterminal_type::VALUE);
            prg_.init_function("cite");
            return true;
        default:
            state_.push(nonterminal_type::VECTOR);
            state_.push(nonterminal_type::VALUE);
            break;
        }
        return false;   //  nonterminal
    }

    bool parser::state_map(symbol const& sym) {
        BOOST_ASSERT(state_.top().nttype_ == nonterminal_type::MAP);
        state_.pop();
        switch (sym.type_) {
        case symbol_type::TXT:
            state_.push(nonterminal_type::TAIL);
            state_.push(nonterminal_type::VALUE);
            state_.push(make_symbol(symbol_type::SYM, std::string(1, ':')));
            fmt::print(
                stdout,
                fg(fmt::color::dim_gray),
                "{}: generate parameter [{}:...] \n", ctx_.get_position(), sym.value_);
            prg_.init_param(sym);
            return true;    //  take key name and advance
        default:
            fmt::print(
                stdout,
                fg(fmt::color::crimson) | fmt::emphasis::bold,
                "{}: error: key/value pair expected but found [{}]\n", ctx_.get_position(), sym.value_);
            break;
        }
        return true;
    }

    bool parser::state_tail(symbol const& sym) {
        BOOST_ASSERT(state_.top().nttype_ == nonterminal_type::TAIL);
        state_.pop();
        if (sym.equals(symbol_type::SYM, ',')) {
            state_.push(nonterminal_type::MAP);
            return true;
        }


        //
        //  end of map: function is complete
        //
        prg_.merge();
        return false;
    }

    bool parser::state_value(symbol const& sym) {
        BOOST_ASSERT(state_.top().nttype_ == nonterminal_type::VALUE);
        state_.pop();
        switch (sym.type_) {
        case symbol_type::TXT:
        case symbol_type::TST:
            //
            //  complete
            //
            switch (state_.top().nttype_) {
            case nonterminal_type::TAIL: 
                //
                //  expect a param on top of the AST
                //
                prg_.finalize_param(sym);
                break;
            default:
                fmt::print(
                    stdout,
                    fg(fmt::color::dim_gray),
                    "{}: advance value [{}] \n", ctx_.get_position(), sym.value_);
                prg_.append(sym);
                //
                //  single value method (SVM)?
                //
                return state_.top().nttype_ != nonterminal_type::SVM;
            }
            break;
        case symbol_type::DQU:
            state_.push(nonterminal(make_symbol(symbol_type::DQU, std::string(1, '"'))));
            state_.push(nonterminal_type::VECTOR);
            break;
         case symbol_type::SYM:
            fmt::print(
                stdout,
                fg(fmt::color::dim_gray),
                "{}: advance [{}] \n", ctx_.get_position(), sym.value_);
            break;
        case symbol_type::FUN:
            //
            //  test for inline/block function
            //
            if (!is_display_inline(sym.value_)) {
                //
                //  start new block
                //
                state_.pop();
                prg_.merge();
                return false;
            }

            state_.push({ sym, nonterminal_type::PARAMS });
            prg_.init_function(sym.value_);

            return true;   //  advance
        default:
            fmt::print(
                stdout,
                fg(fmt::color::crimson) | fmt::emphasis::bold,
                "{}: error: value expected but found [{}]\n", ctx_.get_position(), sym.value_);
            break;
        }
        return true;    //  advance
    }

    bool parser::state_svm(symbol const& sym) {
        state_.pop();
        fmt::print(
            stdout,
            fg(fmt::color::dim_gray),
            "{}: svm is complete: {}\n", ctx_.get_position(), sym.value_);
        prg_.merge();
        return true;
    }

    bool parser::state_params(symbol const& sym) {
        BOOST_ASSERT(state_.top().nttype_ == nonterminal_type::PARAMS);
        auto const name = state_.top().sym_.value_;
        bool const is_map = assess_method_type(name);
        state_.pop();
        if (sym.equals(symbol_type::SYM, '(')) {
            state_.push(nonterminal(make_symbol(symbol_type::SYM, std::string(1, ')'))));
            state_.push(is_map ? nonterminal_type::MAP : nonterminal_type::VECTOR);
            return true;    //  advance
        }
        if (is_map) {
            //  error
            fmt::print(
                stdout,
                fg(fmt::color::crimson) | fmt::emphasis::bold,
                "{}: error: method [{}] is not a single value method (SVM)\n", ctx_.get_position(), name);

        }
        state_.push(nonterminal_type::VECTOR);
        state_.push(nonterminal_type::SVM);  //  single value method (SVM)
        state_.push(nonterminal_type::VALUE); 
        return false;
    }

    bool parser::state_terminal(symbol const& sym) {
        if (sym == state_.top().sym_) {
            state_.pop();
            if (state_.top().nttype_ == nonterminal_type::LIST) {
                //
                //  TERM is complete
                //
                auto const size = prg_.merge();
                fmt::print(
                    stdout,
                    fg(fmt::color::dim_gray),
                    "{}: term is complete - generated {} asts\n", ctx_.get_position(), size);
            }
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


    void parser::eod() {
        fmt::print(
            stdout,
            fg(fmt::color::green_yellow) | fmt::emphasis::bold,
            "***info : EOD\n", ctx_.get_position());

        //if (state_.top().type_ == symbol_type::PAR) {
        //    ctx_.emit("CALL ");
        //    ctx_.emit("\xc2\xb6");    //  pilgrow (¶)
        //    ctx_.emit("\n");
        //}
        //if (state_.top().type_ == symbol_type::FUN) {
        //    ctx_.emit("CALL ");
        //    ctx_.emit(state_.top().value_);
        //    ctx_.emit("\n");

        //    state_.pop();
        //}
        //ctx_.emit("HALT");
        //ctx_.emit("\n");

    }

    void parser::nt_stack::print(std::ostream& os) const {
        bool initialized = false;
        for (auto const& e : this->c) {
            if (initialized) {
                os << ',';
            }
            else {
                initialized = true;
            }
            if (nonterminal_type::TERMINAL == e.nttype_) {
                os << e.sym_;
            }
            else {
                os << e.nttype_;
            }
        }
    }

    std::ostream& operator<<(std::ostream& os, const parser::nt_stack& s) {
        os << '[';
        s.print(os);
        os << "$]";
        return os;
    }
}