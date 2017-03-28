#pragma once
#include <json_tools.h>

namespace JT
{
	struct Paragraph
	{
		JT::OptionalChecked<std::string> command;
		std::vector<std::string> lines;

		JT_STRUCT(JT_MEMBER(command),
			JT_MEMBER(lines));
	};

	struct Comment
	{
		std::vector<Paragraph> paragraphs;

		JT_STRUCT(JT_MEMBER(paragraphs));
	};

	struct Record;

	struct TypeDef
	{
		TypeDef();
		TypeDef(const TypeDef &other);
		TypeDef &operator=(const TypeDef &other);

		std::string type;
		JT::OptionalChecked<std::string> name_space;
		JT::SilentUniquePtr<Record> record_type;
		JT::SilentVector<TypeDef> template_parameters;

		JT_STRUCT(JT_MEMBER(type),
			JT_MEMBER(name_space),
			JT_MEMBER(record_type),
			JT_MEMBER(template_parameters));
	};

	struct Member
	{
		TypeDef type;
		std::string name;
		JT::OptionalChecked<std::string> default_value;
		JT::OptionalChecked<Comment> comment;


		JT_STRUCT(JT_MEMBER(type),
			JT_MEMBER(name),
			JT_MEMBER(default_value),
			JT_MEMBER(comment));
	};

	struct Record
	{
		std::vector<Member> members;
		JT::SilentVector<TypeDef> super_classes;
		JT::OptionalChecked<Comment> comment;

		JT_STRUCT(JT_MEMBER(members),
			JT_MEMBER(super_classes),
			JT_MEMBER(comment));
	};
	struct Function
	{
		std::string name;
		JT::OptionalChecked<TypeDef> argument;
		TypeDef return_type;
		JT::OptionalChecked<Comment> comment;

		JT_STRUCT(JT_MEMBER(name),
			JT_MEMBER(argument),
			JT_MEMBER(return_type),
			JT_MEMBER(comment));
	};

	struct FunctionObject
	{
		std::string type;
		std::vector<Function> functions;
		JT::SilentVector<FunctionObject> super_classes;
		JT::OptionalChecked<Comment> comment;

		JT_STRUCT(JT_MEMBER(type),
			JT_MEMBER(functions),
			JT_MEMBER(super_classes),
			JT_MEMBER(comment));
	};


	inline TypeDef::TypeDef()
	{}
	inline TypeDef::TypeDef(const TypeDef &other)
		: type(other.type)
		, name_space(other.name_space)
		, record_type(other.record_type.get() ? new Record(*other.record_type.get()) : nullptr)
		, template_parameters(other.template_parameters)
	{}

	inline TypeDef &TypeDef::operator=(const TypeDef &other)
	{
		type = other.type;
		name_space = other.name_space;
		record_type.reset(new Record(*other.record_type.get()));
		template_parameters = other.template_parameters;
	}
}