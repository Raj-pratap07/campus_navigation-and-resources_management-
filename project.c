

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#define DEBUG 1
#define MAX_LOC 10
#define MAX_RES 200
#define MAX_USERS 50
#define LINE_LEN 256



typedef struct {
    char name[80];
    int available; 
} Resource;

typedef struct {
    char username[50];
    char password[50];
    char role[20]; 
} User;



const char *USERS_FILE = "users.txt";
const char *RES_FILE = "resources.txt";

char locations[MAX_LOC][40] = {
    "Main Gate", "Library", "Admin Block", "CSE Lab", "Hostel",
    "Canteen", "Auditorium", "Playground", "Parking", "Medical Center"
};
int numLocations = 10;


int campusMap[MAX_LOC][MAX_LOC];

Resource resources[MAX_RES];
int resourceCount = 0;

User users[MAX_USERS];
int userCount = 0;


#if DEBUG
#define DLOG(fmt, ...) do { \
    fprintf(stderr, "[DEBUG] " fmt "\n", ##__VA_ARGS__); \
} while (0)
#else
#define DLOG(fmt, ...) do {} while (0)
#endif



void ensure_default_users_file() {
    FILE *f = fopen(USERS_FILE, "r");
    if (f) { fclose(f); return; }
    DLOG("Creating default users file");
    f = fopen(USERS_FILE, "w");
    if (!f) { perror("users file create"); return; }
    fprintf(f, "admin,admin123,admin\n");
    fprintf(f, "alice,alice123,student\n");
    fprintf(f, "bob,bob123,student\n");
    fclose(f);
}

void ensure_default_resources_file() {
    FILE *f = fopen(RES_FILE, "r");
    if (f) { fclose(f); return; }
    DLOG("Creating default resources file");
    f = fopen(RES_FILE, "w");
    if (!f) { perror("resources file create"); return; }
    fprintf(f, "Projector,1\n");
    fprintf(f, "CSE Lab PC,5\n"); 
    fprintf(f, "Meeting Room,1\n");
    fprintf(f, "Cricket Bat,2\n");
    fclose(f);
}


void load_users() {
    ensure_default_users_file();
    FILE *f = fopen(USERS_FILE, "r");
    if (!f) { perror("open users"); return; }
    char line[LINE_LEN];
    userCount = 0;
    while (fgets(line, sizeof(line), f)) {
        char *p = strtok(line, "\n\r");
        if (!p || strlen(p) == 0) continue;
        char *u = strtok(p, ",");
        char *pw = strtok(NULL, ",");
        char *role = strtok(NULL, ",");
        if (!u || !pw || !role) continue;
        strncpy(users[userCount].username, u, sizeof(users[userCount].username)-1);
        strncpy(users[userCount].password, pw, sizeof(users[userCount].password)-1);
        strncpy(users[userCount].role, role, sizeof(users[userCount].role)-1);
        users[userCount].username[sizeof(users[userCount].username)-1] = '\0';
        users[userCount].password[sizeof(users[userCount].password)-1] = '\0';
        users[userCount].role[sizeof(users[userCount].role)-1] = '\0';
        DLOG("Loaded user: %s (%s)", users[userCount].username, users[userCount].role);
        userCount++;
        if (userCount >= MAX_USERS) break;
    }
    fclose(f);
}


void load_resources() {
    ensure_default_resources_file();
    FILE *f = fopen(RES_FILE, "r");
    if (!f) { perror("open resources"); return; }
    char line[LINE_LEN];
    resourceCount = 0;
    while (fgets(line, sizeof(line), f)) {
        char *p = strtok(line, "\n\r");
        if (!p || strlen(p) == 0) continue;
        char *name = strtok(p, ",");
        char *availS = strtok(NULL, ",");
        if (!name || !availS) continue;
        strncpy(resources[resourceCount].name, name, sizeof(resources[resourceCount].name)-1);
        int avail = atoi(availS);
        resources[resourceCount].available = (avail > 0) ? avail : 0;
        DLOG("Loaded resource: %s (avail=%d)", resources[resourceCount].name, resources[resourceCount].available);
        resourceCount++;
        if (resourceCount >= MAX_RES) break;
    }
    fclose(f);
}


void save_resources() {
    FILE *f = fopen(RES_FILE, "w");
    if (!f) { perror("save resources"); return; }
    for (int i = 0; i < resourceCount; ++i) {
        fprintf(f, "%s,%d\n", resources[i].name, resources[i].available);
    }
    fclose(f);
    DLOG("Resources saved (%d entries)", resourceCount);
}


