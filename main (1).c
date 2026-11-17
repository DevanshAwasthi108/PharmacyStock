/*
 * Project 10: Hospital Pharmacy Stock System
 * Concepts: Arrays, Loops, Structures, Functions, Pointers
 *
 * Features:
 *  - Store drug details (ID, Name, Expiry Date, Price, Stock)
 *  - Alert on expired / soon-to-expire products
 *  - Dynamic restocking and dispensing (real-time updates via pointers)
 *  - Low-stock warning, search, remove expired drugs
 */

#include <stdio.h>
#include <string.h>
#include <time.h>

#define MAX_DRUGS      100
#define NAME_LEN       50
#define LOW_STOCK      10   /* stock at or below this triggers a warning */
#define NEAR_EXPIRY    30   /* days: "expiring soon" window              */

/* ---------- Structures ---------- */
typedef struct {
    int day, month, year;
} Date;

typedef struct {
    int   id;
    char  name[NAME_LEN];
    Date  expiry;
    float price;
    int   stock;
} Drug;

/* ---------- Function prototypes ---------- */
Date  getToday(void);
long  dateToDays(Date d);
int   isExpired(const Drug *d, Date today);
int   isValidDate(Date d);
Drug *findDrug(Drug *inv, int count, int id);

void  addDrug(Drug *inv, int *count);
void  displayAll(const Drug *inv, int count, Date today);
void  expiryAlerts(const Drug *inv, int count, Date today);
void  restockDrug(Drug *inv, int count);
void  dispenseDrug(Drug *inv, int count, Date today);
void  searchDrug(Drug *inv, int count, Date today);
void  removeExpired(Drug *inv, int *count, Date today);
void  lowStockReport(const Drug *inv, int count);
void  printRow(const Drug *d, Date today);
void  loadSampleData(Drug *inv, int *count);
void  clearInput(void);
int   readInt(const char *prompt);
float readFloat(const char *prompt);

/* ---------- Main ---------- */
int main(void)
{
    Drug inventory[MAX_DRUGS];      /* array of structures */
    int  count = 0;
    int  choice;
    Date today = getToday();

    loadSampleData(inventory, &count);

    printf("=====================================\n");
    printf("   HOSPITAL PHARMACY STOCK SYSTEM\n");
    printf("   Today's date: %02d-%02d-%04d\n", today.day, today.month, today.year);
    printf("=====================================\n");

    /* Startup safety alert */
    expiryAlerts(inventory, count, today);

    do {
        printf("\n------------- MENU -------------\n");
        printf("1. Add new drug\n");
        printf("2. Display all drugs\n");
        printf("3. Search drug by ID\n");
        printf("4. Restock drug\n");
        printf("5. Dispense drug (sell)\n");
        printf("6. Expiry alerts\n");
        printf("7. Low stock report\n");
        printf("8. Remove expired drugs\n");
        printf("0. Exit\n");
        choice = readInt("Enter choice: ");

        switch (choice) {
            case 1: addDrug(inventory, &count);                 break;
            case 2: displayAll(inventory, count, today);        break;
            case 3: searchDrug(inventory, count, today);        break;
            case 4: restockDrug(inventory, count);              break;
            case 5: dispenseDrug(inventory, count, today);      break;
            case 6: expiryAlerts(inventory, count, today);      break;
            case 7: lowStockReport(inventory, count);           break;
            case 8: removeExpired(inventory, &count, today);    break;
            case 0: printf("Exiting system. Stay safe!\n");     break;
            default: printf("Invalid choice. Try again.\n");
        }
    } while (choice != 0);

    return 0;
}

/* ---------- Date helpers ---------- */
Date getToday(void)
{
    time_t t = time(NULL);
    struct tm *lt = localtime(&t);
    Date d = { lt->tm_mday, lt->tm_mon + 1, lt->tm_year + 1900 };
    return d;
}

/* Approximate day count (30-day months) - enough for comparisons */
long dateToDays(Date d)
{
    return (long)d.year * 365 + (long)d.month * 30 + d.day;
}

int isValidDate(Date d)
{
    return d.year >= 2000 && d.year <= 2100 &&
           d.month >= 1 && d.month <= 12 &&
           d.day >= 1 && d.day <= 31;
}

int isExpired(const Drug *d, Date today)
{
    return dateToDays(d->expiry) < dateToDays(today);
}

