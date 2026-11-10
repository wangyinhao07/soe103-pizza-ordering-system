/*
 * File Name   : test_utils.c
 * Module      : Unit tests for utils.c
 * Author      : En (Member A, team leader)
 * Description : Checks valid, invalid, and boundary inputs for parsing,
 *               validation, and keyboard helpers. Keyboard tests write a
 *               small file and reopen it as stdin, so nobody has to type.
 *               Run from the project root:  make test
 * Version     : 1.0  2026-11-10
 *
 * Test ids such as T-UT-01 match the Testing table in docs/design_draft.md.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"

static int g_passed = 0;
static int g_failed = 0;

/* Extra parentheses are required when the condition itself contains a comma. */
#define CHECK(cond)                                                         \
    do {                                                                    \
        if (cond) {                                                         \
            g_passed++;                                                     \
        } else {                                                            \
            g_failed++;                                                     \
            printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);         \
        }                                                                   \
    } while (0)

#define TMP_INPUT "tests/tmp_stdin.txt"

/* Pretend the user typed `text`. The next read_* call consumes it. */
static void feed_stdin(const char *text)
{
    FILE *fp;

    fp = fopen(TMP_INPUT, "w");
    if (fp == NULL) {
        printf("FAIL: cannot write %s\n", TMP_INPUT);
        exit(EXIT_FAILURE);
    }
    if (fputs(text, fp) < 0 || fclose(fp) != 0) {
        printf("FAIL: cannot write %s\n", TMP_INPUT);
        exit(EXIT_FAILURE);
    }
    if (freopen(TMP_INPUT, "r", stdin) == NULL) {
        printf("FAIL: cannot reopen stdin\n");
        exit(EXIT_FAILURE);
    }
}

static void test_parse_money(void)
{
    Money m;
    char buf[32];

    printf("\n-- parse_money / format_money --\n");

    /* T-UT-01 */
    m = -1;
    CHECK((parse_money("12.5", &m) == ST_OK));
    CHECK((m == 1250));

    CHECK((parse_money("12", &m) == ST_OK && m == 1200));
    CHECK((parse_money("12.50", &m) == ST_OK && m == 1250));
    CHECK((parse_money("0.01", &m) == ST_OK && m == 1));
    CHECK((parse_money("0.1", &m) == ST_OK && m == 10));
    CHECK((parse_money("0.10", &m) == ST_OK && m == 10));
    CHECK((parse_money("00012.50", &m) == ST_OK && m == 1250));
    CHECK((parse_money("  12.50  ", &m) == ST_OK && m == 1250));
    CHECK((parse_money("0", &m) == ST_OK && m == 0));
    CHECK((parse_money("0.00", &m) == ST_OK && m == 0));

    /* T-UT-02 and the other rejected forms from the test plan. */
    m = -999;
    CHECK((parse_money("12abc", &m) == ST_ERR_INPUT && m == -999));
    CHECK((parse_money("12.505", &m) == ST_ERR_INPUT && m == -999));
    CHECK((parse_money("-1", &m) == ST_ERR_INPUT && m == -999));
    CHECK((parse_money("abc", &m) == ST_ERR_INPUT && m == -999));
    CHECK((parse_money("", &m) == ST_ERR_INPUT && m == -999));
    CHECK((parse_money("1e5", &m) == ST_ERR_INPUT && m == -999));
    CHECK((parse_money("12.", &m) == ST_ERR_INPUT && m == -999));
    CHECK((parse_money(".5", &m) == ST_ERR_INPUT && m == -999));
    CHECK((parse_money("+1.00", &m) == ST_ERR_INPUT && m == -999));
    CHECK((parse_money("1.2.3", &m) == ST_ERR_INPUT && m == -999));
    CHECK((parse_money("12 .50", &m) == ST_ERR_INPUT && m == -999));
    CHECK((parse_money(NULL, &m) == ST_ERR_INPUT && m == -999));
    CHECK((parse_money("1.00", NULL) == ST_ERR_INPUT));

    /* 8-digit yuan would risk a 32-bit long when converted to fen. */
    CHECK((parse_money("10000000", &m) == ST_ERR_INPUT));
    CHECK((parse_money("9999999.99", &m) == ST_OK && m == 999999999L));

    format_money(1250, buf, sizeof buf);
    CHECK((strcmp(buf, "12.50") == 0));
    format_money(0, buf, sizeof buf);
    CHECK((strcmp(buf, "0.00") == 0));
    format_money(5, buf, sizeof buf);
    CHECK((strcmp(buf, "0.05") == 0));
    format_money(50, buf, sizeof buf);
    CHECK((strcmp(buf, "0.50") == 0));
    format_money(-150, buf, sizeof buf);
    CHECK((strcmp(buf, "-1.50") == 0));
    format_money(-5, buf, sizeof buf);
    CHECK((strcmp(buf, "-0.05") == 0));

    /* Round trip: text -> fen -> text. */
    CHECK((parse_money("12.50", &m) == ST_OK));
    format_money(m, buf, sizeof buf);
    CHECK((strcmp(buf, "12.50") == 0));

    /* A tiny buffer must not crash. */
    format_money(1250, buf, 1);
    CHECK((buf[0] == '\0'));
    format_money(1250, NULL, 8);
}

