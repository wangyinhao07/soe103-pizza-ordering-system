/*
 * File Name   : main.c
 * Module      : Program entry and top-level menus
 * Author      : En (Member A, team leader)
 * Description : Starts the program, loads data, shows the main menu,
 *               and sends the user to the manager menu or the customer
 *               menu. On Exit, or when input ends, it saves and frees memory.
 *               Other members' screens are called from here. Several of
 *               them still print [TODO] until that member finishes them.
 * Version     : 1.0  2026-11-10
 *
 * Program outline
 *   1. Create one ShopData object on the heap. It is about 2 MB, which
 *      is too large to put on the stack.
 *   2. Ask fileio to load the data files. Missing pieces use defaults.
 *   3. If the customer list is still empty (Member E's loader is a stub),
 *      read data/customers.txt here so login can be demonstrated.
 *   4. Repeat the main menu until the user chooses Exit, or until
 *      Ctrl+D / Ctrl+Z ends the input.
 *   5. Save, free the memory, and say goodbye.
 *
 * There are no global variables. Every module receives the ShopData pointer.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "common.h"
#include "utils.h"
#include "fileio.h"
#include "auth.h"
#include "menu.h"
#include "customer.h"
#include "order.h"
#include "stats.h"

/* Prototypes for the functions in this file. main() is the story;
 * the helpers below it do one job each. */
static void manager_menu(ShopData *shop);
static void customer_menu(ShopData *shop, int custIndex);
static int input_ended(void);
static void ensure_customers_loaded(ShopData *shop);

/*
 * Function : input_ended
 * Purpose  : True after a read has already hit end of input.
 *            feof stays true, so the menus can stop without asking again.
 */
static int input_ended(void)
{
    return feof(stdin) || ferror(stdin);
}

/*
 * Function : manager_menu
 * Purpose  : Options shown after a successful manager login.
 *            0 logs out and returns to the main menu. Nothing is deleted.
 */
static void manager_menu(ShopData *shop)
{
    int choice;
    int rc;
    int pwdRc;

    if (shop == NULL) {
        return;
    }

    for (;;) {
        printf("\n===== Shop Manager =====\n");
        printf("1. Edit pizza menu and prices\n");
        printf("2. Manage customers\n");
        printf("3. Sales statistics\n");
        printf("4. Change manager password\n");
        printf("0. Log out\n");

        rc = read_int("Choice: ", 0, 4, &choice);
        if (rc != ST_OK) {
            /* End of input, or a programmer error in the range. Stop. */
            return;
        }
        if (choice == 0) {
            printf("Logged out.\n");
            return;
        }

        switch (choice) {
        case 1:
            menu_manage_ui(shop);              /* Member B */
            break;
        case 2:
            customer_manage_ui(shop);          /* Member C */
            break;
        case 3:
            stats_report_ui(shop);             /* Member E */
            break;
        case 4:
            pwdRc = change_manager_password_ui(shop);  /* Member A */
            if (pwdRc == ST_CANCEL) {
                printf("Password change cancelled.\n");
            } else if (pwdRc != ST_OK && pwdRc != ST_EOF) {
                printf("Password was not changed.\n");
            }
            break;
        default:
            /* read_int already limits the range. This is a safety net. */
            break;
        }

        /* Sub-screens print their result and return. Pause so the
         * user can read it before the menu is drawn again. */
        if (!input_ended()) {
            pause_for_enter();
        }
        if (input_ended()) {
            return;
        }
    }
}

/*
 * Function : customer_menu
 * Purpose  : Options shown after a successful customer login.
 * Params   : custIndex - index into shop->customers, not the account number
 */
