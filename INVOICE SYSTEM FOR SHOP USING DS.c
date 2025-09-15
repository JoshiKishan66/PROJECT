/* wild_dmart_full.c
   Single-file Wild D-Mart POS (Windows console)
   Updated: UI improvements, customer search fix, offer delete, reports UI,
            top-customer fix, sales summary (daily/weekly/monthly/year/grand),
            improved invoice viewing UI, billing UI checks (hide out-of-stock),
            duplicate-check message when adding product to invoice.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <windows.h>

#ifndef _WIN32
/* Provide strcasecmp on non-Windows if missing */
#define _stricmp strcasecmp
#else
/* Windows: alias POSIX name to MSVC name if necessary */
#define strcasecmp _stricmp
#endif

#define PRODUCTS_CSV "data/products.csv"
#define CUSTOMERS_CSV "data/customers.csv"
#define OFFERS_CSV "data/offers.csv"
#define INVOICES_TXT "data/invoices.txt"
#define SALES_CSV "data/sales.csv"
#define USERS_TXT "data/users.txt"
#define FEEDBACK_TXT "data/feedback.txt"
#define REPORT_TXT "data/report.txt"

#define MAX_NAME 128
#define LOW_STOCK_THRESHOLD_DEFAULT 5
#define GST_PERCENT 18.0
#define MENU_X 90

/* Global buffer used by many functions that call read_line(...) or use input */
char input[512];  /* <-- fixes 'input' undeclared errors */

/* Global flag: hide main menu while billing or reprinting */
int SHOW_MENU = 1;

/* Console helpers */
void setColor(int color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}
void gotoxy(int x, int y) {
    COORD coord;
    coord.X = (SHORT)x;
    coord.Y = (SHORT)y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}
void clear_screen(void) { system("cls"); }

/* Safe line read */
void read_line(char *buf, int size) {
    char *p;
    if (!fgets(buf, size, stdin)) { buf[0] = '\0'; return; }
    p = strchr(buf, '\n');
    if (p) *p = '\0';
}
int read_int(const char *prompt, int default_value) {
    char buf[128];
    char *endptr;
    long v;
    if (prompt) printf("%s", prompt);
    read_line(buf, sizeof(buf));
    if (buf[0] == '\0') return default_value;
    v = strtol(buf, &endptr, 10);
    if (endptr == buf) return default_value;
    return (int)v;
}
double read_double(const char *prompt, double default_value) {
    char buf[128];
    char *endptr;
    double v;
    if (prompt) printf("%s", prompt);
    read_line(buf, sizeof(buf));
    if (buf[0] == '\0') return default_value;
    v = strtod(buf, &endptr);
    if (endptr == buf) return default_value;
    return v;
}
char *current_datetime_str(void) {
    static char buf[64];
    time_t t = time(NULL);
    struct tm *tmv = localtime(&t);
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", tmv);
    return buf;
}

/* case-insensitive substring search */
char *strcasestr_custom(const char *haystack, const char *needle) {
    const char *h;
    if (!*needle) return (char*)haystack;
    for (h = haystack; *h; h++) {
        const char *hp = h;
        const char *np = needle;
        while (*hp && *np && tolower((unsigned char)*hp) == tolower((unsigned char)*np)) { hp++; np++; }
        if (!*np) return (char*)h;
    }
    return NULL;
}
void ensure_data_dir(void) {
    system("if not exist data mkdir data >nul 2>nul");
}

/* -------- data structures -------- */
typedef struct Product {
    int id;
    char name[MAX_NAME];
    double price;
    int stock;
    int low_threshold;
    struct Product *next;
} Product;

typedef struct Customer {
    int id;
    char name[MAX_NAME];
    char phone[32];
    char email[80];
    char address[160];
    int loyalty_points;
    struct Customer *next;
} Customer;

typedef enum { OFFER_PERCENT = 1, OFFER_BUYXGETY = 2 } OfferType;

typedef struct Offer {
    int id;
    OfferType type;
    int product_id;
    double percent;
    int buy_x; int get_y;
    char desc[160];
    struct Offer *next;
} Offer;

typedef struct BillItem {
    int pid;
    char name[MAX_NAME];
    int qty;
    double unit_price;
    double discount_amount;
    double line_total;
    struct BillItem *next;
} BillItem;

typedef struct Invoice {
    int id;
    char dt[32];
    BillItem *items;
    double total;
    int customer_id;
    double gst_amount;
    double pre_gst_total;
    struct Invoice *next;
} Invoice;

typedef struct User {
    char username[64];
    char password[64];
    char role[32];
    struct User *next;
} User;

typedef struct Feedback {
    int id;
    int cust_id;
    int rating;
    char comment[256];
    char dt[32];
    struct Feedback *next;
} Feedback;

/* Heads */
Product *productHead = NULL;
Customer *customerHead = NULL;
Offer *offerHead = NULL;
Invoice *invoiceHead = NULL;
User *userHead = NULL;
Feedback *feedbackHead = NULL;
Feedback *feedbackTail = NULL;

/* ID helpers */
int next_product_id(void) {
    int max = 0; Product *p = productHead;
    while (p) { if (p->id > max) max = p->id; p = p->next; }
    return max + 1;
}
int next_customer_id(void) {
    int max = 0; Customer *c = customerHead;
    while (c) { if (c->id > max) max = c->id; c = c->next; }
    return max + 1;
}
int next_offer_id(void) {
    int max = 0; Offer *o = offerHead;
    while (o) { if (o->id > max) max = o->id; o = o->next; }
    return max + 1;
}
int next_invoice_id_from_file(void) {
    FILE *f = fopen(INVOICES_TXT, "r");
    char line[512]; int max = 0;
    if (!f) return 1;
    while (fgets(line, sizeof(line), f)) {
        int id;
        if (sscanf(line, "INVOICE_ID:%d", &id) == 1) if (id > max) max = id;
    }
    fclose(f);
    return max + 1;
}
int next_feedback_id(void) {
    int max = 0; Feedback *f = feedbackHead;
    while (f) { if (f->id > max) max = f->id; f = f->next; }
    return max + 1;
}

/* Prototypes */
Product *create_product_node(int id, const char *name, double price, int stock);
void append_product(Product *p);
Product *find_product_by_id(int id);
void save_products_csv(void);
void load_products_csv(void);
void ui_list_products_xy(int x, int y_start);
void ui_create_invoice(void);
void ui_reprint_invoice(void);
void ui_view_invoices_file(void);
Offer *find_offer_for_product(int pid);
int apply_offer_and_calc_line(Product *p, int qty, Offer *o, double *line_total, double *discount_amount);
void append_invoice_file(int inv_id, const char *dt, BillItem *bill, double total, int cust_id, double pre_gst, double gst_amount);
void append_sales_log(int inv_id, const char *dt, double total, int cust_id);
Invoice *create_invoice_node(int id, const char *dt, BillItem *items, double total, int cust_id, double pre_gst, double gst_amount);
void append_invoice_memory(Invoice *inv);
void free_bill_items(BillItem *h);
void save_customers_csv(void);
Customer *find_customer_by_id(int id);
void save_feedback_file(void);
void save_offers_csv(void);
void save_users_file(void);
void seed_or_load_data(void);

/* --- New prototypes for added UI functions --- */
void ui_search_customers(void);
void ui_list_customers_table(void);
void ui_delete_offer(void);
void ui_list_offers_table(void);
void ui_view_sales_summary(void);

/* ========== Product implementation ========== */
Product *create_product_node(int id, const char *name, double price, int stock) {
    Product *p = (Product*)malloc(sizeof(Product));
    if (!p) return NULL;
    p->id = id; strncpy(p->name, name, MAX_NAME-1); p->name[MAX_NAME-1] = '\0';
    p->price = price; p->stock = stock; p->low_threshold = LOW_STOCK_THRESHOLD_DEFAULT; p->next = NULL;
    return p;
}
void append_product(Product *p) {
    Product *cur;
    if (!productHead) { productHead = p; return; }
    cur = productHead;
    while (cur->next) cur = cur->next;
    cur->next = p;
}
Product *find_product_by_id(int id) {
    Product *cur = productHead;
    while (cur) { if (cur->id == id) return cur; cur = cur->next; }
    return NULL;
}
void save_products_csv(void) {
    FILE *f = fopen(PRODUCTS_CSV, "w");
    Product *p;
    if (!f) return;
    fprintf(f, "id,name,price,stock,low_threshold\n");
    p = productHead;
    while (p) {
        fprintf(f, "%d,%s,%.2f,%d,%d\n", p->id, p->name, p->price, p->stock, p->low_threshold);
        p = p->next;
    }
    fclose(f);
}
void load_products_csv(void) {
    FILE *f = fopen(PRODUCTS_CSV, "r");
    char line[512];
    if (!f) return;
    if (!fgets(line, sizeof(line), f)) { fclose(f); return; } /* skip header */
    while (fgets(line, sizeof(line), f)) {
        int id, stock, lt; double price; char name[MAX_NAME];
        if (sscanf(line, "%d,%127[^,],%lf,%d,%d", &id, name, &price, &stock, &lt) == 5) {
            Product *p = create_product_node(id, name, price, stock);
            p->low_threshold = lt;
            append_product(p);
        }
    }
    fclose(f);
}

