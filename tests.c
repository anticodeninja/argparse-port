#include "unity/src/unity.h"
#include "argparse.h"

void init_parser(parser_t** parser,
                 parser_string_arg_t** input_arg,
                 parser_string_arg_t** output_arg,
                 parser_int_arg_t** opt_int_arg,
                 parser_string_arg_t** opt_str_arg,
                 parser_flag_arg_t** opt_flag_arg,
                 bool altEnabled,
                 bool shortDefault) {
    parser_init(parser);

    if (input_arg != NULL) {
        parser_string_add_arg(*parser, input_arg, "input");
        parser_string_set_help(*input_arg, "input file");
    }

    if (output_arg != NULL) {
        parser_string_add_arg(*parser, output_arg, "output");
        parser_string_set_help(*output_arg, "output file");
    }

    if (opt_int_arg != NULL) {
        parser_int_add_arg(*parser, opt_int_arg, shortDefault ? "-f" : "--first");
        if (altEnabled) {
            parser_int_set_alt(*opt_int_arg, shortDefault ? "--first" : "-f");
        }
        parser_int_set_help(*opt_int_arg, "first int optional argument");
        parser_int_set_default(*opt_int_arg, 1);
    }

    if (opt_str_arg != NULL) {
        parser_string_add_arg(*parser, opt_str_arg, shortDefault ? "-s" : "--second");
        if (altEnabled) {
            parser_string_set_alt(*opt_str_arg, shortDefault ? "--second" : "-s");
        }
        parser_string_set_help(*opt_str_arg, "second string optional argument");
        parser_string_set_default(*opt_str_arg, "default");
    }

    if (opt_flag_arg != NULL) {
        parser_flag_add_arg(*parser, opt_flag_arg, shortDefault ? "-m" : "--mark");
        if (altEnabled) {
            parser_flag_set_alt(*opt_flag_arg, shortDefault ? "--mark" : "-m");
        }
        parser_flag_set_help(*opt_flag_arg, "flag optional argument");
    }
}

void test_Parser_OnlyPositionalArgs() {
    parser_t* parser;
    parser_string_arg_t* input_arg;
    parser_string_arg_t* output_arg;
    parser_int_arg_t* opt_int_arg;
    parser_string_arg_t* opt_str_arg;
    char* args[] = { "exename", "input_filename", "output_filename" };

    init_parser(&parser, &input_arg, &output_arg, &opt_int_arg, &opt_str_arg, NULL, true, false);
    TEST_ASSERT_EQUAL_UINT_MESSAGE(PARSER_RESULT_OK, parser_parse(parser, 3, args), "Parse Error");
    TEST_ASSERT_EQUAL_STRING("input_filename", parser_string_get_value(input_arg));
    TEST_ASSERT_EQUAL_STRING("output_filename", parser_string_get_value(output_arg));
    parser_free(&parser);
}

void test_Parser_PositionalArgsError() {
    parser_t* parser;
    parser_string_arg_t* input_arg;
    parser_string_arg_t* output_arg;
    parser_int_arg_t* opt_int_arg;
    parser_string_arg_t* opt_str_arg;
    char* args[] = { "exename" };

    init_parser(&parser, &input_arg, &output_arg, &opt_int_arg, &opt_str_arg, NULL, true, false);
    TEST_ASSERT_EQUAL_UINT_MESSAGE(PARSER_RESULT_ERROR, parser_parse(parser, 1, args), "Parse Error");
    TEST_ASSERT_EQUAL_STRING("usage: exename [-h] [-f FIRST] [-s SECOND] input output\n"
                             "exename: error: the following arguments are required: input, output\n",
                             parser->last_err);
    parser_free(&parser);
}

void test_Parser_OptionalArgsError() {
    parser_t* parser;
    parser_string_arg_t* input_arg;
    parser_string_arg_t* output_arg;
    parser_int_arg_t* opt_int_arg;
    parser_string_arg_t* opt_str_arg;
    char* args[] = { "exename", "--error" };

    init_parser(&parser, &input_arg, &output_arg, &opt_int_arg, &opt_str_arg, NULL, true, false);
    TEST_ASSERT_EQUAL_UINT_MESSAGE(PARSER_RESULT_ERROR, parser_parse(parser, 2, args), "Parse Error");
    TEST_ASSERT_EQUAL_STRING("usage: exename [-h] [-f FIRST] [-s SECOND] input output\n"
                             "exename: error: unrecognized arguments: --error\n",
                             parser->last_err);
    parser_free(&parser);
}

