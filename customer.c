/*
 * File Name   : customer.c
 * Module      : Customer account management
 * Author      : Fanxiang.Meng 2576959
 * Description : Register, search, edit and remove customer accounts, and
 *               deposit money into an account's balance.
 *               The logic functions (customer_*) never read the keyboard
 *               and never print: they return a Status code, an account
 *               number or an array index. The *_ui functions ask the
 *               questions, show the result, and save data/customers.txt
 *               after every change (design draft sections 2.2, 2.4, 2.9).
 * Version     : 1.0  2026-11-23
 */
#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "customer.h"
#include "fileio.h"
#include "order.h"
#include "utils.h"

/* ------------------------------------------------------------------ */
/* Internal helpers. They are static, so they are invisible outside    */
/* this file and cannot clash with another member's function names.    */
/* ------------------------------------------------------------------ */

static int  find_by_phone(const ShopData *shop, const char *phone,
                          int skipIndex);
static int  find_any(const ShopData *shop, int accountNo);
static int  count_active(const ShopData *shop);
static int  next_account_number(const ShopData *shop);
static int  save_customer_file(const ShopData *shop);
static int  contains_ci(const char *haystack, const char *needle);
static void print_table_header(void);
static void print_table_row(const Customer *c);
static void print_name_rule(void);
static void print_phone_rule(void);
static void print_password_rule(void);
static int  ask_account(const ShopData *shop, int activeOnly, int *outIndex);
static int  create_account_ui(ShopData *shop, const char *title);
static void manage_add(ShopData *shop);
static void manage_edit(ShopData *shop);
static void manage_reset_password(ShopData *shop);
static void manage_remove(ShopData *shop);
static void manage_search(ShopData *shop);
static void manage_history(ShopData *shop);
static void change_own_phone(ShopData *shop, int custIndex);
static void change_own_password(ShopData *shop, int custIndex);

/*
 * Function : find_by_phone
 * Purpose  : Find an active customer that already uses this phone number.
 *            The design proposal forbids two active accounts with the same
 *            phone, so the manager can always look a customer up by phone.
 * Params   : shop      - all shop data
 *            phone     - number to look for (not NULL)
 *            skipIndex - record to ignore, or -1 to ignore nobody (used
 *                        when a customer keeps his own number while editing)
 * Returns  : the index, or ST_ERR_NOTFOUND
 */
static int find_by_phone(const ShopData *shop, const char *phone, int skipIndex)
{
    int i;

    if (shop == NULL || phone == NULL) {
        return ST_ERR_NOTFOUND;
    }
    for (i = 0; i < shop->customerCount; i++) {
        if (i == skipIndex) {
            continue;
        }
        if (shop->customers[i].active
            && strcmp(shop->customers[i].phone, phone) == 0) {
            return i;
        }
    }
    return ST_ERR_NOTFOUND;
}

/*
 * Function : find_any
 * Purpose  : Find a customer by account number, removed accounts included.
 *            Needed to show the order history of a removed account, because
 *            customer_find() only returns active ones.
 * Returns  : the index, or ST_ERR_NOTFOUND
 */
static int find_any(const ShopData *shop, int accountNo)
{
    int i;

    if (shop == NULL) {
        return ST_ERR_NOTFOUND;
    }
    for (i = 0; i < shop->customerCount; i++) {
        if (shop->customers[i].accountNo == accountNo) {
            return i;
        }
    }
    return ST_ERR_NOTFOUND;
}

/*
 * Function : count_active
 * Purpose  : How many records still have active == 1.
 * Returns  : the number of active accounts
 */
static int count_active(const ShopData *shop)
{
    int i;
    int n;

    if (shop == NULL) {
        return 0;
    }
    n = 0;
    for (i = 0; i < shop->customerCount; i++) {
        if (shop->customers[i].active) {
            n++;
        }
    }
    return n;
}

/*
 * Function : next_account_number
 * Purpose  : Choose the account number for the next new customer.
 *            Numbers start at FIRST_ACCOUNT_NO and are never reused, so a
 *            removed account still blocks its own number. The scan makes
 *            the result correct even if shop->nextAccountNo is stale (for
 *            example after another module edited the list).
 * Returns  : the number, or ST_ERR_FULL when MAX_ACCOUNT_NO is used up
 */
static int next_account_number(const ShopData *shop)
{
    int candidate;
    int i;

    candidate = shop->nextAccountNo;
    if (candidate < FIRST_ACCOUNT_NO) {
        candidate = FIRST_ACCOUNT_NO;
    }
    for (i = 0; i < shop->customerCount; i++) {
        if (shop->customers[i].accountNo >= candidate) {
            candidate = shop->customers[i].accountNo + 1;
        }
    }
    if (candidate > MAX_ACCOUNT_NO) {
        return ST_ERR_FULL;
    }
    return candidate;
}

/*
 * Function : save_customer_file
 * Purpose  : Write data/customers.txt after a change, the way the design
 *            draft asks ("save immediately after every change").
 *            Member E owns fileio.c; this only calls the agreed interface.
 * Returns  : ST_OK, or ST_ERR_FILE after printing a warning
 */