/* Products printed at (x, y_start). Left aligned table. */
void ui_list_products_xy(int x, int y_start) {
    Product *p = productHead;
    int y = y_start;
    setColor(11);
    gotoxy(x, y++); printf("+-------+-------------------------------+---------+-------+");
    gotoxy(x, y++); printf("| ID    | Name                          |  Price  | Stock |");
    gotoxy(x, y++); printf("+-------+-------------------------------+---------+-------+");
    setColor(7);

    int shown = 0;
    while (p) {
        if (p->stock > 0) {  /* hide products with stock <= 0 */
            gotoxy(x, y++);
            printf("| %-5d | %-29s | %7.2f | %-5d |", 
                   p->id, p->name, p->price, p->stock);
            shown = 1;
        }
        p = p->next;
    }

    if (!shown) {
        gotoxy(x, y++); printf("| -- no products available --");
    }

    setColor(11);
    gotoxy(x, y++); printf("+-------+-------------------------------+---------+-------+");
    setColor(7);
}

/* ========== Customer implementation (load/save minimal) ========== */
Customer *create_customer_node(int id, const char *name, const char *phone, const char *email, const char *address) {
    Customer *c = (Customer*)malloc(sizeof(Customer));
    if (!c) return NULL;
    c->id = id; strncpy(c->name, name, MAX_NAME-1); c->name[MAX_NAME-1] = '\0';
    strncpy(c->phone, phone, 31); c->phone[31] = '\0';
    strncpy(c->email, email, 79); c->email[79] = '\0';
    strncpy(c->address, address, 159); c->address[159] = '\0';
    c->loyalty_points = 0; c->next = NULL;
    return c;
}
void append_customer(Customer *c) {
    Customer *cur;
    if (!customerHead) { customerHead = c; return; }
    cur = customerHead;
    while (cur->next) cur = cur->next;
    cur->next = c;
}
Customer *find_customer_by_id(int id) {
    Customer *cur = customerHead;
    while (cur) { if (cur->id == id) return cur; cur = cur->next; }
    return NULL;
}
void save_customers_csv(void) {
    FILE *f = fopen(CUSTOMERS_CSV, "w");
    Customer *c;
    if (!f) return;
    fprintf(f, "id,name,phone,email,address,points\n");
    c = customerHead;
    while (c) {
        fprintf(f, "%d,%s,%s,%s,%s,%d\n", c->id, c->name, c->phone, c->email, c->address, c->loyalty_points);
        c = c->next;
    }
    fclose(f);
}
void load_customers_csv(void) {
    FILE *f = fopen(CUSTOMERS_CSV, "r");
    char line[1024];
    if (!f) return;
    if (!fgets(line, sizeof(line), f)) { fclose(f); return; }
    while (fgets(line, sizeof(line), f)) {
        int id, pts; char name[MAX_NAME], phone[32], email[80], address[160];
        if (sscanf(line, "%d,%127[^,],%31[^,],%79[^,],%159[^,],%d", &id, name, phone, email, address, &pts) == 6) {
            Customer *c = create_customer_node(id, name, phone, email, address);
            c->loyalty_points = pts;
            append_customer(c);
        }
    }
    fclose(f);
}

/* ========== Offers implementation ========== */
Offer *create_offer_node(int id, OfferType type, int pid, double percent, int bx, int gy, const char *desc) {
    Offer *o = (Offer*)malloc(sizeof(Offer));
    if (!o) return NULL;
    o->id = id; o->type = type; o->product_id = pid; o->percent = percent; o->buy_x = bx; o->get_y = gy;
    strncpy(o->desc, desc, 159); o->desc[159] = '\0'; o->next = NULL;
    return o;
}
void append_offer(Offer *o) {
    Offer *cur;
    if (!offerHead) { offerHead = o; return; }
    cur = offerHead;
    while (cur->next) cur = cur->next;
    cur->next = o;
}
Offer *find_offer_for_product(int pid) {
    Offer *o = offerHead;
    while (o) { if (o->product_id == pid) return o; o = o->next; }
    return NULL;
}
void save_offers_csv(void) {
    FILE *f = fopen(OFFERS_CSV, "w");
    Offer *o;
    if (!f) return;
    fprintf(f, "id,type,product_id,percent,buy_x,get_y,desc\n");
    o = offerHead;
    while (o) {
        fprintf(f, "%d,%d,%d,%.2f,%d,%d,%s\n", o->id, (int)o->type, o->product_id, o->percent, o->buy_x, o->get_y, o->desc);
        o = o->next;
    }
    fclose(f);
}
void load_offers_csv(void) {
    FILE *f = fopen(OFFERS_CSV, "r");
    char line[512];
    if (!f) return;
    if (!fgets(line, sizeof(line), f)) { fclose(f); return; }
    while (fgets(line, sizeof(line), f)) {
        int id, type, pid, bx, gy; double percent; char desc[160];
        if (sscanf(line, "%d,%d,%d,%lf,%d,%d,%159[^\n]", &id, &type, &pid, &percent, &bx, &gy, desc) >= 6) {
            append_offer(create_offer_node(id, (OfferType)type, pid, percent, bx, gy, desc));
        }
    }
    fclose(f);
}

/* Offer application logic */
int apply_offer_and_calc_line(Product *p, int qty, Offer *o, double *line_total, double *discount_amount) {
    int charged = qty;
    double total = 0.0;
    double discountAmt = 0.0;
    if (!o) { total = qty * p->price; charged = qty; discountAmt = 0.0; }
    else if (o->type == OFFER_PERCENT) {
        double raw = qty * p->price;
        discountAmt = raw * (o->percent / 100.0);
        total = raw - discountAmt;
        charged = qty;
    } else { /* buy x get y */
        if (o->buy_x <= 0) { total = qty * p->price; charged = qty; discountAmt = 0.0; }
        else {
            int groups = qty / (o->buy_x + o->get_y);
            int remainder = qty % (o->buy_x + o->get_y);
            int free = groups * o->get_y;
            if (remainder > o->buy_x) free += remainder - o->buy_x;
            charged = qty - free;
            if (charged < 0) charged = 0;
            total = charged * p->price;
            discountAmt = (qty * p->price) - total;
        }
    }
    *line_total = total; *discount_amount = discountAmt;
    return charged;
}

/* Invoice / files */
void append_invoice_file(int inv_id, const char *dt, BillItem *bill, double total, int cust_id, double pre_gst, double gst_amount) {
    FILE *f = fopen(INVOICES_TXT, "a");
    if (!f) return;
    fprintf(f, "INVOICE_ID:%d|%s|CUST:%d|PRE_GST:%.2f|GST:%.2f|TOTAL:%.2f\n", inv_id, dt, cust_id, pre_gst, gst_amount, total);
    BillItem *b = bill;
    while (b) {
        fprintf(f, "%d,%d,%.2f,%.2f\n", b->pid, b->qty, b->unit_price, b->discount_amount);
        b = b->next;
    }
    fprintf(f, "---\n");
    fclose(f);
}
void append_sales_log(int inv_id, const char *dt, double total, int cust_id) {
    FILE *f = fopen(SALES_CSV, "a");
    if (!f) return;
    fprintf(f, "%d,%s,%d,%.2f\n", inv_id, dt, cust_id, total);
    fclose(f);
}
Invoice *create_invoice_node(int id, const char *dt, BillItem *items, double total, int cust_id, double pre_gst, double gst_amount) {
    Invoice *inv = (Invoice*)malloc(sizeof(Invoice));
    if (!inv) return NULL;
    inv->id = id; strncpy(inv->dt, dt, 31); inv->dt[31] = '\0';
    inv->items = items; inv->total = total; inv->customer_id = cust_id;
    inv->gst_amount = gst_amount; inv->pre_gst_total = pre_gst;
    inv->next = NULL;
    return inv;
}
void append_invoice_memory(Invoice *inv) {
    Invoice *cur;
    if (!invoiceHead) { invoiceHead = inv; return; }
    cur = invoiceHead; while (cur->next) cur = cur->next; cur->next = inv;
}

/* free bill items */
void free_bill_items(BillItem *h) {
    BillItem *t;
    while (h) { t = h->next; free(h); h = t; }
}

