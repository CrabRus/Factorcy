#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#if defined(_WIN32)
    #include <windows.h>
    #define sleep_ms(ms) Sleep(ms)
#else
    #include <unistd.h>
    #define sleep_ms(ms) usleep((ms) * 1000)
#endif

// ========== CONFIG ==========
#define WIDTH 20
#define HEIGHT 15
#define BELT_CAPACITY 2
#define DELAY 600
// ============================

// ========== TYPES ===========
typedef enum { TILE_EMPTY, TILE_MINER, TILE_BELT, TILE_CHEST } TileType;
typedef enum { DIR_NORTH, DIR_EAST, DIR_SOUTH, DIR_WEST } Direction;
typedef enum { ITEM_NONE, ITEM_IRON_ORE } ItemType;

typedef struct {
    ItemType type;
    int amount;
} Item;

typedef struct {
    Item items[BELT_CAPACITY];
    int head;
    int tail;
    int count;
} BeltInventory;

typedef struct {
    TileType type;
    Direction dir;
    int arrived;
    BeltInventory belt_inv;
} Tile;

// ========== GLOBALS =========
Tile map[HEIGHT][WIDTH];
int chest_item_count = 0;

// Direction -> coordinate offset (indexed by Direction enum)
//                         N   E  S  W
static const int dy[4] = { -1, 0, 1, 0 };
static const int dx[4] = {  0, 1, 0, -1 };

// Direction -> belt symbol for rendering
static const char *belt_arrow[4] = { " ^ ", " > ", " v ", " < " };

// ========== MAP =============
void init_map(void) {
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            map[y][x].type = TILE_EMPTY;
            map[y][x].dir = DIR_NORTH;
            map[y][x].arrived = 0;

            map[y][x].belt_inv.count = 0;
            map[y][x].belt_inv.head = 0;
            map[y][x].belt_inv.tail = 0;
        }
    }

    map[2][2].type = TILE_MINER;
    map[2][2].dir = DIR_EAST;
    
    map[5][16].type = TILE_MINER;
    map[5][16].dir = DIR_WEST;

    map[10][8].type = TILE_MINER;
    map[10][8].dir = DIR_NORTH;

    for (int x = 3; x <= 7; x++) {
        map[2][x].type = TILE_BELT;
        map[2][x].dir = DIR_EAST;
    }

    for (int y = 2; y <= 4; y++) {
        map[y][8].type = TILE_BELT;
        map[y][8].dir = DIR_SOUTH;
    }

    for (int x = 9; x <= 15; x++) {
        map[5][x].type = TILE_BELT;
        map[5][x].dir = DIR_WEST;
    }

    for (int y = 6; y <= 9; y++) {
        map[y][8].type = TILE_BELT;
        map[y][8].dir = DIR_NORTH;
    }

    map[5][8].type = TILE_CHEST;
}

// ========== UPDATE ==========

void belt_push(BeltInventory *inv, Item item) {
    inv->items[inv->tail] = item;
    inv->tail = (inv->tail + 1) % BELT_CAPACITY;
    inv->count++;
}

Item belt_pop(BeltInventory *inv) {
    Item item = inv->items[inv->head];
    inv->head = (inv->head + 1) % BELT_CAPACITY;
    inv->count--;
    return item;
}

void update_miner(int y, int x) {
    Tile *miner = &map[y][x];

    int target_y = y + dy[miner->dir];
    int target_x = x + dx[miner->dir];

    if (target_y < 0 || target_y >= HEIGHT || target_x < 0 || target_x >= WIDTH) {
        return;
    }

    Tile *target_tile = &map[target_y][target_x];

    if (target_tile->type == TILE_BELT && target_tile->belt_inv.count < BELT_CAPACITY) {
        Item new_ore = { ITEM_IRON_ORE, 1 };
        belt_push(&target_tile->belt_inv, new_ore);
        target_tile->arrived++;
    }
}

void update_belt(int y, int x) {
    Tile *belt = &map[y][x];

    if (belt->belt_inv.count == 0) {
        return;
    }

    if (belt->belt_inv.count <= belt->arrived) {
        return;
    }

    int target_y = y + dy[belt->dir];
    int target_x = x + dx[belt->dir];
    if (target_y < 0 || target_y >= HEIGHT || target_x < 0 || target_x >= WIDTH) {
        return;
    }

    Tile *target_tile = &map[target_y][target_x];

    if (target_tile->type == TILE_BELT && target_tile->belt_inv.count < BELT_CAPACITY) {
        belt_push(&target_tile->belt_inv, belt_pop(&belt->belt_inv));
        target_tile->arrived++;
    }
    else if (target_tile->type == TILE_CHEST) {
        Item item = belt_pop(&belt->belt_inv);
        chest_item_count += item.amount;
    }
}

void update_game(void) {
    // Reset arrival counters at the start of every tick
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            map[y][x].arrived = 0;
        }
    }

    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            switch (map[y][x].type) {
                case TILE_MINER:
                    update_miner(y, x);
                    break;
                case TILE_BELT:
                    update_belt(y, x);
                    break;
                default:
                    break;
            }
        }
    }
}

// ========== RENDER ==========
void render(void) {
    printf("\033[H\033[J");

    printf("=== MINIMAL FACTORIO SIMULATOR (C) ===\n");
    printf("Items in a chest: %d\n\n", chest_item_count);

    printf("     ");
    for (int x = 0; x < WIDTH; x++) {
        printf("%2d ", x);
    }
    printf("\n   --");
    for (int x = 0; x < WIDTH; x++) {
        printf("---");
    }
    printf("\n");

    for (int y = 0; y < HEIGHT; y++) {
        
        printf("%2d | ", y);

        for (int x = 0; x < WIDTH; x++) {
            switch (map[y][x].type) {
                case TILE_MINER:
                    printf("[M]");
                    break;
                case TILE_BELT:
                    if (map[y][x].belt_inv.count == 0) printf("%s", belt_arrow[map[y][x].dir]);
                    else                               printf("(%d)", map[y][x].belt_inv.count);
                    break;
                case TILE_CHEST:
                    printf("[C]");
                    break;
                default:
                    printf(" . ");
                    break;
            }
        }
        printf("\n");
    }
    
    printf("\nKey: [M] Drill | ^ > v < Conveyor | (n) Items | [C] Chest\n");
    printf("------------------------------------------------------------\n");
    printf("Controls:\n");
    printf("  1                            - Next tick\n");
    printf("  a <y> <x> <type> <direction> - Add object to the map\n");
    printf("       [Type]: 0=Empty, 1=Drill, 2=Conveyor, 3=Chest\n");
    printf("       [Direction]: 0=North, 1=East, 2=South, 3=West\n");
    printf("  Any other key                - Quit\n");
    printf("------------------------------------------------------------\n");
    printf("Input: ");
}

void add_tile(int y, int x, TileType type, Direction dir){
    map[y][x].type = type;
    map[y][x].dir = dir;
}

// ========== MAIN ============
int main(void) {
    init_map();
    render();

    bool running = true;
    while (running) {
        char mode;
        scanf(" %c", &mode);

        switch (mode) {
            case '1':
                break;

        case 'a': {
                int y, x, t, d;
                scanf("%d %d %d %d", &y, &x, &t, &d);
                add_tile(y, x, t, d);
                break;
            }

            default:
                running = false;
                break;
        }

        if (!running) break;

        update_game();
        render();
        // sleep_ms(DELAY);
    }
    return 0;
}