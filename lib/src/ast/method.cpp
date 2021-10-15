#include <ast/method.h>
#include <ast/vlist.h>
//#include <context.h>

#include <boost/assert.hpp>

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

		void map_method::compile(std::function<void(std::string const&)> emit) const {
			//std::cout << "map_method::compile()" << std::endl;
			if (params_) {
				auto const count = params_->compile(emit);
				emit("push ");
				emit(std::to_string(count));
				emit("\t; parameter(s)\n");
				emit("make_param_map\n");
			}
			emit("push 1\n");	//	make tuple with one parameter map
			emit("invoke_r ");
			emit(this->get_name());
			emit("\n");
			emit("\n");
		}

		void map_method::set_params(param && p) {
			params_ = std::make_unique<param>(std::move(p));
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
		void vec_method::compile(std::function<void(std::string const&)> emit) const {
			//std::cout << "vec_method::compile()" << std::endl;
			emit("esba");
			emit("\t; ");
			emit(this->get_name());
			emit("\n");
			if (vlist_) {
				vlist_->compile(emit);
			}
			emit("frm\n");
			emit("make_vector\n");
			emit("push 1\n");	//	make tuple with one vector
			emit("invoke_r ");
			emit(this->get_name());
			emit("\n");
			emit("pull\n");
			emit("\n");
		}

		void vec_method::append(value && v) {
			if (vlist_) {
				vlist_->append(std::move(v));
			}
			else {
				vlist_ = std::make_unique<vlist>(std::move(v));
			}
		}

		vec_method::vec_method(std::string const& name, std::optional<docscript::method> optm)
			: method_base(name, optm)
			, vlist_()
		{}
		vec_method::vec_method(vec_method && vecm) noexcept
			: method_base(vecm.name_, vecm.method_)
			, vlist_(std::move(vecm.vlist_))
		{}

		vec_method::~vec_method() = default;


		vec_method vec_method::factory(std::string const& name, std::optional<docscript::method> optm) {
			return vec_method(name, optm);
		}

	}
}