int add_user_to_file(const char *username, const char *password, const char *role) {
    FILE *f = fopen(USERS_FILE, "a");
    if (!f) { perror("append user"); return 0; }
    fprintf(f, "%s,%s,%s\n", username, password, role);
    fclose(f);
    return 1;
}



void init_campus_map_default() {
    
    int tmp[MAX_LOC][MAX_LOC] = {
        
        {0, 3, 0, 0, 0, 6, 0, 0, 0, 0}, /* Main Gate */
        {3, 0, 2, 5, 0, 0, 0, 0, 0, 0}, /* Library */
        {0, 2, 0, 3, 4, 0, 0, 0, 0, 0}, /* Admin Block */
        {0, 5, 3, 0, 2, 0, 4, 0, 0, 0}, /* CSE Lab */
        {0, 0, 4, 2, 0, 3, 0, 5, 0, 0}, /* Hostel */
        {6, 0, 0, 0, 3, 0, 2, 0, 4, 0}, /* Canteen */
        {0, 0, 0, 4, 0, 2, 0, 3, 0, 0}, /* Auditorium */
        {0, 0, 0, 0, 5, 0, 3, 0, 6, 0}, /* Playground */
        {0, 0, 0, 0, 0, 4, 0, 6, 0, 1}, /* Parking */
        {0, 0, 0, 0, 0, 0, 0, 0, 1, 0}  /* Medical Center */
    };
    memcpy(campusMap, tmp, sizeof(tmp));
    DLOG("Campus map initialized");
}


void dijkstra_shortest_path(int src, int dest) {
    if (src < 0 || src >= numLocations || dest < 0 || dest >= numLocations) {
        printf("Invalid source/destination indices.\n");
        return;
    }
    int dist[MAX_LOC];
    int visited[MAX_LOC];
    int parent[MAX_LOC];
    for (int i = 0; i < numLocations; ++i) {
        dist[i] = INT_MAX;
        visited[i] = 0;
        parent[i] = -1;
    }
    dist[src] = 0;
    for (int count = 0; count < numLocations - 1; ++count) {
        int u = -1;
        int min = INT_MAX;
        for (int i = 0; i < numLocations; ++i) {
            if (!visited[i] && dist[i] < min) {
                min = dist[i];
                u = i;
            }
        }
        if (u == -1) break; 
        visited[u] = 1;
        for (int v = 0; v < numLocations; ++v) {
            if (!visited[v] && campusMap[u][v] > 0 && dist[u] + campusMap[u][v] < dist[v]) {
                dist[v] = dist[u] + campusMap[u][v];
                parent[v] = u;
            }
        }
    }

    if (dist[dest] == INT_MAX) {
        printf("No path from %s to %s.\n", locations[src], locations[dest]);
        return;
    }
    printf("Shortest distance from %s to %s: %d\n", locations[src], locations[dest], dist[dest]);
    
    int path[MAX_LOC], idx = 0;
    for (int v = dest; v != -1; v = parent[v]) {
        path[idx++] = v;
    }
    printf("Path: ");
    for (int i = idx - 1; i >= 0; --i) {
        printf("%s", locations[path[i]]);
        if (i > 0) printf(" -> ");
    }
    printf("\n");
}



void show_resources() {
    if (resourceCount == 0) {
        printf("No resources available.\n");
        return;
    }
    printf("\nResources:\n");
    for (int i = 0; i < resourceCount; ++i) {
        int avail = resources[i].available;
        if (avail > 1)
            printf("%d. %-30s [Copies: %d]\n", i + 1, resources[i].name, avail);
        else
            printf("%d. %-30s [%s]\n", i + 1, resources[i].name, avail ? "Available" : "Booked");
    }
    printf("\n");
}


void book_resource() {
    show_resources();
    if (resourceCount == 0) return;
    int id;
    printf("Enter resource number to book (0 to cancel): ");
    if (scanf("%d", &id) != 1) { while (getchar()!='\n'); printf("Invalid input.\n"); return; }
    if (id == 0) return;
    if (id < 1 || id > resourceCount) { printf("Invalid resource number.\n"); return; }
    int idx = id - 1;
    if (resources[idx].available <= 0) {
        printf("Resource '%s' is not available.\n", resources[idx].name);
        return;
    }
    resources[idx].available -= 1;
    printf("Booked '%s'.\n", resources[idx].name);
    save_resources();
    DLOG("Resource booked: %s (remaining=%d)", resources[idx].name, resources[idx].available);
}