/* ========== Print invoice (compact, use coordinates) ========== */
void print_invoice_console(BillItem *bill_head, int inv_id, const char *dt, double total, int cust_id, double pre_gst, double gst_amount, int start_x, int start_y) {
    int x = start_x, y = start_y;
    setColor(10);
    gotoxy(x, y++); printf("+====================================================");
    gotoxy(x, y++); printf("|                  DMART STYLE POS                   ");
    gotoxy(x, y++); printf("|Invoice ID: %-5d Date: %s", inv_id, dt);
    gotoxy(x, y++); printf("|Customer ID: %-5d", cust_id);
    gotoxy(x, y++); printf("|----------------------------------------------------");
    gotoxy(x, y++); printf("|No  Item                 Qty   Price    Disc   Final");
    gotoxy(x, y++); printf("|----------------------------------------------------");
    setColor(7);

    BillItem *cur = bill_head;
    int i = 1;
    while (cur) {
        gotoxy(x, y++);
        printf("|%-3d %-20s %-5d %-8.2f %-7.2f %-8.2f",
               i, cur->name, cur->qty, cur->unit_price,
               cur->discount_amount, cur->line_total);
        cur = cur->next;
        i++;
    }
    setColor(14);
    gotoxy(x, y++); printf("|----------------------------------------------------");
    gotoxy(x, y++); printf("|SubTotal: %.2f", pre_gst);
    gotoxy(x, y++); printf("|GST (%.1f%%): %.2f", GST_PERCENT, gst_amount);
    gotoxy(x, y++); printf("|TOTAL: %.2f", total);
    gotoxy(x, y++); printf("+====================================================");
    gotoxy(x, y++); printf("    Thank you for shopping with us! Visit again :)   \n\n");
    setColor(7);
}

/* ========== Billing: live invoice on right, product list left, immediate stock update ========== */
BillItem *bill_find(BillItem *h, int pid) {
    BillItem *p = h;
    while (p) { if (p->pid == pid) return p; p = p->next; }
    return NULL;
}
BillItem *bill_add_or_update(BillItem **h, Product *p, int qty, Offer *of) {
    double line_total = 0.0, discount_amt = 0.0;
    apply_offer_and_calc_line(p, qty, of, &line_total, &discount_amt);
    BillItem *bi = bill_find(*h, p->id);
    if (bi) {
        /* increase qty and recalc */
        bi->qty += qty;
        bi->discount_amount += discount_amt;
        bi->line_total += line_total;
    } else {
        bi = (BillItem*)malloc(sizeof(BillItem));
        if (!bi) return NULL;
        bi->pid = p->id;
        strncpy(bi->name, p->name, MAX_NAME-1); bi->name[MAX_NAME-1] = '\0';
        bi->qty = qty;
        bi->unit_price = p->price;
        bi->discount_amount = discount_amt;
        bi->line_total = line_total;
        bi->next = *h;
        *h = bi;
    }
    return bi;
}
void bill_remove(BillItem **h, int pid) {
    BillItem *cur = *h, *prev = NULL;
    while (cur) {
        if (cur->pid == pid) {
            if (prev) prev->next = cur->next; else *h = cur->next;
            free(cur); return;
        }
        prev = cur; cur = cur->next;
    }
}
void ui_display_live_invoice(BillItem *bill, int x, int y) {
    int start_y = y;
    setColor(10);
    gotoxy(x, y++); printf("===================== LIVE INVOICE =====================");
    setColor(7);
    gotoxy(x, y++); printf("No  Item                 Qty   Price    Disc   Final");
    gotoxy(x, y++); printf("--------------------------------------------------------");
    int i = 1;
    double subtotal = 0.0;
    BillItem *cur = bill;
    while (cur) {
        gotoxy(x, y++);
        printf("%-3d %-20s %-5d %-8.2f %-7.2f %-8.2f",
               i, cur->name, cur->qty, cur->unit_price, cur->discount_amount, cur->line_total);
        subtotal += cur->line_total;
        cur = cur->next; i++;
    }
    gotoxy(x, y++); printf("--------------------------------------------------------");
    double gst = subtotal * (GST_PERCENT / 100.0);
    gotoxy(x, y++); printf("SubTotal: %.2f", subtotal);
    gotoxy(x, y++); printf("GST (%.1f%%): %.2f", GST_PERCENT, gst);
    gotoxy(x, y++); printf("TOTAL: %.2f", subtotal + gst);
    setColor(7);
}
void ui_refresh_billing_screen(BillItem *bill_head) {
    /* draw product list left and invoice right WITHOUT clearing whole screen */
    ui_list_products_xy(2, 2);
    ui_display_live_invoice(bill_head, 73, 2);
}

/* Billing flow with live updates and edit option */
void ui_create_invoice(void) {
    BillItem *bill_head = NULL;
    double subtotal = 0.0;
    int cust_id = 0;
    int chooseCust = 0;

    SHOW_MENU = 0; /* hide sticky menu while billing */

    /* Ask customer first */
    gotoxy(0,11); setColor(11); printf("Is this sale to a registered customer? 1=Yes 0=No: "); setColor(7);
    chooseCust = read_int(NULL, 0);
    if (chooseCust == 1) {
    	clear_screen();    
        int origX = 2;
        int origY = 16;
        ui_list_products_xy(2,2); 
        gotoxy(origX, origY); printf("Customer list:\n");
        Customer *c = customerHead;
        int cy = origY+1;
        while (c && cy < origY + 10) { gotoxy(origX, cy++); printf("%d: %s %s\n", c->id, c->name, c->phone); c = c->next; }
        cust_id = read_int("\nEnter customer ID: ", 0);
        if (cust_id != 0 && !find_customer_by_id(cust_id)) {
            setColor(12); gotoxy(origX, cy++); printf("Customer not found. Continuing as guest.\n"); setColor(7);
            cust_id = 0;
        }
    }
    if (cust_id == 0) {
        gotoxy(0, 13); printf("Register new customer now? 1=Yes 0=Skip: ");
        chooseCust = read_int(NULL, 0);
        if (chooseCust == 1) {
            char name[MAX_NAME], phone[32], email[80], address[160];
            int newId = next_customer_id();
            printf("Enter name: "); read_line(name, sizeof(name));
            printf("Enter phone: "); read_line(phone, sizeof(phone));
            printf("Enter email: "); read_line(email, sizeof(email));
            printf("Enter address: "); read_line(address, sizeof(address));
            Customer *nc = create_customer_node(newId, name, phone, email, address);
            append_customer(nc); save_customers_csv();
            setColor(10); printf("Registered new customer ID=%d\n", newId); setColor(7);
            cust_id = newId;
        } else { cust_id = 0; }
    }

    /* billing loop */
    while (1) {
        clear_screen();
        ui_refresh_billing_screen(bill_head);
        gotoxy(2, 20); printf("\nActions: [A]dd  [E]dit  [F]inish  [C]ancel : ");
        read_line(input, sizeof(input));
        if (input[0] == '\0') continue;
        char cmd = toupper((unsigned char)input[0]);

        if (cmd == 'F') {
            if (!bill_head) {
                setColor(12); gotoxy(2,22); printf("Invoice empty - cannot finish. Add items or Cancel.\n"); setColor(7);
                continue;
            }
            double gst_amount = 0.0, total = 0.0;
            BillItem *bi = bill_head;
            subtotal = 0.0;
            while (bi) { subtotal += bi->line_total; bi = bi->next; }
            gst_amount = subtotal * (GST_PERCENT / 100.0);
            total = subtotal + gst_amount;

            clear_screen();
            print_invoice_console(bill_head, 0, current_datetime_str(), total, cust_id, subtotal, gst_amount, 2, 2);
            int confirm = read_int("\nConfirm and finalize invoice? 1=Yes 0=No: ", 0);
            if (confirm != 1) {
                setColor(12); printf("\nInvoice cancelled by user. Reverting stock changes and returning to billing.\n"); setColor(7);
                BillItem *b = bill_head;
                while (b) { Product *p = find_product_by_id(b->pid); if (p) p->stock += b->qty; b = b->next; }
                free_bill_items(bill_head); bill_head = NULL;
                SHOW_MENU = 0;
                continue;
            }

            int inv_id = next_invoice_id_from_file();
            char *dt = current_datetime_str();
            append_invoice_file(inv_id, dt, bill_head, total, cust_id, subtotal, gst_amount);
            append_sales_log(inv_id, dt, total, cust_id);
            append_invoice_memory(create_invoice_node(inv_id, dt, bill_head, total, cust_id, subtotal, gst_amount));
            save_products_csv();
            if (cust_id != 0) {
                Customer *c = find_customer_by_id(cust_id);
                if (c) { int pts = (int)(subtotal / 100.0); c->loyalty_points += pts; save_customers_csv(); setColor(10); printf("Added %d loyalty points to customer %d\n", pts, c->id); setColor(7); }
            }

            clear_screen();
            print_invoice_console(bill_head, inv_id, dt, total, cust_id, subtotal, gst_amount, 2, 2);
            setColor(10); printf("\nInvoice saved ID=%d\n", inv_id); setColor(7);
            SHOW_MENU = 1;
            read_line(input, sizeof(input));
            return;
        } 
        else if (cmd == 'C') {
            BillItem *b = bill_head;
            while (b) { Product *p = find_product_by_id(b->pid); if (p) p->stock += b->qty; b = b->next; }
            free_bill_items(bill_head); bill_head = NULL;
            SHOW_MENU = 1;
            setColor(12); gotoxy(2,22); printf("Invoice cancelled and stock reverted.\n"); setColor(7);
            read_line(input, sizeof(input));
            return;
        } 
        else if (cmd == 'A') {
            int pid = read_int("\nEnter product ID: ", 0);
            Product *p = find_product_by_id(pid);

            if (!p) { setColor(12); printf("Product not found\n"); setColor(7); read_line(input, sizeof(input)); continue; }
            if (p->stock <= 0) { setColor(12); printf("Product '%s' is out of stock!\n", p->name); setColor(7); read_line(input, sizeof(input)); continue; }

            int qty = read_int("Enter qty: ", 0);
            if (qty <= 0) { setColor(12); printf("Invalid qty\n"); setColor(7); read_line(input, sizeof(input)); continue; }
            if (qty > p->stock) { setColor(12); printf("Not enough stock! Available %d\n", p->stock); setColor(7); read_line(input, sizeof(input)); continue; }

            /* duplicate check */
            if (bill_find(bill_head, pid)) {
                setColor(12);
                printf("Product '%s' already in invoice! Use [E]dit to update qty.\n", p->name);
                setColor(7);
                read_line(input, sizeof(input));
                continue;
            }

            Offer *of = find_offer_for_product(p->id);
            BillItem *added = bill_add_or_update(&bill_head, p, qty, of);
            p->stock -= qty; 
            if (p->stock <= p->low_threshold) { setColor(14); printf("ALERT: %s low (now %d)\n", p->name, p->stock); setColor(7); }
            read_line(input, sizeof(input));
        } 
        else if (cmd == 'E') {
            /* Edit logic (unchanged) */
            if (!bill_head) { setColor(12); gotoxy(2,22); printf("Invoice empty.\n"); setColor(7); read_line(input, sizeof(input)); continue; }
            int row = 23; BillItem *cur = bill_head; int idx = 1;
            gotoxy(2, row++); printf("Invoice Items:");
            while (cur) { gotoxy(2, row++); printf("%d) %s  qty=%d  line=%.2f", idx, cur->name, cur->qty, cur->line_total); cur = cur->next; idx++; }
            int target_pid = read_int("\n\n\n\n\nEnter Product ID to edit/remove: ", 0);
            BillItem *item = bill_find(bill_head, target_pid);
            if (!item) { setColor(12); printf("Item not in invoice\n"); setColor(7); read_line(input, sizeof(input)); continue; }
            int newqty = read_int("Enter new qty (0 to remove): ", -1);
            if (newqty < 0) { setColor(12); printf("Cancelled edit\n"); setColor(7); read_line(input, sizeof(input)); continue; }
            Product *prod = find_product_by_id(target_pid);
            if (!prod) { setColor(12); printf("Product record missing (unexpected)\n"); setColor(7); read_line(input, sizeof(input)); continue; }
            int oldqty = item->qty;
            if (newqty == 0) {
                prod->stock += oldqty;
                bill_remove(&bill_head, target_pid);
                setColor(10); printf("Removed from invoice, restored stock by %d\n", oldqty); setColor(7);
                read_line(input, sizeof(input)); continue;
            } else {
                int delta = newqty - oldqty;
                if (delta > 0) {
                    if (delta > prod->stock) { setColor(12); printf("Not enough additional stock available. Has %d\n", prod->stock); setColor(7); read_line(input, sizeof(input)); continue; }
                    prod->stock -= delta;
                } else if (delta < 0) {
                    prod->stock += (-delta);
                }
                Offer *of2 = find_offer_for_product(prod->id);
                double new_line = 0.0, new_discount = 0.0;
                apply_offer_and_calc_line(prod, newqty, of2, &new_line, &new_discount);
                item->qty = newqty;
                item->line_total = new_line;
                item->discount_amount = new_discount;
                setColor(10); printf("Updated item qty to %d\n", newqty); setColor(7);
                read_line(input, sizeof(input)); continue;
            }
        } 
        else {
            setColor(12); gotoxy(2,22); printf("Unknown action\n"); setColor(7);
            read_line(input, sizeof(input));
        }
    }
}