static void customer_menu(ShopData *shop, int custIndex)
{
    int choice;
    int rc;
    Customer *cust;

    if (shop == NULL || custIndex < 0 || custIndex >= shop->customerCount) {
        printf("  ! Customer session is invalid.\n");
        return;
    }

    for (;;) {
        cust = &shop->customers[custIndex];
        printf("\n===== Customer: %s =====\n", cust->name);
        customer_print(cust);
        printf("1. Place an order\n");
        printf("2. Order history\n");
        printf("3. Deposit money\n");
        printf("4. Edit my phone / password\n");
        printf("0. Log out\n");

        rc = read_int("Choice: ", 0, 4, &choice);
        if (rc != ST_OK) {
            return;
        }
        if (choice == 0) {
            printf("Logged out.\n");
            return;
        }

        switch (choice) {
        case 1:
            order_place_ui(shop, custIndex);             /* Member D */
            break;
        case 2:
            order_history_ui(shop, cust->accountNo);     /* Member D */
            break;
        case 3:
            customer_deposit_ui(shop, custIndex);        /* Member C */
            break;
        case 4:
            customer_edit_self_ui(shop, custIndex);      /* Member C */
            break;
        default:
            break;
        }

        if (!input_ended()) {
            pause_for_enter();
        }
        if (input_ended()) {
            return;
        }
    }
}

/* --------------------------------------------------------------------- */
/* Temporary customer loader                                              */
/*                                                                        */
/* Member E owns fileio.c. Today load_customers() is still a stub, so    */
/* the list is empty and nobody can log in. The functions below read     */
/* data/customers.txt (design draft section 3.4).                         */
/*                                                                        */
/* They do nothing once load_customers() fills shop->customerCount.      */
/* Delete ensure_customers_loaded() and its helpers at that point.       */
/* --------------------------------------------------------------------- */

/*
 * Function : split_pipe
 * Purpose  : Split line into fields separated by '|'. The '|' characters
 *            are replaced with '\0', so each fields[i] is its own string.
 *            line must be writable. Returns the field count, or -1 when
 *            there are more fields than maxFields.
 */
static int split_pipe(char *line, char *fields[], int maxFields)
{
    int count;
    char *p;

    if (line == NULL || fields == NULL || maxFields <= 0) {
        return -1;
    }

    fields[0] = line;
    count = 1;
    for (p = line; *p != '\0'; p++) {
        if (*p == FIELD_SEP) {
            if (count >= maxFields) {
                return -1;
            }
            *p = '\0';
            fields[count] = p + 1;
            count++;
        }
    }
    return count;
}

/*
 * Function : read_file_line
 * Purpose  : Like read_line, but from a file. Does not print messages.
 * Returns  : ST_OK, ST_ERR_INPUT (line longer than buf), or ST_EOF.
 */
static int read_file_line(FILE *fp, char *buf, size_t size)
{
    size_t len;
    int ch;

    if (fgets(buf, (int)size, fp) == NULL) {
        buf[0] = '\0';
        return ST_EOF;
    }

    len = strlen(buf);
    if (len > 0 && buf[len - 1] == '\n') {
        buf[len - 1] = '\0';
    } else if (!feof(fp)) {
        while ((ch = fgetc(fp)) != '\n' && ch != EOF) {
            /* Drop the rest of a line that did not fit in buf. */
        }
        buf[0] = '\0';
        return ST_ERR_INPUT;
    }

    trim(buf); /* also removes a Windows '\r' left before the newline */
    return ST_OK;
}

/*
 * Function : parse_customer_line
 * Purpose  : Fill one Customer from
 *            accountNo|name|phone|password|balance_fen|active|createdAt
 *            The line is modified by the split. Returns ST_OK or an error.
 *            On error, *out is left untouched.
 */