static int save_customer_file(const ShopData *shop)
{
    if (save_customers(shop, CUSTOMER_FILE) != ST_OK) {
        printf("Warning: the change is kept for this session only; "
               "it could not be saved to %s.\n", CUSTOMER_FILE);
        return ST_ERR_FILE;
    }
    return ST_OK;
}

/*
 * Function : contains_ci
 * Purpose  : Case-insensitive "does haystack contain needle?".
 *            Used by the manager's search screen, so "alice" finds
 *            "Alice Zhang" and "1380000" finds "13800000001".
 * Returns  : 1 when it matches (an empty needle matches everything), else 0
 */
static int contains_ci(const char *haystack, const char *needle)
{
    size_t hLen;
    size_t nLen;
    size_t i;
    size_t j;

    if (haystack == NULL || needle == NULL) {
        return 0;
    }
    nLen = strlen(needle);
    if (nLen == 0) {
        return 1;
    }
    hLen = strlen(haystack);
    if (nLen > hLen) {
        return 0;
    }
    for (i = 0; i + nLen <= hLen; i++) {
        for (j = 0; j < nLen; j++) {
            if (tolower((unsigned char)haystack[i + j])
                != tolower((unsigned char)needle[j])) {
                break;
            }
        }
        if (j == nLen) {
            return 1;
        }
    }
    return 0;
}

/* Print the column titles of the customer table. */
static void print_table_header(void)
{
    printf("\n%-8s %-31s %-15s %12s\n",
           "Account", "Name", "Phone", "Balance");
    printf("-------- ------------------------------- "
           "--------------- ------------\n");
}

/* Print one customer as a table row. Widths match print_table_header(). */
static void print_table_row(const Customer *c)
{
    char money[32];

    if (c == NULL) {
        return;
    }
    format_money(c->balance, money, sizeof money);
    printf("%-8d %-31.31s %-15.15s %12s\n",
           c->accountNo, c->name, c->phone, money);
}

/* The three "format reminder" lines used by the account screens. They are
 * written once here so every screen explains the same rules. */
static void print_name_rule(void)
{
    printf("  Name     : 1 to %d characters, not only spaces, no '%c'.\n",
           NAME_LEN - 1, FIELD_SEP);
}

static void print_phone_rule(void)
{
    printf("  Phone    : %d to %d digits, no letters or spaces.\n",
           MIN_PHONE_LEN, PHONE_LEN - 1);
}

static void print_password_rule(void)
{
    printf("  Password : %d to %d characters, no spaces, no '%c'.\n",
           MIN_PASSWORD_LEN, PASSWORD_LEN - 1, FIELD_SEP);
}

/*
 * Function : ask_account
 * Purpose  : Ask for an account number and turn it into an array index.
 *            Typing 0 cancels, so it never counts as "account 0".
 * Params   : shop      - all shop data
 *            activeOnly- 1 = only an account that can log in, 0 = also a
 *                        removed account (used for order history)
 *            outIndex  - receives the index only on ST_OK
 * Returns  : ST_OK, ST_CANCEL, ST_EOF, or ST_ERR_NOTFOUND
 */
static int ask_account(const ShopData *shop, int activeOnly, int *outIndex)
{
    int accountNo;
    int index;
    int rc;

    if (shop == NULL || outIndex == NULL) {
        return ST_ERR_INPUT;
    }

    rc = read_int("Account number (0 to cancel): ", 0, MAX_ACCOUNT_NO,
                  &accountNo);
    if (rc != ST_OK) {
        return rc;
    }
    if (accountNo == 0) {
        printf("Cancelled.\n");
        return ST_CANCEL;
    }

    index = activeOnly ? customer_find(shop, accountNo)
                       : find_any(shop, accountNo);
    if (index < 0) {
        if (activeOnly) {
            printf("  ! No active customer with account %d.\n", accountNo);
        } else {
            printf("  ! No customer with account %d.\n", accountNo);
        }
        return ST_ERR_NOTFOUND;
    }

    *outIndex = index;
    return ST_OK;
}

/* ------------------------------------------------------------------ */
/* Logic layer: no printf and no keyboard input (coding rule 7).       */
/* ------------------------------------------------------------------ */

/*
 * Function : customer_find
 * Purpose  : Find an account that can still log in.
 * Params   : shop      - all shop data (not NULL)
 *            accountNo - number typed by the user
 * Returns  : the index (0 is a real index), or ST_ERR_NOTFOUND.
 *            A removed account (active == 0) is invisible here, which is
 *            what stops it from logging in.
 */
int customer_find(const ShopData *shop, int accountNo)
{
    int i;

    if (shop == NULL) {
        return ST_ERR_NOTFOUND;
    }
    for (i = 0; i < shop->customerCount; i++) {
        if (shop->customers[i].active
            && shop->customers[i].accountNo == accountNo) {
            return i;
        }
    }
    return ST_ERR_NOTFOUND;
}

