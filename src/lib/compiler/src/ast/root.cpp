#include <docc/ast/root.h>
#include <docc/context.h>

#include <variant>
#include <chrono>

#include <fmt/core.h>
#include <fmt/color.h>

#include <boost/assert.hpp>
#include <boost/algorithm/string.hpp>

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

				if (ctx_.get_verbosity(14)) {
					fmt::print(
						stdout,
						fg(fmt::color::dim_gray),
						"{}: init map method = [{}] \n", ctx_.get_position(), name);
				}

				semantic_stack_.push(map_method::factory(name, m));
			}
			else {
				if (ctx_.get_verbosity(14)) {
					fmt::print(
						stdout,
						fg(fmt::color::dim_gray),
						"{}: init vec method = [{}] \n", ctx_.get_position(), name);
				}
				semantic_stack_.push(vec_method::factory(name, m));
			}
			return m.has_value();
		}

		void program::init_paragraph(std::string const& name) {
			//BOOST_ASSERT(semantic_stack_.size() < 2);
			if (ctx_.get_verbosity(14)) {
				fmt::print(
					stdout,
					fg(fmt::color::dim_gray),
					"{}: start new paragraph - semantic stack size = [{}] \n", ctx_.get_position(), semantic_stack_.size());
			}

			while (!semantic_stack_.empty()) {
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
				auto const count = std::get<4>(top()).append(value::factory(sym));
				if (ctx_.get_verbosity(16)) {
					fmt::print(
						stdout,
						fg(fmt::color::dim_gray),
						"{}: add parameter {}#{} to the \"{}\" vec-method\n", ctx_.get_position(), sym.value_, count, std::get<4>(top()).get_name());
				}
				return true;
			}
			return false;
		}

		void program::merge_ast_param() {
			BOOST_ASSERT_MSG(top().index() == 2, "param expected");

			auto p = std::move(std::get<2>(top()));
			semantic_stack_.pop();

			switch (top().index()) {
			case 2: {
				//
				//	append to params
				//
				std::get<2>(top()).append(std::move(p));

				//
				//	finish map method
				//
				auto p = std::move(std::get<2>(top()));
				semantic_stack_.pop();
				BOOST_ASSERT_MSG(semantic_stack_.top().index() == 3, "map function expected");				
				if (ctx_.get_verbosity(12)) {

					decltype(auto) name = std::get<3>(top()).get_name();
					fmt::print(
						stdout,
						fg(fmt::color::light_gray),
						"{}: map-function \"{}\" is complete. {} parameters specified\n", ctx_.get_position(), name, p.get_param_names().size());

				}

				std::get<3>(top()).set_params(std::move(p), ctx_.get_position());

			}
				  break;
			case 3: {
				decltype(auto) name = std::get<3>(top()).get_name();
				if (ctx_.get_verbosity(12)) {
			
					fmt::print(
						stdout,
						fg(fmt::color::light_gray),
						"{}: map-function \"{}\" is complete. {} parameters specified\n", ctx_.get_position(), name, p.get_param_names().size());

				}

				//
				//	build-in checks
				//
				p.verify(ctx_, name);
				std::get<3>(top()).set_params(std::move(p), ctx_.get_position());

				//
				//	merge down if possible
				//
				if (semantic_stack_.size() > 1) {
					auto m = std::move(std::get<3>(top()));
					semantic_stack_.pop();
					merge_ast_value(value::factory(std::move(m)));
				}
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
				auto const count = std::get<4>(top()).append(std::move(v));
				if (ctx_.get_verbosity(16)) {
					fmt::print(
						stdout,
						fg(fmt::color::dim_gray),
						"{}: add parameter #{} to the \"{}\" method\n", ctx_.get_position(), count, std::get<4>(top()).get_name());
				}

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

			if (ctx_.get_verbosity(12)) {

				fmt::print(
					stdout,
					fg(fmt::color::light_gray),
					"{}: map function \"{}\" is complete. {} parameter(s) specified\n", ctx_.get_position(), m.get_name(), m.param_count());

			}

			merge_ast_value(value::factory(std::move(m)));

		}

		void program::merge_ast_vec_method() {
			BOOST_ASSERT_MSG(top().index() == 4, "vec method expected");

			auto m = std::move(std::get<4>(top()));
			auto const name = std::get<4>(top()).get_name();	//	function name
			semantic_stack_.pop();

			if (ctx_.get_verbosity(12)) {

				fmt::print(
					stdout,
					fg(fmt::color::light_gray),
					"{}: vec function \"{}\" is complete. {} parameter(s) specified\n", ctx_.get_position(), m.get_name(), m.param_count());

			}

			switch (top().index()) {
			case 2: {
				//
				//	parameter list
				//	MODIFIED!
				//
				auto p = std::get<2>(top()).finish(value::factory(std::move(m)));
				semantic_stack_.pop();
				if (top().index() == 2) {
					std::get<2>(top()).append(std::move(p));
				}
				else {
					semantic_stack_.push(std::move(p));
				}
			}
				  break;
			case 4: {
				//
				//	vector method
				//
				auto const count = std::get<4>(top()).append(value::factory(std::move(m)));
				if (ctx_.get_verbosity(16)) {
					fmt::print(
						stdout,
						fg(fmt::color::dim_gray),
						"{}: add parameter \"{}\"#{} to the \"{}\" method\n", ctx_.get_position(), name, count, std::get<4>(top()).get_name());
				}
			}
				  break;
			default:
				BOOST_ASSERT(false);
				break;
			}
		}

		void program::generate() {
			BOOST_ASSERT(semantic_stack_.empty());
			//
			//	transform
			//
			for (auto & decl : decls_) {
				std::visit([&](auto& arg) {
					arg.transform(ctx_);
					}, decl);
			}
			//
			//	generate assembler code
			//
			for (auto const& decl : decls_) {
				std::visit([&](auto const& arg) {
					arg.compile([&](std::string const& s){
						ctx_.emit(s);
						}, 0, 0);
					}, decl);
			}
		}

	}
}