/* ========== Reprint and view invoices ========== */
void ui_reprint_invoice(void) {
    SHOW_MENU = 0; /* hide menu while reprinting */
    int id = read_int("Enter Invoice ID to reprint: ", 0);
    if (id <= 0) { setColor(12); printf("Invalid\n"); setColor(7); SHOW_MENU = 1; return; }
    Invoice *iv = invoiceHead;
    while (iv) { if (iv->id == id) { clear_screen(); print_invoice_console(iv->items, iv->id, iv->dt, iv->total, iv->customer_id, iv->pre_gst_total, iv->gst_amount, 2, 2); SHOW_MENU = 1; return; } iv = iv->next; }
    FILE *f = fopen(INVOICES_TXT, "r");
    if (!f) { setColor(12); printf("No invoices file\n"); setColor(7); SHOW_MENU = 1; return; }
    char line[512];
    int found = 0;
    while (fgets(line, sizeof(line), f)) {
        int inv;
        if (sscanf(line, "INVOICE_ID:%d", &inv) == 1) {
            if (inv == id) {
                double pre_gst=0, gst=0, tot=0;
                int cust = 0;
                char dt[64];
                if (sscanf(line, "INVOICE_ID:%d|%63[^|]|CUST:%d|PRE_GST:%lf|GST:%lf|TOTAL:%lf", &inv, dt, &cust, &pre_gst, &gst, &tot) >= 3) {
                    BillItem *bh = NULL, *bt = NULL;
                    while (fgets(line, sizeof(line), f)) {
                        if (strncmp(line, "---", 3) == 0) break;
                        int pid, q; double up, da;
                        if (sscanf(line, "%d,%d,%lf,%lf", &pid, &q, &up, &da) == 4) {
                            Product *pr = find_product_by_id(pid);
                            BillItem *bi = (BillItem*)malloc(sizeof(BillItem));
                            bi->pid = pid; if (pr) strncpy(bi->name, pr->name, MAX_NAME-1); else strncpy(bi->name, "Unknown", MAX_NAME-1);
                            bi->qty = q; bi->unit_price = up;
                            bi->discount_amount = da;
                            bi->line_total = (q * up) - da;
                            bi->next = NULL;
                            if (!bh) bh = bt = bi; else { bt->next = bi; bt = bi; }
                        }
                    }
                    clear_screen();
                    print_invoice_console(bh, inv, dt, tot, cust, pre_gst, gst, 2, 2);
                    free_bill_items(bh);
                    found = 1;
                }
                break;
            }
        }
    }
    fclose(f);
    if (!found) { setColor(12); printf("Invoice not found in file\n"); setColor(7); }
    SHOW_MENU = 1;
}

/* View raw invoices file in a nicer UI and allow selecting invoice to print */
void ui_view_invoices_file(void) {
    SHOW_MENU = 0;
    clear_screen();
    FILE *f = fopen(INVOICES_TXT, "r");
    char ln[512];
    if (!f) { setColor(12); printf("No invoices yet\n"); setColor(7); SHOW_MENU = 1; return; }

    /* show summary list of invoices (ID, date, customer, total) */
    printf("+-------+---------------------+--------+---------+\n");
    printf("|Inv ID | Date                | CustID | Total   |\n");
    printf("+-------+---------------------+--------+---------+\n");
    rewind(f);
    while (fgets(ln, sizeof(ln), f)) {
        if (strncmp(ln, "INVOICE_ID:", 11) == 0) {
            int inv; char dt[64]; int cust; double pre, gst, tot;
            if (sscanf(ln, "INVOICE_ID:%d|%63[^|]|CUST:%d|PRE_GST:%lf|GST:%lf|TOTAL:%lf", &inv, dt, &cust, &pre, &gst, &tot) == 6) {
                printf("| %-5d | %-19s | %-6d | %7.2f |\n", inv, dt, cust, tot);
            }
        }
    }
    printf("+-------+---------------------+--------+---------+\n");
    fclose(f);

    int showId = read_int("Enter Invoice ID to view (0 to return): ", 0);
    if (showId > 0) {
        /* reuse reprint functionality to show nicely */
        ui_reprint_invoice();
    }
    SHOW_MENU = 1;
}

/* ========== Reports (improved) ========== */

/* helper to parse "YYYY-MM-DD HH:MM:SS" into time_t (returns -1 on fail) */
time_t parse_datetime_to_time(const char *dt) {
    struct tm tmv;
    int y, mo, d, hh, mm, ss;
    if (sscanf(dt, "%d-%d-%d %d:%d:%d", &y, &mo, &d, &hh, &mm, &ss) != 6) return (time_t)-1;
    memset(&tmv, 0, sizeof(tmv));
    tmv.tm_year = y - 1900;
    tmv.tm_mon = mo - 1;
    tmv.tm_mday = d;
    tmv.tm_hour = hh;
    tmv.tm_min = mm;
    tmv.tm_sec = ss;
    tmv.tm_isdst = -1;
    return mktime(&tmv);
}