void release_resource() {
    show_resources();
    if (resourceCount == 0) return;
    int id;
    printf("Enter resource number to release (0 to cancel): ");
    if (scanf("%d", &id) != 1) { while (getchar()!='\n'); printf("Invalid input.\n"); return; }
    if (id == 0) return;
    if (id < 1 || id > resourceCount) { printf("Invalid resource number.\n"); return; }
    int idx = id - 1;
    resources[idx].available += 1;
    printf("Released '%s'.\n", resources[idx].name);
    save_resources();
    DLOG("Resource released: %s (now=%d)", resources[idx].name, resources[idx].available);
}

void admin_add_resource() {
    while (getchar() != '\n'); 
    char name[80];
    int copies = 1;
    printf("Enter resource name: ");
    if (!fgets(name, sizeof(name), stdin)) return;
    name[strcspn(name, "\r\n")] = '\0';
    if (strlen(name) == 0) { printf("Name empty.\n"); return; }
    printf("Enter number of copies (1 if single): ");
    if (scanf("%d", &copies) != 1) { while (getchar()!='\n'); printf("Invalid number.\n"); return; }
    if (resourceCount >= MAX_RES) { printf("Resource limit reached.\n"); return; }
    strncpy(resources[resourceCount].name, name, sizeof(resources[resourceCount].name)-1);
    resources[resourceCount].available = (copies > 0 ? copies : 1);
    resourceCount++;
    save_resources();
    printf("Resource '%s' added with %d copies.\n", name, copies);
    DLOG("Admin added resource: %s copies=%d", name, copies);
}

void admin_remove_resource() {
    show_resources();
    if (resourceCount == 0) return;
    int id;
    printf("Enter resource number to remove (0 to cancel): ");
    if (scanf("%d", &id) != 1) { while (getchar()!='\n'); printf("Invalid input.\n"); return; }
    if (id == 0) return;
    if (id < 1 || id > resourceCount) { printf("Invalid resource number.\n"); return; }
    int idx = id - 1;
    printf("Removing resource '%s'. Are you sure? (y/n): ", resources[idx].name);
    while (getchar() != '\n'); /* flush */
    int c = getchar();
    if (c != 'y' && c != 'Y') { printf("Cancelled.\n"); return; }
    for (int i = idx; i < resourceCount - 1; ++i) resources[i] = resources[i + 1];
    resourceCount--;
    save_resources();
    printf("Resource removed.\n");
    DLOG("Admin removed resource index %d", idx);
}

/* ---------- User Management / Authentication ---------- */

int authenticate(const char *username, const char *password) {
    for (int i = 0; i < userCount; ++i) {
        if (strcmp(users[i].username, username) == 0 && strcmp(users[i].password, password) == 0) {
            DLOG("Authentication success for user %s", username);
            return i; /* return user index */
        }
    }
    DLOG("Authentication failed for user %s", username);
    return -1;
}

/* Register new student (admin only can create users in this implementation OR allow self-register) */
void register_user() {
    char username[50], password[50], role[20] = "student";
    while (getchar() != '\n'); /* flush newline */
    printf("Enter new username: ");
    if (!fgets(username, sizeof(username), stdin)) return;
    username[strcspn(username, "\r\n")] = '\0';
    if (strlen(username) == 0) { printf("Empty username.\n"); return; }
    printf("Enter password: ");
    if (!fgets(password, sizeof(password), stdin)) return;
    password[strcspn(password, "\r\n")] = '\0';
    /* check duplicates */
    for (int i = 0; i < userCount; ++i) {
        if (strcmp(users[i].username, username) == 0) {
            printf("User already exists.\n");
            return;
        }
    }
    if (userCount >= MAX_USERS) { printf("User storage full.\n"); return; }
    strncpy(users[userCount].username, username, sizeof(users[userCount].username)-1);
    strncpy(users[userCount].password, password, sizeof(users[userCount].password)-1);
    strncpy(users[userCount].role, role, sizeof(users[userCount].role)-1);
    userCount++;
    if (!add_user_to_file(username, password, role)) {
        printf("Failed to persist user, but added to session.\n");
    } else {
        printf("User '%s' registered successfully.\n", username);
    }
    DLOG("New user registered: %s", username);
}

/* ---------- Menus ---------- */

void show_locations() {
    printf("\nCampus Locations:\n");
    for (int i = 0; i < numLocations; ++i) {
        printf("%d. %s\n", i, locations[i]);
    }
    printf("\n");
}