/* ---------- Input helpers ---------- */
void clearInput(void)
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF) { }
}

int readInt(const char *prompt)
{
    int v;
    printf("%s", prompt);
    while (scanf("%d", &v) != 1) {
        clearInput();
        printf("Invalid number. %s", prompt);
    }
    clearInput();
    return v;
}

float readFloat(const char *prompt)
{
    float v;
    printf("%s", prompt);
    while (scanf("%f", &v) != 1 || v < 0) {
        clearInput();
        printf("Invalid value. %s", prompt);
    }
    clearInput();
    return v;
}

/* ---------- Pointer-based lookup ---------- */
Drug *findDrug(Drug *inv, int count, int id)
{
    Drug *p;
    for (p = inv; p < inv + count; p++) {      /* pointer traversal */
        if (p->id == id)
            return p;
    }
    return NULL;
}

/* ---------- Core operations ---------- */
void addDrug(Drug *inv, int *count)
{
    Drug *d;
    int id;

    if (*count >= MAX_DRUGS) {
        printf("Inventory full!\n");
        return;
    }

    id = readInt("Enter Drug ID: ");
    if (findDrug(inv, *count, id) != NULL) {
        printf("A drug with ID %d already exists. Use Restock instead.\n", id);
        return;
    }

    d = &inv[*count];
    d->id = id;

    printf("Enter Drug Name: ");
    fgets(d->name, NAME_LEN, stdin);
    d->name[strcspn(d->name, "\n")] = '\0';

    do {
        d->expiry.day   = readInt("Expiry day   (1-31): ");
        d->expiry.month = readInt("Expiry month (1-12): ");
        d->expiry.year  = readInt("Expiry year  (e.g. 2027): ");
        if (!isValidDate(d->expiry))
            printf("Invalid date. Re-enter.\n");
    } while (!isValidDate(d->expiry));

    d->price = readFloat("Enter Price: ");
    do {
        d->stock = readInt("Enter Stock quantity: ");
    } while (d->stock < 0);

    (*count)++;
    printf("Drug added successfully.\n");
}

void printRow(const Drug *d, Date today)
{
    const char *status = isExpired(d, today) ? "EXPIRED" :
                         (d->stock <= LOW_STOCK ? "LOW STOCK" : "OK");
    printf("%-5d %-20s %02d-%02d-%04d %9.2f %6d  %s\n",
           d->id, d->name,
           d->expiry.day, d->expiry.month, d->expiry.year,
           d->price, d->stock, status);
}

void displayAll(const Drug *inv, int count, Date today)
{
    int i;
    if (count == 0) {
        printf("No drugs in inventory.\n");
        return;
    }
    printf("\n%-5s %-20s %-10s %9s %6s  %s\n",
           "ID", "Name", "Expiry", "Price", "Stock", "Status");
    printf("--------------------------------------------------------------------\n");
    for (i = 0; i < count; i++)
        printRow(&inv[i], today);
}

void searchDrug(Drug *inv, int count, Date today)
{
    int id = readInt("Enter Drug ID to search: ");
    Drug *d = findDrug(inv, count, id);
    if (d == NULL) {
        printf("Drug not found.\n");
        return;
    }
    printf("\n%-5s %-20s %-10s %9s %6s  %s\n",
           "ID", "Name", "Expiry", "Price", "Stock", "Status");
    printRow(d, today);
}

/* Restock: updates the stock directly through a pointer */
void restockDrug(Drug *inv, int count)
{
    int id = readInt("Enter Drug ID to restock: ");
    Drug *d = findDrug(inv, count, id);
    int qty;

    if (d == NULL) {
        printf("Drug not found.\n");
        return;
    }
    qty = readInt("Enter quantity to add: ");
    if (qty <= 0) {
        printf("Quantity must be positive.\n");
        return;
    }
    d->stock += qty;                                  /* real-time update */
    printf("Restocked %s. New stock = %d\n", d->name, d->stock);

    /* Optionally refresh the expiry date for the new batch */
    {
        char ans;
        printf("Does the new batch have a different expiry date? (y/n): ");
        scanf(" %c", &ans);
        clearInput();
        if (ans == 'y' || ans == 'Y') {
            Date nd;
            nd.day   = readInt("New expiry day: ");
            nd.month = readInt("New expiry month: ");
            nd.year  = readInt("New expiry year: ");
            if (isValidDate(nd)) {
                d->expiry = nd;
                printf("Expiry date updated.\n");
            } else {
                printf("Invalid date. Expiry unchanged.\n");
            }
        }
    }
}