/* sales summary: day, week (7 days), month, year, grand */
void ui_view_sales_summary(void) {
    clear_screen();
    FILE *f = fopen(SALES_CSV, "r");
    if (!f) {
        setColor(12); printf("No sales recorded yet\n"); setColor(7); return;
    }
    char line[512];
    double grand = 0.0;
    double daily = 0.0, weekly = 0.0, monthly = 0.0, yearly = 0.0;
    time_t now = time(NULL);
    struct tm *tn = localtime(&now);
    int cur_y = tn->tm_year + 1900;
    int cur_m = tn->tm_mon + 1;
    int cur_d = tn->tm_mday;
    while (fgets(line, sizeof(line), f)) {
        int id, cid; char dt[64]; double t;
        if (sscanf(line, "%d,%63[^,],%d,%lf", &id, dt, &cid, &t) == 4) {
            grand += t;
            time_t ts = parse_datetime_to_time(dt);
            if (ts == (time_t)-1) continue;
            double diff_sec = difftime(now, ts);
            double days_diff = diff_sec / (60*60*24);
            if (days_diff < 1.0) daily += t;
            if (days_diff < 7.0) weekly += t;
            struct tm *tmv = localtime(&ts);
            if ((tmv->tm_year + 1900) == cur_y && (tmv->tm_mon +1) == cur_m) monthly += t;
            if ((tmv->tm_year + 1900) == cur_y) yearly += t;
        }
    }
    fclose(f);

    setColor(11); printf("\nSales Summary (calculated):\n"); setColor(7);
    printf("+----------------+----------------+\n");
    printf("| Period         | Total (INR)    |\n");
    printf("+----------------+----------------+\n");
    printf("| Today          | %12.2f   |\n", daily);
    printf("| Last 7 days    | %12.2f   |\n", weekly);
    printf("| This month     | %12.2f   |\n", monthly);
    printf("| This year      | %12.2f   |\n", yearly);
    printf("| Grand total    | %12.2f   |\n", grand);
    printf("+----------------+----------------+\n");
}

/* Top customers: fixed logic (skip guest id 0 and index 0) */
void ui_top_customers(void) {
    clear_screen();
    FILE *f = fopen(SALES_CSV, "r");
    if (!f) { setColor(12); printf("No sales recorded yet\n"); setColor(7); return; }

    /* find maximum customer id to size arrays (sparse but ok) */
    int maxcid = 0;
    Customer *c = customerHead;
    while (c) { if (c->id > maxcid) maxcid = c->id; c = c->next; }
    if (maxcid < 1) { setColor(12); printf("No customers found\n"); setColor(7); fclose(f); return; }

    int *invcount = (int*)calloc(maxcid+1, sizeof(int));
    double *revenue = (double*)calloc(maxcid+1, sizeof(double));
    char ln[256];
    while (fgets(ln, sizeof(ln), f)) {
        int idd; char dt[64]; int cid; double t;
        if (sscanf(ln, "%d,%63[^,],%d,%lf", &idd, dt, &cid, &t) == 4) {
            if (cid > 0 && cid <= maxcid) { invcount[cid]++; revenue[cid] += t; }
        }
    }
    fclose(f);

    printf("\nTop customers by invoices (up to top 5):\n");
    printf("+------+-------------------------------+-----------+-----------+\n");
    printf("| Rank | Name                          | Invoices  | Revenue   |\n");
    printf("+------+-------------------------------+-----------+-----------+\n");
    int rank = 0;
    while (rank < 5) {
        int best = -1;
        int i;
        for (i = 1; i <= maxcid; i++) {
            if (best == -1 || invcount[i] > invcount[best]) best = i;
        }
        if (best == -1 || invcount[best] == 0) break;
        Customer *cc = find_customer_by_id(best);
        printf("| %-4d | %-29s | %-9d | %9.2f |\n", rank+1, cc ? cc->name : "Unknown", invcount[best], revenue[best]);
        invcount[best] = 0; revenue[best] = 0.0;
        rank++;
    }
    if (rank == 0) printf("| No customers with invoices yet                             |\n");
    printf("+------+-------------------------------+-----------+-----------+\n");

    free(invcount); free(revenue);
}

/* low stock & product-wise reports kept as is (UI polished) */
void ui_low_stock_report(void) {
    clear_screen();
    Product *cur = productHead;
    int any = 0;
    setColor(14); printf("\nLow stock items (<= threshold):\n"); setColor(7);
    printf("+------+-------------------------------+-------+\n");
    printf("| ID   | Name                          | Stock |\n");
    printf("+------+-------------------------------+-------+\n");
    while (cur) {
        if (cur->stock <= cur->low_threshold) { printf("| %-4d | %-29s | %-5d |\n", cur->id, cur->name, cur->stock); any = 1; }
        cur = cur->next;
    }
    if (!any) printf("| -- none --                                          |\n");
    printf("+------+-------------------------------+-------+\n");
}
void ui_product_wise_report_hash(void) {
    clear_screen();
    int maxid = 0;
    Product *p = productHead;
    while (p) { if (p->id > maxid) maxid = p->id; p = p->next; }
    if (maxid == 0) { setColor(12); printf("No products\n"); setColor(7); return; }
    {
        int i;
        int *qtys = (int*)calloc(maxid+1, sizeof(int));
        double *revs = (double*)calloc(maxid+1, sizeof(double));
        FILE *f = fopen(INVOICES_TXT, "r");
        char line[512];
        if (f) {
            while (fgets(line, sizeof(line), f)) {
                if (strncmp(line, "INVOICE_ID:", 11) == 0 || strncmp(line,"---",3)==0) continue;
                int pid, q; double up, da;
                if (sscanf(line, "%d,%d,%lf,%lf", &pid, &q, &up, &da) == 4) {
                    if (pid >= 0 && pid <= maxid) { qtys[pid] += q; revs[pid] += (q * up) - da; }
                }
            }
            fclose(f);
        }
        printf("\nProduct-wise sales (aggregated):\n");
        printf("+------+-------------------------------+---------+-----------+\n");
        printf("| ID   | Name                          | Sold    | Revenue   |\n");
        printf("+------+-------------------------------+---------+-----------+\n");
        for (i = 1; i <= maxid; i++) {
            if (qtys[i] > 0) {
                Product *pr = find_product_by_id(i);
                printf("| %-4d | %-29s | %-7d | %9.2f |\n", i, pr?pr->name:"Unknown", qtys[i], revs[i]);
            }
        }
        printf("+------+-------------------------------+---------+-----------+\n");
        free(qtys); free(revs);
    }
}
void generate_reports_to_file(void) {
    FILE *f = fopen(REPORT_TXT, "w");
    if (!f) { setColor(12); printf("Failed to open report file\n"); setColor(7); return; }
    fprintf(f, "WILD DMART REPORT\nGenerated: %s\n\n", current_datetime_str());
    FILE *s = fopen(SALES_CSV, "r");
    double grand = 0.0; int invcount = 0;
    if (s) {
        char line[512];
        while (fgets(line, sizeof(line), s)) {
            double t;
            if (sscanf(line, "%*d,%*[^,],%*d,%lf", &t) == 1) { grand += t; invcount++; }
        }
        fclose(s);
    }
    fprintf(f, "Total invoices: %d\n", invcount);
    fprintf(f, "Grand total: %.2f\n\n", grand);
    /* product-wise */
    {
        int maxid = 0; Product *pp = productHead;
        while (pp) { if (pp->id > maxid) maxid = pp->id; pp = pp->next; }
        if (maxid > 0) {
            int *qtys = (int*)calloc(maxid+1, sizeof(int));
            double *revs = (double*)calloc(maxid+1, sizeof(double));
            FILE *fi = fopen(INVOICES_TXT, "r");
            if (fi) {
                char ln[512];
                while (fgets(ln, sizeof(ln), fi)) {
                    if (strncmp(ln, "INVOICE_ID:", 11) == 0 || strncmp(ln,"---",3)==0) continue;
                    int pid, q; double up, da;
                    if (sscanf(ln, "%d,%d,%lf,%lf", &pid, &q, &up, &da) == 4) {
                        if (pid >= 0 && pid <= maxid) { qtys[pid] += q; revs[pid] += (q * up) - da; }
                    }
                }
                fclose(fi);
            }
            fprintf(f, "Product-wise sales:\n");
            {
                int i;
                for (i = 1; i <= maxid; i++) {
                    if (qtys[i] > 0) {
                        Product *pr = find_product_by_id(i);
                        fprintf(f, "Product %d (%s): Sold %d, Revenue %.2f\n", i, pr?pr->name:"Unknown", qtys[i], revs[i]);
                    }
                }
            }
            free(qtys); free(revs);
        }
    }
    fclose(f);
    setColor(10); printf("Report written to %s\n", REPORT_TXT); setColor(7);
}

/* ========== Users & Feedback minimal ========== */
User *create_user_node(const char *username, const char *password, const char *role) {
    User *u = (User*)malloc(sizeof(User));
    if (!u) return NULL;
    strncpy(u->username, username, 63); u->username[63] = '\0';
    strncpy(u->password, password, 63); u->password[63] = '\0';
    strncpy(u->role, role, 31); u->role[31] = '\0';
    u->next = NULL; return u;
}
void append_user(User *u) {
    User *cur;
    if (!userHead) { userHead = u; return; }
    cur = userHead;
    while (cur->next) cur = cur->next;
    cur->next = u;
}
void load_users_file(void) {
    FILE *f = fopen(USERS_TXT, "r");
    char line[256];
    if (!f) return;
    while (fgets(line, sizeof(line), f)) {
        char username[64], password[64], role[32];
        if (sscanf(line, "%63[^,],%63[^,],%31[^\n]", username, password, role) == 3) {
            append_user(create_user_node(username, password, role));
        }
    }
    fclose(f);
}
void save_users_file(void) {
    FILE *f = fopen(USERS_TXT, "w");
    User *u = userHead;
    if (!f) return;
    while (u) {
        fprintf(f, "%s,%s,%s\n", u->username, u->password, u->role);
        u = u->next;
    }
    fclose(f);
}
int authenticate_user(char *username, char *role_out) {
    char input_local[128], pass[128];
    printf("Username: "); read_line(input_local, sizeof(input_local));
    printf("Password: "); read_line(pass, sizeof(pass));
    {
        User *u = userHead;
        while (u) {
            if (strcmp(u->username, input_local) == 0 && strcmp(u->password, pass) == 0) {
                strncpy(role_out, u->role, 31); role_out[31] = '\0'; return 1;
            }
            u = u->next;
        }
    }
    return 0;
}

