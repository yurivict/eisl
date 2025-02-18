module disl;

enum {
    UNDEF_VAR = 101,
    UNDEF_FUN,
    NOT_COMPUTABLE,
    OUT_OF_RANGE,
    MALLOC_OVERF,
    UNDEF_CLASS,
    WRONG_ARGS,
    NOT_NUM,
    NOT_STR,
    NOT_LIST,
    NOT_SYM,
    ILLEGAL_INPUT,
    NOT_FUNC,
    UNDEF_TAG,
    CANT_OPEN,
    ILLEGAL_ARGS,
    NOT_VEC,
    NOT_ARR,
    NOT_CLASS,
    NOT_METHOD,
    NOT_CONS,
    CANT_MODIFY,
    NOT_INT,
    NOT_STREAM,
    NOT_OUT_STREAM,
    NOT_IN_STREAM,
    NOT_CHAR,
    NOT_FLT,
    NOT_INSTANCE,
    CTRL_OVERF,
    ILLEGAL_RPAREN,
    END_STREAM,
    ILLEGAL_FORM,
    IMPROPER_FORM,
    DIV_ZERO,
    NOT_VECARR,
    CANT_CREATE,
    CANT_PARSE,
    CANT_ASSURE,
    NOT_EXIST_METHOD,
    HAS_COMMON_CLASS,
    ILLEGAL_CLASS,
    NOT_TOP_LEVEL,
    NOT_POSITIVE,
    FLT_OVERF,
    FLT_UNDERF,
    CANT_REDEFINE,
    STACK_OVERF,
    SHELTER_OVERF,
    STACK_UNDERF,
    SHELTER_UNDERF,
    SYSTEM_ERR,
    RESOURCE_ERR,
    NOT_EXIST_ARG,
    IMPROPER_ARGS,
    OUT_OF_DOMAIN,
    FLT_OUT_OF_DOMAIN,
    OUT_OF_REAL,
    NOT_BASIC_ARRAY,
    SERIOUS_ERR,
    ARITHMETIC_ERR,
    DOMAIN_ERR,
    UNDEF_DYN,
    UNDEF_ENTITY,
    SIMPLE_ERR,
    EXHAUSTED_ERR,
    DYNAMIC_OVERF,
};

enum int NIL = 0;

extern (C):

int car(int addr);
void error(int errnum, const char *fun, int arg);
char *get_name(int x);
int length(int addr);
int makestr(const char *string);
