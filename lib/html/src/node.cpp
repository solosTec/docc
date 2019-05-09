/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Sylko Olzscher
 *
 */

#include <html/node.hpp>

namespace html
{
	std::string node::to_str() const
	{
		std::string res;

		//
		//	open tag
		//
		res.append(1, '<');

		if (attrs_.empty() && nodes_.empty()) {
			//	void 
			res.append(tag_.get());
			res.append(" />");
		}
		else {

			res.append(tag_.get());
			for (auto const &v : attrs_) {
				res.append(1, ' ');
				res.append(v());
			}

			if (nodes_.empty()) {
				res.append(" />");
			}
			else
			{
				res.append(1, '>');

				//
				//	content
				//
				for (auto const &v : nodes_) {
					res.append(v());
				}

				//
				//	closing tag
				//
				res.append("</");
				res.append(tag_.get());
				res.append(">");
			}
		}
		return res;
	}

	node& node::operator+=(node&& child)
	{
		//
		//	The function operator of node is overloaded
		//	so this function is recognized as invocable.
		//
		nodes_.emplace_back(child);
		return *this;
	}

}
