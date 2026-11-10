/*
 * File Name   : test_auth.c
 * Module      : Unit tests for auth.c
 * Author      : En (Member A, team leader)
 * Description : Checks manager and customer password checks, the 3-attempt
 *               limit, removed accounts, cancel (0), and end of input.
 *               The shop is built in memory. It does not read data files.
 *               Run from the project root:  make test
 * Version     : 1.0  2026-11-10
 *
 * Accounts used here are the ones in the design draft:
 *   admin / admin123
 *   100001 Alice Zhang / alice123   (active)
 *   100002 Bob Li      / bob12345   (active)
 *   100003 Carol Wang  / carol123   (removed, must not log in)
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "auth.h"
#include "utils.h"

static int g_passed = 0;
static int g_failed = 0;

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

/* ShopData is large, so it is allocated on the heap, same as main(). */
static ShopData *make_shop(void)
{
    ShopData *shop;
    Customer *cust;

    shop = (ShopData *)calloc(1, sizeof *shop);
    if (shop == NULL) {
        printf("FAIL: out of memory\n");
        exit(EXIT_FAILURE);
    }

    snprintf(shop->manager.username, NAME_LEN, "%s", "admin");
    snprintf(shop->manager.password, PASSWORD_LEN, "%s", "admin123");

    cust = &shop->customers[0];
    cust->accountNo = 100001;
    snprintf(cust->name, NAME_LEN, "%s", "Alice Zhang");
    snprintf(cust->phone, PHONE_LEN, "%s", "13800000001");
    snprintf(cust->password, PASSWORD_LEN, "%s", "alice123");
    cust->balance = 20000;
    cust->active = 1;

    cust = &shop->customers[1];
    cust->accountNo = 100002;
    snprintf(cust->name, NAME_LEN, "%s", "Bob Li");
    snprintf(cust->phone, PHONE_LEN, "%s", "13800000002");
    snprintf(cust->password, PASSWORD_LEN, "%s", "bob12345");
    cust->balance = 500;
    cust->active = 1;

    cust = &shop->customers[2];
    cust->accountNo = 100003;
    snprintf(cust->name, NAME_LEN, "%s", "Carol Wang");
    snprintf(cust->phone, PHONE_LEN, "%s", "13800000003");
    snprintf(cust->password, PASSWORD_LEN, "%s", "carol123");
    cust->balance = 0;
    cust->active = 0; /* removed account */

    shop->customerCount = 3;
    shop->nextAccountNo = 100004;
    return shop;
}

static void test_manager_check(void)
{
    ShopData *shop;

    printf("\n-- auth_check_manager --\n");
    shop = make_shop();

    /* T-AU-01 correct pair. T-AU-02 wrong password and wrong username. */
    CHECK((auth_check_manager(&shop->manager, "admin", "admin123") == 1));
    CHECK((auth_check_manager(&shop->manager, "admin", "wrong123") == 0));
    CHECK((auth_check_manager(&shop->manager, "Admin", "admin123") == 0));
    CHECK((auth_check_manager(&shop->manager, "admin", "") == 0));
    CHECK((auth_check_manager(&shop->manager, "", "admin123") == 0));
    CHECK((auth_check_manager(NULL, "admin", "admin123") == 0));
    CHECK((auth_check_manager(&shop->manager, NULL, "admin123") == 0));
    CHECK((auth_check_manager(&shop->manager, "admin", NULL) == 0));

    free(shop);
}

static void test_customer_check(void)
{
    ShopData *shop;

    printf("\n-- auth_check_customer --\n");
    shop = make_shop();

    /* Index 0 is Alice. That is success, not an error. */
    CHECK((auth_check_customer(shop, 100001, "alice123") == 0));
    CHECK((auth_check_customer(shop, 100002, "bob12345") == 1));

    /* T-AU-03: Carol's password is right, but the account was removed. */
    CHECK((auth_check_customer(shop, 100003, "carol123") == ST_ERR_AUTH));

    CHECK((auth_check_customer(shop, 100001, "wrongpwd") == ST_ERR_AUTH));
    CHECK((auth_check_customer(shop, 100099, "alice123") == ST_ERR_AUTH));
    CHECK((auth_check_customer(shop, 100001, "") == ST_ERR_AUTH));
    CHECK((auth_check_customer(NULL, 100001, "alice123") == ST_ERR_AUTH));
    CHECK((auth_check_customer(shop, 100001, NULL) == ST_ERR_AUTH));

    free(shop);
}

