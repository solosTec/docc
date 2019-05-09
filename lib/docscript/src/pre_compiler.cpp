/*
 * The MIT License (MIT)
 * 
 * Copyright (c) 2019 Sylko Olzscher 
 * 
 */ 

#include <docscript/pre_compiler.h>
#include <cyng/util/split.h>
#include <boost/algorithm/string.hpp>

namespace docscript	
{
	pre_compiler::pre_compiler(std::string inp)
		: input_(inp)
	{}

	incl_t pre_compiler::parse_include() const
	{
		//
		//	example:
		//	.include (snippet.docscript:0:2)
		//

		boost::filesystem::path p;
		if (boost::algorithm::starts_with(input_, ".include")) {

			//
			//	get expression
			//
			auto const expr = input_.substr(8);

			//
			//	split expression
			//
			std::vector<std::string> const r = cyng::split(expr, "(:)");
			switch (r.size()) {
			case 3:
				return std::make_tuple(boost::filesystem::path(r.at(1))
					, 0
					, std::numeric_limits<std::size_t>::max());
			case 4:
				return std::make_tuple(boost::filesystem::path(r.at(1))
					, std::stoul(r.at(2))
					, std::numeric_limits<std::size_t>::max());
			case 5:
				return std::make_tuple(boost::filesystem::path(r.at(1))
					, std::stoul(r.at(2))
					, std::stoul(r.at(3)));
			default:
				break;
			}
		}
		return std::make_tuple(boost::filesystem::path(), 0u, 0u);
	}
}


