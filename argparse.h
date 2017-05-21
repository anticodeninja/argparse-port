#ifndef ARGPARSE_H
#define ARGPARSE_H

#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#ifdef __cplusplus
extern "C" {
#endif

    typedef struct parser_base_arg_t {
        char const * keyword;
        char const * keyshort;
        char const * help;
        bool is_filled;
        void (*set_value)(void* element, char const * value);
        struct parser_base_arg_t* next;
    } parser_base_arg_t;

    typedef struct parser_flag_arg_t {
        parser_base_arg_t base;
    } parser_flag_arg_t;

    typedef struct parser_int_arg_t {
        parser_base_arg_t base;
        int default_value;
        int value;
    } parser_int_arg_t;

    typedef struct parser_string_arg_t {
        parser_base_arg_t base;
        char const * default_value;
        char const * value;
    } parser_string_arg_t;

    typedef struct parser_t {
        int argc;
        char** argv;

        char* last_err;
        int last_err_pos;
        int last_err_size;

        parser_flag_arg_t* help_arg;
        parser_base_arg_t* optional_args;
        parser_base_arg_t* positional_args;
    } parser_t;

    typedef enum parser_result_t {
        PARSER_RESULT_OK,
        PARSER_RESULT_HELP,
        PARSER_RESULT_ERROR,
    } parser_result_t;

    parser_result_t parser_init(parser_t** parser);
    parser_result_t parser_free(parser_t** parser);
    parser_result_t parser_parse(parser_t* parser, int argc, char** argv);
    const char* parser_get_last_err(parser_t* parser);

    parser_result_t parser_flag_add_arg(parser_t* parser, parser_flag_arg_t** arg, char const * keyword);
    bool parser_flag_is_filled(parser_flag_arg_t* arg);
    void parser_flag_set_alt(parser_flag_arg_t* arg, char const * alt);
    void parser_flag_set_help(parser_flag_arg_t* arg, char const * help);

    parser_result_t parser_int_add_arg(parser_t* parser, parser_int_arg_t** arg, char const * keyword);
    int parser_int_get_value(parser_int_arg_t* arg);
    bool parser_int_is_filled(parser_int_arg_t* arg);
    void parser_int_set_alt(parser_int_arg_t* arg, char const * alt);
    void parser_int_set_help(parser_int_arg_t* arg, char const * help);
    void parser_int_set_default(parser_int_arg_t* arg, int default_value);

    parser_result_t parser_string_add_arg(parser_t* parser, parser_string_arg_t** arg, char const * keyword);
    const char* parser_string_get_value(parser_string_arg_t* arg);
    bool parser_string_is_filled(parser_string_arg_t* arg);
    void parser_string_set_alt(parser_string_arg_t* arg, char const * alt);
    void parser_string_set_help(parser_string_arg_t* arg, char const * help);
    void parser_string_set_default(parser_string_arg_t* arg, char const * default_value);

#ifdef __cplusplus
}
#endif

#endif // ARGPARSE_H
