/*
 * File Name   : auth.c
 * Module      : Login / authentication
 * Author      : En (Member A, team leader)
 * Description : Check manager and customer passwords, run the login
 *               screens (maximum 3 attempts), and change the manager
 *               password. Checking a password does not print anything.
 *               Only the _ui functions talk to the user.
 * Version     : 1.0  2026-11-10
 */
#include <stdio.h>
#include <string.h>

#include "auth.h"
#include "customer.h"
#include "fileio.h"
#include "utils.h"

/*
 * Function : auth_check_manager
 * Purpose  : Return 1 only when username and password both match.
 */
int auth_check_manager(const Manager *mgr, const char *user, const char *pwd)
{
    if (mgr == NULL || user == NULL || pwd == NULL) {
        return 0;
    }
    if (strcmp(mgr->username, user) != 0) {
        return 0;
    }
    if (strcmp(mgr->password, pwd) != 0) {
        return 0;
    }
    return 1;
}

/*
 * Function : auth_check_customer
 * Purpose  : Return the customer's index, or ST_ERR_AUTH.
 *            customer_find only returns active accounts, so a removed
 *            customer (Carol in the test data) cannot log in.
 */
int auth_check_customer(const ShopData *shop, int accountNo, const char *pwd)
{
    int index;

    if (shop == NULL || pwd == NULL) {
        return ST_ERR_AUTH;
    }

    index = customer_find(shop, accountNo);
    if (index < 0) {
        return ST_ERR_AUTH;
    }
    if (strcmp(shop->customers[index].password, pwd) != 0) {
        return ST_ERR_AUTH;
    }
    return index;
}

/*
 * Function : manager_login_ui
 * Purpose  : Manager login screen. 0 cancels. Three wrong pairs go back.
 */
int manager_login_ui(const ShopData *shop)
{
    char username[LINE_LEN];
    char password[LINE_LEN];
    int attemptsLeft;
    int rc;

    if (shop == NULL) {
        return ST_ERR_AUTH;
    }

    printf("\n----- Shop Manager Login -----\n");
    printf("Enter 0 as the username to cancel.\n");

    attemptsLeft = MAX_LOGIN_ATTEMPTS;
    while (attemptsLeft > 0) {
        rc = read_line("Username: ", username, sizeof username);
        if (rc == ST_EOF) {
            return ST_EOF;
        }
        if (rc == ST_ERR_INPUT) {
            /* The line was too long. It does not count as an attempt. */
            continue;
        }
        if (strcmp(username, "0") == 0) {
            printf("Login cancelled.\n");
            return ST_CANCEL;
        }
        if (username[0] == '\0') {
            printf("  ! Username cannot be empty.\n");
            continue;
        }

        rc = read_line("Password: ", password, sizeof password);
        if (rc == ST_EOF) {
            return ST_EOF;
        }
        if (rc == ST_ERR_INPUT) {
            continue;
        }
        if (strcmp(password, "0") == 0) {
            printf("Login cancelled.\n");
            return ST_CANCEL;
        }
        if (password[0] == '\0') {
            printf("  ! Password cannot be empty.\n");
            continue;
        }

        if (auth_check_manager(&shop->manager, username, password)) {
            printf("Manager login successful. Welcome, %s.\n",
                   shop->manager.username);
            return ST_OK;
        }

        attemptsLeft--;
        if (attemptsLeft > 0) {
            printf("Invalid account or password. (%d attempt(s) left)\n",
                   attemptsLeft);
        }
    }

    printf("Invalid account or password.\n");
    printf("Too many failed attempts. Returning to the main menu.\n");
    return ST_ERR_AUTH;
}

/*
 * Function : customer_login_ui
 * Purpose  : Customer login screen. Account 0 cancels.
 *            Letters in the account number are rejected by read_int
 *            and do not use up one of the 3 attempts.
 */
