/*
 * Tests for private helper functions
 */

#undef NDEBUG
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "c-shquote-private.h"
#include "c-shquote.h"

static void test_append_str(void) {
        const char *string = "foo";
        char buf[strlen(string)];
        char *out = buf;
        size_t n_out = sizeof(buf);
        int r;

        r = c_shquote_append_str(&out, &n_out, string, strlen(string));
        assert(!r);
        assert(!memcmp(buf, string, strlen(string)));
        assert(out == buf + strlen(string));
        assert(n_out == sizeof(buf) - strlen(string));

        r = c_shquote_append_str(&out, &n_out, string, 1);
        assert(r == C_SHQUOTE_E_NO_SPACE);
}

static void test_append_char(void) {
        char c = 'c';
        char buf[1];
        char *out = buf;
        size_t n_out = sizeof(buf);
        int r;

        r = c_shquote_append_char(&out, &n_out, c);
        assert(!r);
        assert(buf[0] = c);
        assert(out == buf + 1);
        assert(n_out == sizeof(buf) - 1);

        r = c_shquote_append_char(&out, &n_out, c);
        assert(r == C_SHQUOTE_E_NO_SPACE);
}

static void test_skip_str(void) {
        const char *string = "foo";
        const char *in = string;
        size_t n_in = strlen(string);

        c_shquote_skip_str(&in, &n_in, 2);
        assert(in == string + 2);
        assert(n_in == strlen(string) - 2);

        c_shquote_skip_str(&in, &n_in, 1);
        assert(in == string + 3);
        assert(n_in == strlen(string) - 3);
}

static void test_skip_char(void) {
        const char *string = "foo";
        const char *in = string;
        size_t n_in = strlen(string);

        c_shquote_skip_char(&in, &n_in);
        assert(in == string + 1);
        assert(n_in == strlen(string) - 1);

        c_shquote_skip_char(&in, &n_in);
        assert(in == string + 2);
        assert(n_in == strlen(string) - 2);

        c_shquote_skip_char(&in, &n_in);
        assert(in == string + 3);
        assert(n_in == strlen(string) - 3);
}

static void test_consume_str(void) {
        const char *string = "foo";
        char buf[strlen(string)];
        char *out = buf;
        size_t n_out = sizeof(buf);
        const char *in = string;
        size_t n_in = strlen(string);
        int r;

        r = c_shquote_consume_str(&out, &n_out, &in, &n_in, 2);
        assert(!r);
        assert(!memcmp(buf, string, 2));
        assert(out == buf + 2);
        assert(n_out == sizeof(buf) - 2);
        assert(in == string + 2);
        assert(n_in == strlen(string) - 2);

        r = c_shquote_consume_str(&out, &n_out, &in, &n_in, 1);
        assert(!r);
        assert(!memcmp(buf, string, 3));
        assert(out == buf + 3);
        assert(n_out == sizeof(buf) - 3);
        assert(in == string + 3);
        assert(n_in == strlen(string) - 3);

        in = string;
        n_in = 1;

        r = c_shquote_consume_str(&out, &n_out, &in, &n_in, 1);
        assert(r == C_SHQUOTE_E_NO_SPACE);

        out = buf;
        n_out = sizeof(buf);

        r = c_shquote_consume_str(&out, &n_out, &in, &n_in, 2);
        assert(r < 0);
}

static void test_consume_char(void) {
        const char *string = "a";
        char buf[strlen(string)];
        char *out = buf;
        size_t n_out = sizeof(buf);
        const char *in = string;
        size_t n_in = strlen(string);
        int r;

        r = c_shquote_consume_char(&out, &n_out, &in, &n_in);
        assert(!r);
        assert(*buf == *string);
        assert(out == buf + 1);
        assert(n_out == sizeof(buf) - 1);
        assert(in == string + 1);
        assert(n_in == strlen(string) - 1);
}

static void test_strnspn(void) {
        size_t len;

        len = c_shquote_strnspn(NULL, 0, "a");
        assert(len == 0);

        len = c_shquote_strnspn("a", 1, "");
        assert(len == 0);

        len = c_shquote_strnspn("ab", 2, "ac");
        assert(len == 1);

        len = c_shquote_strnspn("ab", 2, "bc");
        assert(len == 0);
}

