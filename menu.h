/*
 * File Name   : menu.h
 * Module      : Pizza menu & pricing
 * Author      : Member B
 * Description : Logic functions (no I/O, unit-testable) for editing the
 *               menu and computing prices, plus the manager's menu UI.
 *               Pizza price = size base price + crust surcharge
 *                             + topping surcharge.
 */
#ifndef MENU_H
#define MENU_H

#include "common.h"

/* ----- logic ----- */
const char       *option_type_name(OptionType type);          /* "Crust"... */
int               menu_option_count(const Menu *menu, OptionType type);
const MenuOption *menu_get_option(const Menu *menu, OptionType type, int index);
int               menu_find_option(const Menu *menu, OptionType type, int id); /* index or ST_ERR_NOTFOUND */
int               menu_add_option(Menu *menu, OptionType type, const char *name, Money price); /* new id or error */
int               menu_rename_option(Menu *menu, OptionType type, int id, const char *newName);
int               menu_set_price(Menu *menu, OptionType type, int id, Money price);
int               menu_set_available(Menu *menu, OptionType type, int id, int available);
int               menu_remove_option(Menu *menu, OptionType type, int id);
Money             menu_calc_price(const Menu *menu, int crustId, int sizeId, int toppingId); /* <0 if invalid */

/* ----- user interface ----- */
void menu_display(const Menu *menu, int showHidden);       /* print price list  */
int  menu_select_option(const Menu *menu, OptionType type); /* id, or ST_CANCEL */
void menu_manage_ui(ShopData *shop);                       /* manager submenu   */

#endif /* MENU_H */