/* Dispense: blocks expired drugs, reduces stock via pointer */
void dispenseDrug(Drug *inv, int count, Date today)
{
    int id = readInt("Enter Drug ID to dispense: ");
    Drug *d = findDrug(inv, count, id);
    int qty;

    if (d == NULL) {
        printf("Drug not found.\n");
        return;
    }
    if (isExpired(d, today)) {
        printf("ALERT: %s is EXPIRED. Cannot dispense!\n", d->name);
        return;
    }
    qty = readInt("Enter quantity to dispense: ");
    if (qty <= 0) {
        printf("Quantity must be positive.\n");
        return;
    }
    if (qty > d->stock) {
        printf("Insufficient stock. Available: %d\n", d->stock);
        return;
    }
    d->stock -= qty;                                  /* real-time update */
    printf("Dispensed %d x %s. Bill = %.2f. Remaining stock = %d\n",
           qty, d->name, qty * d->price, d->stock);
    if (d->stock <= LOW_STOCK)
        printf("WARNING: Low stock for %s. Please restock!\n", d->name);
}

void expiryAlerts(const Drug *inv, int count, Date today)
{
    int i, expiredCnt = 0, soonCnt = 0;
    long diff;

    printf("\n===== EXPIRY ALERTS =====\n");
    for (i = 0; i < count; i++) {
        const Drug *d = inv + i;
        diff = dateToDays(d->expiry) - dateToDays(today);
        if (diff < 0) {
            printf("[EXPIRED]  ID %d - %s (expired %02d-%02d-%04d, stock %d)\n",
                   d->id, d->name, d->expiry.day, d->expiry.month,
                   d->expiry.year, d->stock);
            expiredCnt++;
        } else if (diff <= NEAR_EXPIRY) {
            printf("[EXPIRING SOON] ID %d - %s (expires %02d-%02d-%04d, ~%ld days left)\n",
                   d->id, d->name, d->expiry.day, d->expiry.month,
                   d->expiry.year, diff);
            soonCnt++;
        }
    }
    if (expiredCnt == 0 && soonCnt == 0)
        printf("All drugs are within safe expiry limits.\n");
    else
        printf("Summary: %d expired, %d expiring within %d days.\n",
               expiredCnt, soonCnt, NEAR_EXPIRY);
}

void lowStockReport(const Drug *inv, int count)
{
    int i, found = 0;
    printf("\n===== LOW STOCK (<= %d units) =====\n", LOW_STOCK);
    for (i = 0; i < count; i++) {
        if (inv[i].stock <= LOW_STOCK) {
            printf("ID %d - %s : %d units\n", inv[i].id, inv[i].name, inv[i].stock);
            found = 1;
        }
    }
    if (!found)
        printf("No low-stock items.\n");
}

/* Removes expired drugs by shifting the array using pointers */
void removeExpired(Drug *inv, int *count, Date today)
{
    Drug *src, *dst = inv;
    int removed = 0;

    for (src = inv; src < inv + *count; src++) {
        if (isExpired(src, today)) {
            printf("Removed: ID %d - %s\n", src->id, src->name);
            removed++;
        } else {
            if (dst != src)
                *dst = *src;
            dst++;
        }
    }
    *count -= removed;
    if (removed == 0)
        printf("No expired drugs to remove.\n");
    else
        printf("%d expired drug(s) removed from inventory.\n", removed);
}

/* ---------- Sample data so the demo works immediately ---------- */
void loadSampleData(Drug *inv, int *count)
{
    Drug samples[] = {
        {101, "Paracetamol 500mg", {15,  8, 2028}, 2.50f, 200},
        {102, "Amoxicillin 250mg", {10,  1, 2025}, 8.75f,  45},
        {103, "Insulin Vial",      {30, 12, 2026}, 450.0f,  8},
        {104, "Cetirizine 10mg",   { 1,  3, 2024}, 3.20f,  60},
        {105, "Ibuprofen 400mg",   {20,  6, 2028}, 4.00f,  15}
    };
    int i, n = (int)(sizeof(samples) / sizeof(samples[0]));
    for (i = 0; i < n; i++)
        inv[(*count)++] = samples[i];
}