/*
 * File Name   : order.c
 * Module      : Ordering & order history
 * Author      : Member D
 * Status      : SKELETON - bodies are TODO.
 */
#include <stdio.h>
#include <string.h>
#include "order.h"
#include "menu.h"
#include "utils.h"

void order_init(Order *order, int accountNo)
{
    memset(order, 0, sizeof *order);
    order->accountNo = accountNo;
}

int order_add_item(Order *order, const Menu *menu, int crustId, int sizeId,
                   int toppingId, int quantity)
{
    (void)order; (void)menu; (void)crustId; (void)sizeId; (void)toppingId; (void)quantity;
    printf("[TODO] order_add_item\n");
    return ST_ERR_INPUT;
}

int order_remove_item(Order *order, int itemIndex)
{
    (void)order; (void)itemIndex;
    printf("[TODO] order_remove_item\n");
    return ST_ERR_INPUT;
}

Money order_calc_total(const Order *order)
{
    Money total = 0;
    int i;
    for (i = 0; i < order->itemCount; i++) {
        total += order->items[i].unitPrice * order->items[i].quantity;
    }
    return total;
}

int order_checkout(ShopData *shop, Order *order, int custIndex)
{
    (void)shop; (void)order; (void)custIndex;
    printf("[TODO] order_checkout\n");
    return ST_CANCEL;
}

int order_count_for_customer(const ShopData *shop, int accountNo)
{
    int i, n = 0;
    for (i = 0; i < shop->orderCount; i++) {
        if (shop->orders[i].accountNo == accountNo) n++;
    }
    return n;
}

void order_print(const Order *order)
{
    (void)order;
    printf("[TODO] order_print\n");
}

void order_place_ui(ShopData *shop, int custIndex)
{
    (void)shop; (void)custIndex;
    printf("[TODO] order_place_ui\n");
}

void order_history_ui(const ShopData *shop, int accountNo)
{
    (void)shop; (void)accountNo;
    printf("[TODO] order_history_ui\n");
}