void append_feedback(int cust_id, int rating, const char *comment) {
    Feedback *fb = (Feedback*)malloc(sizeof(Feedback));
    if (!fb) return;
    fb->id = next_feedback_id(); fb->cust_id = cust_id; fb->rating = rating;
    strncpy(fb->comment, comment, 255); fb->comment[255] = '\0';
    strncpy(fb->dt, current_datetime_str(), 31); fb->dt[31] = '\0';
    fb->next = NULL;
    if (!feedbackHead) { feedbackHead = feedbackTail = fb; }
    else { feedbackTail->next = fb; feedbackTail = fb; }
}
void load_feedback_file(void) {
    FILE *f = fopen(FEEDBACK_TXT, "r");
    char line[512];
    if (!f) return;
    while (fgets(line, sizeof(line), f)) {
        int id, cust, rating; char comment[256], dt[64];
        if (sscanf(line, "%d|%d|%d|%63[^|]|%31[^\n]", &id, &cust, &rating, comment, dt) == 5) {
            Feedback *fb = (Feedback*)malloc(sizeof(Feedback));
            fb->id = id; fb->cust_id = cust; fb->rating = rating;
            strncpy(fb->comment, comment, 255); fb->comment[255] = '\0';
            strncpy(fb->dt, dt, 31); fb->dt[31] = '\0';
            fb->next = NULL;
            if (!feedbackHead) feedbackHead = feedbackTail = fb; else { feedbackTail->next = fb; feedbackTail = fb; }
        }
    }
    fclose(f);
}
void save_feedback_file(void) {
    FILE *f = fopen(FEEDBACK_TXT, "w");
    Feedback *fb = feedbackHead;
    if (!f) return;
    while (fb) {
        fprintf(f, "%d|%d|%d|%s|%s\n", fb->id, fb->cust_id, fb->rating, fb->comment, fb->dt);
        fb = fb->next;
    }
    fclose(f);
}

/* ========== Menus ========== */
void draw_main_menu(void) {
    if (!SHOW_MENU) return;
    int x = 80; int y = 2;
    setColor(11);
    gotoxy(x, y++); printf("+==============================+");
    gotoxy(x, y++); printf("|        WILD DMART MENU       |");
    gotoxy(x, y++); printf("+==============================+");
    setColor(7);
    gotoxy(x, y++); printf("|1. Product Management         |");
    gotoxy(x, y++); printf("|2. Customer Management        |");
    gotoxy(x, y++); printf("|3. Offers Management          |");
    gotoxy(x, y++); printf("|4. Billing                    |");
    gotoxy(x, y++); printf("|5. Reports                    |");
    gotoxy(x, y++); printf("|6. Admin Panel                |");
    gotoxy(x, y++); printf("|7. Feedback                   |");
    gotoxy(x, y++); printf("|8. Exit                       |");
    setColor(11);
    gotoxy(x, y++); printf("+==============================+");
    setColor(7);
}

/* Product menu implementations (simple) */
void ui_list_products(void) { ui_list_products_xy(2,2); }
void ui_add_product(void) {
    int id = next_product_id();
    char name[MAX_NAME]; 
    double price; 
    int stock;

    printf("Enter product name: "); 
    read_line(name, sizeof(name));

    if (name[0] == '\0') { 
        setColor(12); 
        printf("Name required\n"); 
        setColor(7); 
        return; 
    }

    /* Duplicate product check */
    Product *cur = productHead;
    while (cur) {
        if (strcasecmp(cur->name, name) == 0) {   // case-insensitive check
            setColor(12);
            printf("? Product '%s' already exists with ID=%d\n", cur->name, cur->id);
            setColor(7);
            return;
        }
        cur = cur->next;
    }

    price = read_double("Enter price: ", -1.0);
    if (price <= 0) { 
        setColor(12); 
        printf("Invalid price\n"); 
        setColor(7); 
        return; 
    }

    stock = read_int("Enter stock: ", -1);
    if (stock < 0) { 
        setColor(12); 
        printf("Invalid stock\n"); 
        setColor(7); 
        return; 
    }

    append_product(create_product_node(id, name, price, stock));
    save_products_csv();

    setColor(10); 
    printf("Product Added ID=%d , Name='%s'\n", id, name); 
    setColor(7);
}

void ui_update_product(void) {
    int id = read_int("Enter product ID: ", 0);
    Product *p = find_product_by_id(id);
    if (!p) { setColor(12); printf("Not found\n"); setColor(7); return; }
    char tmp[MAX_NAME]; double price; int stock, lt;
    printf("New name (- skip): "); read_line(tmp, sizeof(tmp));
    price = read_double("New price (0 skip): ", 0.0);
    stock = read_int("New stock (-1 skip): ", -1);
    lt = read_int("New low threshold (-1 skip): ", -1);
    if (tmp[0] != '\0' && strcmp(tmp, "-") != 0) strncpy(p->name, tmp, MAX_NAME-1);
    if (price > 0.0) p->price = price;
    if (stock >= 0) p->stock = stock;
    if (lt >= 0) p->low_threshold = lt;
    save_products_csv();
    setColor(10); printf("Product updated\n"); setColor(7);
}
void ui_delete_product(void) {
    int id = read_int("Enter product ID to delete: ", 0);
    if (id <= 0) { setColor(12); printf("Invalid\n"); setColor(7); return; }
    Product *prev = NULL, *cur = productHead;
    while (cur) {
        if (cur->id == id) {
            if (prev) prev->next = cur->next; else productHead = cur->next;
            free(cur); save_products_csv(); setColor(10); printf("Deleted %d\n", id); setColor(7); return;
        }
        prev = cur; cur = cur->next;
    }
    setColor(12); printf("Not found\n"); setColor(7);
}
void ui_search_products(void) {
    char q[128]; printf("Enter name or ID to search: "); read_line(q, sizeof(q));
    if (q[0] == '\0') { setColor(12); printf("Empty\n"); setColor(7); return; }
    int id = atoi(q); if (id > 0) { Product *p = find_product_by_id(id); if (p) { setColor(10); printf("Found: %d %s %.2f stock=%d\n", p->id, p->name, p->price, p->stock); setColor(7); return; } }
    Product *cur = productHead; int found = 0;
    while (cur) {
        if (strcasestr_custom(cur->name, q)) { setColor(10); printf("Found: %d %s %.2f stock=%d\n", cur->id, cur->name, cur->price, cur->stock); setColor(7); found = 1; }
        cur = cur->next;
    }
    if (!found) { setColor(12); printf("No match\n"); setColor(7); }
}
void ui_inventory_alerts(void) {
    Product *cur = productHead; int any = 0;
    setColor(14); printf("\nInventory Alerts (Low Stock):\n"); setColor(7);
    while (cur) { if (cur->stock <= cur->low_threshold) { printf("ID %d: %s stock=%d\n", cur->id, cur->name, cur->stock); any = 1; } cur = cur->next; }
    if (!any) printf("None\n");
}

/* Customer menus */
void ui_list_customers(void) {
    Customer *c = customerHead;
    setColor(11); printf("\nCustomers:\n"); setColor(7);
    while (c) { printf("%d: %s %s\n", c->id, c->name, c->phone); c = c->next; }
}

/* nicer table list */
void ui_list_customers_table(void) {
    Customer *c = customerHead;
    printf("+------+-------------------------------+--------------+-------------------------+\n");
    printf("| ID   | Name                          | Phone        | Email                   |\n");
    printf("+------+-------------------------------+--------------+-------------------------+\n");
    while (c) {
        printf("| %-4d | %-29s | %-12s | %-23s |\n", c->id, c->name, c->phone, c->email);
        c = c->next;
    }
    printf("+------+-------------------------------+--------------+-------------------------+\n");
}

/* search UI for customers (fixed) */
void ui_search_customers(void) {
    char q[128];
    printf("Enter ID or name/phone/email to search: ");
    read_line(q, sizeof(q));
    if (q[0] == '\0') { setColor(12); printf("Empty\n"); setColor(7); return; }
    int id = atoi(q);
    int found = 0;
    Customer *c = customerHead;
    while (c) {
        if ((id > 0 && c->id == id) ||
            strcasestr_custom(c->name, q) ||
            strcasestr_custom(c->phone, q) ||
            strcasestr_custom(c->email, q)) {
            setColor(10);
            printf("\nFound Customer: ID=%d\nName: %s\nPhone: %s\nEmail: %s\nAddress: %s\nPoints: %d\n",
                   c->id, c->name, c->phone, c->email, c->address, c->loyalty_points);
            setColor(7);
            found = 1;
        }
        c = c->next;
    }
    if (!found) { setColor(12); printf("No matching customer\n"); setColor(7); }
}

