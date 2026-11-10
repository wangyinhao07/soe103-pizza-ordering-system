/*
 * File Name   : auth.c
 * Module      : Login / authentication
 * Author      : Member A
 * Status      : SKELETON - UI bodies are TODO.
 */
#include <stdio.h>
#include <string.h>
#include "auth.h"
#include "customer.h"
#include "utils.h"

int auth_check_manager(const Manager *mgr, const char *user, const char *pwd)
{
    return strcmp(mgr->username, user) == 0 && strcmp(mgr->password, pwd) == 0;
}

int auth_check_customer(const ShopData *shop, int accountNo, const char *pwd)
{
    int idx = customer_find(shop, accountNo);
    if (idx < 0 || strcmp(shop->customers[idx].password, pwd) != 0) return ST_ERR_AUTH;
    return idx;
}

int manager_login_ui(const ShopData *shop)
{
    (void)shop;
    printf("[TODO] manager_login_ui (up to %d attempts)\n", MAX_LOGIN_ATTEMPTS);
    return ST_ERR_AUTH;
}

int customer_login_ui(const ShopData *shop)
{
    (void)shop;
    printf("[TODO] customer_login_ui (up to %d attempts)\n", MAX_LOGIN_ATTEMPTS);
    return ST_ERR_AUTH;
}

int change_manager_password_ui(ShopData *shop)
{
    (void)shop;
    printf("[TODO] change_manager_password_ui\n");
    return ST_CANCEL;
}
