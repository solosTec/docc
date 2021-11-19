#include <docc/ast/method.h>
#include <docc/ast/params.h>
#include <docc/context.h>

#include <boost/assert.hpp>
#include <boost/algorithm/string.hpp>

namespace docscript {
	namespace ast {
		//
		//	----------------------------------------------------------*
		//	-- method_base
		//	----------------------------------------------------------*
		//
		method_base::method_base(std::string const& name, std::optional<docscript::method> optm)
			: name_(name)
			, method_(optm)
		{}

		std::string const& method_base::get_name() const {
			return name_;
		}

		void method_base::rename(docscript::method m) {
			name_ = m.get_name();
			method_.emplace(m);
		}

		//
		//	----------------------------------------------------------*
		//	-- map_method
		//	----------------------------------------------------------*
		//
		map_method::map_method(std::string const& name, std::optional<docscript::method> optm)
			: method_base(name, optm)
			, params_(nullptr)
		{}
		map_method::map_method(map_method&& mapm) noexcept
			: method_base(mapm.name_, mapm.method_)
			, params_(std::move(mapm.params_))
		{}

		map_method::~map_method() = default;

		void map_method::compile(std::function<void(std::string const&)> emit, std::size_t depth, std::size_t index) const {
			//std::cout << "map_method::compile()" << std::endl;
			if (params_) {
				auto const count = params_->compile(emit, depth + 1, index);
				emit("push ");
				emit(std::to_string(count));
				emit("\t; parameter(s)\n");
				emit("make_param_map\n");
			}
			else {
				//
				//	push an empty parameter map
				//
				emit("push 0\t; no parameters\n");
				emit("make_param_map\n");
			}
			emit("push 1\t; tuple size is always one\n");	//	make tuple with one parameter map
			emit("invoke_r ");
			emit(this->get_name());
			emit("\n");
			emit("\n");
		}

		void map_method::transform(context const& ctx) {
			transform_quote(ctx);
		}

		void map_method::transform_quote(context const& ctx) {
			//
			//	replace quote() calls on values with range() calls
			//
			if (params_) {
				params_->transform(ctx);
			}
		}

		void map_method::set_params(param&& p, std::string pos) {
			//
			//	set named parameter
			//
			BOOST_ASSERT_MSG(!params_, "parameters already set");
			params_ = std::make_unique<param>(std::move(p));

			//
			//	check parameter list
			//
			BOOST_ASSERT_MSG(method_.has_value(), "method is unknown");
			if (method_.has_value()) {
				//
				//	get the required and the defined parameters
				//
				auto& req = method_->get_param_names();
				auto const def = params_->get_param_names();
				for (auto const& name : req) {
					if (def.find(name) == def.end()) {
						fmt::print(
							stdout,
							fg(fmt::color::crimson) | fmt::emphasis::bold,
							"{}: error: missing parameter [{}] in method \"{}\"\n", pos, name, method_->get_name());

					}
				}
			}
		}

		std::size_t map_method::param_count() const {
			return (params_)
				? params_->size() + 1u
				: 0
				;
		}

		map_method map_method::factory(std::string const& name, std::optional<docscript::method> optm) {
			return map_method(name, optm);
		}

		//
		//	----------------------------------------------------------*
		//	-- vec_method
		//	----------------------------------------------------------*
		//
		void vec_method::compile(std::function<void(std::string const&)> emit, std::size_t depth, std::size_t index) const {

			if (boost::algorithm::equals(this->get_name(), std::string("\xc2\xb6")) && vlist_.empty()) {
				emit("; skip empty paragraph\n");	//	
			}
			else {
				emit("esba");
				emit("\t; ");
				emit(this->get_name());
				emit("\n");

				//
				//	generate assembler code
				//
				for (auto const& p : vlist_) {
					BOOST_ASSERT(!!p);
					if (p) (*p).compile(emit, depth + 1, index);
				}

				emit("frm\n");
				emit("make_vector\t; ");
				emit(std::to_string(vlist_.size()));
				emit("\n");
				emit("push 1\t; one vector\n");	//	make tuple with one vector
				emit("invoke_r ");
				emit(this->get_name());
				emit("\n");
				//
				//	range produces a vector that should be splitted
				//	
				//if (boost::algorithm::equals(this->get_name(), "range")) {
				//	emit("split\t; dissect vector\n");
				//	emit("pop\t; remove size info\n");
				//}
				emit("pull\n");
				emit("\n");

			}
		}

		void vec_method::transform(context const& ctx) {
			transform_concatenations(ctx);
		}

		void vec_method::transform_concatenations(context const& ctx) {
			//
			//	Find the trailing punctuation marks.
			//
			for (auto pos = vlist_.begin(); pos != vlist_.end(); ) {
				auto const r = (*pos)->is_constant_txt();
				auto const idx = std::distance(vlist_.begin(), pos);
				if (r.second && idx != 0) {
					if (boost::algorithm::equals(r.first, ".")
						|| boost::algorithm::equals(r.first, ",")
						|| boost::algorithm::equals(r.first, ";")
						|| boost::algorithm::equals(r.first, "?")
						|| boost::algorithm::equals(r.first, "!")) {
						//std::cout << get_name() << " CAT #" << idx << " - " << **(pos - 1) << r.first << std::endl;

						//
						//	substitute this by a cat() function
						//
						std::string const cat("cat");
						auto m = factory(cat, ctx.lookup_method(cat));
						m.append(std::move(**(pos - 1)));
						m.append(std::move(**(pos)));

						pos = vlist_.erase(pos - 1);
						pos = vlist_.erase(pos);
						pos = vlist_.emplace(pos, std::make_unique<value>(value::factory(std::move(m))));
					}
				}
				++pos;
			}
		}

		std::size_t vec_method::append(value&& v) {
			vlist_.push_back(std::make_unique<value>(std::move(v)));
			return vlist_.size();
		}

		std::size_t vec_method::param_count() const {
			return vlist_.size();
		}

		vec_method::vec_method(std::string const& name, std::optional<docscript::method> optm)
			: method_base(name, optm)
			, vlist_()
		{}
		vec_method::vec_method(vec_method&& vecm) noexcept
			: method_base(vecm.name_, vecm.method_)
			, vlist_(std::move(vecm.vlist_))
		{}

		vec_method::~vec_method() = default;


		vec_method vec_method::factory(std::string const& name, std::optional<docscript::method> optm) {
			return vec_method(name, optm);
		}

	}
}
