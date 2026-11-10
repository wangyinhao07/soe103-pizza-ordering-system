/*
 * File Name   : utils.c
 * Module      : Safe input and small helpers
 * Author      : En (Member A, team leader)
 * Description : Line-based input. Nothing in this file uses scanf.
 *               A bad line is rejected with a message, and end of input
 *               (Ctrl+D / Ctrl+Z) always returns ST_EOF instead of looping.
 * Version     : 1.0  2026-11-10
 *
 * How a read works
 * ----------------
 * 1. fgets stores one line, but never more characters than the buffer.
 * 2. If the user typed more than that, the rest of the line is thrown
 *    away. The next question then starts on a fresh line.
 * 3. The text is checked. "12abc" is not the number 12: the whole line
 *    must be the number, with nothing left over.
 * 4. Money is stored as an integer number of fen (1 yuan = 100 fen) so
 *    prices do not pick up float rounding errors.
 */

#include <stdio.h>
#include <stdlib.h>   /* strtol, required by the design for whole-line numbers */
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <time.h>

#include "utils.h"

/* Largest whole-yuan value parse_money will accept.
 * 7 digits keeps yuan * 100 inside a 32-bit long. MinGW on Windows
 * still uses a 32-bit long, while Linux uses 64-bit. The shop's real
 * limits (1000.00 price, 99999.99 balance) are much smaller. */
#define MAX_YUAN_DIGITS 9999999L

/*
 * Function : flush_rest_of_line
 * Purpose  : Drop every character up to and including the next newline.
 *            Stops on end of input, so it cannot spin forever.
 */
static void flush_rest_of_line(void)
{
    int ch;

    do {
        ch = getchar();
    } while (ch != '\n' && ch != EOF);
}

/*
 * Function : read_digits
 * Purpose  : Read exactly n digits from s into *out.
 * Returns  : 1 on success, 0 if any character is not a digit.
 */
static int read_digits(const char *s, int n, int *out)
{
    int i;
    int value;

    value = 0;
    for (i = 0; i < n; i++) {
        if (!isdigit((unsigned char)s[i])) {
            return 0;
        }
        value = value * 10 + (s[i] - '0');
    }
    *out = value;
    return 1;
}

/*
 * Function : is_leap_year
 * Purpose  : Return 1 when year has a 29 February.
 *            Rule: divisible by 4, but century years must be divisible by 400.
 */
static int is_leap_year(int year)
{
    if (year % 400 == 0) {
        return 1;
    }
    if (year % 100 == 0) {
        return 0;
    }
    if (year % 4 == 0) {
        return 1;
    }
    return 0;
}

/*
 * Function : days_in_month
 * Purpose  : How many days that month has in that year. 0 if month is invalid.
 */
static int days_in_month(int year, int month)
{
    /* February is listed as 28; the leap-year case adds the extra day. */
    static const int days[12] = {
        31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
    };

    if (month < 1 || month > 12) {
        return 0;
    }
    if (month == 2 && is_leap_year(year)) {
        return 29;
    }
    return days[month - 1];
}

/*
 * Function : trim
 * Purpose  : Remove leading and trailing white space from s, in place.
 */
void trim(char *s)
{
    char *start;
    size_t len;

    if (s == NULL) {
        return;
    }

    /* Skip spaces at the front, then slide the rest back to s[0]. */
    start = s;
    while (*start != '\0' && isspace((unsigned char)*start)) {
        start++;
    }
    if (start != s) {
        len = strlen(start);
        memmove(s, start, len + 1);
    }

    /* Chop spaces at the end by writing a new '\0' earlier in the string. */
    len = strlen(s);
    while (len > 0 && isspace((unsigned char)s[len - 1])) {
        len--;
        s[len] = '\0';
    }
}

/*
 * Function : read_line
 * Purpose  : Read one trimmed line. See utils.h for the return codes.
 */
int read_line(const char *prompt, char *buf, size_t size)
{
    size_t len;

    if (buf == NULL || size < 2 || size > (size_t)INT_MAX) {
        return ST_ERR_INPUT;
    }

    if (prompt != NULL) {
        printf("%s", prompt);
        fflush(stdout); /* the prompt has no newline, so flush it now */
    }

    if (fgets(buf, (int)size, stdin) == NULL) {
        buf[0] = '\0';
        return ST_EOF;
    }

    len = strlen(buf);
    if (len > 0 && buf[len - 1] == '\n') {
        /* Normal case: the whole line, including its newline, fitted. */
        buf[len - 1] = '\0';
    } else if (!feof(stdin)) {
        /* The buffer filled up before the newline. The rest of this
         * line is still sitting in the input, so throw it away. */
        flush_rest_of_line();
        buf[0] = '\0';
        printf("  ! Input too long (max %lu characters).\n",
               (unsigned long)(size - 2));
        return ST_ERR_INPUT;
    }

    /* A last line with no newline (end of file) is accepted as-is. */
    trim(buf);
    return ST_OK;
}

