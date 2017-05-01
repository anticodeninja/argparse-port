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

    const int INITIAL_BUFFER_SIZE = 256;
    const int PADDING = 2;
    const int FIRST_COLUMN_SIZE = 24;

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

    bool _parser_prefix(const char *pre, const char *str)
    {
        return strncmp(pre, str, strlen(pre)) == 0;
    }

    void _parser_append_list(parser_base_arg_t** list, parser_base_arg_t* element) {
        parser_base_arg_t** pointer = list;
        while (*pointer != NULL) {
            pointer = (parser_base_arg_t**)&(*pointer)->next;
        }
        *pointer = element;
    }

    void _parser_free_list(parser_base_arg_t** list) {
        parser_base_arg_t* prev = *list;
        parser_base_arg_t* next;

        while (prev != NULL) {
            next = (parser_base_arg_t*)prev->next;
            free(prev);
            prev = next;
        }

        *list = NULL;
    }

    void _parser_clear_last_err(parser_t* parser) {
        if (parser->last_err != NULL) {
            parser->last_err_pos = 0;
            parser->last_err[parser->last_err_pos] = '\0';
        }
    }

    int _parser_append_last_err(parser_t* parser, char const * format, ...) {
        if (parser->last_err == NULL) {
            parser->last_err_pos = 0;
            parser->last_err_size = INITIAL_BUFFER_SIZE;
            parser->last_err = (char*)malloc(sizeof(char) * parser->last_err_size);
            parser->last_err[parser->last_err_pos] = '\0';
        }

        int writed = 0;
        for (;;) {
            va_list args;
            va_start(args, format);
            writed = vsnprintf(&parser->last_err[parser->last_err_pos],
                               parser->last_err_size - parser->last_err_pos,
                               format,
                               args);
            va_end(args);

            if ((parser->last_err_pos + writed) < parser->last_err_size) {
                parser->last_err_pos += writed;
                break;
            } else {
                while (parser->last_err_size <= (parser->last_err_pos + writed)) {
                    parser->last_err_size = 2 * parser->last_err_size;
                }

                char* old_buf = parser->last_err;
                parser->last_err = (char*)malloc(sizeof(char) * parser->last_err_size);
                memcpy(parser->last_err, old_buf, parser->last_err_pos);
                parser->last_err[parser->last_err_pos] = '\0';
                free(old_buf);
            }
        }

        return writed;
    }

    int _parser_append_optional_name(parser_t* parser, parser_base_arg_t* element) {
        int offset = 0;
        if (element->set_value != NULL) {
            char const * name = element->keyword != NULL
                ? &element->keyword[2]
                : &element->keyshort[1];

            offset += _parser_append_last_err(parser, " ");
            while (*name != '\0') {
                offset += _parser_append_last_err(parser, "%c", toupper(*name));
                name++;
            }
        }
        return offset;
    }

    void _parser_append_usage_message(parser_t* parser) {
        _parser_append_last_err(parser, "usage: %s", parser->argv[0]);

        parser_base_arg_t* current_optional = parser->optional_args;
        while (current_optional != NULL) {
            _parser_append_last_err(parser, " [%s",
                                    current_optional->keyshort != NULL
                                    ? current_optional->keyshort
                                    : current_optional->keyword);
            _parser_append_optional_name(parser, current_optional);
            _parser_append_last_err(parser, "]");

            current_optional = (parser_base_arg_t*)current_optional->next;
        }

        parser_base_arg_t* current_positional = parser->positional_args;
        while (current_positional != NULL) {
            _parser_append_last_err(parser, " %s", current_positional->keyword);
            current_positional = (parser_base_arg_t*)current_positional->next;
        }
        _parser_append_last_err(parser, "\n");
    }

    void _parser_set_positional_error_message(parser_t* parser, parser_base_arg_t* current_positional) {
        _parser_clear_last_err(parser);
        _parser_append_usage_message(parser);
        _parser_append_last_err(parser, "%s: error: the following arguments are required:", parser->argv[0]);
        while (current_positional != NULL) {
            _parser_append_last_err(parser, " %s", current_positional->keyword);
            if (current_positional->next != NULL) {
                _parser_append_last_err(parser, ",");
            }
            current_positional = (parser_base_arg_t*)current_positional->next;
        }
        _parser_append_last_err(parser, "\n");
    }

    void _parser_set_optional_error_message(parser_t* parser, char* argv) {
        _parser_clear_last_err(parser);
        _parser_append_usage_message(parser);
        _parser_append_last_err(parser, "%s: error: unrecognized arguments: %s\n", parser->argv[0], argv);
    }

    void _parser_append_arg_help(parser_t* parser, int offset, parser_base_arg_t* arg) {
        if (offset >= FIRST_COLUMN_SIZE) {
            _parser_append_last_err(parser, "\n", NULL);
            offset = 0;
        }

        while (offset < FIRST_COLUMN_SIZE) {
            offset += _parser_append_last_err(parser, " ");
        }

        if (arg->help != NULL) {
            _parser_append_last_err(parser, "%s", arg->help);
        }
        _parser_append_last_err(parser, "\n");
    }

    void _parser_set_help_message(parser_t* parser) {
        _parser_clear_last_err(parser);
        _parser_append_usage_message(parser);
        _parser_append_last_err(parser, "\n");

        int offset = 0;

        parser_base_arg_t* current_positional = parser->positional_args;
        if (current_positional != NULL) {
            _parser_append_last_err(parser, "positional arguments:\n");
            while (current_positional != NULL) {
                offset = 0;
                while (offset < PADDING) {
                    offset += _parser_append_last_err(parser, " ");
                }
                offset += _parser_append_last_err(parser, "%s", current_positional->keyword);
                _parser_append_arg_help(parser, offset, current_positional);
                current_positional = (parser_base_arg_t*)current_positional->next;
            }
            _parser_append_last_err(parser, "\n");
        }

        parser_base_arg_t* current_optional = parser->optional_args;
        if (current_optional != NULL) {
            _parser_append_last_err(parser, "optional arguments:\n");
            while (current_optional != NULL) {
                offset = 0;
                while (offset < PADDING) {
                    offset += _parser_append_last_err(parser, " ");
                }

                if (current_optional->keyshort != NULL) {
                    offset += _parser_append_last_err(parser, "%s", current_optional->keyshort);
                    offset += _parser_append_optional_name(parser, current_optional);
                    if (current_optional->keyword != NULL) {
                        offset += _parser_append_last_err(parser, ", ");
                    }
                }

                if (current_optional->keyword != NULL) {
                    offset += _parser_append_last_err(parser, "%s", current_optional->keyword);
                    offset += _parser_append_optional_name(parser, current_optional);
                }

                _parser_append_arg_help(parser, offset, current_optional);
                current_optional = (parser_base_arg_t*)current_optional->next;
            }
            _parser_append_last_err(parser, "\n");
        }
    }

    void _parser_set_alt(parser_base_arg_t* element, char const * keyword) {
        if (_parser_prefix("-", keyword) && !_parser_prefix("--", keyword)) {
            element->keyshort = keyword;
        } else {
            element->keyword = keyword;
        }
    }

    bool _parser_is_filled(parser_base_arg_t* element) {
        return element->is_filled;
    }

    void _parser_set_help(parser_base_arg_t* element, char const * help) {
        element->help = help;
    }

    void _parser_add_arg(parser_t* parser,
                         parser_base_arg_t* element,
                         char const * keyword,
                         void (*set_value)(void*, char const *)) {
        element->keyshort = NULL;
        element->keyword = NULL;
        element->help = NULL;
        element->next = NULL;
        element->is_filled = false;
        element->set_value = set_value;

        _parser_set_alt(element, keyword);
        if (_parser_prefix("--", keyword) || _parser_prefix("-", keyword)) {
            _parser_append_list(&parser->optional_args, element);
        } else {
            _parser_append_list(&parser->positional_args, element);
        }
    }

    parser_result_t parser_init(parser_t** parser) {
        parser_t* temp = (parser_t*)malloc(sizeof(parser_t));
        parser_flag_arg_t* help_arg = (parser_flag_arg_t*)malloc(sizeof(parser_flag_arg_t));
        if (temp == NULL) {
            return PARSER_RESULT_ERROR;
        }
        if (help_arg == NULL) {
            free(temp);
            return PARSER_RESULT_ERROR;
        }

        temp->last_err = NULL;
        temp->last_err_pos = 0;
        temp->last_err_size = 0;
        temp->help_arg = help_arg;
        temp->optional_args = NULL;
        temp->positional_args = NULL;

        _parser_add_arg(temp, (parser_base_arg_t*)help_arg, "--help", NULL);
        _parser_set_alt((parser_base_arg_t*)help_arg, "-h");
        _parser_set_help((parser_base_arg_t*)help_arg, "show this help message and exit");

        *parser = temp;
        return PARSER_RESULT_OK;
    }

    parser_result_t parser_free(parser_t** parser) {
        parser_t* temp = *parser;
        if (temp == NULL) {
            return PARSER_RESULT_ERROR;
        }

        _parser_free_list(&temp->positional_args);
        _parser_free_list(&temp->optional_args);
        if (temp->last_err != NULL) {
            free(temp->last_err);
        }

        free(temp);
        *parser = NULL;
        return PARSER_RESULT_OK;
    }

    parser_result_t parser_parse(parser_t* parser, int argc, char** argv) {
        parser_base_arg_t* current_optional = NULL;
        parser_base_arg_t* current_positional = parser->positional_args;

        parser->argc = argc;
        parser->argv = argv;

        for (int i=1; i < argc; ++i) {
            if (current_optional != NULL) {
                if (current_optional->set_value != NULL) {
                    current_optional->set_value(current_optional, argv[i]);
                }
                current_optional->is_filled = true;
                current_optional = NULL;
                continue;
            }

            if (_parser_prefix("-", argv[i])) {
                if (_parser_prefix("--", argv[i])) {
                    current_optional = parser->optional_args;
                    while (current_optional != NULL) {
                        if (current_optional->keyword != NULL && strcmp(current_optional->keyword, argv[i]) == 0) {
                            break;
                        }
                        current_optional = (parser_base_arg_t*)current_optional->next;
                    }
                } else {
                    current_optional = parser->optional_args;
                    while (current_optional != NULL) {
                        if (current_optional->keyshort != NULL && strcmp(current_optional->keyshort, argv[i]) == 0) {
                            break;
                        }
                        current_optional = (parser_base_arg_t*)current_optional->next;
                    }
                }

                if (current_optional == NULL) {
                    _parser_set_optional_error_message(parser, argv[i]);
                    return PARSER_RESULT_ERROR;
                }

                if (current_optional->set_value == NULL) {
                    current_optional->is_filled = true;
                }

                continue;
            }

            if (current_positional != NULL) {
                current_positional->set_value(current_positional, argv[i]);
                current_positional->is_filled = true;
                current_positional = (parser_base_arg_t*)current_positional->next;
            }
        }

        if (_parser_is_filled((parser_base_arg_t*)parser->help_arg)) {
            _parser_set_help_message(parser);
            return PARSER_RESULT_HELP;
        }

        if (current_positional != NULL) {
            _parser_set_positional_error_message(parser, current_positional);
            return PARSER_RESULT_ERROR;
        }

        return PARSER_RESULT_OK;
    }

    const char* parser_get_last_err(parser_t* parser) {
        return parser->last_err;
    }



    parser_result_t parser_flag_add_arg(parser_t* parser, parser_flag_arg_t** arg, char const * keyword) {
        parser_flag_arg_t* temp = (parser_flag_arg_t*)malloc(sizeof(parser_flag_arg_t));
        if (temp == NULL) {
            return PARSER_RESULT_ERROR;
        }

        _parser_add_arg(parser, (parser_base_arg_t*)temp, keyword, NULL);

        *arg = temp;
        return PARSER_RESULT_OK;
    }

    bool parser_flag_is_filled(parser_flag_arg_t* arg) {
        return _parser_is_filled((parser_base_arg_t*)arg);
    }

    void parser_flag_set_alt(parser_flag_arg_t* arg, char const * alt) {
        _parser_set_alt((parser_base_arg_t*)arg, alt);
    }

    void parser_flag_set_help(parser_flag_arg_t* arg, char const * help) {
        _parser_set_help((parser_base_arg_t*)arg, help);
    }



    void _parser_set_int_value(void* element, char const * value) {
        char* end_ptr;
        ((parser_int_arg_t*)element)->value = strtol(value, &end_ptr, 10);
    }

    parser_result_t parser_int_add_arg(parser_t* parser, parser_int_arg_t** arg, char const * keyword) {
        parser_int_arg_t* temp = (parser_int_arg_t*)malloc(sizeof(parser_int_arg_t));
        if (temp == NULL) {
            return PARSER_RESULT_ERROR;
        }

        _parser_add_arg(parser, (parser_base_arg_t*)temp, keyword, _parser_set_int_value);
        temp->default_value = 0;
        temp->value = 0;

        *arg = temp;
        return PARSER_RESULT_OK;
    }

    int parser_int_get_value(parser_int_arg_t* arg) {
        if (_parser_is_filled((parser_base_arg_t*)arg)) {
            return arg->value;
        }
        return arg->default_value;
    }

    bool parser_int_is_filled(parser_int_arg_t* arg) {
        return _parser_is_filled((parser_base_arg_t*)arg);
    }

    void parser_int_set_alt(parser_int_arg_t* arg, char const * alt) {
        _parser_set_alt((parser_base_arg_t*)arg, alt);
    }

    void parser_int_set_help(parser_int_arg_t* arg, char const * help) {
        _parser_set_help((parser_base_arg_t*)arg, help);
    }

    void parser_int_set_default(parser_int_arg_t* arg, int default_value) {
        arg->default_value = default_value;
    }



    void _parser_set_string_value(void* element, char const * value) {
        ((parser_string_arg_t*)element)->value = value;
    }

    parser_result_t parser_string_add_arg(parser_t* parser, parser_string_arg_t** arg, char const * keyword) {
        parser_string_arg_t* temp = (parser_string_arg_t*)malloc(sizeof(parser_string_arg_t));
        if (temp == NULL) {
            return PARSER_RESULT_ERROR;
        }

        _parser_add_arg(parser, (parser_base_arg_t*)temp, keyword, _parser_set_string_value);
        temp->default_value = "";
        temp->value = "";

        *arg = temp;
        return PARSER_RESULT_OK;
    }

    const char* parser_string_get_value(parser_string_arg_t* arg) {
        if (_parser_is_filled((parser_base_arg_t*)arg)) {
            return arg->value;
        }
        return arg->default_value;
    }

    bool parser_string_is_filled(parser_string_arg_t* arg) {
        return _parser_is_filled((parser_base_arg_t*)arg);
    }

    void parser_string_set_alt(parser_string_arg_t* arg, char const * alt) {
        _parser_set_alt((parser_base_arg_t*)arg, alt);
    }

    void parser_string_set_help(parser_string_arg_t* arg, char const * help) {
        _parser_set_help((parser_base_arg_t*)arg, help);
    }

    void parser_string_set_default(parser_string_arg_t* arg, char const * default_value) {
        arg->default_value = default_value;
    }

#ifdef __cplusplus
}
#endif

#endif // ARGPARSE_H
