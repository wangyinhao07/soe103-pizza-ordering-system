/*
 * File Name   : fileio.c
 * Module      : Data persistence (text files)
 * Author      : Member E
 * Description : '|' separated text files, see design_draft section 3.4.
 * Status      : SKELETON - load/save bodies are TODO.
 */
#include <stdio.h>
#include <string.h>
#include "fileio.h"

/* Helper: copy a default option into a menu array */
static void set_option(MenuOption *o, int id, const char *name, Money price)
{
    o->id = id;
    strncpy(o->name, name, NAME_LEN - 1);
    o->name[NAME_LEN - 1] = '\0';
    o->price = price;
    o->available = 1;
}

void init_default_menu(Menu *menu)
{
    /* Proposed example values (fen). Price = size + crust + topping */
    set_option(&menu->crusts[0], 1, "Hand Tossed", 0);
    set_option(&menu->crusts[1], 2, "Thin and Crispy", 500);
    set_option(&menu->crusts[2], 3, "Stuffed Crust", 1200);
    menu->crustCount = 3;

    set_option(&menu->sizes[0], 1, "10 inch", 3900);
    set_option(&menu->sizes[1], 2, "12 inch", 4900);
    set_option(&menu->sizes[2], 3, "14 inch", 5900);
    set_option(&menu->sizes[3], 4, "16 inch", 6900);
    menu->sizeCount = 4;

    set_option(&menu->toppings[0], 1, "Margherita", 0);
    set_option(&menu->toppings[1], 2, "Hawaiian", 800);
    set_option(&menu->toppings[2], 3, "BBQ Chicken", 1200);
    set_option(&menu->toppings[3], 4, "Supreme", 1500);
    menu->toppingCount = 4;
}

void init_default_manager(Manager *mgr)
{
    strcpy(mgr->username, "admin");
    strcpy(mgr->password, "admin123");
}

int load_menu(Menu *menu, const char *path)
{
    (void)menu; (void)path;
    printf("[TODO] load_menu\n");
    return ST_ERR_FILE;
}

int save_menu(const Menu *menu, const char *path)
{
    (void)menu; (void)path;
    printf("[TODO] save_menu\n");
    return ST_OK;
}

int load_customers(ShopData *shop, const char *path)
{
    (void)path;
    shop->customerCount = 0;
    printf("[TODO] load_customers\n");
    return ST_ERR_FILE;
}

int save_customers(const ShopData *shop, const char *path)
{
    (void)shop; (void)path;
    printf("[TODO] save_customers\n");
    return ST_OK;
}

int load_orders(ShopData *shop, const char *path)
{
    (void)path;
    shop->orderCount = 0;
    printf("[TODO] load_orders\n");
    return ST_ERR_FILE;
}

int save_orders(const ShopData *shop, const char *path)
{
    (void)shop; (void)path;
    printf("[TODO] save_orders\n");
    return ST_OK;
}

int append_order(const Order *order, const char *path)
{
    (void)order; (void)path;
    printf("[TODO] append_order\n");
    return ST_OK;
}

int load_manager(Manager *mgr, const char *path)
{
    (void)mgr; (void)path;
    printf("[TODO] load_manager\n");
    return ST_ERR_FILE;
}

int save_manager(const Manager *mgr, const char *path)
{
    (void)mgr; (void)path;
    printf("[TODO] save_manager\n");
    return ST_OK;
}

int load_all_data(ShopData *shop)
{
    if (load_menu(&shop->menu, MENU_FILE) != ST_OK) init_default_menu(&shop->menu);
    if (load_manager(&shop->manager, MANAGER_FILE) != ST_OK) init_default_manager(&shop->manager);
    (void)load_customers(shop, CUSTOMER_FILE);
    (void)load_orders(shop, ORDER_FILE);
    /* TODO: compute next ids as max(existing)+1 */
    shop->nextAccountNo = FIRST_ACCOUNT_NO + shop->customerCount;
    shop->nextOrderId   = FIRST_ORDER_ID + shop->orderCount;
    return ST_OK;
}

int save_all_data(const ShopData *shop)
{
    int rc = ST_OK;
    if (save_menu(&shop->menu, MENU_FILE) != ST_OK) rc = ST_ERR_FILE;
    if (save_manager(&shop->manager, MANAGER_FILE) != ST_OK) rc = ST_ERR_FILE;
    if (save_customers(shop, CUSTOMER_FILE) != ST_OK) rc = ST_ERR_FILE;
    if (save_orders(shop, ORDER_FILE) != ST_OK) rc = ST_ERR_FILE;
    return rc;
}
