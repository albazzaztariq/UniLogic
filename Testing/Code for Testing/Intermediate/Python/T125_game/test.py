from dataclasses import dataclass, field

def _ul_sort(arr, n): arr[:n] = sorted(arr[:n])

class _ul_Exception(Exception):
    def __init__(self, type_name, message):
        self.type_name = type_name
        self.message = message
        super().__init__(message)


def clamp_hp(hp):
    if (hp < 0):
        return 0
    if (hp > 100):
        return 100
    return hp

def calc_attack(level):
    return (10 + ((level - 1) * 5))

def format_status(name, hp, gold, level):
    return ((((((('[' + name) + '] HP:') + str(hp)) + ' Gold:') + str(gold)) + ' Lv:') + str(level))

def room_name(id):
    names = ['Entrance Hall', 'Dark Corridor', 'Treasure Room', 'Monster Den', 'Boss Chamber']
    return names[id]

def room_monster(id):
    monsters = [(-1), 0, (-1), 1, 4]
    return monsters[id]

def room_treasure(id):
    treasure = [0, 0, 50, 0, 0]
    return treasure[id]

def monster_name(id):
    names = ['Goblin', 'Skeleton', 'Spider', 'Orc', 'Dragon']
    return names[id]

def monster_hp(id):
    hp = [20, 30, 15, 50, 100]
    return hp[id]

def monster_attack(id):
    atk = [5, 8, 3, 12, 25]
    return atk[id]

def monster_gold(id):
    gold = [10, 15, 5, 30, 100]
    return gold[id]

def fight_monster(m_name, m_hp, m_atk, player_atk):
    total_damage = 0
    print((('  Fighting ' + m_name) + '!'))
    while (m_hp > 0):
        m_hp = (m_hp - player_atk)
        if (m_hp <= 0):
            print((('  ' + m_name) + ' defeated!'))
            return total_damage
        total_damage = (total_damage + m_atk)
    return total_damage

def show_enter(name):
    print(('>> Entering: ' + name))

def show_divider():
    print('---')

def main():
    name = 'Hero'
    hp = 100
    gold = 0
    level = 1
    atk = calc_attack(level)
    print('=== DUNGEON CRAWLER ===')
    print(format_status(name, hp, gold, level))
    show_divider()
    room = 0
    while (room < 5):
        show_enter(room_name(room))
        treasure = room_treasure(room)
        if (treasure > 0):
            print((('  Found treasure: ' + str(treasure)) + ' gold!'))
            gold = (gold + treasure)
        mid = room_monster(room)
        if (mid >= 0):
            dmg = fight_monster(monster_name(mid), monster_hp(mid), monster_attack(mid), atk)
            hp = clamp_hp((hp - dmg))
            gold = (gold + monster_gold(mid))
            print((('  Gained ' + str(monster_gold(mid))) + ' gold'))
        else:
            print('  The room is quiet.')
        print(format_status(name, hp, gold, level))
        show_divider()
        if (room == 2):
            level = (level + 1)
            atk = calc_attack(level)
            print(('>> Level Up! Now Lv:' + str(level)))
            show_divider()
        if (room == 3):
            print('>> Using healing potion...')
            hp = clamp_hp((hp + 50))
            print(format_status(name, hp, gold, level))
            show_divider()
        room = (room + 1)
    print('=== GAME OVER ===')
    if (hp > 0):
        print(('Victory! Final gold: ' + str(gold)))
    else:
        print('Defeat!')
    return 0


if __name__ == "__main__":
    main()
