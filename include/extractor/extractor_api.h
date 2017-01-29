#include <json_tools.h>

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

struct Member
{
    std::string name;
    std::string type;
    JT::OptionalChecked<std::string> default_value;
    JT::OptionalChecked<Comment> comment;

    JT_STRUCT(JT_MEMBER(name),
              JT_MEMBER(type),
              JT_MEMBER(default_value),
              JT_MEMBER(comment));
};
struct JsonStruct
{
    std::string type;
    std::vector<Member> members;
    JT::OptionalChecked<Comment> comment;

    JT_STRUCT(JT_MEMBER(type),
              JT_MEMBER(members),
              JT_MEMBER(comment));
};