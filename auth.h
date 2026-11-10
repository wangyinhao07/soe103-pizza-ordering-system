/*
 * File Name   : auth.h
 * Module      : Login / authentication
 * Author      : En (Member A, team leader)
 * Description : Shop-manager and customer login. Each visit to the login
 *               screen allows at most 3 failed attempts, then returns to
 *               the main menu. Typing 0 cancels and goes back at once.
 *               A wrong account and a wrong password use the same message,
 *               so the screen does not reveal which one failed.
 * Version     : 1.0  2026-11-10
 */
#ifndef AUTH_H
#define AUTH_H

#include "common.h"

/*
 * Function : auth_check_manager
 * Purpose  : Compare a typed username and password with the stored manager.
 *            Does not print anything, so unit tests can call it directly.
 * Params   : mgr  - stored manager record (NULL is treated as "no match")
 *            user - typed username
 *            pwd  - typed password
 * Returns  : 1 when both match, 0 otherwise
 */
int auth_check_manager(const Manager *mgr, const char *user, const char *pwd);

/*
 * Function : auth_check_customer
 * Purpose  : Find an active customer with this account number and password.
 *            A removed account (active == 0) is rejected even when the
 *            password is correct. Unknown account and wrong password
 *            return the same code.
 * Params   : shop      - all shop data
 *            accountNo - typed account number
 *            pwd       - typed password
 * Returns  : the customer's index in shop->customers (0 or greater;
 *            0 is a real index, not an error), or ST_ERR_AUTH
 */
int auth_check_customer(const ShopData *shop, int accountNo, const char *pwd);

/*
 * Function : manager_login_ui
 * Purpose  : Ask for the manager username and password, up to 3 attempts.
 * Params   : shop - shop data; the manager record is shop->manager
 * Returns  : ST_OK       - login succeeded
 *            ST_ERR_AUTH - 3 failures, back to the main menu
 *            ST_CANCEL   - the user typed 0
 *            ST_EOF      - end of input; the program should save and exit
 */
int manager_login_ui(const ShopData *shop);

/*
 * Function : customer_login_ui
 * Purpose  : Ask for an account number and a password, up to 3 attempts.
 * Params   : shop - shop data
 * Returns  : customer index (>= 0) on success. Index 0 is success.
 *            ST_ERR_AUTH, ST_CANCEL, or ST_EOF as for the manager login.
 */
int customer_login_ui(const ShopData *shop);

/*
 * Function : change_manager_password_ui
 * Purpose  : Let the logged-in manager set a new password.
 *            Asks for the current password, the new one, and a confirmation.
 *            On success the new password is stored and save_manager is called
 *            so Member E's file code can write data/manager.txt.
 * Params   : shop - shop data (the manager password inside it may change)
 * Returns  : ST_OK, ST_CANCEL, ST_ERR_AUTH (3 wrong current passwords),
 *            ST_EOF, or ST_ERR_FILE if the save call fails
 */
int change_manager_password_ui(ShopData *shop);

#endif /* AUTH_H */
