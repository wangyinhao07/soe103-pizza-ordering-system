/*
 * File Name   : utils.h
 * Module      : Safe input & helper functions
 * Author      : Member A
 * Description : All keyboard input in the project MUST go through these
 *               functions (fgets based, never raw scanf), so that invalid
 *               input can never crash the program.
 */
#ifndef UTILS_H
#define UTILS_H

#include <stddef.h>
#include "common.h"

/* Read one line (without '\n'). Too-long lines are rejected and flushed.
 * Returns ST_OK, ST_ERR_INPUT (too long) or ST_EOF.                      */
int  read_line(const char *prompt, char *buf, size_t size);

/* Read an integer in [min,max]; re-prompts until valid. "0" may be used
 * by callers as "cancel". Returns ST_OK or ST_EOF.                       */
int  read_int(const char *prompt, int min, int max, int *out);

/* Read a money amount such as "12", "12.5", "12.50" in [min,max] fen.
 * Returns ST_OK, ST_CANCEL (user typed 0 when min>0) or ST_EOF.          */
int  read_money(const char *prompt, Money min, Money max, Money *out);

/* Ask a y/n question. Returns 1 for yes, 0 for no.                       */
int  read_yes_no(const char *prompt);

/* Parse a money string into fen. Returns ST_OK or ST_ERR_INPUT.          */
int  parse_money(const char *str, Money *out);

/* Format fen as "123.45" into buf.                                       */
void format_money(Money amount, char *buf, size_t size);

/* Validation helpers: return 1 if valid, 0 otherwise.                    */
int  is_valid_name(const char *s);      /* 1..31 chars, no '|', not blank */
int  is_valid_phone(const char *s);     /* 5..15 digits                   */
int  is_valid_password(const char *s);  /* 6..16 printable, no '|'/space  */
int  is_valid_date(const char *s);      /* "YYYY-MM-DD"                   */

/* Remove leading / trailing white space in place.                        */
void trim(char *s);

/* Current local time as "YYYY-MM-DD HH:MM".                              */
void get_current_datetime(char *buf, size_t size);

/* Print "Press Enter to continue..." and wait.                           */
void pause_for_enter(void);

#endif /* UTILS_H */