static void test_strncspn(void) {
        size_t len;

        len = c_shquote_strncspn(NULL, 0, "a");
        assert(len == 0);

        len = c_shquote_strncspn("a", 1, "");
        assert(len == 1);

        len = c_shquote_strncspn("ab", 2, "ac");
        assert(len == 0);

        len = c_shquote_strncspn("ab", 2, "bc");
        assert(len == 1);

        len = c_shquote_strncspn("ab", 2, "cd");
        assert(len == 2);
}

static void test_discard_comment(void) {
        const char *string = "#foo\\\n";
        const char *comment;
        size_t n_comment;

        comment = string;
        n_comment = strlen(comment);
        c_shquote_discard_comment(&comment, &n_comment);
        assert(n_comment == 1);
        assert(comment == string + strlen(string) - 1);

        comment = string;
        n_comment = strlen(comment) - 1;
        c_shquote_discard_comment(&comment, &n_comment);
        assert(n_comment == 0);
        assert(comment == string + strlen(string) - 1);
}

static void test_discard_whitespace(void) {
        const char *string = "a \t\nb";
        const char *whitespace;
        size_t n_whitespace;

        whitespace = string;
        n_whitespace = strlen(whitespace);
        c_shquote_discard_whitespace(&whitespace, &n_whitespace);
        assert(n_whitespace == strlen(whitespace));
        assert(whitespace == string);

        whitespace = string + 1;
        n_whitespace = strlen(whitespace);
        c_shquote_discard_whitespace(&whitespace, &n_whitespace);
        assert(n_whitespace == 1);
        assert(whitespace == string + strlen(string) - 1);

        whitespace = string + 1;
        n_whitespace = strlen(whitespace) - 1;
        c_shquote_discard_whitespace(&whitespace, &n_whitespace);
        assert(n_whitespace == 0);
        assert(whitespace == string + strlen(string) - 1);
}

static void test_unescape_char_quoted_one(const char *string, size_t n_string, bool escaped) {
        char buf[2];
        char *out = buf;
        size_t n_out = sizeof(buf);
        const char *in = string;
        size_t n_in = n_string;
        int r;

        r = c_shquote_unescape_char_quoted(&out, &n_out, &in, &n_in);
        assert(!r);
        assert(in == string + 2);
        assert(n_in == n_string - 2);

        if (escaped) {
                assert(out == buf + 1);
                assert(n_out == sizeof(buf) - 1);
                assert(buf[0] == string[1]);
        } else {
                assert(out == buf + 2);
                assert(n_out == sizeof(buf) - 2);
                assert(!memcmp(buf, string, 2));
        }
}

static void test_unescape_char_quoted(void) {
        const char *string = "\\a";
        const char *in = string;
        size_t n_in = strlen(string);
        char *out = NULL;
        size_t n_out = 0;
        int r;

        r = c_shquote_unescape_char_quoted(&out, &n_out, &in, &n_in);
        assert(r == C_SHQUOTE_E_NO_SPACE);
        assert(in == string);
        assert(n_in == strlen(string));

        --n_in;
        r = c_shquote_unescape_char_quoted(&out, &n_out, &in, &n_in);
        assert(r == C_SHQUOTE_E_BAD_QUOTING);
        assert(in == string);
        assert(n_in == strlen(string) - 1);

        test_unescape_char_quoted_one("\\a", 2, false);
        test_unescape_char_quoted_one("\\\"", 2, true);
        test_unescape_char_quoted_one("\\\\", 2, true);
        test_unescape_char_quoted_one("\\`", 2, true);
        test_unescape_char_quoted_one("\\$", 2, true);
        test_unescape_char_quoted_one("\\\n", 2, true);
}

