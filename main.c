/*
 * File Name   : main.c
 * Module      : Program entry & top-level menus
 * Author      : Member A
 * Description : Loads data, shows the main menu, dispatches to the
 *               manager or customer menus, saves data on exit.
 */
#include <stdio.h>
#include <stdlib.h>
#include "common.h"
#include "utils.h"
#include "fileio.h"
#include "auth.h"
#include "menu.h"
#include "customer.h"
#include "order.h"
#include "stats.h"

/* Shop manager menu (after successful login) */
static void manager_menu(ShopData *shop)
{
    int choice = -1;
    while (choice != 0) {
        printf("\n===== Shop Manager =====\n"
               "1. Edit pizza menu & prices\n"
               "2. Manage customers\n"
               "3. Sales statistics\n"
               "4. Change manager password\n"
               "0. Logout\n");
        if (read_int("Choice: ", 0, 4, &choice) == ST_EOF) return;
        switch (choice) {
        case 1: menu_manage_ui(shop);             break;
        case 2: customer_manage_ui(shop);         break;
        case 3: stats_report_ui(shop);            break;
        case 4: change_manager_password_ui(shop); break;
        default: break;
        }
    }
}

/* Customer self-service menu (after successful login) */
static void customer_menu(ShopData *shop, int custIndex)
{
    int choice = -1;
    while (choice != 0) {
        printf("\n===== Welcome, %s =====\n", shop->customers[custIndex].name);
        customer_print(&shop->customers[custIndex]);
        printf("1. Place an order\n"
               "2. Order history\n"
               "3. Deposit money\n"
               "4. Edit my details / password\n"
               "0. Logout\n");
        if (read_int("Choice: ", 0, 4, &choice) == ST_EOF) return;
        switch (choice) {
        case 1: order_place_ui(shop, custIndex); break;
        case 2: order_history_ui(shop, shop->customers[custIndex].accountNo); break;
        case 3: customer_deposit_ui(shop, custIndex); break;
        case 4: customer_edit_self_ui(shop, custIndex); break;
        default: break;
        }
    }
}

int main(void)
{
    ShopData *shop;
    int choice = -1, idx;

    /* ShopData is large, so allocate it on the heap, not the stack */
    shop = (ShopData *)calloc(1, sizeof *shop);
    if (shop == NULL) {
        printf("Fatal: out of memory.\n");
        return EXIT_FAILURE;
    }
    load_all_data(shop);

    while (choice != 0) {
        printf("\n===== Pizza Ordering System =====\n"
               "1. View menu\n"
               "2. Customer login\n"
               "3. Register new account\n"
               "4. Shop manager login\n"
               "0. Exit\n");
        if (read_int("Choice: ", 0, 4, &choice) == ST_EOF) break;
        switch (choice) {
        case 1:
            menu_display(&shop->menu, 0);
            break;
        case 2:
            idx = customer_login_ui(shop);
            if (idx >= 0) customer_menu(shop, idx);
            break;
        case 3:
            customer_register_ui(shop);
            break;
        case 4:
            if (manager_login_ui(shop) == ST_OK) manager_menu(shop);
            break;
        default:
            break;
        }
    }

    if (save_all_data(shop) != ST_OK) printf("Warning: some data could not be saved.\n");
    free(shop);
    printf("Goodbye!\n");
    return EXIT_SUCCESS;
}
