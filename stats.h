/*
 * File Name   : stats.h
 * Module      : Sales statistics
 * Author      : Member E
 * Description : Revenue and quantity statistics computed from orders.
 *               fromDate/toDate are "YYYY-MM-DD" (inclusive), NULL = all.
 */
#ifndef STATS_H
#define STATS_H

#include "common.h"

#define MAX_STAT_ENTRIES 64

typedef struct {
    char  name[3 * NAME_LEN + 8];   /* option name, combo or date */
    int   quantity;                 /* pizzas sold                */
    Money revenue;
} StatEntry;

Money stats_total_revenue(const ShopData *shop, const char *fromDate, const char *toDate);
int   stats_total_pizzas(const ShopData *shop, const char *fromDate, const char *toDate);
int   stats_order_count(const ShopData *shop, const char *fromDate, const char *toDate);
int   stats_by_option(const ShopData *shop, OptionType type, StatEntry out[], int maxOut);
int   stats_top_sellers(const ShopData *shop, StatEntry out[], int maxOut); /* combos, sorted */
int   stats_by_day(const ShopData *shop, const char *fromDate, const char *toDate,
                   StatEntry out[], int maxOut);
void  stats_report_ui(const ShopData *shop);   /* manager submenu */

#endif /* STATS_H */