/* Student menu */
void student_menu(int userIndex) {
    int choice;
    do {
        printf("\n--- Student Menu (Logged in as %s) ---\n", users[userIndex].username);
        printf("1. Show Locations\n");
        printf("2. Find Shortest Path\n");
        printf("3. View Resources\n");
        printf("4. Book Resource\n");
        printf("5. Release Resource\n");
        printf("6. Register new student (self-registration)\n");
        printf("0. Logout\n");
        printf("Choose: ");
        if (scanf("%d", &choice) != 1) { while (getchar()!='\n'); printf("Invalid input\n"); continue; }
        switch (choice) {
            case 1: show_locations(); break;
            case 2: {
                show_locations();
                int s, e;
                printf("Enter source index: "); if (scanf("%d", &s) != 1) { while (getchar()!='\n'); printf("Invalid input.\n"); break; }
                printf("Enter destination index: "); if (scanf("%d", &e) != 1) { while (getchar()!='\n'); printf("Invalid input.\n"); break; }
                dijkstra_shortest_path(s, e);
                break;
            }
            case 3: show_resources(); break;
            case 4: book_resource(); break;
            case 5: release_resource(); break;
            case 6: register_user(); break;
            case 0: printf("Logging out...\n"); break;
            default: printf("Unknown option.\n");
        }
    } while (choice != 0);
}

/* Admin menu */
void admin_menu(int userIndex) {
    int choice;
    do {
        printf("\n--- Admin Menu (Logged in as %s) ---\n", users[userIndex].username);
        printf("1. Show Locations\n");
        printf("2. Find Shortest Path\n");
        printf("3. View Resources\n");
        printf("4. Book Resource (as user)\n");
        printf("5. Release Resource (as user)\n");
        printf("6. Add Resource\n");
        printf("7. Remove Resource\n");
        printf("8. Register new user (admin)\n");
        printf("9. Show all users\n");
        printf("0. Logout\n");
        printf("Choose: ");
        if (scanf("%d", &choice) != 1) { while (getchar()!='\n'); printf("Invalid input\n"); continue; }
        switch (choice) {
            case 1: show_locations(); break;
            case 2: {
                show_locations();
                int s, e;
                printf("Enter source index: "); if (scanf("%d", &s) != 1) { while (getchar()!='\n'); printf("Invalid input.\n"); break; }
                printf("Enter destination index: "); if (scanf("%d", &e) != 1) { while (getchar()!='\n'); printf("Invalid input.\n"); break; }
                dijkstra_shortest_path(s, e);
                break;
            }
            case 3: show_resources(); break;
            case 4: book_resource(); break;
            case 5: release_resource(); break;
            case 6: admin_add_resource(); break;
            case 7: admin_remove_resource(); break;
            case 8: register_user(); break;
            case 9:
                printf("\nUsers:\n");
                for (int i = 0; i < userCount; ++i)
                    printf("%d. %s (%s)\n", i+1, users[i].username, users[i].role);
                break;
            case 0: printf("Logging out...\n"); break;
            default: printf("Unknown option.\n");
        }
    } while (choice != 0);
}

/* ---------- Main ---------- */

int main(void) {
    printf("Campus Navigation & Resource Management System (C)\n");
    /* initialize */
    init_campus_map_default();
    load_users();
    load_resources();

    /* Main program loop */
    while (1) {
        printf("\n=== Main Menu ===\n");
        printf("1. Login\n");
        printf("2. Register (student)\n");
        printf("3. Show Locations\n");
        printf("4. Show Resources\n");
        printf("0. Exit\n");
        printf("Choose option: ");
        int opt;
        if (scanf("%d", &opt) != 1) { while (getchar()!='\n'); printf("Invalid input, try again.\n"); continue; }

        if (opt == 0) {
            printf("Exiting... saving data.\n");
            save_resources();
            break;
        }
        else if (opt == 1) {
            char username[50], password[50];
            printf("Username: "); scanf("%49s", username);
            printf("Password: "); scanf("%49s", password);
            int idx = authenticate(username, password);
            if (idx == -1) {
                printf("Login failed. Try again.\n");
            } else {
                if (strcmp(users[idx].role, "admin") == 0) admin_menu(idx);
                else student_menu(idx);
            }
        }
        else if (opt == 2) {
            register_user();
        }
        else if (opt == 3) {
            show_locations();
        }
        else if (opt == 4) {
            show_resources();
        }
        else {
            printf("Invalid option.\n");
        }
    }

    return 0;
}