static void test_trim_and_validators(void)
{
    char text[64];
    char name[40];
    char phone[20];
    char password[24];
    int i;

    printf("\n-- trim / name / phone / password / date --\n");

    snprintf(text, sizeof text, "%s", "  hello  ");
    trim(text);
    CHECK((strcmp(text, "hello") == 0));

    snprintf(text, sizeof text, "%s", "\t\n  x \r\n");
    trim(text);
    CHECK((strcmp(text, "x") == 0));

    snprintf(text, sizeof text, "%s", "nospaces");
    trim(text);
    CHECK((strcmp(text, "nospaces") == 0));

    snprintf(text, sizeof text, "%s", "   ");
    trim(text);
    CHECK((strcmp(text, "") == 0));

    text[0] = '\0';
    trim(text);
    CHECK((strcmp(text, "") == 0));
    trim(NULL); /* must not crash */

    CHECK((is_valid_name("Alice Zhang") == 1));
    CHECK((is_valid_name("A") == 1));
    CHECK((is_valid_name("") == 0));
    CHECK((is_valid_name("   ") == 0));          /* blank, not a name */
    CHECK((is_valid_name(" Alice") == 1));       /* a space is allowed */
    CHECK((is_valid_name("Alice ") == 1));
    CHECK((is_valid_name("A|B") == 0));
    CHECK((is_valid_name(NULL) == 0));

    /* Boundary: 31 characters fit in NAME_LEN, 32 do not. */
    for (i = 0; i < 31; i++) {
        name[i] = 'A';
    }
    name[31] = '\0';
    CHECK((is_valid_name(name) == 1));
    name[31] = 'A';
    name[32] = '\0';
    CHECK((is_valid_name(name) == 0));

    CHECK((is_valid_phone("12345") == 1));          /* shortest, 5 */
    CHECK((is_valid_phone("1234") == 0));           /* one too short */
    CHECK((is_valid_phone("13800000001") == 1));
    CHECK((is_valid_phone("123456789012345") == 1));  /* longest, 15 */
    CHECK((is_valid_phone("1234567890123456") == 0)); /* 16, too long */
    CHECK((is_valid_phone("1234a") == 0));
    CHECK((is_valid_phone("") == 0));
    CHECK((is_valid_phone(NULL) == 0));

    CHECK((is_valid_password("abc123") == 1));            /* shortest, 6 */
    CHECK((is_valid_password("abc12") == 0));             /* 5 */
    CHECK((is_valid_password("admin123") == 1));
    CHECK((is_valid_password("abcdefghijklmnop") == 1));  /* 16 */
    CHECK((is_valid_password("abcdefghijklmnopq") == 0)); /* 17 */
    CHECK((is_valid_password("abc 123") == 0));
    CHECK((is_valid_password("abc|123") == 0));
    CHECK((is_valid_password(NULL) == 0));

    /* Build the 16/17 character passwords a second way, so a miscount
     * in the literal above cannot hide a boundary bug. */
    for (i = 0; i < 16; i++) {
        password[i] = 'a';
    }
    password[16] = '\0';
    CHECK((strlen(password) == 16 && is_valid_password(password) == 1));
    password[16] = 'a';
    password[17] = '\0';
    CHECK((strlen(password) == 17 && is_valid_password(password) == 0));

    for (i = 0; i < 15; i++) {
        phone[i] = '1';
    }
    phone[15] = '\0';
    CHECK((strlen(phone) == 15 && is_valid_phone(phone) == 1));

    CHECK((is_valid_date("2026-11-10") == 1));
    CHECK((is_valid_date("2026-01-31") == 1));
    CHECK((is_valid_date("2026-02-28") == 1));
    CHECK((is_valid_date("2024-02-29") == 1)); /* leap year */
    CHECK((is_valid_date("2000-02-29") == 1)); /* century leap year */
    CHECK((is_valid_date("2026-02-29") == 0)); /* 2026 is not a leap year */
    CHECK((is_valid_date("2026-04-31") == 0));
    CHECK((is_valid_date("2026-11-31") == 0));
    CHECK((is_valid_date("2026-00-10") == 0));
    CHECK((is_valid_date("2026-13-01") == 0));
    CHECK((is_valid_date("2026-01-00") == 0));
    CHECK((is_valid_date("1999-01-01") == 0));
    CHECK((is_valid_date("2100-01-01") == 1));
    CHECK((is_valid_date("2100-02-29") == 0));   /* 2100 is not a leap year */
    CHECK((is_valid_date("2026/11/10") == 0));
    CHECK((is_valid_date("20261110") == 0));
    CHECK((is_valid_date("") == 0));
    CHECK((is_valid_date(NULL) == 0));
}

