/*
 * File Name   : common.h
 * Module      : Common definitions (shared by ALL members)
 * Author      : Member A (owner) - changes must be agreed by the whole group
 * Description : Constants, return codes and core data structures of the
 *               Pizza Ordering System. All money is stored as an integer
 *               number of fen (1 yuan = 100 fen) to avoid float rounding.
 * Version     : 0.1 (design draft, 2026-11-10)
 */
#ifndef COMMON_H
#define COMMON_H

/* ---------- String length limits (including the trailing '\0') ---------- */
#define NAME_LEN        32      /* customer name / menu option name        */
#define PHONE_LEN       16      /* telephone, digits only, 5..15 chars      */
#define MIN_PHONE_LEN   5       /* shortest telephone (digits)              */
#define PASSWORD_LEN    17      /* password, 6..16 chars                    */
#define MIN_PASSWORD_LEN 6      /* shortest password                        */
#define DATETIME_LEN    17      /* "YYYY-MM-DD HH:MM"                       */
#define DATE_LEN        11      /* "YYYY-MM-DD"                             */
#define LINE_LEN        256     /* one line of keyboard / file input        */

/* ---------- Capacity limits ---------- */
#define MAX_CRUSTS          10
#define MAX_SIZES           10
#define MAX_TOPPINGS        20
#define MAX_CUSTOMERS       500
#define MAX_ORDERS          2000
#define MAX_ITEMS_PER_ORDER 10      /* different pizza lines in one order */
#define MAX_QTY_PER_ITEM    20      /* quantity of one line               */

/* ---------- Business rules (proposed values, fen) ---------- */
#define MIN_DEPOSIT         100L        /* 1.00 yuan                  */
#define MAX_DEPOSIT         500000L     /* 5000.00 yuan per deposit   */
#define MAX_BALANCE         9999999L    /* 99999.99 yuan              */
#define MIN_PRICE           0L          /* surcharge may be 0         */
#define MAX_PRICE           100000L     /* 1000.00 yuan per component */
#define FIRST_ACCOUNT_NO    100001      /* 6-digit account numbers    */
#define MAX_ACCOUNT_NO      999999      /* largest 6-digit account    */
#define FIRST_ORDER_ID      1
#define MAX_LOGIN_ATTEMPTS  3

/* ---------- Data files ---------- */
#define MENU_FILE       "data/menu.txt"
#define CUSTOMER_FILE   "data/customers.txt"
#define ORDER_FILE      "data/orders.txt"
#define MANAGER_FILE    "data/manager.txt"
#define FIELD_SEP       '|'

typedef long Money;     /* amount in fen */

/* ---------- Unified return codes (>= 0 means success) ---------- */
typedef enum {
    ST_OK          =  0,
    ST_ERR_INPUT   = -1,   /* invalid / out-of-range input  */
    ST_ERR_NOTFOUND= -2,   /* record not found              */
    ST_ERR_FULL    = -3,   /* array capacity reached        */
    ST_ERR_FILE    = -4,   /* file open / read / write fail */
    ST_ERR_BALANCE = -5,   /* insufficient balance          */
    ST_ERR_AUTH    = -6,   /* wrong account / password      */
    ST_CANCEL      = -7,   /* user cancelled (entered 0)    */
    ST_EOF         = -8,   /* end of input stream           */
    ST_ERR_DUP     = -9    /* duplicate (e.g. name exists)  */
} Status;

/* The three kinds of menu options a pizza is built from */
typedef enum {
    OPT_CRUST = 0,
    OPT_SIZE,
    OPT_TOPPING,
    OPT_TYPE_COUNT
} OptionType;

/* One selectable option: a crust, a size or a topping */
typedef struct {
    int   id;                 /* unique inside its type, never reused   */
    char  name[NAME_LEN];     /* e.g. "Hand Tossed", "12 inch"          */
    Money price;              /* base price (size) or surcharge (others)*/
    int   available;          /* 1 = can be ordered, 0 = hidden          */
} MenuOption;

/* The whole pizza menu */
typedef struct {
    MenuOption crusts[MAX_CRUSTS];
    int        crustCount;
    MenuOption sizes[MAX_SIZES];
    int        sizeCount;
    MenuOption toppings[MAX_TOPPINGS];
    int        toppingCount;
} Menu;

/* A customer account */
typedef struct {
    int   accountNo;              /* unique 6-digit number, never reused */
    char  name[NAME_LEN];
    char  phone[PHONE_LEN];
    char  password[PASSWORD_LEN];
    Money balance;
    int   active;                 /* 1 = normal, 0 = removed (soft delete) */
    char  createdAt[DATETIME_LEN];
} Customer;

/* One line of an order. Names are copied (snapshot) so that history and
 * statistics stay correct even if the manager later edits the menu.     */
typedef struct {
    char  crust[NAME_LEN];
    char  size[NAME_LEN];
    char  topping[NAME_LEN];
    int   quantity;
    Money unitPrice;              /* price of ONE pizza at order time */
} OrderItem;

/* A completed order */
typedef struct {
    int       orderId;
    int       accountNo;
    char      datetime[DATETIME_LEN];
    OrderItem items[MAX_ITEMS_PER_ORDER];
    int       itemCount;
    Money     total;
} Order;

/* Shop manager credentials */
typedef struct {
    char username[NAME_LEN];
    char password[PASSWORD_LEN];
} Manager;

/* All run-time data, created once in main() and passed by pointer
 * (no global variables).                                             */
typedef struct {
    Menu     menu;
    Customer customers[MAX_CUSTOMERS];
    int      customerCount;
    Order    orders[MAX_ORDERS];
    int      orderCount;
    Manager  manager;
    int      nextAccountNo;
    int      nextOrderId;
} ShopData;

#endif /* COMMON_H */
