/*
 * The MIT License (MIT)
 * 
 * Copyright (c) 2019 Sylko Olzscher 
 * 
 */ 

#ifndef DOCSCRIPT_NODE_H
#define DOCSCRIPT_NODE_H

#include <docscript/symbol.h>
#include <memory>
#include <map>
#include <deque>
#include <any>

namespace docscript
{
	/**
	 * Parsing a docscript document produces an AST that
	 * consists of nodes.
	 */
//	namespace impl {
//		class node;
//	}

	class node
	{
	public:
		using p_args = std::map<std::string, docscript::node>;
		using v_args = std::vector<docscript::node>;
		using d_args = std::deque<docscript::node>;
		using s_args = std::vector<symbol>;

		friend node make_node();	//!< empty node
		friend node make_node_root();
		friend node make_node_symbol(symbol sym);
		friend node make_node_list(std::vector<symbol>&& list);
		friend node make_node_function_par(std::string name);
		friend node make_node_function_vec(std::string name);
		friend node make_node_paragraph(d_args&&);
		friend node make_node_content(d_args&&);
		friend node make_node_vector(v_args&&);

		friend p_args* access_function_params(node& n);
		friend p_args const* access_function_params(node const& n);
		friend std::string get_function_par_name(node const&);

		friend v_args* access_function_vector(node& n);
		friend v_args const* access_function_vector(node const& n);
		friend std::string get_function_vec_name(node const&);

		friend d_args* access_node_root(node& n);
		friend d_args const* access_node_root(node const& n);
		friend d_args const* access_node_paragraph(node const& n);
		friend d_args const* access_node_content(node const& n);
		friend symbol const* access_node_symbol(node const&);
		friend s_args const* access_node_list(node const&);
		friend v_args const* access_node_vector(node const&);

	public:
		enum type {
			NODE_ROOT,
			NODE_EMPTY,
			NODE_SYMBOL,
			NODE_LIST,
			NODE_META,
			NODE_FUNCTION_PAR,
			NODE_FUNCTION_VEC,
			NODE_PARAGRAPH,
			NODE_CONTENT,
			NODE_VECTOR,
		};

	public:
		node();
		node(type);
		node(node const&);
		node(node&&); // = default;

		node(std::deque<docscript::node>&& children);
		node(symbol sym);
		node(std::vector<symbol>&& list);
		node(std::string name, docscript::node::p_args&& args);
		node(std::string name, docscript::node::v_args&& args);
		node(bool b, docscript::node::d_args&& args);
		node(docscript::node::v_args&& args);

		virtual ~node();
		node& operator=(node&&); // = default;

		//
		//	obvious functions
		//
		type get_type() const;


	private:
		docscript::node::type const type_;
		std::string const name_;
		std::any data_;
	};

	//
	//	factories
	//
	node make_node();
	node make_node_root();
	node make_node_symbol(symbol);
	node make_node_list(node::s_args&& list);
	node make_node_function_par(std::string name);
	node make_node_function_vec(std::string name);
	node make_node_paragraph(node::d_args&&);
	node make_node_content(node::d_args&&);
	node make_node_vector(node::v_args&&);

	//
	//	node access
	//
	node::p_args* access_function_params(node&);
	node::p_args const* access_function_params(node const&);
	std::string get_function_par_name(node const&);

	node::v_args* access_function_vector(node&);
	node::v_args const* access_function_vector(node const&);
	std::string get_function_vec_name(node const&);

	node::d_args* access_node_root(node& n);
	node::d_args const* access_node_root(node const&);
	symbol const* access_node_symbol(node const&);
	node::s_args const* access_node_list(node const&);
	node::d_args const* access_node_paragraph(node const&);
	node::d_args const* access_node_content(node const&);
	node::v_args const* access_node_vector(node const&);

}

#endif
