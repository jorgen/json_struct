#define JS_INTERNAL_FIRST_(a, ...) a
#define JS_INTERNAL_SECOND_(a, b, ...) b

#define JS_INTERNAL_FIRST(...) JS_INTERNAL_EXPAND(JS_INTERNAL_FIRST_(__VA_ARGS__))
#define JS_INTERNAL_SECOND(...) JS_INTERNAL_EXPAND(JS_INTERNAL_SECOND_(__VA_ARGS__))

#define JS_INTERNAL_EMPTY()

#define JS_INTERNAL_EVAL(...) JS_INTERNAL_EVAL1024(__VA_ARGS__)
#define JS_INTERNAL_EVAL1024(...) JS_INTERNAL_EVAL512(JS_INTERNAL_EVAL512(__VA_ARGS__))
#define JS_INTERNAL_EVAL512(...) JS_INTERNAL_EVAL256(JS_INTERNAL_EVAL256(__VA_ARGS__))
#define JS_INTERNAL_EVAL256(...) JS_INTERNAL_EVAL128(JS_INTERNAL_EVAL128(__VA_ARGS__))
#define JS_INTERNAL_EVAL128(...) JS_INTERNAL_EVAL64(JS_INTERNAL_EVAL64(__VA_ARGS__))
#define JS_INTERNAL_EVAL64(...) JS_INTERNAL_EVAL32(JS_INTERNAL_EVAL32(__VA_ARGS__))
#define JS_INTERNAL_EVAL32(...) JS_INTERNAL_EVAL16(JS_INTERNAL_EVAL16(__VA_ARGS__))
#define JS_INTERNAL_EVAL16(...) JS_INTERNAL_EVAL8(JS_INTERNAL_EVAL8(__VA_ARGS__))
#define JS_INTERNAL_EVAL8(...) JS_INTERNAL_EVAL4(JS_INTERNAL_EVAL4(__VA_ARGS__))
#define JS_INTERNAL_EVAL4(...) JS_INTERNAL_EVAL2(JS_INTERNAL_EVAL2(__VA_ARGS__))
#define JS_INTERNAL_EVAL2(...) JS_INTERNAL_EVAL1(JS_INTERNAL_EVAL1(__VA_ARGS__))
#define JS_INTERNAL_EVAL1(...) __VA_ARGS__

#define JS_INTERNAL_DEFER1(m) m JS_INTERNAL_EMPTY()
#define JS_INTERNAL_DEFER2(m) m JS_INTERNAL_EMPTY JS_INTERNAL_EMPTY()()

#define JS_INTERNAL_IS_PROBE(...) JS_INTERNAL_SECOND(__VA_ARGS__, 0)
#define JS_INTERNAL_PROBE() ~, 1

#define JS_INTERNAL_CAT(a,b) a ## b

#define JS_INTERNAL_NOT(x) JS_INTERNAL_IS_PROBE(JS_INTERNAL_CAT(JS_INTERNAL__NOT_, x))
#define JS_INTERNAL__NOT_0 JS_INTERNAL_PROBE()

#define JS_INTERNAL_BOOL(x) JS_INTERNAL_NOT(JS_INTERNAL_NOT(x))

#define JS_INTERNAL_IF_ELSE(condition) JS_INTERNAL__IF_ELSE(JS_INTERNAL_BOOL(condition))
#define JS_INTERNAL__IF_ELSE(condition) JS_INTERNAL_CAT(JS_INTERNAL__IF_, condition)

#define JS_INTERNAL__IF_1(...) __VA_ARGS__ JS_INTERNAL__IF_1_ELSE
#define JS_INTERNAL__IF_0(...)             JS_INTERNAL__IF_0_ELSE

#define JS_INTERNAL__IF_1_ELSE(...)
#define JS_INTERNAL__IF_0_ELSE(...) __VA_ARGS__

#define JS_INTERNAL_HAS_ARGS(...) JS_INTERNAL_BOOL(JS_INTERNAL_FIRST(JS_INTERNAL__END_OF_ARGUMENTS_ __VA_ARGS__)())
#define JS_INTERNAL__END_OF_ARGUMENTS_() 0