static void test_manager_login_ui(void)
{
    ShopData *shop;
    char longLine[420];
    int i;

    printf("\n-- manager_login_ui --\n");
    shop = make_shop();

    feed_stdin("admin\nadmin123\n");
    CHECK((manager_login_ui(shop) == ST_OK));

    feed_stdin("admin\nwrong111\nadmin\nadmin123\n");
    CHECK((manager_login_ui(shop) == ST_OK));

    /* T-AU-04: three failures stop, even if the next lines would be correct. */
    feed_stdin("admin\nbadpass1\nadmin\nbadpass2\nadmin\nbadpass3\n"
               "admin\nadmin123\n");
    CHECK((manager_login_ui(shop) == ST_ERR_AUTH));

    feed_stdin("0\n");
    CHECK((manager_login_ui(shop) == ST_CANCEL));

    feed_stdin("admin\n0\n");
    CHECK((manager_login_ui(shop) == ST_CANCEL));

    feed_stdin("");
    CHECK((manager_login_ui(shop) == ST_EOF));

    feed_stdin("admin\n");
    CHECK((manager_login_ui(shop) == ST_EOF));

    /* A too-long line is not an attempt. The real login after it works. */
    for (i = 0; i < 400; i++) {
        longLine[i] = 'x';
    }
    memcpy(longLine + 400, "\nadmin\nadmin123\n", 16);
    longLine[416] = '\0';
    feed_stdin(longLine);
    CHECK((manager_login_ui(shop) == ST_OK));

    feed_stdin("\nadmin\nadmin123\n");
    CHECK((manager_login_ui(shop) == ST_OK));

    CHECK((manager_login_ui(NULL) == ST_ERR_AUTH));
    free(shop);
}

static void test_customer_login_ui(void)
{
    ShopData *shop;

    printf("\n-- customer_login_ui --\n");
    shop = make_shop();

    feed_stdin("100001\nalice123\n");
    CHECK((customer_login_ui(shop) == 0)); /* Alice's index */

    feed_stdin("100002\nbob12345\n");
    CHECK((customer_login_ui(shop) == 1));

    /* Letters and a blank line are bad input, not failed attempts. */
    feed_stdin("abc\n\n12abc\n100001\nalice123\n");
    CHECK((customer_login_ui(shop) == 0));

    feed_stdin("100001\nbadpass\n100001\nalice123\n");
    CHECK((customer_login_ui(shop) == 0));

    feed_stdin("100003\ncarol123\n100003\ncarol123\n100003\ncarol123\n");
    CHECK((customer_login_ui(shop) == ST_ERR_AUTH));

    feed_stdin("100099\nalice123\n100099\nalice123\n100099\nalice123\n");
    CHECK((customer_login_ui(shop) == ST_ERR_AUTH));

    feed_stdin("0\n");
    CHECK((customer_login_ui(shop) == ST_CANCEL));

    feed_stdin("100001\n0\n");
    CHECK((customer_login_ui(shop) == ST_CANCEL));

    feed_stdin("");
    CHECK((customer_login_ui(shop) == ST_EOF));

    feed_stdin("100001\n");
    CHECK((customer_login_ui(shop) == ST_EOF));

    /* A huge account number is rejected by read_int, then a real one works. */
    feed_stdin("99999999999999\n100002\nbob12345\n");
    CHECK((customer_login_ui(shop) == 1));

    CHECK((customer_login_ui(NULL) == ST_ERR_AUTH));
    free(shop);
}

static void test_change_password_ui(void)
{
    ShopData *shop;

    printf("\n-- change_manager_password_ui --\n");
    shop = make_shop();

    feed_stdin("0\n");
    CHECK((change_manager_password_ui(shop) == ST_CANCEL));
    CHECK((auth_check_manager(&shop->manager, "admin", "admin123") == 1));

    /* Mismatch, then a password that is too short, then a good change.
     * Neither mistake uses up one of the 3 attempts. */
    feed_stdin("admin123\nnewpass99\nother999\n"
               "admin123\nabc\nabc\n"
               "admin123\nnewpass99\nnewpass99\n");
    CHECK((change_manager_password_ui(shop) == ST_OK));
    CHECK((auth_check_manager(&shop->manager, "admin", "newpass99") == 1));
    CHECK((auth_check_manager(&shop->manager, "admin", "admin123") == 0));

    free(shop);
    shop = make_shop();

    /* The current password is checked before the new password's format.
     * Three wrong current passwords stop, even when later lines are too
     * short to be a legal new password. */
    feed_stdin("wrong111\nabc\nabc\n"
               "wrong222\nabc\nabc\n"
               "wrong333\nabc\nabc\n");
    CHECK((change_manager_password_ui(shop) == ST_ERR_AUTH));
    CHECK((auth_check_manager(&shop->manager, "admin", "admin123") == 1));

    feed_stdin("");
    CHECK((change_manager_password_ui(shop) == ST_EOF));
    CHECK((change_manager_password_ui(NULL) == ST_ERR_INPUT));

    free(shop);
}

int main(void)
{
    printf("=== test_auth ===\n");
    test_manager_check();
    test_customer_check();
    test_manager_login_ui();
    test_customer_login_ui();
    test_change_password_ui();

    if (remove(TMP_INPUT) != 0) {
        /* Nothing to delete. */
    }

    printf("\ntest_auth: %d passed, %d failed\n", g_passed, g_failed);
    return g_failed == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