void ui_add_customer(void) {
    int id = next_customer_id(); char name[MAX_NAME], phone[32], email[80], address[160];
    printf("Enter name: "); read_line(name, sizeof(name));
    if (name[0] == '\0') { setColor(12); printf("Name required\n"); setColor(7); return; }
    printf("Enter phone: "); read_line(phone, sizeof(phone));
    printf("Enter email: "); read_line(email, sizeof(email));
    printf("Enter address: "); read_line(address, sizeof(address));
    append_customer(create_customer_node(id, name, phone, email, address));
    save_customers_csv();
    setColor(10); printf("Added customer ID=%d\n", id); setColor(7);
}
void ui_update_customer(void) {
    int id = read_int("Customer ID: ", 0);
    Customer *c = find_customer_by_id(id);
    if (!c) { setColor(12); printf("Not found\n"); setColor(7); return; }
    char tmp[MAX_NAME], phone[32], email[80], address[160];
    printf("New name (- skip): "); read_line(tmp, sizeof(tmp));
    printf("New phone (- skip): "); read_line(phone, sizeof(phone));
    printf("New email (- skip): "); read_line(email, sizeof(email));
    printf("New address (- skip): "); read_line(address, sizeof(address));
    if (tmp[0] != '\0' && strcmp(tmp, "-") != 0) strncpy(c->name, tmp, MAX_NAME-1);
    if (phone[0] != '\0' && strcmp(phone, "-") != 0) strncpy(c->phone, phone, 31);
    if (email[0] != '\0' && strcmp(email, "-") != 0) strncpy(c->email, email, 79);
    if (address[0] != '\0' && strcmp(address, "-") != 0) strncpy(c->address, address, 159);
    save_customers_csv();
    setColor(10); printf("Customer updated\n"); setColor(7);
}
void ui_delete_customer(void) {
    int id = read_int("Enter customer ID to delete: ", 0);
    if (id <= 0) { setColor(12); printf("Invalid\n"); setColor(7); return; }
    Customer *cur = customerHead, *prev = NULL;
    while (cur) {
        if (cur->id == id) {
            if (prev) prev->next = cur->next; else customerHead = cur->next;
            free(cur); save_customers_csv(); setColor(10); printf("Deleted %d\n", id); setColor(7); return;
        }
        prev = cur; cur = cur->next;
    }
    setColor(12); printf("Not found\n"); setColor(7);
}

/* Offers menu minimal (with UI table and delete) */
void ui_list_offers(void) {
    Offer *o = offerHead;
    setColor(11); printf("\n[ Offers ]\n"); setColor(7);
    while (o) {
        if (o->type == OFFER_PERCENT) printf("ID %d: %s (prod %d) - %.2f%%\n", o->id, o->desc, o->product_id, o->percent);
        else printf("ID %d: %s (prod %d) - Buy%dGet%d\n", o->id, o->desc, o->product_id, o->buy_x, o->get_y);
        o = o->next;
    }
}
void ui_list_offers_table(void) {
    Offer *o = offerHead;
    printf("+------+------------+-------------------------------+----------+\n");
    printf("| ID   | Product ID | Description                   | Discount |\n");
    printf("+------+------------+-------------------------------+----------+\n");
    while (o) {
        if (o->type == OFFER_PERCENT)
            printf("| %-4d | %-10d | %-29s | %7.2f%% |\n", o->id, o->product_id, o->desc, o->percent);
        else
            printf("| %-4d | %-10d | %-29s | Buy%dGet%d |\n", o->id, o->product_id, o->desc, o->buy_x, o->get_y);
        o = o->next;
    }
    printf("+------+------------+-------------------------------+----------+\n");
}
void ui_delete_offer(void) {
		clear_screen();
	ui_list_products_xy(2,2);
    int id = read_int("\n\nEnter Offer ID to delete: ", 0);
    if (id <= 0) { setColor(12); printf("Invalid\n"); setColor(7); return; }
    Offer *cur = offerHead, *prev = NULL;
    while (cur) {
        if (cur->id == id) {
            if (prev) prev->next = cur->next; else offerHead = cur->next;
            free(cur); save_offers_csv(); setColor(10); printf("Offer %d deleted\n", id); setColor(7); return;
        }
        prev = cur; cur = cur->next;
    }
    setColor(12); printf("Offer not found\n"); setColor(7);
}
void ui_add_offer(void) {
	clear_screen();
	ui_list_products_xy(2,2);
    int id = next_offer_id(), t, pid, bx=0, gy=0; double percent=0; char desc[160];
    t = read_int("\n\nOffer type (1=percent,2=BuyXGetY): ", 0);
    if (t != 1 && t != 2) { setColor(12); printf("Invalid...\n"); setColor(7); return; }
    pid = read_int("Product ID to apply: ", 0);
    if (!find_product_by_id(pid)) { setColor(12); printf("Product not found\n"); setColor(7); return; }
    if (t == 1) { percent = read_double("Percent (0-100): ", -1.0); printf("Desc: "); read_line(desc, sizeof(desc)); append_offer(create_offer_node(id, OFFER_PERCENT, pid, percent, 0, 0, desc)); }
    else { bx = read_int("Buy X: ", 0); gy = read_int("Get Y: ", 0); printf("Desc: "); read_line(desc, sizeof(desc)); append_offer(create_offer_node(id, OFFER_BUYXGETY, pid, 0.0, bx, gy, desc)); }
    save_offers_csv(); setColor(10); printf("Offer added ID=%d\n", id); setColor(7);
}

/* Feedback menu minimal */
void ui_feedback_menu(void) {
    int ch = 0;
    while (1) {
        clear_screen();
        draw_main_menu();
        setColor(9); gotoxy(0,2); printf("[ Feedback Menu ]"); setColor(7);
        printf("\n\n[1] Add feedback\n[2] View feedbacks\n[3] Back\n\n| Choose -> ");
        ch = read_int(NULL, -1);
        if (ch == 1) {
            int cust = read_int("Customer ID (0 if guest): ", 0);
            int rating = read_int("Rating 1-5: ", 5);
            char comment[256];
            printf("Comment: "); read_line(comment, sizeof(comment));
            append_feedback(cust, rating, comment);
            save_feedback_file();
            setColor(10); printf("Thanks for feedback!\n"); setColor(7);
        } else if (ch == 2) {
            Feedback *fb = feedbackHead;
            clear_screen();
            setColor(11); printf("Feedbacks:\n"); setColor(7);
            while (fb) { printf("ID %d Cust %d Rating %d Date %s\nComment: %s\n\n", fb->id, fb->cust_id, fb->rating, fb->dt, fb->comment); fb = fb->next; }
            read_line(input, sizeof(input));
        } else if (ch == 3) return;
        else { setColor(12); printf("Invalid...\n"); setColor(7); }
        read_line(input, sizeof(input));
    }
}

/* Admin & user mgmt skeleton */
void ui_user_management(void) {
    int ch = 0;
    while (1) {
        clear_screen();
        draw_main_menu();
        printf("\n[ User Management ]\n\n[1] List users\n[2] Add user\n[3] Back\n\n| Choose -> ");
        ch = read_int(NULL, -1);
        if (ch == 1) { User *u = userHead; printf("\nUsers:\n"); while (u) { printf("User: %s Role: %s\n", u->username, u->role); u = u->next; } read_line(input, sizeof(input)); }
        else if (ch == 2) { char username[64], password[64], role[32]; printf("Username: "); read_line(username, sizeof(username)); printf("Password: "); read_line(password, sizeof(password)); printf("Role: "); read_line(role, sizeof(role)); append_user(create_user_node(username, password, role)); save_users_file(); setColor(10); printf("User added\n"); setColor(7); read_line(input, sizeof(input)); }
        else if (ch == 3) return;
        else { setColor(12); printf("Invalid\n"); setColor(7); read_line(input, sizeof(input)); }
    }
}

/* Admin panel */
void admin_panel(void) {
    clear_screen();
    char role[32];
    printf("[ Admin login required ]\n\n");
    if (!authenticate_user(NULL, role)) { setColor(12); printf("Auth failed\n"); setColor(7); read_line(input, sizeof(input)); return; }
    if (strcmp(role, "admin") != 0 && strcmp(role, "manager") != 0) { setColor(12); printf("Access denied: need admin/manager role\n"); setColor(7); read_line(input, sizeof(input)); return; }
    setColor(10); printf("\nAdmin access granted\n"); setColor(7);
    while (1) {
        int ch;
        printf("\n[ Admin Panel ]\n\n[1] Seed demo data\n[2] User management\n[3] View feedbacks\n[4] Back\n\n| Choose: ");
        ch = read_int(NULL, -1);
        if (ch == 1) {
            append_product(create_product_node(next_product_id(), "Sugar", 45.0, 40));
            append_product(create_product_node(next_product_id(), "Tea", 120.0, 20));
            append_customer(create_customer_node(next_customer_id(), "Vikas", "9000000000", "vikas@ex.com", "Patan"));
            save_products_csv(); save_customers_csv();
            setColor(10); printf("Demo seed added\n"); setColor(7);
        } else if (ch == 2) ui_user_management();
        else if (ch == 3) { Feedback *fb = feedbackHead; clear_screen(); setColor(11); printf("Feedbacks:\n"); setColor(7); while (fb) { printf("ID %d Cust %d Rating %d Date %s\nComment: %s\n\n", fb->id, fb->cust_id, fb->rating, fb->dt, fb->comment); fb = fb->next; } read_line(input, sizeof(input)); }
        else if (ch == 4) break;
        else { setColor(12); printf("Invalid\n"); setColor(7); }
        read_line(input, sizeof(input));
    }
}

