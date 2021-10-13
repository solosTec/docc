#include <ast/root.h>
#include <context.h>

#include <variant>
#include <chrono>

#include <fmt/core.h>
#include <fmt/color.h>

#include <boost/assert.hpp>

namespace docscript {
	namespace ast {


		program::program(context& ctx)
			: ctx_(ctx)
			, semantic_stack_()
			, decls_()
		{	}

		decl_t& program::top() {
			BOOST_ASSERT(!semantic_stack_.empty());
			return semantic_stack_.top();
		}

		void program::finalize_param(symbol const& sym) {

			BOOST_ASSERT_MSG(top().index() == 2, "param expected");
			append(std::get<2>(top()).finish(ast::value::factory(sym)));
		}

		void program::append(param&& p) {
			fmt::print(
				stdout,
				fg(fmt::color::dim_gray),
				"{}: generate parameter [{}] \n", ctx_.get_position(), p);

			semantic_stack_.pop();

			//
			//  append if top declaration is of type param
			//
			//BOOST_ASSERT_MSG(top().index() == 2, "param expected");
			switch (top().index()) {
			case 2:
				std::get<2>(top()).append(std::move(p));
				break;
			default:
				semantic_stack_.push(std::move(p));
				break;
			}

		}


		bool program::init_function(std::string const& name) {
			auto m = ctx_.lookup_method(name);
			if (m.has_value() && m->is_parameter_type(parameter_type::MAP)) {
				semantic_stack_.push(map_method::factory(name, m));
			}
			else {
				semantic_stack_.push(vec_method::factory(name, m));
			}
			return m.has_value();
		}

		void program::init_paragraph(std::string const& name) {
			BOOST_ASSERT(semantic_stack_.size() < 2);
			if (!semantic_stack_.empty()) {
				transfer_ast();
			}
			semantic_stack_.push(vec_method::factory(name, ctx_.lookup_method(name)));
		}

		std::size_t program::merge() {

			if (semantic_stack_.size() == 1) {
				//
				//	transfer to program 
				//
				transfer_ast();
			}
			else if (!semantic_stack_.empty()) {
				merge_ast();
				BOOST_ASSERT_MSG(!semantic_stack_.empty(), "semantic stack is empty");
			}
			return decls_.size();
		}

		void program::transfer_ast() {
			decls_.push_back(std::move(semantic_stack_.top()));
			semantic_stack_.pop();
		}

		void program::merge_ast() {

			switch (top().index()) {
			case 2:
				BOOST_ASSERT(semantic_stack_.size() > 1);
				//
				//	There is a parameter list on the stack that is complete.
				//	This list will be added to the associated function.
				//
				merge_ast_param();
				break;
			case 3:
				BOOST_ASSERT(semantic_stack_.size() > 1);
				//
				//  map method is complete.
				//
				merge_ast_map_method();
				break;
			case 4:
				//
				//  vec method is complete
				//	
				merge_ast_vec_method();
				break;
			default:
				break;
			}
		}


		void program::init_param(symbol const& sym) {
			semantic_stack_.push(param::factory(sym));
		}

		bool program::append(symbol const& sym) {
			if (top().index() == 4) {
				std::get<4>(top()).append(value::factory(sym));
				return true;
			}
			return false;
		}

		void program::merge_ast_param() {
			BOOST_ASSERT_MSG(top().index() == 2, "param expected");

			auto p = std::move(std::get<2>(top()));
			semantic_stack_.pop();

			switch (top().index()) {
			case 3: {
				decltype(auto) name = std::get<3>(top()).get_name();
				fmt::print(
					stdout,
					fg(fmt::color::dim_gray),
					"{}: map function \"{}\" is complete \n", ctx_.get_position(), name);
				std::get<3>(top()).set_params(std::move(p));
			}
				  break;
			default:
				BOOST_ASSERT_MSG(false, "map function expected");
				break;
			}

		}

		void program::merge_ast_value(value&& v) {

			switch (top().index()) {
			case 2: {
				//
				//	parameter list
				//
				auto p = std::get<2>(top()).finish(std::move(v));
				//
				//  append to parameter list
				semantic_stack_.pop();
				switch (top().index()) {
				case 2: // param
					//std::get<2>(top()).append(std::move(p));
					semantic_stack_.push(std::move(p));
					break;
				case 3:
					semantic_stack_.push(std::move(p));
					//std::get<3>(top()).set_params(std::move(p));
					break;
				default:
					BOOST_ASSERT_MSG(false, "param or map method expected");
					break;
				}
			}
				  break;
			case 4: {
				//
				//	vector method
				//
				std::get<4>(top()).append(std::move(v));
			}
				  break;
			default:
				BOOST_ASSERT_MSG(false, "cannot merge map method");
				break;
			}

		}

		void program::merge_ast_map_method() {
			BOOST_ASSERT_MSG(top().index() == 3, "map method expected");

			auto m = std::move(std::get<3>(top()));
			semantic_stack_.pop();

			merge_ast_value(value::factory(std::move(m)));

		}

		void program::merge_ast_vec_method() {
			BOOST_ASSERT_MSG(top().index() == 4, "vec method expected"); 

			auto m = std::move(std::get<4>(top()));
			semantic_stack_.pop();

			merge_ast_value(value::factory(std::move(m)));

		}

		void program::generate() {
			BOOST_ASSERT(semantic_stack_.empty());
			for (auto const& decl : decls_) {
				std::visit([&](auto const& arg) {
					arg.compile([&](std::string const& s){
						ctx_.emit(s);
						});
					}, decl);
			}
		}

	}
}