static int parse_customer_line(char *line, Customer *out)
{
    char *fields[7];
    int fieldCount;
    int i;
    char *end;
    long accountNo;
    long balance;
    long active;
    Customer rec;

    fieldCount = split_pipe(line, fields, 7);
    if (fieldCount != 7) {
        return ST_ERR_INPUT;
    }
    for (i = 0; i < 7; i++) {
        trim(fields[i]);
    }

    errno = 0;
    accountNo = strtol(fields[0], &end, 10);
    if (errno != 0 || end == fields[0] || *end != '\0') {
        return ST_ERR_INPUT;
    }
    if (accountNo < FIRST_ACCOUNT_NO || accountNo > MAX_ACCOUNT_NO) {
        return ST_ERR_INPUT;
    }

    if (!is_valid_name(fields[1]) || !is_valid_phone(fields[2])
        || !is_valid_password(fields[3])) {
        return ST_ERR_INPUT;
    }

    errno = 0;
    balance = strtol(fields[4], &end, 10);
    if (errno != 0 || end == fields[4] || *end != '\0') {
        return ST_ERR_INPUT;
    }
    if (balance < 0 || balance > MAX_BALANCE) {
        return ST_ERR_INPUT;
    }

    errno = 0;
    active = strtol(fields[5], &end, 10);
    if (errno != 0 || end == fields[5] || *end != '\0') {
        return ST_ERR_INPUT;
    }
    if (active != 0 && active != 1) {
        return ST_ERR_INPUT;
    }

    if (fields[6][0] == '\0' || strlen(fields[6]) >= DATETIME_LEN) {
        return ST_ERR_INPUT;
    }

    /* Every check passed, so it is safe to publish the record. */
    rec.accountNo = (int)accountNo;
    snprintf(rec.name, NAME_LEN, "%s", fields[1]);
    snprintf(rec.phone, PHONE_LEN, "%s", fields[2]);
    snprintf(rec.password, PASSWORD_LEN, "%s", fields[3]);
    rec.balance = balance;
    rec.active = (int)active;
    snprintf(rec.createdAt, DATETIME_LEN, "%s", fields[6]);
    *out = rec;
    return ST_OK;
}

/*
 * Function : append_customer_line
 * Purpose  : Parse one data line and append it when the account is new.
 */
static int append_customer_line(ShopData *shop, char *line)
{
    Customer rec;
    int i;

    if (parse_customer_line(line, &rec) != ST_OK) {
        return ST_ERR_INPUT;
    }
    for (i = 0; i < shop->customerCount; i++) {
        if (shop->customers[i].accountNo == rec.accountNo) {
            return ST_ERR_DUP;
        }
    }
    if (shop->customerCount >= MAX_CUSTOMERS) {
        return ST_ERR_FULL;
    }
    shop->customers[shop->customerCount] = rec;
    shop->customerCount++;
    return ST_OK;
}

/*
 * Function : load_customers_from_file
 * Purpose  : Read path into shop. Blank lines and lines starting with '#'
 *            are skipped. A bad line is skipped with a warning.
 * Returns  : ST_OK if the file was opened (even when it added nobody),
 *            ST_ERR_FILE if it could not be opened.
 */
static int load_customers_from_file(ShopData *shop, const char *path)
{
    FILE *fp;
    char line[LINE_LEN];
    int lineNo;
    int rc;

    fp = fopen(path, "r");
    if (fp == NULL) {
        return ST_ERR_FILE;
    }

    lineNo = 0;
    while ((rc = read_file_line(fp, line, sizeof line)) != ST_EOF) {
        lineNo++;
        if (rc == ST_ERR_INPUT) {
            printf("Warning: line %d in %s is too long and was skipped.\n",
                   lineNo, path);
            continue;
        }
        if (line[0] == '\0' || line[0] == '#') {
            continue;
        }

        rc = append_customer_line(shop, line);
        if (rc == ST_OK) {
            continue;
        }
        if (rc == ST_ERR_FULL) {
            printf("Warning: customer list is full. Stopped reading %s.\n",
                   path);
            break;
        }
        if (rc == ST_ERR_DUP) {
            printf("Warning: line %d in %s duplicates an account "
                   "and was skipped.\n", lineNo, path);
        } else {
            printf("Warning: line %d in %s has a bad format "
                   "and was skipped.\n", lineNo, path);
        }
    }

    if (fclose(fp) != 0) {
        printf("Warning: could not close %s after reading.\n", path);
    }
    return ST_OK;
}

/*
 * Function : load_builtin_customers
 * Purpose  : Last-resort copies of the three accounts in the design draft,
 *            used only when data/customers.txt cannot be read.
 *            100003 Carol is inactive and must not be able to log in.
 */
static void load_builtin_customers(ShopData *shop)
{
    static const char *lines[] = {
        "100001|Alice Zhang|13800000001|alice123|20000|1|2026-11-20 10:00",
        "100002|Bob Li|13800000002|bob12345|500|1|2026-11-20 10:05",
        "100003|Carol Wang|13800000003|carol123|0|0|2026-11-20 10:10"
    };
    int count;
    int i;
    char line[LINE_LEN];

    /* Copy each literal into a writable buffer: parsing inserts '\0'. */
    count = (int)(sizeof lines / sizeof lines[0]);
    for (i = 0; i < count; i++) {
        snprintf(line, sizeof line, "%s", lines[i]);
        if (append_customer_line(shop, line) != ST_OK) {
            printf("Warning: built-in demo account %d was not loaded.\n",
                   i + 1);
        }
    }
}