static void test_datetime(void)
{
    char when[DATETIME_LEN];
    char date[DATE_LEN];
    char tiny[8];

    printf("\n-- get_current_datetime --\n");

    get_current_datetime(when, sizeof when);
    CHECK((strlen(when) == 16));
    CHECK((when[4] == '-' && when[7] == '-' && when[10] == ' '));
    CHECK((when[13] == ':'));

    /* The first 10 characters are a date the shop can store.
     * Copy them by hand: when[] is longer than date[]. */
    memcpy(date, when, 10);
    date[10] = '\0';
    CHECK((is_valid_date(date) == 1));

    get_current_datetime(tiny, sizeof tiny);
    CHECK((strlen(tiny) < sizeof tiny));
    get_current_datetime(NULL, 8);
}

static void test_read_line_and_int(void)
{
    char buf[8];
    char big[512];
    int value;
    int i;

    printf("\n-- read_line / read_int --\n");

    CHECK((read_line(">", NULL, 8) == ST_ERR_INPUT));
    CHECK((read_line(">", buf, 1) == ST_ERR_INPUT));
    CHECK((read_int("n: ", 5, 1, &value) == ST_ERR_INPUT)); /* min > max */
    CHECK((read_int("n: ", 0, 1, NULL) == ST_ERR_INPUT));

    feed_stdin("  hello world  \n");
    CHECK((read_line(">", big, sizeof big) == ST_OK));
    CHECK((strcmp(big, "hello world") == 0));

    /* size 8 holds 6 characters plus the newline plus '\0'. */
    feed_stdin("123456\n");
    CHECK((read_line(">", buf, 8) == ST_OK));
    CHECK((strcmp(buf, "123456") == 0));

    /* 7 characters do not fit. The following line must still be readable. */
    feed_stdin("1234567\nnext\n");
    CHECK((read_line(">", buf, 8) == ST_ERR_INPUT));
    CHECK((read_line(">", buf, 8) == ST_OK));
    CHECK((strcmp(buf, "next") == 0));

    /* Last line of a file often has no newline. That is still a real line. */
    feed_stdin("hi");
    CHECK((read_line(">", big, sizeof big) == ST_OK));
    CHECK((strcmp(big, "hi") == 0));
    CHECK((read_line(">", big, sizeof big) == ST_EOF));

    feed_stdin("");
    CHECK((read_int("n: ", 0, 10, &value) == ST_EOF));

    /* T-UT style: letters, a decimal, a blank, then a good value. */
    feed_stdin("abc\n1.5\n\n8\n");
    CHECK((read_int("n: ", 0, 10, &value) == ST_OK));
    CHECK((value == 8));

    feed_stdin("12abc\n7\n");
    CHECK((read_int("n: ", 0, 10, &value) == ST_OK));
    CHECK((value == 7));

    /* Boundaries of the allowed range, after two values that are outside. */
    feed_stdin("-1\n11\n0\n");
    CHECK((read_int("n: ", 0, 10, &value) == ST_OK));
    CHECK((value == 0));

    feed_stdin("10\n");
    CHECK((read_int("n: ", 0, 10, &value) == ST_OK));
    CHECK((value == 10));

    feed_stdin("   7\n");
    CHECK((read_int("n: ", 0, 10, &value) == ST_OK));
    CHECK((value == 7));

    /* Huge numbers must be rejected, not wrapped around. */
    feed_stdin("99999999999999\n4\n");
    CHECK((read_int("n: ", 0, 10, &value) == ST_OK));
    CHECK((value == 4));

    /* A 400-character line is discarded. The next line is the real answer. */
    for (i = 0; i < 400; i++) {
        big[i] = 'x';
    }
    big[400] = '\n';
    memcpy(big + 401, "42\n", 4);
    big[405] = '\0';
    feed_stdin(big);
    CHECK((read_int("n: ", 0, 100, &value) == ST_OK));
    CHECK((value == 42));

    /* Garbage and then end of file must return, not retry forever. */
    feed_stdin("abc\n");
    CHECK((read_int("n: ", 0, 10, &value) == ST_EOF));
}