/*
 * Function : customer_add
 * Purpose  : Create one new active account with a new, never reused number
 *            and a balance of 0.00. Both the self-service registration and
 *            the manager's "add customer" go through this function.
 * Params   : shop     - all shop data (not NULL)
 *            name     - 1..NAME_LEN-1 characters, no '|'
 *            phone    - MIN_PHONE_LEN..PHONE_LEN-1 digits, unique
 *            password - MIN_PASSWORD_LEN..PASSWORD_LEN-1 characters
 * Returns  : the new account number (>= FIRST_ACCOUNT_NO), or
 *            ST_ERR_INPUT, ST_ERR_DUP (phone already used),
 *            ST_ERR_FULL (list, or the six-digit numbers, are used up)
 */
int customer_add(ShopData *shop, const char *name, const char *phone,
                 const char *password)
{
    Customer rec;
    int newNo;

    if (shop == NULL || name == NULL || phone == NULL || password == NULL) {
        return ST_ERR_INPUT;
    }
    if (!is_valid_name(name) || !is_valid_phone(phone)
        || !is_valid_password(password)) {
        return ST_ERR_INPUT;
    }
    if (shop->customerCount < 0 || shop->customerCount >= MAX_CUSTOMERS) {
        return ST_ERR_FULL;
    }
    if (find_by_phone(shop, phone, -1) >= 0) {
        return ST_ERR_DUP;
    }

    newNo = next_account_number(shop);
    if (newNo < 0) {
        return newNo;
    }

    /* Build the record in a local variable first, so a failure in the
     * middle of these lines cannot leave a half-written record behind. */
    memset(&rec, 0, sizeof rec);
    rec.accountNo = newNo;
    snprintf(rec.name, NAME_LEN, "%s", name);
    snprintf(rec.phone, PHONE_LEN, "%s", phone);
    snprintf(rec.password, PASSWORD_LEN, "%s", password);
    rec.balance = 0;
    rec.active = 1;
    get_current_datetime(rec.createdAt, sizeof rec.createdAt);

    shop->customers[shop->customerCount] = rec;
    shop->customerCount++;
    shop->nextAccountNo = newNo + 1;
    return newNo;
}

/*
 * Function : customer_update
 * Purpose  : Change the name and/or the phone number of an active account.
 *            A NULL argument means "keep the old value", which is how the
 *            manager edit screen leaves one field untouched.
 * Params   : shop      - all shop data (not NULL)
 *            accountNo - account to change
 *            name      - new name, or NULL
 *            phone     - new phone, or NULL
 * Returns  : ST_OK, ST_ERR_NOTFOUND, ST_ERR_INPUT (bad text),
 *            ST_ERR_DUP (phone used by another active account)
 */
int customer_update(ShopData *shop, int accountNo, const char *name,
                    const char *phone)
{
    int index;
    int other;

    if (shop == NULL) {
        return ST_ERR_INPUT;
    }
    index = customer_find(shop, accountNo);
    if (index < 0) {
        return ST_ERR_NOTFOUND;
    }

    /* Check both fields before changing either one, so a rejected phone
     * cannot leave a new name behind. */
    if (name != NULL && !is_valid_name(name)) {
        return ST_ERR_INPUT;
    }
    if (phone != NULL && !is_valid_phone(phone)) {
        return ST_ERR_INPUT;
    }
    if (phone != NULL) {
        other = find_by_phone(shop, phone, index);
        if (other >= 0) {
            return ST_ERR_DUP;
        }
    }

    if (name != NULL) {
        snprintf(shop->customers[index].name, NAME_LEN, "%s", name);
    }
    if (phone != NULL) {
        snprintf(shop->customers[index].phone, PHONE_LEN, "%s", phone);
    }
    return ST_OK;
}

/*
 * Function : customer_set_password
 * Purpose  : Replace the password of an active account.
 *            The manager may reset it without knowing the old one; a
 *            customer changing his own password must type the old one in
 *            the UI first (customer_edit_self_ui).
 * Returns  : ST_OK, ST_ERR_NOTFOUND, ST_ERR_INPUT (bad format)
 */
int customer_set_password(ShopData *shop, int accountNo, const char *password)
{
    int index;

    if (shop == NULL || password == NULL) {
        return ST_ERR_INPUT;
    }
    if (!is_valid_password(password)) {
        return ST_ERR_INPUT;
    }
    index = customer_find(shop, accountNo);
    if (index < 0) {
        return ST_ERR_NOTFOUND;
    }

    snprintf(shop->customers[index].password, PASSWORD_LEN, "%s", password);
    return ST_OK;
}

/*
 * Function : customer_remove
 * Purpose  : Soft delete. active becomes 0, so the account can no longer
 *            log in and disappears from every list, but the record and its
 *            order history stay in the file and in the statistics.
 *            The balance is kept on the record as proof of what is owed to
 *            the customer (the manager refunds it offline).
 * Returns  : ST_OK, ST_ERR_NOTFOUND (unknown or already removed)
 */
int customer_remove(ShopData *shop, int accountNo)
{
    int index;

    if (shop == NULL) {
        return ST_ERR_INPUT;
    }
    index = customer_find(shop, accountNo);
    if (index < 0) {
        return ST_ERR_NOTFOUND;
    }

    shop->customers[index].active = 0;
    return ST_OK;
}