/*
 * Function : read_int
 * Purpose  : Keep asking until the line is an integer in [min, max].
 */
int read_int(const char *prompt, int min, int max, int *out)
{
    char buf[LINE_LEN];
    char *end;
    long value;
    int rc;

    if (out == NULL || min > max) {
        return ST_ERR_INPUT;
    }

    for (;;) {
        rc = read_line(prompt, buf, sizeof buf);
        if (rc == ST_EOF) {
            return ST_EOF;
        }
        if (rc == ST_ERR_INPUT) {
            /* "too long" was already printed. Ask again. */
            continue;
        }
        if (buf[0] == '\0') {
            printf("  ! Please enter a whole number between %d and %d.\n",
                   min, max);
            continue;
        }

        /* Base 10, so "010" is ten and not octal eight.
         * errno is cleared first so a previous error cannot confuse us.
         * The character after the number must be the end of the string,
         * which rejects "12abc" and "1.5". */
        errno = 0;
        value = strtol(buf, &end, 10);
        if (errno != 0 || end == buf || *end != '\0'
            || value < min || value > max) {
            printf("  ! Please enter a whole number between %d and %d.\n",
                   min, max);
            continue;
        }

        /* value is inside an int range, so the cast cannot overflow. */
        *out = (int)value;
        return ST_OK;
    }
}

/*
 * Function : parse_money
 * Purpose  : Turn "12.50" into 1250. See utils.h for what is rejected.
 */
int parse_money(const char *str, Money *out)
{
    const char *p;
    Money yuan;
    int fen;
    int decimals;
    int sawDigit;

    if (str == NULL || out == NULL) {
        return ST_ERR_INPUT;
    }

    p = str;
    while (*p != '\0' && isspace((unsigned char)*p)) {
        p++;
    }
    if (*p == '\0') {
        return ST_ERR_INPUT;
    }

    yuan = 0;
    fen = 0;
    decimals = -1;   /* -1 means "we have not seen a dot yet" */
    sawDigit = 0;

    while (*p != '\0' && !isspace((unsigned char)*p)) {
        if (*p == '.') {
            if (decimals >= 0 || !sawDigit) {
                /* A second dot, or a dot with no digit before it (".5"). */
                return ST_ERR_INPUT;
            }
            decimals = 0;
        } else if (isdigit((unsigned char)*p)) {
            sawDigit = 1;
            if (decimals < 0) {
                if (yuan > MAX_YUAN_DIGITS / 10) {
                    return ST_ERR_INPUT;
                }
                yuan = yuan * 10 + (*p - '0');
            } else {
                decimals++;
                if (decimals > 2) {
                    return ST_ERR_INPUT; /* "12.505" has 3 decimal places */
                }
                fen = fen * 10 + (*p - '0');
            }
        } else {
            /* Letters, a sign, or an exponent ("-1", "1e5", "12abc"). */
            return ST_ERR_INPUT;
        }
        p++;
    }

    while (*p != '\0' && isspace((unsigned char)*p)) {
        p++;
    }
    if (*p != '\0' || !sawDigit || decimals == 0) {
        /* Leftover junk, no digits at all, or a trailing dot ("12."). */
        return ST_ERR_INPUT;
    }

    /* "12.5" means 12 yuan and 50 fen, not 5 fen. */
    if (decimals == 1) {
        fen = fen * 10;
    }

    if (yuan > (LONG_MAX - fen) / 100) {
        return ST_ERR_INPUT;
    }
    *out = yuan * 100 + fen;
    return ST_OK;
}

/*
 * Function : read_money
 * Purpose  : Ask for an amount in range. 0 cancels when min > 0.
 */
int read_money(const char *prompt, Money min, Money max, Money *out)
{
    char buf[LINE_LEN];
    char lo[32];
    char hi[32];
    Money value;
    int rc;

    if (out == NULL || min > max) {
        return ST_ERR_INPUT;
    }

    format_money(min, lo, sizeof lo);
    format_money(max, hi, sizeof hi);

    for (;;) {
        rc = read_line(prompt, buf, sizeof buf);
        if (rc == ST_EOF) {
            return ST_EOF;
        }
        if (rc == ST_ERR_INPUT) {
            continue;
        }
        if (parse_money(buf, &value) != ST_OK) {
            printf("  ! Please enter an amount between %s and %s\n"
                   "    (digits only, at most 2 decimal places).\n",
                   lo, hi);
            continue;
        }
        if (value == 0 && min > 0) {
            return ST_CANCEL;
        }
        if (value < min || value > max) {
            printf("  ! Please enter an amount between %s and %s.\n", lo, hi);
            if (min > 0) {
                printf("    Enter 0 to cancel.\n");
            }
            continue;
        }
        *out = value;
        return ST_OK;
    }
}