static void test_read_money_and_yes(void)
{
    Money amount;
    int answer;

    printf("\n-- read_money / read_yes_no --\n");

    CHECK((read_money("Amount: ", 10, 1, &amount) == ST_ERR_INPUT));

    feed_stdin("12.5\n");
    CHECK((read_money("Amount: ", 0, 5000, &amount) == ST_OK));
    CHECK((amount == 1250));

    /* 0 cancels only when 0 is below the minimum (a deposit, for example). */
    feed_stdin("0\n");
    CHECK((read_money("Amount: ", MIN_DEPOSIT, MAX_DEPOSIT, &amount)
           == ST_CANCEL));
    feed_stdin("0.00\n");
    CHECK((read_money("Amount: ", MIN_DEPOSIT, MAX_DEPOSIT, &amount)
           == ST_CANCEL));

    /* 0.99 is under the 1.00 minimum; 5000.01 is over the maximum;
     * 1.00 and 5000.00 are the two boundaries that must be accepted. */
    feed_stdin("0.99\n5000.01\n1.00\n");
    CHECK((read_money("Amount: ", MIN_DEPOSIT, MAX_DEPOSIT, &amount) == ST_OK));
    CHECK((amount == MIN_DEPOSIT));

    feed_stdin("5000.00\n");
    CHECK((read_money("Amount: ", MIN_DEPOSIT, MAX_DEPOSIT, &amount) == ST_OK));
    CHECK((amount == MAX_DEPOSIT));

    feed_stdin("abc\n12.505\n1e5\n-1\n5.00\n");
    CHECK((read_money("Amount: ", MIN_DEPOSIT, MAX_DEPOSIT, &amount) == ST_OK));
    CHECK((amount == 500));

    feed_stdin("");
    CHECK((read_money("Amount: ", MIN_DEPOSIT, MAX_DEPOSIT, &amount) == ST_EOF));

    /* When the minimum is 0, zero is a real amount and must not cancel. */
    feed_stdin("0\n");
    CHECK((read_money("Amount: ", 0, 100, &amount) == ST_OK));
    CHECK((amount == 0));

    feed_stdin("maybe\nY\n");
    answer = read_yes_no("ok? ");
    CHECK((answer == 1));

    feed_stdin("  n  \n");
    answer = read_yes_no("ok? ");
    CHECK((answer == 0));

    feed_stdin("yes\nNO\nn\n");
    answer = read_yes_no("ok? ");
    CHECK((answer == 0));

    feed_stdin("");
    answer = read_yes_no("ok? ");
    CHECK((answer == 0)); /* EOF counts as no, and must return */

    /* pause must return on Enter and also on end of input. */
    feed_stdin("\n");
    pause_for_enter();
    feed_stdin("");
    pause_for_enter();
    CHECK((1));
}

int main(void)
{
    printf("=== test_utils ===\n");
    test_parse_money();
    test_trim_and_validators();
    test_datetime();
    test_read_line_and_int();
    test_read_money_and_yes();

    if (remove(TMP_INPUT) != 0) {
        /* The file may already be gone. That is not a test failure. */
    }

    printf("\ntest_utils: %d passed, %d failed\n", g_passed, g_failed);
    return g_failed == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