/*
 * Function : customer_deposit
 * Purpose  : Add money to a customer's balance.
 * Params   : c      - customer to update (not NULL)
 *            amount - amount in fen, MIN_DEPOSIT..MAX_DEPOSIT
 * Returns  : ST_OK, or ST_ERR_INPUT when amount is out of range or the
 *            new balance would pass MAX_BALANCE
 */
int customer_deposit(Customer *c, Money amount)
{
    if (c == NULL) {
        return ST_ERR_INPUT;
    }
    if (amount < MIN_DEPOSIT || amount > MAX_DEPOSIT) {
        return ST_ERR_INPUT;
    }
    /* amount <= MAX_DEPOSIT and balance <= MAX_BALANCE, so this addition
     * cannot overflow a long. */
    if (c->balance + amount > MAX_BALANCE) {
        return ST_ERR_INPUT;
    }
    c->balance += amount;
    return ST_OK;
}

/*
 * Function : customer_charge
 * Purpose  : Take money out of a customer's balance when an order is paid.
 *            A balance can never become negative.
 * Returns  : ST_OK, ST_ERR_INPUT (negative amount), ST_ERR_BALANCE (too little)
 */
int customer_charge(Customer *c, Money amount)
{
    if (c == NULL) {
        return ST_ERR_INPUT;
    }
    if (amount < 0) {
        return ST_ERR_INPUT;
    }
    if (c->balance < amount) {
        return ST_ERR_BALANCE;
    }
    c->balance -= amount;
    return ST_OK;
}

/* ------------------------------------------------------------------ */
/* Display helpers                                                     */
/* ------------------------------------------------------------------ */

/*
 * Function : customer_print
 * Purpose  : Print one customer on a single line. main.c uses it as the
 *            heading of the customer menu, and the manager screens use it
 *            to show the record before a change.
 */
void customer_print(const Customer *c)
{
    char money[32];

    if (c == NULL) {
        return;
    }
    format_money(c->balance, money, sizeof money);
    printf("Account %d | %s | %s | balance %s\n",
           c->accountNo, c->name, c->phone, money);
}

/*
 * Function : customer_list_all
 * Purpose  : Print every active account as a table, oldest account first.
 *            Removed accounts are left out; the file keeps them.
 */
void customer_list_all(const ShopData *shop)
{
    char limit[32];
    int i;
    int shown;

    if (shop == NULL) {
        return;
    }

    print_table_header();
    shown = 0;
    for (i = 0; i < shop->customerCount; i++) {
        if (!shop->customers[i].active) {
            continue;
        }
        print_table_row(&shop->customers[i]);
        shown++;
    }
    if (shown == 0) {
        printf("(no active customers)\n");
        return;
    }

    format_money(MAX_BALANCE, limit, sizeof limit);
    printf("\n%d active customer(s). The balance limit is %s per account.\n",
           shown, limit);
}

/* ------------------------------------------------------------------ */
/* Creating an account: shared by self-registration and the manager    */
/* ------------------------------------------------------------------ */

/*
 * Function : create_account_ui
 * Purpose  : Ask for name, phone and password (twice) and create the
 *            account. Nothing is added until all three fields are valid,
 *            so a mistake at the last prompt does not leave a half-made
 *            account behind.
 * Params   : shop  - all shop data
 *            title - heading printed above the questions
 * Returns  : the new account number, ST_CANCEL (typed 0), ST_EOF, or the
 *            negative error code of customer_add / the save call
 */
static int create_account_ui(ShopData *shop, const char *title)
{
    char name[LINE_LEN];
    char phone[LINE_LEN];
    char password[LINE_LEN];
    char confirm[LINE_LEN];
    int rc;

    printf("\n----- %s -----\n", title);
    printf("Enter 0 at any prompt to cancel.\n");
    print_name_rule();
    print_phone_rule();
    print_password_rule();

    /* --- name --- */
    for (;;) {
        rc = read_line("Name: ", name, sizeof name);
        if (rc == ST_EOF) {
            return ST_EOF;
        }
        if (rc == ST_ERR_INPUT) {
            continue; /* read_line already explained the problem */
        }
        if (strcmp(name, "0") == 0) {
            printf("Cancelled.\n");
            return ST_CANCEL;
        }
        if (!is_valid_name(name)) {
            printf("  ! Name must be 1 to %d characters, not only spaces, "
                   "and must not contain '%c'.\n", NAME_LEN - 1, FIELD_SEP);
            continue;
        }
        break;
    }

    /* --- phone --- */
    for (;;) {
        rc = read_line("Phone: ", phone, sizeof phone);
        if (rc == ST_EOF) {
            return ST_EOF;
        }
        if (rc == ST_ERR_INPUT) {
            continue;
        }
        if (strcmp(phone, "0") == 0) {
            printf("Cancelled.\n");
            return ST_CANCEL;
        }
        if (!is_valid_phone(phone)) {
            printf("  ! Phone must be %d to %d digits, digits only.\n",
                   MIN_PHONE_LEN, PHONE_LEN - 1);
            continue;
        }
        if (find_by_phone(shop, phone, -1) >= 0) {
            printf("  ! That phone number already belongs to another "
                   "account. Please use a different one.\n");
            continue;
        }
        break;
    }

    /* --- password, typed twice --- */
    for (;;) {
        rc = read_line("Password: ", password, sizeof password);
        if (rc == ST_EOF) {
            return ST_EOF;
        }
        if (rc == ST_ERR_INPUT) {
            continue;
        }
        if (strcmp(password, "0") == 0) {
            printf("Cancelled.\n");
            return ST_CANCEL;
        }
        if (!is_valid_password(password)) {
            printf("  ! Password must be %d to %d characters,\n"
                   "    with no spaces and no '%c'.\n",
                   MIN_PASSWORD_LEN, PASSWORD_LEN - 1, FIELD_SEP);
            continue;
        }

        rc = read_line("Confirm password: ", confirm, sizeof confirm);
        if (rc == ST_EOF) {
            return ST_EOF;
        }
        if (rc == ST_ERR_INPUT) {
            continue;
        }
        if (strcmp(confirm, "0") == 0) {
            printf("Cancelled.\n");
            return ST_CANCEL;
        }
        if (strcmp(password, confirm) != 0) {
            printf("  ! The two passwords do not match. Please try again.\n");
            continue;
        }
        break;
    }

    rc = customer_add(shop, name, phone, password);
    if (rc == ST_ERR_DUP) {
        printf("  ! That phone number is already in use.\n");
        return rc;
    }
    if (rc == ST_ERR_FULL) {
        printf("  ! No new account can be created: the list is full.\n");
        return rc;
    }
    if (rc < 0) {
        printf("  ! The account could not be created.\n");
        return rc;
    }

    printf("\nAccount created.\n");
    printf("  Account number : %d\n", rc);
    printf("  Name           : %s\n",
           shop->customers[customer_find(shop, rc)].name);
    printf("Keep the account number: you need it to log in.\n");

    save_customer_file(shop);
    return rc;
}