void test_Parser_HelpArgs() {
    parser_t* parser;
    parser_string_arg_t* input_arg;
    parser_string_arg_t* output_arg;
    parser_int_arg_t* opt_int_arg;
    parser_string_arg_t* opt_str_arg;
    char* args[] = { "exename", "--help" };

    init_parser(&parser, &input_arg, &output_arg, &opt_int_arg, &opt_str_arg, NULL, true, false);
    TEST_ASSERT_EQUAL_UINT_MESSAGE(PARSER_RESULT_HELP, parser_parse(parser, 2, args), "Parse Error");
    TEST_ASSERT_EQUAL_STRING("usage: exename [-h] [-f FIRST] [-s SECOND] input output\n"
                             "\n"
                             "positional arguments:\n"
                             "  input                 input file\n"
                             "  output                output file\n"
                             "\n"
                             "optional arguments:\n"
                             "  -h, --help            show this help message and exit\n"
                             "  -f FIRST, --first FIRST\n"
                             "                        first int optional argument\n"
                             "  -s SECOND, --second SECOND\n"
                             "                        second string optional argument\n"
                             "\n",
                             parser->last_err);
    parser_free(&parser);
}

void test_Parser_OnlyPositionalAndShortArgsAfter() {
    parser_t* parser;
    parser_string_arg_t* input_arg;
    parser_string_arg_t* output_arg;
    parser_int_arg_t* opt_int_arg;
    parser_string_arg_t* opt_str_arg;
    char* args[] = { "exename", "input_filename", "output_filename", "-f", "123", "-s", "value" };

    init_parser(&parser, &input_arg, &output_arg, &opt_int_arg, &opt_str_arg, NULL, true, false);
    TEST_ASSERT_EQUAL_UINT_MESSAGE(PARSER_RESULT_OK, parser_parse(parser, 7, args), "Parse Error");
    TEST_ASSERT_EQUAL_STRING("input_filename", parser_string_get_value(input_arg));
    TEST_ASSERT_EQUAL_STRING("output_filename", parser_string_get_value(output_arg));
    TEST_ASSERT_TRUE(parser_int_is_filled(opt_int_arg));
    TEST_ASSERT_EQUAL_INT(123, parser_int_get_value(opt_int_arg));
    TEST_ASSERT_TRUE(parser_string_is_filled(opt_str_arg));
    TEST_ASSERT_EQUAL_STRING("value", parser_string_get_value(opt_str_arg));
    parser_free(&parser);
}

void test_Parser_OnlyPositionalAndShortArgsBefore() {
    parser_t* parser;
    parser_string_arg_t* input_arg;
    parser_string_arg_t* output_arg;
    parser_int_arg_t* opt_int_arg;
    parser_string_arg_t* opt_str_arg;
    char* args[] = { "exename",  "-f", "123", "-s", "value", "input_filename", "output_filename"};

    init_parser(&parser, &input_arg, &output_arg, &opt_int_arg, &opt_str_arg, NULL, true, false);
    TEST_ASSERT_EQUAL_UINT_MESSAGE(PARSER_RESULT_OK, parser_parse(parser, 7, args), "Parse Error");
    TEST_ASSERT_EQUAL_STRING("input_filename", parser_string_get_value(input_arg));
    TEST_ASSERT_EQUAL_STRING("output_filename", parser_string_get_value(output_arg));
    TEST_ASSERT_TRUE(parser_int_is_filled(opt_int_arg));
    TEST_ASSERT_EQUAL_INT(123, parser_int_get_value(opt_int_arg));
    TEST_ASSERT_TRUE(parser_string_is_filled(opt_str_arg));
    TEST_ASSERT_EQUAL_STRING("value", parser_string_get_value(opt_str_arg));
    parser_free(&parser);
}