/* Seed / load */
void seed_or_load_data(void) {
    ensure_data_dir();
    FILE *f;
    f = fopen(PRODUCTS_CSV, "r"); if (f) { fclose(f); load_products_csv(); } else {
        append_product(create_product_node(101, "Pen", 10.0, 100));
        append_product(create_product_node(102, "Notebook", 50.0, 200));
        append_product(create_product_node(103, "Soap", 25.0, 50));
        append_product(create_product_node(104, "Biscuit", 10.0, 100));
        append_product(create_product_node(105, "Milk", 45.0, 12));
        save_products_csv();
    }
    f = fopen(CUSTOMERS_CSV, "r"); if (f) { fclose(f); load_customers_csv(); } else {
        append_customer(create_customer_node(1, "Rahul", "9876543210", "rahul@example.com", "Patan"));
        append_customer(create_customer_node(2, "Anita", "9123456780", "anita@example.com", "Patan"));
        save_customers_csv();
    }
    f = fopen(OFFERS_CSV, "r"); if (f) { fclose(f); load_offers_csv(); } else {
        append_offer(create_offer_node(1, OFFER_PERCENT, 102, 10.0, 0, 0, "10%_off_Notebook"));
        append_offer(create_offer_node(2, OFFER_BUYXGETY, 101, 0.0, 2, 1, "Buy2Get1_Pen"));
        save_offers_csv();
    }
    f = fopen(USERS_TXT, "r"); if (f) { fclose(f); load_users_file(); } else {
        append_user(create_user_node("admin", "admin123", "admin"));
        append_user(create_user_node("staff", "staff123", "staff"));
        save_users_file();
    }
    f = fopen(FEEDBACK_TXT, "r"); if (f) { fclose(f); load_feedback_file(); } else { save_feedback_file(); }
}

/* ========== Main menu and program flow ========== */
void pause_console(void) {
    char tmp[32];
    printf("\nPress Enter to continue...");
    read_line(tmp, sizeof(tmp));
}

void product_submenu(void) {
    int ch = 0;
    while (1) {
        clear_screen();
        draw_main_menu();
        setColor(9); 
        gotoxy(0,2); 
        printf("[ Product Management ]"); setColor(7);
        printf("\n\n[1] List products\n[2] Add product\n[3] Update product\n[4] Delete product\n[5] Search products\n[6] Inventory alerts\n[7] Back\n\n-> Choose :  ");
        ch = read_int(NULL, -1);
        if (ch == 1) { clear_screen(); ui_list_products(); }
        else if (ch == 2) ui_add_product();
        else if (ch == 3) ui_update_product();
        else if (ch == 4) ui_delete_product();
        else if (ch == 5) ui_search_products();
        else if (ch == 6) ui_inventory_alerts();
        else if (ch == 7) return;
        else { setColor(12); printf("Invalid...\n"); setColor(7); }
        pause_console();
    }
}
void customer_submenu(void) {
    int ch = 0;
    while (1) {
        clear_screen();
        draw_main_menu();
        setColor(9); 
        gotoxy(0,2); 
        printf("[ Customer Management ]"); setColor(7);
        printf("\n\n[1] List customers\n[2] Add customer\n[3] Update customer\n[4] Delete customer\n[5] Search customers\n[6] Customer receipts\n[7] Back\n\n| Choose -> ");
        ch = read_int(NULL, -1);
        if (ch == 1) { clear_screen(); ui_list_customers_table(); }
        else if (ch == 2) ui_add_customer();
        else if (ch == 3) ui_update_customer();
        else if (ch == 4) ui_delete_customer();
        else if (ch == 5) ui_search_customers();
        else if (ch == 6) {
            int id = read_int("Enter customer ID to fetch receipts: ", 0);
            if (id <= 0) { setColor(12); printf("Invalid\n"); setColor(7); }
            else {
                FILE *f = fopen(INVOICES_TXT, "r");
                char line[512];
                int found = 0;
                if (!f) { setColor(12); printf("No invoices file\n"); setColor(7); }
                else {
                    while (fgets(line, sizeof(line), f)) if (strncmp(line, "INVOICE_ID:", 11) == 0) {
                        int inv; char dt[64]; int cust; double pre, gst, tot;
                        if (sscanf(line, "INVOICE_ID:%d|%63[^|]|CUST:%d|PRE_GST:%lf|GST:%lf|TOTAL:%lf", &inv, dt, &cust, &pre, &gst, &tot) == 6) {
                            if (cust == id) { printf("Invoice %d Date:%s Total:%.2f\n", inv, dt, tot); found = 1; }
                        }
                    }
                    fclose(f);
                    if (!found) printf("No receipts found for customer %d\n", id);
                }
            }
        }
        else if (ch == 7) return;
        else { setColor(12); printf("Invalid...\n"); setColor(7); }
        pause_console();
    }
}
void offers_submenu(void) {
    int ch = 0;
    while (1) {
        clear_screen();
        draw_main_menu();
        setColor(9); gotoxy(0,2); printf("[ Offers Management ]"); setColor(7);
        printf("\n\n[1] List offers\n[2] Add offer\n[3] Delete offer\n[4] Back\n\n| Choose -> ");
        ch = read_int(NULL, -1);
        if (ch == 1) { clear_screen(); ui_list_offers_table(); }
        else if (ch == 2) ui_add_offer();
        else if (ch == 3) ui_delete_offer();
        else if (ch == 4) return;
        else { setColor(12); printf("Invalid...\n"); setColor(7); }
        pause_console();
    }
}
void billing_submenu(void) {
    int ch = 0;
    while (1) {
        clear_screen();
        draw_main_menu();
        setColor(9); gotoxy(0,2); printf("[ Billing Counter ]"); setColor(7);
        printf("\n\n[1] Create invoice \n[2] Reprint invoice\n[3] View invoices file\n[4] Back\n\n| Choose -> ");
        ch = read_int(NULL, -1);
        if (ch == 1) ui_create_invoice();
        else if (ch == 2) ui_reprint_invoice();
        else if (ch == 3) ui_view_invoices_file();
        else if (ch == 4) return;
        else { setColor(12); printf("Invalid...\n"); setColor(7); }
        pause_console();
    }
}
void reports_submenu(void) {
    int ch = 0;
    while (1) {
        clear_screen();
        draw_main_menu();
        setColor(9); gotoxy(0,2); printf("[ Reports ]"); setColor(7);
        printf("\n\n[1] Sales summary\n[2] Top customers\n[3] Low-stock report\n[4] Product-wise report\n[5] Generate report file\n[6] Back\n\n| Choose -> ");
        ch = read_int(NULL, -1);
        if (ch == 1) ui_view_sales_summary();
        else if (ch == 2) ui_top_customers();
        else if (ch == 3) ui_low_stock_report();
        else if (ch == 4) ui_product_wise_report_hash();
        else if (ch == 5) generate_reports_to_file();
        else if (ch == 6) return;
        else { setColor(12); printf("Invalid...\n"); setColor(7); }
        pause_console();
    }
}

/* Main menu */
void main_menu(void) {
    while (1) {
        clear_screen();
        draw_main_menu();
        setColor(13);
        gotoxy(2,2); printf("+==================================================+");
        gotoxy(2,3); printf("|                WILD XYZ POS SYSTEM               |");
        gotoxy(2,4); printf("+==================================================+");
        setColor(7);
        gotoxy(2,6); printf("Enter choice (or press menu number on right): ");
        int ch = read_int(NULL, -1);
        if (ch == 1) product_submenu();
        else if (ch == 2) customer_submenu();
        else if (ch == 3) offers_submenu();
        else if (ch == 4) billing_submenu();
        else if (ch == 5) reports_submenu();
        else if (ch == 6) admin_panel();
        else if (ch == 7) ui_feedback_menu();
        else if (ch == 8) {
            save_products_csv(); save_customers_csv(); save_offers_csv(); save_users_file(); save_feedback_file();
            setColor(10); gotoxy(2,18); printf("Saved. Exiting. Good luck!\n"); setColor(7);
            break;
        } else { setColor(12); gotoxy(2,18); printf("Invalid choice!\n"); setColor(7); }
        pause_console();
    }
}

/* ========== MAIN ========== */
int main(void) {
    system("cls");
    seed_or_load_data();
    main_menu();
    return 0;
}

