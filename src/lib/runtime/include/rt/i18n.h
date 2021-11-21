/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 Sylko Olzscher
 *
 */
#ifndef DOCC_RUNTIME_I18N_H
#define DOCC_RUNTIME_I18N_H

#include <cyng/io/iso_639_1.h>

#include <string>
#include <cstdio>
#include <fstream>
#include <utility>
#include <filesystem>

namespace docruntime
{
	namespace i18n {
		enum word_id {
			WID_FIGURE,
			WID_TABLE,
			WID_TOC,
		};
	}

	std::string get_name(cyng::io::language_codes lc, i18n::word_id id);

}

#endif