void test_Parser_OnlyPositionalAndLongArgsWithFlags() {
    parser_t* parser;
    parser_string_arg_t* input_arg;
    parser_string_arg_t* output_arg;
    parser_int_arg_t* opt_int_arg;
    parser_string_arg_t* opt_str_arg;
    parser_flag_arg_t* opt_flag_arg;
    char* args[] = { "exename", "--mark", "input_filename", "output_filename", "--first", "123", "--second", "value"};

    init_parser(&parser, &input_arg, &output_arg, &opt_int_arg, &opt_str_arg, &opt_flag_arg, true, false);
    TEST_ASSERT_EQUAL_UINT_MESSAGE(PARSER_RESULT_OK, parser_parse(parser, 8, args), "Parse Error");
    TEST_ASSERT_EQUAL_STRING("input_filename", parser_string_get_value(input_arg));
    TEST_ASSERT_EQUAL_STRING("output_filename", parser_string_get_value(output_arg));
    TEST_ASSERT_TRUE(parser_int_is_filled(opt_int_arg));
    TEST_ASSERT_EQUAL_INT(123, parser_int_get_value(opt_int_arg));
    TEST_ASSERT_TRUE(parser_string_is_filled(opt_str_arg));
    TEST_ASSERT_EQUAL_STRING("value", parser_string_get_value(opt_str_arg));
    TEST_ASSERT_TRUE(parser_flag_is_filled(opt_flag_arg));
    parser_free(&parser);
}

void test_Parser_DashedArgs() {
    parser_t* parser;
    parser_string_arg_t* input_arg;
    parser_string_arg_t* opt_str_arg;
    char* args[] = { "exename", "-", "--second", "-" };

    init_parser(&parser, &input_arg, NULL, NULL, &opt_str_arg, NULL, true, false);
    TEST_ASSERT_EQUAL_UINT_MESSAGE(PARSER_RESULT_OK, parser_parse(parser, 4, args), "Parse Error");
    TEST_ASSERT_EQUAL_STRING("-", parser_string_get_value(input_arg));
    TEST_ASSERT_TRUE(parser_string_is_filled(opt_str_arg));
    TEST_ASSERT_EQUAL_STRING("-", parser_string_get_value(opt_str_arg));
    parser_free(&parser);
}

void test_OnlyPositionalParser_OnlyPositionalArgs() {
    parser_t* parser;
    parser_string_arg_t* input_arg;
    parser_string_arg_t* output_arg;
    char* args[] = { "exename", "input_filename", "output_filename" };

    init_parser(&parser, &input_arg, &output_arg, NULL, NULL, NULL, false, false);
    TEST_ASSERT_EQUAL_UINT_MESSAGE(PARSER_RESULT_OK, parser_parse(parser, 3, args), "Parse Error");
    TEST_ASSERT_EQUAL_STRING("input_filename", parser_string_get_value(input_arg));
    TEST_ASSERT_EQUAL_STRING("output_filename", parser_string_get_value(output_arg));
    parser_free(&parser);
}

void test_OnlyOptionalParser_WithoutArgs() {
    parser_t* parser;
    parser_int_arg_t* opt_int_arg;
    parser_string_arg_t* opt_str_arg;
    char* args[] = { "exename" };

    init_parser(&parser, NULL, NULL, &opt_int_arg, &opt_str_arg, NULL, true, false);
    TEST_ASSERT_EQUAL_UINT_MESSAGE(PARSER_RESULT_OK, parser_parse(parser, 1, args), "Parse Error");
    TEST_ASSERT_FALSE(parser_int_is_filled(opt_int_arg));
    TEST_ASSERT_EQUAL_INT(1, parser_int_get_value(opt_int_arg));
    TEST_ASSERT_FALSE(parser_string_is_filled(opt_str_arg));
    TEST_ASSERT_EQUAL_STRING("default", parser_string_get_value(opt_str_arg));
    parser_free(&parser);
}

int main(int argc, char** argv)
{
    UNITY_BEGIN();
    RUN_TEST(test_Parser_OnlyPositionalArgs);
    RUN_TEST(test_Parser_OnlyPositionalAndShortArgsAfter);
    RUN_TEST(test_Parser_OnlyPositionalAndShortArgsBefore);
    RUN_TEST(test_Parser_OnlyPositionalAndLongArgsWithFlags);
    RUN_TEST(test_Parser_DashedArgs);
    RUN_TEST(test_OnlyPositionalParser_OnlyPositionalArgs);
    RUN_TEST(test_OnlyOptionalParser_WithoutArgs);

    RUN_TEST(test_Parser_PositionalArgsError);
    RUN_TEST(test_Parser_OptionalArgsError);
    RUN_TEST(test_Parser_HelpArgs);

    return UNITY_END();
}