/*
 * Function : read_yes_no
 * Purpose  : Accept only y or n. End of input counts as no.
 */
int read_yes_no(const char *prompt)
{
    char buf[LINE_LEN];
    int rc;

    for (;;) {
        rc = read_line(prompt, buf, sizeof buf);
        if (rc == ST_EOF) {
            return 0;
        }
        if (rc == ST_ERR_INPUT) {
            continue;
        }
        if (strcmp(buf, "y") == 0 || strcmp(buf, "Y") == 0) {
            return 1;
        }
        if (strcmp(buf, "n") == 0 || strcmp(buf, "N") == 0) {
            return 0;
        }
        printf("  ! Please enter y or n.\n");
    }
}

/*
 * Function : format_money
 * Purpose  : Write fen as "12.50" (or "-1.50"). Safe with a short buffer.
 */
void format_money(Money amount, char *buf, size_t size)
{
    const char *sign;
    Money yuan;
    Money fen;

    if (buf == NULL || size == 0) {
        return;
    }

    sign = "";
    if (amount < 0) {
        sign = "-";
        /* The most negative long cannot be negated. Show a safe fallback. */
        if (amount == LONG_MIN) {
            snprintf(buf, size, "%s", "-");
            return;
        }
        amount = -amount;
    }

    yuan = amount / 100;
    fen = amount % 100;
    snprintf(buf, size, "%s%ld.%02ld", sign, yuan, fen);
}

/*
 * Function : is_valid_name
 * Purpose  : 1..31 printable characters, not blank, no '|'.
 *            Names may contain a space ("Alice Zhang"). A line that is
 *            only spaces counts as blank.
 */
int is_valid_name(const char *s)
{
    size_t len;
    size_t i;
    int sawLetter;

    if (s == NULL) {
        return 0;
    }
    len = strlen(s);
    if (len == 0 || len > (size_t)NAME_LEN - 1) {
        return 0;
    }

    sawLetter = 0;
    for (i = 0; i < len; i++) {
        if (!isprint((unsigned char)s[i]) || s[i] == FIELD_SEP) {
            return 0;
        }
        if (!isspace((unsigned char)s[i])) {
            sawLetter = 1;
        }
    }
    return sawLetter;
}

/*
 * Function : is_valid_phone
 * Purpose  : 5..15 digits and nothing else.
 */
int is_valid_phone(const char *s)
{
    size_t len;
    size_t i;

    if (s == NULL) {
        return 0;
    }
    len = strlen(s);
    if (len < 5 || len > (size_t)PHONE_LEN - 1) {
        return 0;
    }
    for (i = 0; i < len; i++) {
        if (!isdigit((unsigned char)s[i])) {
            return 0;
        }
    }
    return 1;
}

/*
 * Function : is_valid_password
 * Purpose  : 6..16 visible characters, no space and no '|'.
 */
int is_valid_password(const char *s)
{
    size_t len;
    size_t i;

    if (s == NULL) {
        return 0;
    }
    len = strlen(s);
    if (len < 6 || len > (size_t)PASSWORD_LEN - 1) {
        return 0;
    }
    for (i = 0; i < len; i++) {
        /* isgraph means "printable and not a space". */
        if (!isgraph((unsigned char)s[i]) || s[i] == FIELD_SEP) {
            return 0;
        }
    }
    return 1;
}

/*
 * Function : is_valid_date
 * Purpose  : True calendar date written as YYYY-MM-DD.
 *            Years before 2000 are rejected (same rule as the skeleton).
 */
int is_valid_date(const char *s)
{
    int year;
    int month;
    int day;

    if (s == NULL || strlen(s) != (size_t)DATE_LEN - 1) {
        return 0;
    }
    if (s[4] != '-' || s[7] != '-') {
        return 0;
    }
    if (!read_digits(s, 4, &year)
        || !read_digits(s + 5, 2, &month)
        || !read_digits(s + 8, 2, &day)) {
        return 0;
    }
    if (year < 2000) {
        return 0;
    }
    if (day < 1 || day > days_in_month(year, month)) {
        return 0;
    }
    return 1;
}

/*
 * Function : get_current_datetime
 * Purpose  : Local time as "YYYY-MM-DD HH:MM".
 */
void get_current_datetime(char *buf, size_t size)
{
    time_t now;
    struct tm *parts;

    if (buf == NULL || size == 0) {
        return;
    }

    now = time(NULL);
    parts = localtime(&now);
    if (parts == NULL || strftime(buf, size, "%Y-%m-%d %H:%M", parts) == 0) {
        snprintf(buf, size, "%s", "1970-01-01 00:00");
    }
}

/*
 * Function : pause_for_enter
 * Purpose  : Wait until the user presses Enter. Any extra typed text
 *            is discarded with that line. End of input returns at once.
 */
void pause_for_enter(void)
{
    char buf[LINE_LEN];

    (void)read_line("\nPress Enter to continue...", buf, sizeof buf);
}
