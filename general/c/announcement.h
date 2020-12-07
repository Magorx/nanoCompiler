#ifndef GENERAL_ANNOUNCMENT
#define GENERAL_ANNOUNCMENT

#include <string.h>
#include <cstdio>

const char *THE_LAST_ANNOUNCER     = nullptr;
int         THE_LAST_ANNOUNCER_LEN = 0 ;

const char *THE_LAST_CODE          = nullptr;
int         THE_LAST_CODE_LEN      = 0 ;


#define ANNOUNCEMENT(fileptr, code, announcer, format, ...)                                               \
    do {                                                                                                  \
        bool to_print_code      = false;                                                                  \
        bool to_print_announcer = false;                                                                  \
                                                                                                          \
        if ((announcer) && (!(THE_LAST_ANNOUNCER) || strcmp(THE_LAST_ANNOUNCER, (announcer)))) {          \
            THE_LAST_ANNOUNCER = (const char *)(announcer);                                               \
            THE_LAST_ANNOUNCER_LEN = strlen((announcer));                                                 \
            to_print_code = true;                                                                         \
            to_print_announcer = true;                                                                    \
        }                                                                                                 \
                                                                                                          \
        if ((code) && (!(THE_LAST_CODE) || strcmp(THE_LAST_CODE, (code)))) {                              \
            THE_LAST_CODE = (const char *)(code);                                                         \
            THE_LAST_CODE_LEN = strlen((code));                                                           \
            to_print_code = true;                                                                         \
            to_print_announcer = true;                                                                    \
        }                                                                                                 \
                                                                                                          \
        if (to_print_code) {                                                                              \
            fprintf((fileptr), "[%s]", (code));                                                           \
        } else {                                                                                          \
            fputc('[', (fileptr));                                                                        \
            for (int i = 0; i < THE_LAST_CODE_LEN; ++i) {                                                 \
                fputc(' ', (fileptr));                                                                    \
            }                                                                                             \
            fputc(']', (fileptr));                                                                        \
        }                                                                                                 \
                                                                                                          \
        if (to_print_announcer) {                                                                         \
            fprintf((fileptr), "<%s>: ", (announcer));                                                    \
        } else {                                                                                          \
            fputc('<', (fileptr));                                                                        \
            for (int i = 0; i < THE_LAST_ANNOUNCER_LEN; ++i) {                                            \
                fputc(' ', (fileptr));                                                                    \
            }                                                                                             \
            fprintf((fileptr), ">: ");                                                                    \
        }                                                                                                 \
                                                                                                          \
        fprintf((fileptr), (format) __VA_OPT__(,) __VA_ARGS__);                                           \
        fprintf((fileptr), "\n");                                                                         \
                                                                                                          \
    } while (0)

#define ANNOUNCE(code, announcer, format, ...) ANNOUNCEMENT(stdout, code, announcer, format, __VA_ARGS__)

#define RAISE_ERROR(err_format, ...) ANNOUNCEMENT(stderr, "ERR", __FUNCTION__, err_format, __VA_ARGS__)

#define RAISE_ERROR_SYNTAX(string, pos)                                                   \
    do {                                                                                  \
		if (pos >= 5) {                                                                   \
			RAISE_ERROR("syntax error: ...%.10s...", string + (pos >= 5 ? pos - 5 : 0));  \
			RAISE_ERROR("                   ~~^~~ ");                                     \
		} else {                                                                          \
			RAISE_ERROR("syntax error: ...%.10s...", string + (pos >= 5 ? pos - 5 : 0));  \
			RAISE_ERROR("                 ~~~~~");                                        \
		}                                                                                 \
	} while(0)

#endif // GENERAL_ANNOUNCMENT