/* ------------------------------------------------------------------ */
/* Public user interface                                               */
/* ------------------------------------------------------------------ */

/*
 * Function : customer_register_ui
 * Purpose  : Self-service registration from the main menu. On success the
 *            new account number is shown and the customer may deposit
 *            money at once (design draft 2.2: balance 0.00, then deposit).
 * Returns  : the new account number, ST_CANCEL, ST_EOF, or a negative error
 */
int customer_register_ui(ShopData *shop)
{
    int accountNo;
    int index;

    if (shop == NULL) {
        return ST_ERR_INPUT;
    }

    accountNo = create_account_ui(shop, "Register a New Account");
    if (accountNo < 0) {
        return accountNo;
    }

    index = customer_find(shop, accountNo);
    if (index >= 0) {
        printf("\nWelcome, %s! Your account number is %d.\n",
               shop->customers[index].name, accountNo);
        if (read_yes_no("Deposit money now? (y/n): ")) {
            customer_deposit_ui(shop, index);
        }
    }
    return accountNo;
}

/*
 * Function : customer_deposit_ui
 * Purpose  : Deposit screen: show the balance, read an amount in
 *            MIN_DEPOSIT..MAX_DEPOSIT, refuse an amount that would pass
 *            MAX_BALANCE, confirm, then save the file.
 *            Typing 0 (or 0.00) at the amount cancels.
 */
void customer_deposit_ui(ShopData *shop, int custIndex)
{
    Customer *c;
    Money amount;
    Money room;
    char text[32];
    char after[32];
    int rc;

    if (shop == NULL || custIndex < 0 || custIndex >= shop->customerCount) {
        printf("  ! Invalid customer.\n");
        return;
    }
    c = &shop->customers[custIndex];
    if (!c->active) {
        printf("  ! This account has been removed.\n");
        return;
    }

    printf("\n----- Deposit Money -----\n");
    customer_print(c);
    format_money(MIN_DEPOSIT, text, sizeof text);
    printf("One deposit is %s to ", text);
    format_money(MAX_DEPOSIT, text, sizeof text);
    printf("%s. The balance limit is ", text);
    format_money(MAX_BALANCE, text, sizeof text);
    printf("%s.\n", text);
    printf("Enter 0 to cancel.\n");

    /* An almost full account may have less room than MIN_DEPOSIT left,
     * so refuse straight away instead of asking for an amount that can
     * never be accepted. */
    room = MAX_BALANCE - c->balance;
    if (room < MIN_DEPOSIT) {
        printf("This account is already at the balance limit; "
               "no more money can be added.\n");
        return;
    }

    for (;;) {
        rc = read_money("Amount: ", MIN_DEPOSIT, MAX_DEPOSIT, &amount);
        if (rc == ST_EOF) {
            return;
        }
        if (rc == ST_CANCEL) {
            printf("Deposit cancelled.\n");
            return;
        }
        if (rc != ST_OK) {
            printf("  ! The deposit was not accepted.\n");
            return;
        }

        if (amount > room) {
            format_money(room, text, sizeof text);
            printf("  ! That would pass the balance limit. "
                   "You can deposit at most %s.\n", text);
            continue;
        }
        break;
    }

    format_money(c->balance + amount, after, sizeof after);
    format_money(amount, text, sizeof text);
    printf("\nDeposit %s to account %d (%s).\n", text, c->accountNo, c->name);
    printf("Balance after the deposit: %s\n", after);
    if (!read_yes_no("Confirm? (y/n): ")) {
        printf("Deposit cancelled.\n");
        return;
    }

    if (customer_deposit(c, amount) != ST_OK) {
        printf("  ! The deposit was not accepted.\n");
        return;
    }

    format_money(c->balance, after, sizeof after);
    printf("Deposit successful. New balance: %s\n", after);
    save_customer_file(shop);
}

