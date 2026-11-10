/*
 * File Name   : stats.c
 * Module      : Sales statistics
 * Author      : Member E
 * Status      : SKELETON - bodies are TODO.
 */
#include <stdio.h>
#include <string.h>
#include "stats.h"

/* Return 1 if datetime "YYYY-MM-DD HH:MM" lies in [from,to] (NULL = open) */
static int in_range(const char *datetime, const char *from, const char *to)
{
    if (from != NULL && strncmp(datetime, from, DATE_LEN - 1) < 0) return 0;
    if (to != NULL && strncmp(datetime, to, DATE_LEN - 1) > 0) return 0;
    return 1;
}

Money stats_total_revenue(const ShopData *shop, const char *fromDate, const char *toDate)
{
    Money sum = 0;
    int i;
    for (i = 0; i < shop->orderCount; i++) {
        if (in_range(shop->orders[i].datetime, fromDate, toDate)) sum += shop->orders[i].total;
    }
    return sum;
}

int stats_total_pizzas(const ShopData *shop, const char *fromDate, const char *toDate)
{
    (void)shop; (void)fromDate; (void)toDate;
    printf("[TODO] stats_total_pizzas\n");
    return 0;
}

int stats_order_count(const ShopData *shop, const char *fromDate, const char *toDate)
{
    (void)shop; (void)fromDate; (void)toDate;
    printf("[TODO] stats_order_count\n");
    return 0;
}

int stats_by_option(const ShopData *shop, OptionType type, StatEntry out[], int maxOut)
{
    (void)shop; (void)type; (void)out; (void)maxOut;
    printf("[TODO] stats_by_option\n");
    return 0;
}

int stats_top_sellers(const ShopData *shop, StatEntry out[], int maxOut)
{
    (void)shop; (void)out; (void)maxOut;
    printf("[TODO] stats_top_sellers\n");
    return 0;
}

int stats_by_day(const ShopData *shop, const char *fromDate, const char *toDate,
                 StatEntry out[], int maxOut)
{
    (void)shop; (void)fromDate; (void)toDate; (void)out; (void)maxOut;
    printf("[TODO] stats_by_day\n");
    return 0;
}

void stats_report_ui(const ShopData *shop)
{
    (void)shop;
    printf("[TODO] stats_report_ui: summary / by crust / by size / by topping / top sellers / by day\n");
}
