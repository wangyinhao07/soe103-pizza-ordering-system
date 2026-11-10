/*
 * File Name   : customer.h
 * Module      : Customer account management
 * Author      : Member C
 * Description : Register / edit / remove customers, deposit & balance.
 */
#ifndef CUSTOMER_H
#define CUSTOMER_H

#include "common.h"

/* ----- logic ----- */
int  customer_find(const ShopData *shop, int accountNo);        /* index or ST_ERR_NOTFOUND (active only) */
int  customer_add(ShopData *shop, const char *name, const char *phone,
                  const char *password);                         /* new accountNo or error */
int  customer_update(ShopData *shop, int accountNo, const char *name,
                     const char *phone);                         /* NULL = keep old value  */
int  customer_set_password(ShopData *shop, int accountNo, const char *password);
int  customer_remove(ShopData *shop, int accountNo);            /* soft delete            */
int  customer_deposit(Customer *c, Money amount);               /* checks MIN/MAX rules   */
int  customer_charge(Customer *c, Money amount);                /* ST_ERR_BALANCE if low  */

/* ----- user interface ----- */
void customer_print(const Customer *c);
void customer_list_all(const ShopData *shop);
int  customer_register_ui(ShopData *shop);                 /* new accountNo or error */
void customer_manage_ui(ShopData *shop);                   /* manager submenu        */
void customer_deposit_ui(ShopData *shop, int custIndex);
void customer_edit_self_ui(ShopData *shop, int custIndex); /* phone / password       */

#endif /* CUSTOMER_H */