#define JS_INTERNAL_MAP_MEMBER(m, first, ...)              \
  m(#first, &JS_OBJECT_T::first)           \
  JS_INTERNAL_IF_ELSE(JS_INTERNAL_HAS_ARGS(__VA_ARGS__))(,   \
    JS_INTERNAL_DEFER2(JS_INTERNAL__MAP_MEMBER)()(m, __VA_ARGS__)   \
  )(                                       \
    /* Do nothing, just terminate */       \
  )
#define JS_INTERNAL__MAP_MEMBER() JS_INTERNAL_MAP_MEMBER

#define JS_INTERNAL_MAKE_MEMBERS(...) JS_INTERNAL_EXPAND(JS_INTERNAL_EVAL(JS_INTERNAL_MAP_MEMBER(JS::makeMemberInfo, __VA_ARGS__)))
#define JS_OBJ(...) \
    template<typename JS_OBJECT_T> \
    struct JsonStructBase \
    { \
       using TT = decltype(JS::makeTuple(JS_INTERNAL_MAKE_MEMBERS(__VA_ARGS__))); \
       static const TT &js_static_meta_data_info() \
       { static auto ret = JS::makeTuple(JS_INTERNAL_MAKE_MEMBERS(__VA_ARGS__)); return ret; } \
       static const decltype(JS::makeTuple()) &js_static_meta_super_info() \
       { static auto ret = JS::makeTuple(); return ret; } \
    }

#define JS_INTERNAL_MAP_SUPER(m, first, ...)              \
  m<first>(#first)                              \
  JS_INTERNAL_IF_ELSE(JS_INTERNAL_HAS_ARGS(__VA_ARGS__))(,         \
    JS_INTERNAL_DEFER2(JS_INTERNAL__MAP_SUPER)()(m, __VA_ARGS__)   \
  )(                                             \
    /* Do nothing, just terminate */             \
  )
#define JS_INTERNAL__MAP_SUPER() JS_INTERNAL_MAP_SUPER

#define JS_INTERNAL_MAKE_SUPER_CLASSES(...) JS_INTERNAL_EXPAND(JS_INTERNAL_EVAL(JS_INTERNAL_MAP_SUPER(JS::makeSuperInfo, __VA_ARGS__)))
#define JS_SUPER(...) JS::makeTuple(JS_INTERNAL_EXPAND(JS_INTERNAL_MAKE_SUPER_CLASSES(__VA_ARGS__)))

#define JS_OBJ_SUPER(super_list, ...) \
    template<typename JS_OBJECT_T> \
    struct JsonStructBase \
    { \
       using TT = decltype(JS::makeTuple(JS_INTERNAL_EXPAND(JS_INTERNAL_MAKE_MEMBERS(__VA_ARGS__)))); \
       static const TT &js_static_meta_data_info() \
       { static auto ret = JS::makeTuple(JS_INTERNAL_EXPAND(JS_INTERNAL_MAKE_MEMBERS(__VA_ARGS__))); return ret; } \
        static const decltype(super_list) &js_static_meta_super_info() \
        { static auto ret = super_list; return ret; } \
    }

#define JS_OBJ_EXT(Type, ...) \
    namespace JS {\
    namespace Internal {\
    template<typename JS_OBJECT_T> \
    struct JsonStructBaseDummy<Type, JS_OBJECT_T> \
    { \
       using TT = decltype(JS::makeTuple(JS_INTERNAL_MAKE_MEMBERS(__VA_ARGS__))); \
       static const TT &js_static_meta_data_info() \
       { static auto ret = JS::makeTuple(JS_INTERNAL_MAKE_MEMBERS(__VA_ARGS__)); return ret; } \
       static const decltype(JS::makeTuple()) &js_static_meta_super_info() \
       { static auto ret = JS::makeTuple(); return ret; } \
    };\
    }}

#define JS_OBJ_EXT_SUPER(Type,super_list, ...) \
    namespace JS {\
    namespace Internal {\
    template<typename JS_OBJECT_T> \
    struct JsonStructBaseDummy<Type, JS_OBJECT_T> \
    { \
       using TT = decltype(JS::makeTuple(JS_INTERNAL_EXPAND(JS_INTERNAL_MAKE_MEMBERS(__VA_ARGS__)))); \
       static const TT &js_static_meta_data_info() \
       { static auto ret = JS::makeTuple(JS_INTERNAL_EXPAND(JS_INTERNAL_MAKE_MEMBERS(__VA_ARGS__))); return ret; } \
        static const decltype(super_list) &js_static_meta_super_info() \
        { static auto ret = super_list; return ret; } \
    };\
    }}

#define JS_MEMBER(member) JS::makeMemberInfo(#member, &JS_OBJECT_T::member)

struct DataRef
{
    /*!
     * Constructs a null Dataref pointing to "" with size 0.
     */
    JS_CONSTEXPR explicit DataRef()
        : data("")
        , size(0)
    {}

    /*!
     * Constructs a DataRef pointing to data and size.
     * \param data points to start of data.
     * \param size size of data.
     */
    JS_CONSTEXPR explicit DataRef(const char *data, size_t size)
        : data(data)
        , size(size)
    {}

    /*!  Cobstructs a DataRef pointing to an array. This will \b NOT look for
     * the null terminator, but just initialize the DataRef to the size of the
     * array - 1. This function is intended to be used with string literals.
     * \param data  start of the data.
     */
    template <size_t N>
    JS_CONSTEXPR explicit DataRef(const char (&data)[N])
        : data(data)
        , size(N -1)
    {}

    const char *data;
    size_t size;
};

template<typename T, typename U, size_t NAME_COUNT>
struct MI
{
    DataRef name[NAME_COUNT];
    T U::* member;
    typedef T type;
};

namespace Internal {
    template<typename T, typename U, size_t NAME_COUNT>
    using MemberInfo = MI <T, U, NAME_COUNT>;

    template<typename T>
    struct SuperInfo
    {
        explicit
            SuperInfo(const DataRef &name)
            : name(name)
            {}
        const DataRef name;
        typedef T type;
    };
}

template<typename T, typename U, size_t NAME_SIZE, typename ...Aliases>
JS_CONSTEXPR const MI<T, U, sizeof...(Aliases) + 1> makeMemberInfo(const char(&name)[NAME_SIZE], T U::* member, Aliases ... aliases)
{
    return { {DataRef(name), DataRef(aliases)... }, member };
}

template<typename T, typename U, size_t NAME_SIZE, typename ...Aliases>
JS_CONSTEXPR const MI<T, U, sizeof...(Aliases) + 1> makeMemberInfo(const char(&name)[NAME_SIZE], const MI<T,U,sizeof...(Aliases) + 1> mi)
{
  return mi;
}

template<typename T, size_t NAME_SIZE>
JS_CONSTEXPR const Internal::SuperInfo<T> makeSuperInfo(const char(&name)[NAME_SIZE])
{
  return Internal::SuperInfo<T>(DataRef(name));
}

struct SubStruct2
{
    float Field1;
    bool Field2;
    JS_OBJ(Field1,
      JS_MEMBER(Field2));
};

struct SubStruct3 : public SubStruct2
{
    int Field3;
    int Field4;
    int Field5;
    JS_OBJ_SUPER(
      JS_SUPER(SubStruct2),
      Field3,
      Field4,
      Field5);
};

int main(int argc, char** argv)
{
  return 4;
}
