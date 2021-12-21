/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 Sylko Olzscher
 *
 */

#include <tex/tex.hpp>
#include <boost/algorithm/string.hpp>

namespace tex
{

	int class_structure_offset(std::string const& type) {

		//	-1	part
		//	 0	chapter
		//	 1	section
		//	 2	subsection
		//	 3	subsubsection
		//	 4	paragraph
		//	 5	subparagraph
		if (boost::algorithm::equals(type, "article")
			|| boost::algorithm::equals(type, "beamer")
			|| boost::algorithm::equals(type, "scrartcl")) return 1;

		if (boost::algorithm::equals(type, "report")
			|| boost::algorithm::equals(type, "book")
			|| boost::algorithm::equals(type, "scrbook")
			|| boost::algorithm::equals(type, "scrreprt"))	return -1;

		return 0;
	}

}