/*
 * Function : customer_edit_self_ui
 * Purpose  : The logged-in customer changes his own phone number or
 *            password. The name is changed by the manager only (2.2),
 *            and the balance can only change by depositing or ordering.
 */
void customer_edit_self_ui(ShopData *shop, int custIndex)
{
    int choice;
    int rc;

    if (shop == NULL || custIndex < 0 || custIndex >= shop->customerCount) {
        printf("  ! Invalid customer.\n");
        return;
    }
    if (!shop->customers[custIndex].active) {
        printf("  ! This account has been removed.\n");
        return;
    }

    for (;;) {
        printf("\n----- My Details -----\n");
        customer_print(&shop->customers[custIndex]);
        printf("1. Change my phone number\n");
        printf("2. Change my password\n");
        printf("0. Back\n");

        rc = read_int("Choice: ", 0, 2, &choice);
        if (rc != ST_OK || choice == 0) {
            return;
        }

        if (choice == 1) {
            change_own_phone(shop, custIndex);
        } else {
            change_own_password(shop, custIndex);
        }

        /* End of input: leave without drawing the menu again. */
        if (feof(stdin) || ferror(stdin)) {
            return;
        }
    }
}

/*
 * Function : change_own_phone
 * Purpose  : Ask for a new phone number and store it.
 *            The old number may be typed again (nothing changes).
 */
static void change_own_phone(ShopData *shop, int custIndex)
{
    Customer *c;
    char phone[LINE_LEN];
    int rc;

    c = &shop->customers[custIndex];

    printf("\nCurrent phone: %s\n", c->phone);
    print_phone_rule();
    for (;;) {
        rc = read_line("New phone (0 to cancel): ", phone, sizeof phone);
        if (rc == ST_EOF) {
            return;
        }
        if (rc == ST_ERR_INPUT) {
            continue;
        }
        if (strcmp(phone, "0") == 0) {
            printf("Cancelled.\n");
            return;
        }
        if (!is_valid_phone(phone)) {
            printf("  ! Phone must be %d to %d digits, digits only.\n",
                   MIN_PHONE_LEN, PHONE_LEN - 1);
            continue;
        }
        break;
    }

    if (strcmp(phone, c->phone) == 0) {
        printf("The phone number is unchanged.\n");
        return;
    }

    rc = customer_update(shop, c->accountNo, NULL, phone);
    if (rc == ST_ERR_DUP) {
        printf("  ! That phone number belongs to another account.\n");
        return;
    }
    if (rc != ST_OK) {
        printf("  ! The phone number was not changed.\n");
        return;
    }

    printf("Phone number updated. New phone: %s\n", c->phone);
    save_customer_file(shop);
}

/*
 * Function : change_own_password
 * Purpose  : Ask for the current password (at most MAX_LOGIN_ATTEMPTS
 *            tries), then for a new one twice, then store it.
 *            A wrong format or a mismatched confirmation does not use up
 *            one of the attempts, because the current password was right.
 */
static void change_own_password(ShopData *shop, int custIndex)
{
    Customer *c;
    char current[LINE_LEN];
    char password[LINE_LEN];
    char confirm[LINE_LEN];
    int attemptsLeft;
    int rc;

    c = &shop->customers[custIndex];

    printf("\n----- Change My Password -----\n");
    printf("Enter 0 at any prompt to cancel.\n");
    print_password_rule();

    attemptsLeft = MAX_LOGIN_ATTEMPTS;
    for (;;) {
        if (attemptsLeft <= 0) {
            printf("Too many failed attempts. The password was not "
                   "changed.\n");
            return;
        }
        rc = read_line("Current password: ", current, sizeof current);
        if (rc == ST_EOF) {
            return;
        }
        if (rc == ST_ERR_INPUT) {
            continue;
        }
        if (strcmp(current, "0") == 0) {
            printf("Cancelled.\n");
            return;
        }
        if (strcmp(c->password, current) != 0) {
            attemptsLeft--;
            if (attemptsLeft > 0) {
                printf("  ! Current password is incorrect. "
                       "(%d attempt(s) left)\n", attemptsLeft);
            }
            continue;
        }
        break;
    }

    for (;;) {
        rc = read_line("New password: ", password, sizeof password);
        if (rc == ST_EOF) {
            return;
        }
        if (rc == ST_ERR_INPUT) {
            continue;
        }
        if (strcmp(password, "0") == 0) {
            printf("Cancelled.\n");
            return;
        }

        rc = read_line("Confirm new password: ", confirm, sizeof confirm);
        if (rc == ST_EOF) {
            return;
        }
        if (rc == ST_ERR_INPUT) {
            continue;
        }
        if (strcmp(confirm, "0") == 0) {
            printf("Cancelled.\n");
            return;
        }
        if (strcmp(password, confirm) != 0) {
            printf("  ! The two new passwords do not match.\n");
            continue;
        }
        if (!is_valid_password(password)) {
            printf("  ! New password must be %d to %d characters,\n"
                   "    with no spaces and no '%c'.\n",
                   MIN_PASSWORD_LEN, PASSWORD_LEN - 1, FIELD_SEP);
            continue;
        }
        if (strcmp(password, current) == 0) {
            printf("  ! The new password must differ from the current one.\n");
            continue;
        }
        break;
    }

    if (customer_set_password(shop, c->accountNo, password) != ST_OK) {
        printf("  ! The password was not changed.\n");
        return;
    }
    printf("Password changed.\n");
    save_customer_file(shop);
}

