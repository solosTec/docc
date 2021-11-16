/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 Sylko Olzscher
 *
 */
#ifndef DOCC_RUNTIME_NUMBERING_H
#define DOCC_RUNTIME_NUMBERING_H

#include <cyng/obj/object.h>
#include <cyng/obj/intrinsics/container.h>

#include <memory>
#include <string>

#include <boost/uuid/uuid.hpp>

namespace docruntime
{

	/**
	 * Takes a vector with hierarchical information
	 * and generates the numbering.
	 */
	std::string get_numbering(std::vector<std::size_t> const&);

	/**
	 * element like table, figure, footnote with an UUID
	 * and text
	 */
	class element
	{
	public:
		element(boost::uuids::uuid, std::string);

		std::string get_tag() const;
		std::string const& get_text() const;

	private:
		boost::uuids::uuid const tag_;
		std::string const text_;
	};

	/**
	 * Simple list of footnotes
	 */
	using footnotes_t = std::list<element>;

	/**
	 * Simple list of figures
	 */
	using figures_t = std::list<element>;

	/**
	 * Simple list of tables
	 */
	using tables_t = std::list<element>;

	class toc
	{
	private:
		class entry : public element {
		public:
			entry(boost::uuids::uuid, std::string);

			std::shared_ptr <toc> sub_;	//	child
		};
	public:
		toc();
		toc(boost::uuids::uuid, std::string);

		std::pair<std::string, bool> add(std::size_t level, boost::uuids::uuid, std::string);

		friend cyng::vector_t to_vector(toc const&);
		friend cyng::vector_t to_vector(toc const&, std::vector<std::size_t>);

		/**
		 * Convert a tree of toc classes into a string
		 * like 3.2.5 ...
		 */
		friend std::ostream& operator<<(std::ostream& os, toc const&);
		friend void serialize(std::ostream& os, toc const&, std::vector<std::size_t>);

	private:
		std::pair<std::string, bool> add(std::size_t level, boost::uuids::uuid, std::string, std::vector<std::size_t>);

	private:
		std::list<entry>	headings_;
	};

}

#endif
