#include <stdio.h>
#include "argparse.h"

int main(int argc, char** argv) {
    printf("sizeof(parser_t)=%lu\n", sizeof(parser_t));
    printf("sizeof(parser_base_arg_t)=%lu\n", sizeof(parser_base_arg_t));
    printf("sizeof(parser_int_arg_t)=%lu\n", sizeof(parser_int_arg_t));
    printf("sizeof(parser_string_arg_t)=%lu\n", sizeof(parser_string_arg_t));
    printf("\n");

    for (int i = 0; i < argc; ++i) {
        printf("%s ", argv[i]);
    }
    printf("\n\n");

    parser_t* parser;
    parser_init(&parser);

    parser_string_arg_t* input_arg;
    parser_string_add_arg(parser, &input_arg, "input");
    parser_string_set_help(input_arg, "input file");

    parser_string_arg_t* output_arg;
    parser_string_add_arg(parser, &output_arg, "output");
    parser_string_set_help(output_arg, "output file");

    parser_int_arg_t* opt_int_arg;
    parser_int_add_arg(parser, &opt_int_arg, "--first");
    parser_int_set_alt(opt_int_arg, "-f");
    parser_int_set_help(opt_int_arg, "first int optional argument");
    parser_int_set_default(opt_int_arg, 1);

    parser_string_arg_t* opt_str_arg;
    parser_string_add_arg(parser, &opt_str_arg, "--second");
    parser_string_set_alt(opt_str_arg, "-s");
    parser_string_set_help(opt_str_arg, "second string optional argument");
    parser_string_set_default(opt_str_arg, "default");

    if (parser_parse(parser, argc, argv) == PARSER_RESULT_OK) {
        printf("input: '%s'\n", parser_string_get_value(input_arg));
        printf("output: '%s'\n", parser_string_get_value(output_arg));
        printf("first: %d\n", parser_int_get_value(opt_int_arg));
        printf("second: '%s'\n", parser_string_get_value(opt_str_arg));
    } else {
        printf("%s", parser_get_last_err(parser));
    }

    parser_free(&parser);

    return 0;
}
