#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#define __ul_malloc(size) malloc(size)
#define __ul_free(ptr)    free(ptr)
static char* __ul_strcat(const char* a, const char* b) {
    int la = (int)strlen(a), lb = (int)strlen(b);
    char* r = (char*)__ul_malloc(la + lb + 1); memcpy(r, a, la); memcpy(r + la, b, lb); r[la+lb] = 0; return r; }
static int __ul_cmp_int(const void* a, const void* b) { return (*(int*)a - *(int*)b); }
static int __ul_cmp_float(const void* a, const void* b) { float fa=*(float*)a, fb=*(float*)b; return (fa>fb)-(fa<fb); }
static int __ul_cmp_double(const void* a, const void* b) { double da=*(double*)a, db=*(double*)b; return (da>db)-(da<db); }
static int __ul_cmp_string(const void* a, const void* b) { return strcmp(*(char**)a, *(char**)b); }

int clamp_hp(int hp);
int calc_attack(int level);
char* format_status(char* name, int hp, int gold, int level);
char* room_name(int id);
int room_monster(int id);
int room_treasure(int id);
char* monster_name(int id);
int monster_hp(int id);
int monster_attack(int id);
int monster_gold(int id);
int fight_monster(char* m_name, int m_hp, int m_atk, int player_atk);
void show_enter(char* name);
void show_divider(void);
int main(void);

int clamp_hp(int hp)
{
    if ((hp < 0)) {
        return 0;
    }
    if ((hp > 100)) {
        return 100;
    }
    return hp;
}

int calc_attack(int level)
{
    return (10 + ((level - 1) * 5));
}

char* format_status(char* name, int hp, int gold, int level)
{
    static char _cast_buf_0[64];
    snprintf(_cast_buf_0, sizeof(_cast_buf_0), "%d", hp);
    static char _cast_buf_1[64];
    snprintf(_cast_buf_1, sizeof(_cast_buf_1), "%d", gold);
    static char _cast_buf_2[64];
    snprintf(_cast_buf_2, sizeof(_cast_buf_2), "%d", level);
    return __ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat(__ul_strcat("[", name), "] HP:"), _cast_buf_0), " Gold:"), _cast_buf_1), " Lv:"), _cast_buf_2);
}

char* room_name(int id)
{
    char* names[] = {"Entrance Hall", "Dark Corridor", "Treasure Room", "Monster Den", "Boss Chamber"};
    return names[id];
}

int room_monster(int id)
{
    int monsters[] = {(-1), 0, (-1), 1, 4};
    return monsters[id];
}

int room_treasure(int id)
{
    int treasure[] = {0, 0, 50, 0, 0};
    return treasure[id];
}

char* monster_name(int id)
{
    char* names[] = {"Goblin", "Skeleton", "Spider", "Orc", "Dragon"};
    return names[id];
}

int monster_hp(int id)
{
    int hp[] = {20, 30, 15, 50, 100};
    return hp[id];
}

int monster_attack(int id)
{
    int atk[] = {5, 8, 3, 12, 25};
    return atk[id];
}

int monster_gold(int id)
{
    int gold[] = {10, 15, 5, 30, 100};
    return gold[id];
}

int fight_monster(char* m_name, int m_hp, int m_atk, int player_atk)
{
    int total_damage = 0;
    printf("%s\n", __ul_strcat(__ul_strcat("  Fighting ", m_name), "!"));
    while ((m_hp > 0)) {
        m_hp = (m_hp - player_atk);
        if ((m_hp <= 0)) {
            printf("%s\n", __ul_strcat(__ul_strcat("  ", m_name), " defeated!"));
            return total_damage;
        }
        total_damage = (total_damage + m_atk);
    }
    return total_damage;
}

void show_enter(char* name)
{
    printf("%s\n", __ul_strcat(">> Entering: ", name));
}

void show_divider(void)
{
    printf("%s\n", "---");
}

int main(void)
{
    char* name = "Hero";
    int hp = 100;
    int gold = 0;
    int level = 1;
    int atk = calc_attack(level);
    printf("%s\n", "=== DUNGEON CRAWLER ===");
    printf("%s\n", format_status(name, hp, gold, level));
    show_divider();
    int room = 0;
    while ((room < 5)) {
        show_enter(room_name(room));
        int treasure = room_treasure(room);
        if ((treasure > 0)) {
            static char _cast_buf_3[64];
            snprintf(_cast_buf_3, sizeof(_cast_buf_3), "%d", treasure);
            printf("%s\n", __ul_strcat(__ul_strcat("  Found treasure: ", _cast_buf_3), " gold!"));
            gold = (gold + treasure);
        }
        int mid = room_monster(room);
        if ((mid >= 0)) {
            int dmg = fight_monster(monster_name(mid), monster_hp(mid), monster_attack(mid), atk);
            hp = clamp_hp((hp - dmg));
            gold = (gold + monster_gold(mid));
            static char _cast_buf_4[64];
            snprintf(_cast_buf_4, sizeof(_cast_buf_4), "%d", monster_gold(mid));
            printf("%s\n", __ul_strcat(__ul_strcat("  Gained ", _cast_buf_4), " gold"));
        } else {
            printf("%s\n", "  The room is quiet.");
        }
        printf("%s\n", format_status(name, hp, gold, level));
        show_divider();
        if ((room == 2)) {
            level = (level + 1);
            atk = calc_attack(level);
            static char _cast_buf_5[64];
            snprintf(_cast_buf_5, sizeof(_cast_buf_5), "%d", level);
            printf("%s\n", __ul_strcat(">> Level Up! Now Lv:", _cast_buf_5));
            show_divider();
        }
        if ((room == 3)) {
            printf("%s\n", ">> Using healing potion...");
            hp = clamp_hp((hp + 50));
            printf("%s\n", format_status(name, hp, gold, level));
            show_divider();
        }
        room = (room + 1);
    }
    printf("%s\n", "=== GAME OVER ===");
    if ((hp > 0)) {
        static char _cast_buf_6[64];
        snprintf(_cast_buf_6, sizeof(_cast_buf_6), "%d", gold);
        printf("%s\n", __ul_strcat("Victory! Final gold: ", _cast_buf_6));
    } else {
        printf("%s\n", "Defeat!");
    }
    return 0;
}


