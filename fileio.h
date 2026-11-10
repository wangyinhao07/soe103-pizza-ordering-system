/*
 * File Name   : fileio.h
 * Module      : Data persistence (text files)
 * Author      : Member E
 * Description : Load all data at start-up and save after every change.
 *               Missing files are created with default data.
 */
#ifndef FILEIO_H
#define FILEIO_H

#include "common.h"

int  load_all_data(ShopData *shop);          /* load every file, or defaults */
int  save_all_data(const ShopData *shop);    /* save every file              */

int  load_menu(Menu *menu, const char *path);
int  save_menu(const Menu *menu, const char *path);
int  load_customers(ShopData *shop, const char *path);
int  save_customers(const ShopData *shop, const char *path);
int  load_orders(ShopData *shop, const char *path);
int  save_orders(const ShopData *shop, const char *path);
int  append_order(const Order *order, const char *path);  /* add one order */
int  load_manager(Manager *mgr, const char *path);
int  save_manager(const Manager *mgr, const char *path);

void init_default_menu(Menu *menu);          /* proposed default menu       */
void init_default_manager(Manager *mgr);     /* admin / admin123            */

#endif /* FILEIO_H */