/* ------------------------------------------------------------------ */
/* Manager submenu: list / add / edit / reset / remove / search /      */
/* order history (design draft 4.2: customer_manage_ui).               */
/* ------------------------------------------------------------------ */

/*
 * Function : customer_manage_ui
 * Purpose  : The manager's "Manage customers" screen. It loops until the
 *            manager types 0, and never deletes anything by accident: the
 *            remove screen asks for confirmation first.
 *            main.c calls pause_for_enter() after this function returns,
 *            so the submenu itself only pauses after long output.
 */
void customer_manage_ui(ShopData *shop)
{
    int choice;
    int rc;

    if (shop == NULL) {
        return;
    }

    for (;;) {
        printf("\n===== Manage Customers (%d active) =====\n",
               count_active(shop));
        printf("1. List all customers\n");
        printf("2. Add a customer\n");
        printf("3. Edit name / phone\n");
        printf("4. Reset a password\n");
        printf("5. Remove a customer\n");
        printf("6. Search by account / name / phone\n");
        printf("7. View a customer's order history\n");
        printf("0. Back\n");

        rc = read_int("Choice: ", 0, 7, &choice);
        if (rc != ST_OK) {
            return; /* end of input, or a programmer error in the range */
        }

        switch (choice) {
        case 0:
            return;
        case 1:
            customer_list_all(shop);
            pause_for_enter();
            break;
        case 2:
            manage_add(shop);
            break;
        case 3:
            manage_edit(shop);
            break;
        case 4:
            manage_reset_password(shop);
            break;
        case 5:
            manage_remove(shop);
            break;
        case 6:
            manage_search(shop);
            pause_for_enter();
            break;
        case 7:
            manage_history(shop);
            break;
        default:
            break; /* read_int already limits the range */
        }

        if (feof(stdin) || ferror(stdin)) {
            return;
        }
    }
}

/* Add a customer from the manager menu. Uses the same prompts and the
 * same customer_add() as the customer's own registration. */
static void manage_add(ShopData *shop)
{
    int newNo;

    newNo = create_account_ui(shop, "Add a Customer");
    if (newNo >= 0) {
        printf("\nCustomer added. Account number: %d\n", newNo);
    }
}

/* Change the name and/or the phone number of one account. An empty answer
 * keeps the old value, which is why customer_update() accepts NULL. */
static void manage_edit(ShopData *shop)
{
    char name[LINE_LEN];
    char phone[LINE_LEN];
    const char *newName;
    const char *newPhone;
    int index;
    int rc;

    rc = ask_account(shop, 1, &index);
    if (rc != ST_OK) {
        return;
    }

    printf("\nCurrent details:\n");
    customer_print(&shop->customers[index]);

    rc = read_line("New name (press Enter to keep it): ", name, sizeof name);
    if (rc == ST_EOF || rc == ST_ERR_INPUT) {
        return;
    }
    if (strcmp(name, "0") == 0) {
        printf("Cancelled.\n");
        return;
    }
    if (name[0] != '\0' && !is_valid_name(name)) {
        printf("  ! Name must be 1 to %d characters, not only spaces, "
               "and must not contain '%c'.\n", NAME_LEN - 1, FIELD_SEP);
        return;
    }

    rc = read_line("New phone (press Enter to keep it): ", phone,
                   sizeof phone);
    if (rc == ST_EOF || rc == ST_ERR_INPUT) {
        return;
    }
    if (strcmp(phone, "0") == 0) {
        printf("Cancelled.\n");
        return;
    }
    if (phone[0] != '\0' && !is_valid_phone(phone)) {
        printf("  ! Phone must be %d to %d digits, digits only.\n",
               MIN_PHONE_LEN, PHONE_LEN - 1);
        return;
    }

    newName = (name[0] != '\0') ? name : NULL;
    newPhone = (phone[0] != '\0') ? phone : NULL;
    if (newName == NULL && newPhone == NULL) {
        printf("Nothing changed.\n");
        return;
    }

    rc = customer_update(shop, shop->customers[index].accountNo,
                         newName, newPhone);
    if (rc == ST_ERR_DUP) {
        printf("  ! That phone number belongs to another account.\n");
        return;
    }
    if (rc != ST_OK) {
        printf("  ! The customer was not updated.\n");
        return;
    }

    printf("\nCustomer updated.\n");
    customer_print(&shop->customers[index]);
    save_customer_file(shop);
}