/*
 * Function : update_next_account_no
 * Purpose  : Set the next new account number to (largest loaded number + 1).
 *            Removed accounts count too, so their numbers are never reused.
 *            Member C's customer_add() will read this field.
 */
static void update_next_account_no(ShopData *shop)
{
    int i;
    int nextNo;

    nextNo = FIRST_ACCOUNT_NO;
    for (i = 0; i < shop->customerCount; i++) {
        if (shop->customers[i].accountNo >= nextNo) {
            nextNo = shop->customers[i].accountNo + 1;
        }
    }
    shop->nextAccountNo = nextNo;
}

/*
 * Function : ensure_customers_loaded
 * Purpose  : Fill the customer list when Member E's loader has not done so.
 */
static void ensure_customers_loaded(ShopData *shop)
{
    if (shop->customerCount > 0) {
        /* Real loader already ran. Do not add the demo accounts on top. */
        return;
    }

    if (load_customers_from_file(shop, CUSTOMER_FILE) != ST_OK) {
        printf("Warning: could not open %s. Using built-in demo accounts.\n",
               CUSTOMER_FILE);
        load_builtin_customers(shop);
    } else if (shop->customerCount == 0) {
        printf("Warning: %s has no usable accounts. "
               "Using built-in demo accounts.\n", CUSTOMER_FILE);
        load_builtin_customers(shop);
    }

    update_next_account_no(shop);
    printf("Customer accounts ready: %d\n", shop->customerCount);
}

/*
 * Function : main
 * Purpose  : Program entry. Load, menu loop, save, free.
 */
int main(void)
{
    ShopData *shop;
    int choice;
    int rc;
    int custIndex;
    int running;

    printf("===== Pizza Ordering System =====\n");
    printf("Loading data...\n");

    /* calloc zeroes the memory, so counts start at 0 and strings start empty. */
    shop = (ShopData *)calloc(1, sizeof *shop);
    if (shop == NULL) {
        printf("Fatal: not enough memory to start.\n");
        return EXIT_FAILURE;
    }

    /* Member E: loads files, or leaves the defaults when a file is missing.
     * The menu and the manager account already have working defaults. */
    if (load_all_data(shop) != ST_OK) {
        printf("Warning: could not load every data file. "
               "Continuing with defaults.\n");
    }
    ensure_customers_loaded(shop);

    running = 1;
    while (running) {
        if (input_ended()) {
            break;
        }

        printf("\n===== Pizza Ordering System =====\n");
        printf("1. View menu\n");
        printf("2. Customer login\n");
        printf("3. Register a new account\n");
        printf("4. Shop manager login\n");
        printf("0. Exit\n");
        printf("Tip: enter 0 to go back or log out.\n");

        rc = read_int("Choice: ", 0, 4, &choice);
        if (rc != ST_OK) {
            /* Ctrl+D / Ctrl+Z. Fall out of the loop and save. */
            break;
        }

        switch (choice) {
        case 0:
            running = 0;
            break;
        case 1:
            menu_display(&shop->menu, 0); /* 0 = hide options that are off */
            if (!input_ended()) {
                pause_for_enter();
            }
            break;
        case 2:
            custIndex = customer_login_ui(shop);
            if (custIndex >= 0) {
                customer_menu(shop, custIndex);
            }
            /* A negative result is "failed", "cancelled", or end of input.
             * Index 0 would be the first customer, and that is success. */
            break;
        case 3:
            customer_register_ui(shop); /* Member C */
            if (!input_ended()) {
                pause_for_enter();
            }
            break;
        case 4:
            if (manager_login_ui(shop) == ST_OK) {
                manager_menu(shop);
            }
            break;
        default:
            break;
        }
    }

    printf("\nSaving data...\n");
    if (save_all_data(shop) != ST_OK) {
        printf("Warning: some data could not be saved.\n");
    }
    free(shop);
    printf("Goodbye!\n");
    return EXIT_SUCCESS;
}
