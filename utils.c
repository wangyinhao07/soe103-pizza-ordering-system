/*
 * File Name   : utils.c
 * Module      : Safe input & helper functions
 * Author      : Member A
 * Description : fgets-based input that never leaves junk in stdin and
 *               never accepts partially numeric input such as "12abc".
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <time.h>
#include "utils.h"

/* Discard the rest of the current input line */
static void flush_line(void)
{
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF) {
        /* discard */
    }
}

void trim(char *s)
{
    size_t len, start = 0;
    if (s == NULL) return;
    len = strlen(s);
    while (len > 0 && isspace((unsigned char)s[len - 1])) s[--len] = '\0';
    while (s[start] != '\0' && isspace((unsigned char)s[start])) start++;
    if (start > 0) memmove(s, s + start, len - start + 1);
}

int read_line(const char *prompt, char *buf, size_t size)
{
    size_t len;
    if (prompt != NULL) {
        printf("%s", prompt);
        fflush(stdout);
    }
    if (fgets(buf, (int)size, stdin) == NULL) {
        buf[0] = '\0';
        return ST_EOF;
    }
    len = strlen(buf);
    if (len > 0 && buf[len - 1] == '\n') {
        buf[len - 1] = '\0';
    } else if (len == size - 1) {      /* line longer than buffer */
        flush_line();
        buf[0] = '\0';
        printf("  ! Input too long (max %lu characters).\n", (unsigned long)(size - 2));
        return ST_ERR_INPUT;
    }
    trim(buf);
    return ST_OK;
}

int read_int(const char *prompt, int min, int max, int *out)
{
    char buf[LINE_LEN];
    char *end;
    long value;
    int rc;

    for (;;) {
        rc = read_line(prompt, buf, sizeof buf);
        if (rc == ST_EOF) return ST_EOF;
        if (rc != ST_OK || buf[0] == '\0') {
            printf("  ! Please enter a number between %d and %d.\n", min, max);
            continue;
        }
        errno = 0;
        value = strtol(buf, &end, 10);
        if (errno != 0 || *end != '\0' || value < min || value > max) {
            printf("  ! Please enter a number between %d and %d.\n", min, max);
            continue;
        }
        *out = (int)value;
        return ST_OK;
    }
}

int parse_money(const char *str, Money *out)
{
    Money yuan = 0;
    int fen = 0, digits = 0, decimals = 0;
    const char *p = str;

    if (str == NULL || *p == '\0') return ST_ERR_INPUT;
    while (isdigit((unsigned char)*p)) {
        yuan = yuan * 10 + (*p - '0');
        if (yuan > MAX_BALANCE) return ST_ERR_INPUT;   /* overflow guard */
        p++; digits++;
    }
    if (*p == '.') {
        p++;
        while (isdigit((unsigned char)*p) && decimals < 2) {
            fen = fen * 10 + (*p - '0');
            p++; decimals++;
        }
        if (decimals == 1) fen *= 10;
        if (decimals == 0 && digits == 0) return ST_ERR_INPUT;
    }
    if (*p != '\0' || (digits == 0 && decimals == 0)) return ST_ERR_INPUT;
    *out = yuan * 100 + fen;
    return ST_OK;
}

int read_money(const char *prompt, Money min, Money max, Money *out)
{
    char buf[LINE_LEN], lo[32], hi[32];
    Money value;
    int rc;

    format_money(min, lo, sizeof lo);
    format_money(max, hi, sizeof hi);
    for (;;) {
        rc = read_line(prompt, buf, sizeof buf);
        if (rc == ST_EOF) return ST_EOF;
        if (rc == ST_OK && strcmp(buf, "0") == 0 && min > 0) return ST_CANCEL;
        if (rc != ST_OK || parse_money(buf, &value) != ST_OK
            || value < min || value > max) {
            printf("  ! Please enter an amount between %s and %s (max 2 decimals).\n", lo, hi);
            continue;
        }
        *out = value;
        return ST_OK;
    }
}

int read_yes_no(const char *prompt)
{
    char buf[LINE_LEN];
    for (;;) {
        if (read_line(prompt, buf, sizeof buf) == ST_EOF) return 0;
        if (strcmp(buf, "y") == 0 || strcmp(buf, "Y") == 0) return 1;
        if (strcmp(buf, "n") == 0 || strcmp(buf, "N") == 0) return 0;
        printf("  ! Please enter y or n.\n");
    }
}

void format_money(Money amount, char *buf, size_t size)
{
    const char *sign = amount < 0 ? "-" : "";
    if (amount < 0) amount = -amount;
    snprintf(buf, size, "%s%ld.%02ld", sign, amount / 100, amount % 100);
}

int is_valid_name(const char *s)
{
    size_t len = strlen(s);
    if (len == 0 || len >= NAME_LEN) return 0;
    if (strchr(s, FIELD_SEP) != NULL) return 0;
    for (; *s != '\0'; s++) {
        if (!isprint((unsigned char)*s)) return 0;
    }
    return 1;
}

int is_valid_phone(const char *s)
{
    size_t len = strlen(s), i;
    if (len < 5 || len >= PHONE_LEN) return 0;
    for (i = 0; i < len; i++) {
        if (!isdigit((unsigned char)s[i])) return 0;
    }
    return 1;
}

int is_valid_password(const char *s)
{
    size_t len = strlen(s), i;
    if (len < 6 || len >= PASSWORD_LEN) return 0;
    for (i = 0; i < len; i++) {
        if (!isgraph((unsigned char)s[i]) || s[i] == FIELD_SEP) return 0;
    }
    return 1;
}

int is_valid_date(const char *s)
{
    static const int days[12] = {31,29,31,30,31,30,31,31,30,31,30,31};
    int y, m, d, i;
    if (strlen(s) != DATE_LEN - 1 || s[4] != '-' || s[7] != '-') return 0;
    for (i = 0; i < 10; i++) {
        if (i != 4 && i != 7 && !isdigit((unsigned char)s[i])) return 0;
    }
    y = atoi(s); m = atoi(s + 5); d = atoi(s + 8);
    if (y < 2000 || m < 1 || m > 12 || d < 1 || d > days[m - 1]) return 0;
    return 1;
}

void get_current_datetime(char *buf, size_t size)
{
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    if (t == NULL || strftime(buf, size, "%Y-%m-%d %H:%M", t) == 0) {
        snprintf(buf, size, "1970-01-01 00:00");
    }
}

void pause_for_enter(void)
{
    char buf[LINE_LEN];
    (void)read_line("\nPress Enter to continue...", buf, sizeof buf);
}
