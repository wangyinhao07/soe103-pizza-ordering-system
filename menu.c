/*
 * File Name   : menu.c
 * Module      : Pizza menu & pricing
 * Author      : Member B
 * Status      : SKELETON - most bodies are TODO.
 */
#include <stdio.h>
#include "menu.h"
#include "utils.h"

const char *option_type_name(OptionType type)
{
    switch (type) {
    case OPT_CRUST:   return "Crust";
    case OPT_SIZE:    return "Size";
    case OPT_TOPPING: return "Topping";
    default:          return "Unknown";
    }
}

int menu_option_count(const Menu *menu, OptionType type)
{
    switch (type) {
    case OPT_CRUST:   return menu->crustCount;
    case OPT_SIZE:    return menu->sizeCount;
    case OPT_TOPPING: return menu->toppingCount;
    default:          return 0;
    }
}

const MenuOption *menu_get_option(const Menu *menu, OptionType type, int index)
{
    if (index < 0 || index >= menu_option_count(menu, type)) return NULL;
    switch (type) {
    case OPT_CRUST:   return &menu->crusts[index];
    case OPT_SIZE:    return &menu->sizes[index];
    case OPT_TOPPING: return &menu->toppings[index];
    default:          return NULL;
    }
}

int menu_find_option(const Menu *menu, OptionType type, int id)
{
    int i, n = menu_option_count(menu, type);
    for (i = 0; i < n; i++) {
        if (menu_get_option(menu, type, i)->id == id) return i;
    }
    return ST_ERR_NOTFOUND;
}

int menu_add_option(Menu *menu, OptionType type, const char *name, Money price)
{
    (void)menu; (void)type; (void)name; (void)price;
    printf("[TODO] menu_add_option\n");
    return ST_ERR_INPUT;
}

int menu_rename_option(Menu *menu, OptionType type, int id, const char *newName)
{
    (void)menu; (void)type; (void)id; (void)newName;
    printf("[TODO] menu_rename_option\n");
    return ST_ERR_INPUT;
}

int menu_set_price(Menu *menu, OptionType type, int id, Money price)
{
    (void)menu; (void)type; (void)id; (void)price;
    printf("[TODO] menu_set_price\n");
    return ST_ERR_INPUT;
}

int menu_set_available(Menu *menu, OptionType type, int id, int available)
{
    (void)menu; (void)type; (void)id; (void)available;
    printf("[TODO] menu_set_available\n");
    return ST_ERR_INPUT;
}

int menu_remove_option(Menu *menu, OptionType type, int id)
{
    (void)menu; (void)type; (void)id;
    printf("[TODO] menu_remove_option\n");
    return ST_ERR_INPUT;
}

Money menu_calc_price(const Menu *menu, int crustId, int sizeId, int toppingId)
{
    int c = menu_find_option(menu, OPT_CRUST, crustId);
    int s = menu_find_option(menu, OPT_SIZE, sizeId);
    int t = menu_find_option(menu, OPT_TOPPING, toppingId);
    if (c < 0 || s < 0 || t < 0) return ST_ERR_NOTFOUND;
    /* TODO: also reject options that are not available */
    return menu->sizes[s].price + menu->crusts[c].price + menu->toppings[t].price;
}

void menu_display(const Menu *menu, int showHidden)
{
    int type, i;
    char money[32];
    for (type = 0; type < OPT_TYPE_COUNT; type++) {
        printf("\n-- %s --\n", option_type_name((OptionType)type));
        for (i = 0; i < menu_option_count(menu, (OptionType)type); i++) {
            const MenuOption *o = menu_get_option(menu, (OptionType)type, i);
            if (!o->available && !showHidden) continue;
            format_money(o->price, money, sizeof money);
            printf("  %2d. %-20s %s%8s%s\n", o->id, o->name,
                   type == OPT_SIZE ? "  " : "+ ", money,
                   o->available ? "" : "  (hidden)");
        }
    }
}

int menu_select_option(const Menu *menu, OptionType type)
{
    (void)menu; (void)type;
    printf("[TODO] menu_select_option\n");
    return ST_CANCEL;
}

void menu_manage_ui(ShopData *shop)
{
    menu_display(&shop->menu, 1);
    printf("\n[TODO] menu_manage_ui: add / rename / set price / hide / remove\n");
}
