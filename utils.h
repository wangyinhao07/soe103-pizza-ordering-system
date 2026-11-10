/*
 * File Name   : utils.h
 * Module      : Safe input and small helpers
 * Author      : En (Member A, team leader)
 * Description : Every keyboard read in this project goes through these
 *               functions. They use fgets (a whole line) and never scanf,
 *               so a bad keystroke cannot crash the program or leave
 *               leftover characters for the next question.
 * Version     : 1.0  2026-11-10
 */
#ifndef UTILS_H
#define UTILS_H

#include <stddef.h>
#include "common.h"

/*
 * Function : read_line
 * Purpose  : Read one line from the keyboard into buf, without the newline.
 *            Leading and trailing spaces are removed.
 * Params   : prompt - text printed first (NULL prints nothing)
 *            buf    - where the text is stored (not NULL)
 *            size   - size of buf, including room for '\0' (at least 2)
 * Returns  : ST_OK        - a line was stored in buf (it may be empty)
 *            ST_ERR_INPUT - the line was longer than buf, and it was
 *                           thrown away so the next read is clean
 *            ST_EOF       - the user pressed Ctrl+D / Ctrl+Z (end of input)
 * Notes    : The longest text that fits is size - 2, because fgets also
 *            needs one byte for the newline and one byte for '\0'.
 */
int read_line(const char *prompt, char *buf, size_t size);

/*
 * Function : read_int
 * Purpose  : Read an integer in the closed range [min, max].
 *            Keeps asking until the line is a whole number in range.
 *            "12abc", "1.5", a blank line, and a huge number are rejected.
 *            The value 0 is a normal result when min <= 0 <= max. Menus
 *            use that for "Exit" / "Log out". It is NOT treated as cancel.
 * Params   : prompt - question printed before each try
 *            min    - smallest accepted value
 *            max    - largest accepted value (must be >= min)
 *            out    - receives the value only on success
 * Returns  : ST_OK, ST_EOF, or ST_ERR_INPUT if min > max or out is NULL
 */
int read_int(const char *prompt, int min, int max, int *out);

/*
 * Function : read_money
 * Purpose  : Read an amount such as "12", "12.5" or "12.50" and convert
 *            it to fen. Keeps asking until the amount is inside [min, max].
 *            When min > 0, typing 0 (also 0.0 or 0.00) means "cancel".
 *            That is how a deposit can be abandoned. When min is 0, 0 is
 *            a real amount (a free crust surcharge, for example).
 * Params   : prompt - question printed before each try
 *            min    - smallest accepted amount, in fen
 *            max    - largest accepted amount, in fen
 *            out    - receives the amount in fen only on ST_OK
 * Returns  : ST_OK, ST_CANCEL, ST_EOF, or ST_ERR_INPUT if min > max
 */
int read_money(const char *prompt, Money min, Money max, Money *out);

/*
 * Function : read_yes_no
 * Purpose  : Ask a yes/no question. Accepts y/Y and n/N only.
 * Params   : prompt - the question, for example "Confirm? (y/n): "
 * Returns  : 1 for yes, 0 for no.
 *            End of input is treated as "no" so the caller can move on
 *            and the next read will see ST_EOF and quit cleanly.
 */
int read_yes_no(const char *prompt);

/*
 * Function : parse_money
 * Purpose  : Convert a money string to fen without reading the keyboard.
 *            Used by read_money and by unit tests.
 *            Accepted: "12", "12.5", "12.50", "0.01", optional spaces.
 *            Rejected: "", "abc", "12abc", "-1", "1e5", "12.505", "12.", ".5".
 * Params   : str - the text to parse (not changed)
 *            out - receives the amount in fen only on success
 * Returns  : ST_OK or ST_ERR_INPUT
 */
int parse_money(const char *str, Money *out);

/*
 * Function : format_money
 * Purpose  : Write an amount in fen as a decimal string such as "12.50".
 * Params   : amount - value in fen (negative values are shown with '-')
 *            buf    - destination buffer
 *            size   - size of buf
 */
void format_money(Money amount, char *buf, size_t size);

/*
 * Function : is_valid_name
 * Purpose  : Check a customer name or a menu option name.
 *            The design rule is 1 to 31 characters, not blank, and no '|'.
 * Returns  : 1 if valid, 0 otherwise (also 0 when s is NULL)
 */
int is_valid_name(const char *s);

/*
 * Function : is_valid_phone
 * Purpose  : Check a telephone number: 5 to 15 digits, nothing else.
 * Returns  : 1 if valid, 0 otherwise
 */
int is_valid_phone(const char *s);

/*
 * Function : is_valid_password
 * Purpose  : Check a password: 6 to 16 visible characters, no spaces
 *            and no '|'.
 * Returns  : 1 if valid, 0 otherwise
 */
int is_valid_password(const char *s);

/*
 * Function : is_valid_date
 * Purpose  : Check a calendar date written as YYYY-MM-DD.
 *            29 February is accepted only in a leap year.
 *            Years before 2000 are rejected, as in the shared skeleton.
 * Returns  : 1 if valid, 0 otherwise
 */
int is_valid_date(const char *s);

/*
 * Function : trim
 * Purpose  : Remove spaces, tabs and newlines from both ends of s.
 *            The string is changed in place. A NULL pointer is ignored.
 */
void trim(char *s);

/*
 * Function : get_current_datetime
 * Purpose  : Write the local time as "YYYY-MM-DD HH:MM".
 *            If the clock cannot be read, writes "1970-01-01 00:00"
 *            (or a shorter fallback when buf is tiny).
 */
void get_current_datetime(char *buf, size_t size);

/*
 * Function : pause_for_enter
 * Purpose  : Print "Press Enter to continue..." and wait for one line.
 *            A very long line is discarded. End of input just returns.
 */
void pause_for_enter(void);

#endif /* UTILS_H */
