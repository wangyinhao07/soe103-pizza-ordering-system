/*
 * File Name   : auth.h
 * Module      : Login / authentication
 * Author      : Member A
 * Description : Manager and customer login, max 3 attempts each time.
 */
#ifndef AUTH_H
#define AUTH_H

#include "common.h"

int auth_check_manager(const Manager *mgr, const char *user, const char *pwd);   /* 1/0 */
int auth_check_customer(const ShopData *shop, int accountNo, const char *pwd);  /* index or ST_ERR_AUTH */
int manager_login_ui(const ShopData *shop);        /* ST_OK or ST_ERR_AUTH            */
int customer_login_ui(const ShopData *shop);       /* customer index or ST_ERR_AUTH   */
int change_manager_password_ui(ShopData *shop);

#endif /* AUTH_H */