static void test_unescape_char_unquoted_one(const char *string, size_t n_string, bool escaped) {
        char buf[1];
        char *out = buf;
        size_t n_out = sizeof(buf);
        const char *in = string;
        size_t n_in = n_string;
        int r;

        r = c_shquote_unescape_char_unquoted(&out, &n_out, &in, &n_in);
        assert(!r);

        if (escaped) {
                assert(in == string + 2);
                assert(n_in == n_string - 2);
                assert(out == buf);
                assert(n_out == sizeof(buf));
        } else {
                if (n_string == 1) {
                        assert(in == string + 1);
                        assert(n_in == n_string - 1);
                        assert(out == buf);
                        assert(n_out == sizeof(buf));
                } else {
                        assert(in == string + 2);
                        assert(n_in == n_string - 2);
                        assert(out == buf + 1);
                        assert(n_out == sizeof(buf) - 1);
                        assert(buf[0] == string[1]);
                }
        }
}

static void test_unescape_char_unquoted(void) {
        const char *string = "\\a";
        const char *in = string;
        size_t n_in = strlen(string);
        char *out = NULL;
        size_t n_out = 0;
        int r;

        r = c_shquote_unescape_char_unquoted(&out, &n_out, &in, &n_in);
        assert(r == C_SHQUOTE_E_NO_SPACE);
        assert(in == string);
        assert(n_in == strlen(string));

        test_unescape_char_unquoted_one("\\", 1, false);
        test_unescape_char_unquoted_one("\\a", 2, false);
        test_unescape_char_unquoted_one("\\\n", 2, true);
}

static void test_unquote_single(void) {
        const char *string = "'fo\"obar'";
        char buf[strlen(string)];
        const char *in = string;
        size_t n_in = strlen(string);
        char *out;
        size_t n_out;
        int r;

        out = NULL;
        n_out = 0;
        r = c_shquote_unquote_single(&out, &n_out, &in, &n_in);
        assert(r == C_SHQUOTE_E_NO_SPACE);
        assert(in == string);
        assert(n_in == strlen(string));

        out = buf;
        n_out = sizeof(buf);
        n_in = strlen(string) - 1;
        r = c_shquote_unquote_single(&out, &n_out, &in, &n_in);
        assert(r == C_SHQUOTE_E_BAD_QUOTING);
        assert(in == string);
        assert(n_in == strlen(string) - 1);

        n_in = strlen(string);
        r = c_shquote_unquote_single(&out, &n_out, &in, &n_in);
        assert(!r);
        assert(in == string + strlen(string));
        assert(!n_in);
        assert(out == buf + sizeof(buf) - 2);
        assert(n_out == 2);
        assert(!memcmp(buf, string + 1, strlen(string) - 2));
}

static void test_unquote_double(void) {
        const char *string = "\"fo\\\"obar\"";
        char buf[strlen(string)];
        const char *in = string;
        size_t n_in = strlen(string);
        char *out;
        size_t n_out;
        int r;

        out = NULL;
        n_out = 0;
        r = c_shquote_unquote_double(&out, &n_out, &in, &n_in);
        assert(r == C_SHQUOTE_E_NO_SPACE);
        assert(in == string);
        assert(n_in == strlen(string));

        out = buf;
        n_out = sizeof(buf);
        n_in = strlen(string) - 1;
        r = c_shquote_unquote_double(&out, &n_out, &in, &n_in);
        assert(r == C_SHQUOTE_E_BAD_QUOTING);
        assert(in == string);
        assert(n_in == strlen(string) - 1);

        n_in = strlen(string);
        r = c_shquote_unquote_double(&out, &n_out, &in, &n_in);
        assert(!r);
        assert(in == string + strlen(string));
        assert(!n_in);
        assert(out == buf + sizeof(buf) - 3);
        assert(n_out == 3);
        assert(!memcmp(buf, "fo\"obar", strlen(string) - 3));
}

int main(int argc, char *argv[]) {
        test_append_str();
        test_append_char();
        test_skip_str();
        test_skip_char();
        test_consume_str();
        test_consume_char();
        test_strnspn();
        test_strncspn();
        test_discard_comment();
        test_discard_whitespace();
        test_unescape_char_quoted();
        test_unescape_char_unquoted();
        test_unquote_single();
        test_unquote_double();
        return 0;
}
