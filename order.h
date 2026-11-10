/*
 * File Name   : order.h
 * Module      : Ordering & order history
 * Author      : Member D
 * Description : Build a cart of pizzas, confirm, pay from balance,
 *               record the order and show history.
 */
#ifndef ORDER_H
#define ORDER_H

#include "common.h"

/* ----- logic ----- */
void  order_init(Order *order, int accountNo);
int   order_add_item(Order *order, const Menu *menu, int crustId, int sizeId,
                     int toppingId, int quantity);   /* item index or error */
int   order_remove_item(Order *order, int itemIndex);
Money order_calc_total(const Order *order);
int   order_checkout(ShopData *shop, Order *order, int custIndex); /* pay+save */
int   order_count_for_customer(const ShopData *shop, int accountNo);

/* ----- user interface ----- */
void  order_print(const Order *order);
void  order_place_ui(ShopData *shop, int custIndex);
void  order_history_ui(const ShopData *shop, int accountNo);

#endif /* ORDER_H */