/* Reset the password of one account without knowing the old one. */
static void manage_reset_password(ShopData *shop)
{
    char password[LINE_LEN];
    char confirm[LINE_LEN];
    int index;
    int rc;

    rc = ask_account(shop, 1, &index);
    if (rc != ST_OK) {
        return;
    }

    printf("\nResetting the password of account %d (%s).\n",
           shop->customers[index].accountNo, shop->customers[index].name);
    print_password_rule();
    printf("Enter 0 to cancel.\n");

    for (;;) {
        rc = read_line("New password: ", password, sizeof password);
        if (rc == ST_EOF) {
            return;
        }
        if (rc == ST_ERR_INPUT) {
            continue;
        }
        if (strcmp(password, "0") == 0) {
            printf("Cancelled.\n");
            return;
        }

        rc = read_line("Confirm new password: ", confirm, sizeof confirm);
        if (rc == ST_EOF) {
            return;
        }
        if (rc == ST_ERR_INPUT) {
            continue;
        }
        if (strcmp(confirm, "0") == 0) {
            printf("Cancelled.\n");
            return;
        }
        if (strcmp(password, confirm) != 0) {
            printf("  ! The two new passwords do not match.\n");
            continue;
        }
        if (!is_valid_password(password)) {
            printf("  ! New password must be %d to %d characters,\n"
                   "    with no spaces and no '%c'.\n",
                   MIN_PASSWORD_LEN, PASSWORD_LEN - 1, FIELD_SEP);
            continue;
        }
        break;
    }

    if (customer_set_password(shop, shop->customers[index].accountNo,
                              password) != ST_OK) {
        printf("  ! The password was not changed.\n");
        return;
    }
    printf("Password changed for account %d.\n",
           shop->customers[index].accountNo);
    save_customer_file(shop);
}

/* Soft delete one account, after showing it and asking for confirmation.
 * A remaining balance is pointed out, because the money has to be
 * refunded at the counter (design draft 2.2). */
static void manage_remove(ShopData *shop)
{
    Customer *c;
    char money[32];
    int index;
    int rc;

    rc = ask_account(shop, 1, &index);
    if (rc != ST_OK) {
        return;
    }
    c = &shop->customers[index];

    printf("\nAbout to remove this account:\n");
    customer_print(c);
    if (c->balance > 0) {
        format_money(c->balance, money, sizeof money);
        printf("  ! The balance %s stays on the removed record.\n", money);
        printf("    Remember to refund it at the counter.\n");
    }
    printf("The account can no longer log in. Its orders are kept.\n");

    if (!read_yes_no("Remove this account? (y/n): ")) {
        printf("Nothing was removed.\n");
        return;
    }

    if (customer_remove(shop, c->accountNo) != ST_OK) {
        printf("  ! The account was not removed.\n");
        return;
    }
    printf("Account %d removed.\n", c->accountNo);
    save_customer_file(shop);
}

/* Search by account number, or by part of the name / phone number.
 * Only active accounts are found here; the order history screen can still
 * reach a removed one. */
static void manage_search(ShopData *shop)
{
    char text[LINE_LEN];
    int choice;
    int accountNo;
    int index;
    int i;
    int found;
    int rc;

    printf("\n----- Search Customers -----\n");
    printf("1. By account number\n");
    printf("2. By name (part of the name is enough)\n");
    printf("3. By phone (part of the number is enough)\n");
    printf("0. Back\n");

    rc = read_int("Choice: ", 0, 3, &choice);
    if (rc != ST_OK || choice == 0) {
        return;
    }

    if (choice == 1) {
        rc = read_int("Account number (0 to cancel): ", 0, MAX_ACCOUNT_NO,
                      &accountNo);
        if (rc != ST_OK) {
            return;
        }
        if (accountNo == 0) {
            printf("Cancelled.\n");
            return;
        }
        index = customer_find(shop, accountNo);
        if (index < 0) {
            printf("  ! No active customer with account %d.\n", accountNo);
            return;
        }
        printf("\n");
        customer_print(&shop->customers[index]);
        return;
    }

    rc = read_line("Search text (0 to cancel): ", text, sizeof text);
    if (rc == ST_EOF || rc == ST_ERR_INPUT) {
        return;
    }
    if (strcmp(text, "0") == 0) {
        printf("Cancelled.\n");
        return;
    }

    found = 0;
    print_table_header();
    for (i = 0; i < shop->customerCount; i++) {
        if (!shop->customers[i].active) {
            continue;
        }
        if (!contains_ci(shop->customers[i].name, text)
            && !contains_ci(shop->customers[i].phone, text)) {
            continue;
        }
        print_table_row(&shop->customers[i]);
        found++;
    }
    if (found == 0) {
        printf("(no match)\n");
    } else {
        printf("\n%d matching customer(s).\n", found);
    }
}

/* Show the order history of any account. Member D owns the report itself;
 * this only hands the account number over. */
static void manage_history(ShopData *shop)
{
    int index;
    int rc;

    rc = ask_account(shop, 0, &index);
    if (rc != ST_OK) {
        return;
    }
    if (!shop->customers[index].active) {
        printf("Note: this account has been removed.\n");
    }
    order_history_ui(shop, shop->customers[index].accountNo);
}
