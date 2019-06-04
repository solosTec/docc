/*
 * The MIT License (MIT)
 * 
 * Copyright (c) 2019 Sylko Olzscher 
 * 
 */ 

#include <docscript/node.h>

#include <cyng/object.h>
#include <cyng/intrinsics/sets.h>
#include <any>

#pragma pack(4)

namespace docscript
{
	node::node()
		: type_(docscript::node::NODE_EMPTY)
		, name_()
//		, name_(u8"∅")
		, data_()
	{}

	node::node(docscript::node::type t)
		: type_(t)
		, name_()
		, data_()
	{}

	node::node(node const& other)
		: type_(other.type_)
		, name_(other.name_)
		, data_(other.data_)
	{}

	node::node(node&& other)
		: type_(other.type_)
		, name_(std::move(other.name_))
		, data_()
	{
		data_.swap(other.data_);
	}

	node::node(std::deque<docscript::node>&& children)
		: type_(docscript::node::NODE_ROOT)
		, name_()
//		, name_(u8"↓")
		, data_(std::move(children))
	{}

	node::node(symbol sym)
		: type_(docscript::node::NODE_SYMBOL)
		, name_(sym.value_)
		, data_(sym)
	{}

	node::node(std::vector<symbol>&& list)
		: type_(docscript::node::NODE_LIST)
		, name_()
		, data_(std::move(list))
	{}

	node::node(std::string name, docscript::node::p_args&& args)
		: type_(docscript::node::NODE_FUNCTION_PAR)
		, name_(name)
		, data_(std::move(args))
	{}

	node::node(std::string name, docscript::node::v_args&& args)
		: type_(docscript::node::NODE_FUNCTION_VEC)
		, name_(name)
		, data_(std::move(args))
	{}

	node::node(bool b, docscript::node::d_args&& args)
		: type_(b ? docscript::node::NODE_PARAGRAPH : docscript::node::NODE_CONTENT)
		, name_()
//		, name_(b ? u8"¶" : u8"•")
		, data_(std::move(args))
	{}

	node::node(docscript::node::v_args&& args)
		: type_(docscript::node::NODE_VECTOR)
		, name_()
		, data_(std::move(args))
	{}

	node::~node()
	{}


	node& node::operator=(node && other)
	{
		const_cast<node::type&>(type_) = other.type_;
		const_cast<std::string&>(name_) = std::move(other.name_);
		data_.swap(other.data_);
		return *this;
	}

	node::type node::get_type() const
	{
		return type_;
	}

	node make_node()
	{
		return node();
	}

	node make_node_root()
	{
		return node(std::deque<node>());
	}

	node make_node_symbol(symbol sym)
	{
		return node(sym);
	}

	node make_node_list(std::vector<symbol>&& list)
	{
		return node(std::move(list));
	}

	node make_node_function_par(std::string name)
	{
		return node(name, node::p_args());
	}

	node make_node_function_vec(std::string name)
	{
		return node(name, node::v_args());
	}

	node make_node_paragraph(node::d_args&& args)
	{
		return node(true, std::move(args));
	}

	node make_node_content(node::d_args&& args)
	{
		return node(false, std::move(args));
	}

	node make_node_vector(node::v_args&& args)
	{
		return node(std::move(args));
	}

	node::p_args* access_function_params(node& n)
	{
		try {
			return (node::NODE_FUNCTION_PAR == n.type_)
					? &std::any_cast<node::p_args&>(n.data_)
					: nullptr;
		}
		catch(std::bad_cast const& ex) {
			std::cerr << "access_function_params: " << ex.what() << std::endl;
		}
		return nullptr;
	}

	node::p_args const* access_function_params(node const& n)
	{
		try {
			return (node::NODE_FUNCTION_PAR == n.type_)
					? &std::any_cast<node::p_args const&>(n.data_)
					: nullptr;
		}
		catch(std::bad_cast const& ex) {
			std::cerr << "access_function_params (const): " << ex.what() << std::endl;
		}
		return nullptr;
	}

	std::string get_function_par_name(node const& n)
	{
		return (node::NODE_FUNCTION_PAR == n.type_)
			? n.name_
			: ""
			;
	}

	node::v_args* access_function_vector(node& n)
	{
		try {
			return (node::NODE_FUNCTION_VEC == n.type_)
					? &std::any_cast<node::v_args&>(n.data_)
					: nullptr;
		}
		catch(std::bad_cast const& ex) {
			std::cerr << "access_function_vector: " << ex.what() << std::endl;
		}
		return nullptr;
	}

	node::v_args const* access_function_vector(node const& n)
	{
		try {
			return (node::NODE_FUNCTION_VEC == n.type_)
				? &std::any_cast<node::v_args const&>(n.data_)
				: nullptr;
		}
		catch(std::bad_cast const& ex) {
			std::cerr << "access_function_vector (const): " << ex.what() << std::endl;
		}
		return nullptr;
	}

	std::string get_function_vec_name(docscript::node const& n)
	{
		return (node::NODE_FUNCTION_VEC == n.type_)
			? n.name_
			: ""
			;
	}

	std::string get_name(node const& n)
	{
		return n.name_;
	}

	node::d_args* access_node_root(node& n)
	{
		try {
			return (node::NODE_ROOT == n.type_)
				? &std::any_cast<node::d_args&>(n.data_)
				: nullptr;
		}
		catch(std::bad_cast const& ex) {
			std::cerr << "access_node_root: " << ex.what() << std::endl;
		}
		return nullptr;
	}

	node::d_args const* access_node_root(node const& n)
	{
		try {
			return (node::NODE_ROOT == n.type_)
				? &std::any_cast<node::d_args const&>(n.data_)
				: nullptr;
		}
		catch(std::bad_cast const& ex) {
			std::cerr << "access_node_root (const): " << ex.what() << std::endl;
		}
		return nullptr;
	}

	node::d_args const* access_node_paragraph(node const& n)
	{
		try {
			return (node::NODE_PARAGRAPH == n.type_)
				? &std::any_cast<node::d_args const&>(n.data_)
				: nullptr;
		}
		catch(std::bad_cast const& ex) {
			std::cerr << "access_node_paragraph (const): " << ex.what() << std::endl;
		}
		return nullptr;
	}

	node::d_args const* access_node_content(node const& n)
	{
		try {
			return (node::NODE_CONTENT == n.type_)
				? &std::any_cast<node::d_args const&>(n.data_)
				: nullptr;
		}
		catch(std::bad_cast const& ex) {
			std::cerr << "access_node_content (const): " << ex.what() << std::endl;
		}
		return nullptr;
	}

	node::v_args const* access_node_vector(node const& n)
	{
		try {
			return (node::NODE_VECTOR == n.type_)
				? &std::any_cast<node::v_args const&>(n.data_)
				: nullptr;
		}
		catch(std::bad_cast const& ex) {
			std::cerr << "access_node_vector (const): " << ex.what() << std::endl;
		}
		return nullptr;
	}

	symbol const* access_node_symbol(node const& n)
	{
		try {
			return (node::NODE_SYMBOL == n.type_)
				? &std::any_cast<symbol const&>(n.data_)
				: nullptr;
		}
		catch(std::bad_cast const& ex) {
			std::cerr << "access_node_symbol (const): " << ex.what() << std::endl;
		}
		return nullptr;
	}

	node::s_args const* access_node_list(node const& n)
	{
		return (node::NODE_LIST == n.type_)
			? &std::any_cast<node::s_args const&>(n.data_)
			: nullptr;
	}
}

