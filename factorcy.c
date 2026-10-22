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

#define WIDTH 20
#define HEIGHT 10

typedef enum {
    TILE_EMPTY,
    TILE_MINER,
    TILE_BELT_RIGHT,
    TILE_CHEST
} TileType;

typedef struct {
    TileType type;
    bool has_ore;
} Tile;

Tile map[HEIGHT][WIDTH];
int chest_item_count = 0;

void init_map(void) {
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            map[y][x].type = TILE_EMPTY;
            map[y][x].has_ore = false;
        }
    }


    map[2][1].type = TILE_MINER;
    map[2][2].type = TILE_BELT_RIGHT;
    map[2][3].type = TILE_BELT_RIGHT;
    map[2][4].type = TILE_BELT_RIGHT;
    map[2][5].type = TILE_BELT_RIGHT;
    map[2][6].type = TILE_BELT_RIGHT;
    map[2][7].type = TILE_BELT_RIGHT;
    map[2][8].type = TILE_CHEST;
}

void update_game(void) {
    for (int x = WIDTH - 2; x >= 1; x--) {
        if (map[2][x].type == TILE_BELT_RIGHT && map[2][x].has_ore) {
            Tile *next_tile = &map[2][x + 1];

            if (next_tile->type == TILE_BELT_RIGHT && !next_tile->has_ore) {
                next_tile->has_ore = true;
                map[2][x].has_ore = false;
            } else if (next_tile->type == TILE_CHEST) {
                chest_item_count++;
                map[2][x].has_ore = false;
            }
        }
    }

    if (map[2][1].type == TILE_MINER) {
        if (map[2][2].type == TILE_BELT_RIGHT && !map[2][2].has_ore) {
            map[2][2].has_ore = true;
        }
    }
}

void render(void) {
    printf("\033[H\033[J");

    printf("=== MINIMAL FACTORIO SIMULATOR (C) ===\n");
    printf("Items in a chest: %d\n\n", chest_item_count);

    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            Tile t = map[y][x];

            switch (t.type) {
                case TILE_MINER:
                    printf("[M]");
                    break;
                case TILE_BELT_RIGHT:
                    printf(t.has_ore ? "(o)" : " > ");
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
    printf("\nKey: [M] Drill |  >  Conveyor | (o) Ore | [C] Chest\n");
}

int main(void) {
    init_map();

    while (1) {
        update_game();
        render();
        sleep_ms(300);
        if(chest_item_count == 10){
            break;
        }
    }
    int i;
    scanf("%d", &i);

    return 0;
}