int customer_login_ui(const ShopData *shop)
{
    int accountNo;
    char password[LINE_LEN];
    int attemptsLeft;
    int rc;
    int index;

    if (shop == NULL) {
        return ST_ERR_AUTH;
    }

    accountNo = 0;
    printf("\n----- Customer Login -----\n");
    printf("Enter account number 0 to cancel.\n");

    attemptsLeft = MAX_LOGIN_ATTEMPTS;
    while (attemptsLeft > 0) {
        /* Account numbers in the design are 6 digits. 0 means cancel. */
        rc = read_int("Account number: ", 0, 999999, &accountNo);
        if (rc == ST_EOF) {
            return ST_EOF;
        }
        if (rc != ST_OK) {
            return rc;
        }
        if (accountNo == 0) {
            printf("Login cancelled.\n");
            return ST_CANCEL;
        }

        rc = read_line("Password: ", password, sizeof password);
        if (rc == ST_EOF) {
            return ST_EOF;
        }
        if (rc == ST_ERR_INPUT) {
            continue;
        }
        if (strcmp(password, "0") == 0) {
            printf("Login cancelled.\n");
            return ST_CANCEL;
        }
        if (password[0] == '\0') {
            printf("  ! Password cannot be empty.\n");
            continue;
        }

        index = auth_check_customer(shop, accountNo, password);
        if (index >= 0) {
            printf("Login successful. Welcome, %s.\n",
                   shop->customers[index].name);
            return index;
        }

        /* Same sentence for an unknown account, a removed account,
         * and a wrong password. Do not say which one it was. */
        attemptsLeft--;
        if (attemptsLeft > 0) {
            printf("Invalid account or password. (%d attempt(s) left)\n",
                   attemptsLeft);
        }
    }

    printf("Invalid account or password.\n");
    printf("Too many failed attempts. Returning to the main menu.\n");
    return ST_ERR_AUTH;
}

/*
 * Function : change_manager_password_ui
 * Purpose  : Ask for the current password and a new password twice.
 *            A format error does not use up an attempt. A wrong current
 *            password does. Three of those and we stop.
 */
int change_manager_password_ui(ShopData *shop)
{
    char oldPwd[LINE_LEN];
    char newPwd[LINE_LEN];
    char confirmPwd[LINE_LEN];
    int attemptsLeft;
    int rc;

    if (shop == NULL) {
        return ST_ERR_INPUT;
    }

    printf("\n----- Change Manager Password -----\n");
    printf("Enter 0 at any prompt to cancel.\n");
    printf("A password is 6 to 16 characters, with no spaces and no '%c'.\n",
           FIELD_SEP);

    attemptsLeft = MAX_LOGIN_ATTEMPTS;
    while (attemptsLeft > 0) {
        rc = read_line("Current password: ", oldPwd, sizeof oldPwd);
        if (rc == ST_EOF) {
            return ST_EOF;
        }
        if (rc == ST_ERR_INPUT) {
            continue;
        }
        if (strcmp(oldPwd, "0") == 0) {
            printf("Password change cancelled.\n");
            return ST_CANCEL;
        }

        rc = read_line("New password: ", newPwd, sizeof newPwd);
        if (rc == ST_EOF) {
            return ST_EOF;
        }
        if (rc == ST_ERR_INPUT) {
            continue;
        }
        if (strcmp(newPwd, "0") == 0) {
            printf("Password change cancelled.\n");
            return ST_CANCEL;
        }

        rc = read_line("Confirm new password: ", confirmPwd, sizeof confirmPwd);
        if (rc == ST_EOF) {
            return ST_EOF;
        }
        if (rc == ST_ERR_INPUT) {
            continue;
        }
        if (strcmp(confirmPwd, "0") == 0) {
            printf("Password change cancelled.\n");
            return ST_CANCEL;
        }

        /* These three problems are typing mistakes, not failed logins. */
        if (strcmp(newPwd, confirmPwd) != 0) {
            printf("  ! The two new passwords do not match.\n");
            continue;
        }
        if (!is_valid_password(newPwd)) {
            printf("  ! New password must be 6-16 characters,\n"
                   "    with no spaces and no '%c'.\n", FIELD_SEP);
            continue;
        }
        if (strcmp(newPwd, oldPwd) == 0) {
            printf("  ! The new password must differ from the current one.\n");
            continue;
        }

        if (strcmp(shop->manager.password, oldPwd) != 0) {
            attemptsLeft--;
            if (attemptsLeft > 0) {
                printf("  ! Current password is incorrect. "
                       "(%d attempt(s) left)\n", attemptsLeft);
            }
            continue;
        }

        /* is_valid_password already checked the length, so this fits.
         * strncpy is the copy the design allows; the last byte is set
         * to '\0' so the password is always a finished string. */
        strncpy(shop->manager.password, newPwd, PASSWORD_LEN - 1);
        shop->manager.password[PASSWORD_LEN - 1] = '\0';

        /* Member E implements the real write. The stub still returns. */
        if (save_manager(&shop->manager, MANAGER_FILE) != ST_OK) {
            printf("Warning: password changed for this session "
                   "but was not saved.\n");
            return ST_ERR_FILE;
        }
        printf("Password updated.\n");
        return ST_OK;
    }

    printf("Too many failed attempts. Password was not changed.\n");
    return ST_ERR_AUTH;
}
