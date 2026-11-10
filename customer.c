/*
 * File Name   : customer.c
 * Module      : Customer account management
 * Author      : Member C
 * Status      : SKELETON - bodies are TODO.
 */
#include <stdio.h>
#include "customer.h"
#include "utils.h"

int customer_find(const ShopData *shop, int accountNo)
{
    int i;
    for (i = 0; i < shop->customerCount; i++) {
        if (shop->customers[i].active && shop->customers[i].accountNo == accountNo) return i;
    }
    return ST_ERR_NOTFOUND;
}

int customer_add(ShopData *shop, const char *name, const char *phone, const char *password)
{
    (void)shop; (void)name; (void)phone; (void)password;
    printf("[TODO] customer_add\n");
    return ST_ERR_INPUT;
}

int customer_update(ShopData *shop, int accountNo, const char *name, const char *phone)
{
    (void)shop; (void)accountNo; (void)name; (void)phone;
    printf("[TODO] customer_update\n");
    return ST_ERR_INPUT;
}

int customer_set_password(ShopData *shop, int accountNo, const char *password)
{
    (void)shop; (void)accountNo; (void)password;
    printf("[TODO] customer_set_password\n");
    return ST_ERR_INPUT;
}

int customer_remove(ShopData *shop, int accountNo)
{
    (void)shop; (void)accountNo;
    printf("[TODO] customer_remove\n");
    return ST_ERR_INPUT;
}

int customer_deposit(Customer *c, Money amount)
{
    if (amount < MIN_DEPOSIT || amount > MAX_DEPOSIT) return ST_ERR_INPUT;
    if (c->balance + amount > MAX_BALANCE) return ST_ERR_INPUT;
    c->balance += amount;
    return ST_OK;
}

int customer_charge(Customer *c, Money amount)
{
    if (amount < 0) return ST_ERR_INPUT;
    if (c->balance < amount) return ST_ERR_BALANCE;
    c->balance -= amount;
    return ST_OK;
}

void customer_print(const Customer *c)
{
    char money[32];
    format_money(c->balance, money, sizeof money);
    printf("Account %d | %s | %s | balance %s\n", c->accountNo, c->name, c->phone, money);
}

void customer_list_all(const ShopData *shop)
{
    (void)shop;
    printf("[TODO] customer_list_all\n");
}

int customer_register_ui(ShopData *shop)
{
    (void)shop;
    printf("[TODO] customer_register_ui\n");
    return ST_CANCEL;
}

void customer_manage_ui(ShopData *shop)
{
    (void)shop;
    printf("[TODO] customer_manage_ui: list / add / edit / remove / search\n");
}

void customer_deposit_ui(ShopData *shop, int custIndex)
{
    (void)shop; (void)custIndex;
    printf("[TODO] customer_deposit_ui\n");
}

void customer_edit_self_ui(ShopData *shop, int custIndex)
{
    (void)shop; (void)custIndex;
    printf("[TODO] customer_edit_self_ui\n");
}
