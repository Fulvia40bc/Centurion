"""savediff.py - name each differing element of two version-0x38 save files (the M7 check tool).

Usage: python -I savediff.py A.DAT B.DAT [--ignore NAME]... [--max N]
       python -I savediff.py --layout [A.DAT]

Today a difference between two autosaves shows only as a byte offset; this names the field, the record and
the value, so the first difference points at the routine that went wrong.

The layout below is the writer's field list (port/src/savewrite.c f5E9B0, the same list as the reader f5F05C in
port/src/saveload.c; file offsets D, checked on a real save: version stamp "1.11B1" at 0x4535, seed 0xDE9B76A2 at
0x3D0C). Version 0x38 only. All integers little-endian. D/G labels from the HANDOFF task are kept in comments;
the output does not print them.

Sizes: an ordinary save/autosave is 44,062 bytes; the end-of-game autosave is 44,066 (a second CRC appended after
the first, V 19 Nov: 0xAC1A..0xAC1D first CRC, 0xAC1E..0xAC21 the CRC over 0x4B..end-4). CRC-32 reflected
0xEDB88320, start 0, no final inversion, over 0x4B up to the last 4 bytes, stored LE - imported from hsave.py.
The PBEM block (bit 0x01 of the setup byte at file 0x53) is refused: PBEM was dropped (Erik, 18 Nov).

Output: a header line per file (name, size, description, phase+round, CRC ok/bad), then one line per differing
element in file order:  0x3D0C  seed  0xDE9B76A2 -> 0xA7BB1DC8.  The CRC bytes are never listed. Exit 0 when no
element differs (after --ignore), 1 when any does, 2 on a refusal (wrong version, wrong size, PBEM bit).
"""
import argparse
import os
import struct
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))   # python -I leaves the script's folder out
from hsave import crc32

VERSION = 0x38
SIZE, SIZE_END = 44062, 44066          # ordinary save / end-of-game autosave (a second CRC appended)
BODY_OFF = 0x4B
CRC_OFF = 0xAC1A                        # first CRC; an end-game file has a second at 0xAC1E
DESC_OFF, DESC_LEN = 0x18, 51
PHASE_OFF, ROUND_OFF = 0x395D, 0x395E
SETUP_FLAGS = 0x53                      # setup +4: bit 0x01 = the PBEM block is present

NATIONS = {1: 'AFRICA', 2: 'IBERIA', 3: 'ILLYRIA', 4: 'THRACE', 5: 'CRETE', 6: 'ASIA', 7: 'ASSYRIA',
           8: 'BABYLON', 9: 'EGYPT', 10: 'PIRATES', 11: 'BARBARIANS', 12: 'FIRST FACTION', 13: 'SECOND FACTION'}
PHASES = ['TAXATION', 'POPULATION EXPANSION', 'CENSUS', 'SHIP MAINTENANCE', 'SHIP CONSTRUCTION', 'MOVEMENT',
          'VIEW MOVEMENT', 'CONFLICT', 'CITY CONSTRUCTION', 'REMOVE SURPLUS', 'CITY SUPPORT', 'ACQUIRE GOODS',
          'DECLARE OFFERS', 'COUNTER OFFERS', 'ACCEPT TRADES', 'CALAMITIES', 'MONOTHEISM CONVERT', 'CITY SUPPORT',
          'ACQUIRE CIVILIZATION', 'DISCARD EXCESS', 'SHOW AST', 'SELECT START']

FMT = {'u8': 'B', 's8': 'b', 'u16': 'H', 's16': 'h', 'u32': 'I', 's32': 'i'}
WIDTH = {'u8': 2, 's8': 2, 'u16': 4, 's16': 4, 'u32': 8, 's32': 8}

ELEMS = []    # (off, size, typ, label, field, hexed) in file order; typ 'text' = one repr element, 'phase' = u8 + name
FIELDS = []   # (off, size, link, name, count, esize, is_crc)


def E(off, size, typ, label, field, hexed=False):
    ELEMS.append((off, size, typ, label, field, hexed))


def F(off, size, link, name, count, esize, is_crc=False):
    FIELDS.append((off, size, link, name, count, esize, is_crc))


def nat_idx(name, n, extra=''):
    """label for a [n]-indexed array: the nation's name too (grain_vs_famine[5 CRETE])"""
    nm = NATIONS.get(n)
    return f'{name}[{n} {nm}]{extra}' if nm else f'{name}[{n}]{extra}'


def nat_rec(name, n):
    return f'{name} {n} {NATIONS[n]}' if n in NATIONS else f'{name} {n}'


def area_rec(name, r):
    return f'{name} {r} (t {r - 1})'


def rec_elems(base, field, rlabel, subs):
    for (soff, size, typ, sub, hexed) in subs:
        E(base + soff, size, typ, f'{rlabel} +0x{soff:02X} {sub}', field, hexed)


# ---- the record layouts (D: the writer's field list; G where the task says so) ------------------------------

def nation_subs():
    s = [(0x00, 1, 'u8', 'byte', True)]                       # +0x00 u8 ?
    for c in range(1, 19):                                     # +0x01..+0x12 s8 cards[c], c 1..18
        s.append((c, 1, 's8', f'cards[{c}]', False))
    s.append((0x13, 1, 's8', 'calamity_count', False))
    for j in range(8):                                         # +0x14..+0x1B u8[8] calamity_id[j]
        s.append((0x14 + j, 1, 'u8', f'calamity_id[{j}]', False))
    for j in range(8):                                         # +0x1C..+0x23 u8[8] calamity_giver[j] (0xFF drawn)
        s.append((0x1C + j, 1, 'u8', f'calamity_giver[{j}]', False))
    # +0x24..+0x3B u8[24] advance[i]; i: 0 Pottery, 1 Clothmaking, 2 Metalworking, 3 Agriculture, 4 Roadbuilding,
    # 5 Mining, 6 Engineering, 7 Astronomy, 8 Coinage, 9 Medicine, 10 Mathematics, 11 Drama & Poetry, 12 Music,
    # 13 Architecture, 14 Literacy, 15 Law, 16 Democracy, 17 Military, 18 Philosophy, 19 Mysticism, 20 Deism,
    # 21 Enlightenment, 22 Monotheism, 23 Theology
    for i in range(24):
        s.append((0x24 + i, 1, 'u8', f'advance[{i}]', False))
    s += [(0x3C, 1, 's8', 'cards_bought', False), (0x3D, 1, 'u8', 'number', False), (0x3E, 1, 's8', 'treasury', False),
          (0x3F, 1, 's8', 'tax_rate', False), (0x40, 1, 'u8', 'ships', False)]
    for k in range(4):                                         # +0x41..+0x44 u8[4] ship_points[k]
        s.append((0x41 + k, 1, 'u8', f'ship_points[{k}]', False))
    for k in range(4):                                         # +0x45..+0x4C s16[4] ship_area[k] (territory + 1)
        s.append((0x45 + 2 * k, 2, 's16', f'ship_area[{k}]', False))
    for k in range(4):                                         # +0x4D..+0x50 s8[4] ship_tokens[k]
        s.append((0x4D + k, 1, 's8', f'ship_tokens[{k}]', False))
    s.append((0x51, 1, 's8', 'cities', False))
    for j in range(10):                                        # +0x52..+0x5B u8[10] city[j] (territory + 1)
        s.append((0x52 + j, 1, 'u8', f'city[{j}]', False))
    s += [(0x5C, 1, 's8', 'ast', False), (0x5D, 1, 'u8', 'byte', True), (0x5E, 2, 's16', 'advance_points', False),
          (0x60, 1, 's8', 'occupied_count', False)]
    for j in range(55):                                        # +0x61..+0x97 u8[55] occupied[j] (territory + 1, 0xFF unused)
        s.append((0x61 + j, 1, 'u8', f'occupied[{j}]', False))
    s += [(0x98, 1, 's8', 'tokens_on_board', False), (0x99, 1, 's8', 'tokens_in_stock', False),
          (0x9A, 1, 's8', 'player_type', False), (0x9B, 1, 'u8', 'ai_trait_9b', False)]   # +0x9B (G)
    for k in range(2):                                         # +0x9C..+0x9D u8[2] offer_stated
        s.append((0x9C + k, 1, 'u8', f'offer_stated[{k}]', False))
    for k in range(3):                                         # +0x9E..+0xA0 u8[3] offer_bluff
        s.append((0x9E + k, 1, 'u8', f'offer_bluff[{k}]', False))
    for k in range(5):                                         # +0xA1..+0xA5 u8[5] offer_wanted
        s.append((0xA1 + k, 1, 'u8', f'offer_wanted[{k}]', False))
    for k in range(3):                                         # +0xA6..+0xA8 u8[3] offer_bluff_given
        s.append((0xA6 + k, 1, 'u8', f'offer_bluff_given[{k}]', False))
    for m in range(1, 10):                                     # +0xA9..+0xB1 s8 hostility[m] at +0xA8+m (G)
        s.append((0xA8 + m, 1, 's8', f'hostility[{m}]', False))
    for m in range(1, 10):                                     # +0xB2..+0xBA s8 trust[m] at +0xB1+m (G)
        s.append((0xB1 + m, 1, 's8', f'trust[{m}]', False))
    s += [(0xBB, 1, 'u8', 'byte', True), (0xBC, 1, 'u8', 'ai_trait_bc', False),   # +0xBC (G)
          (0xBD, 1, 'u8', 'byte', True)]
    for m in range(1, 10):                                     # +0xBE..+0xC6 u8 passed_calamity_to[m] at +0xBD+m
        s.append((0xBD + m, 1, 'u8', f'passed_calamity_to[{m}]', False))
    return s


def area_subs():
    s = [(0x00, 1, 'u8', 'index', False), (0x01, 1, 's8', 'nations_present', False)]
    for n in range(1, 14):                                     # +0x02..+0x0E s8 present[n] at +0x01+n (n 1..13)
        s.append((0x01 + n, 1, 's8', f'present[{n}]', False))
    for n in range(1, 14):                                     # +0x0F..+0x1B s8 tokens[n] at +0x0E+n
        s.append((0x0E + n, 1, 's8', f'tokens[{n}]', False))
    for n in range(1, 10):                                     # +0x1C..+0x24 u8 ships[n] at +0x1B+n (n 1..9)
        s.append((0x1B + n, 1, 'u8', f'ships[{n}]', False))
    for n in range(1, 10):                                     # +0x25..+0x2D s8 movable[n] at +0x24+n (scratch)
        s.append((0x24 + n, 1, 's8', f'movable[{n}]', False))
    for n in range(1, 10):                                     # +0x2E..+0x36 u8 selected_ship[n] at +0x2D+n
        s.append((0x2D + n, 1, 'u8', f'selected_ship[{n}]', False))
    s += [(0x37, 2, 's16', 's16', True), (0x39, 2, 's16', 's16', True)]   # +0x37..+0x3A s16[2] ?
    for n in range(0, 10):                                     # +0x3B..+0x44 s8 lost[n] at +0x3B+n (G)
        s.append((0x3B + n, 1, 's8', f'lost[{n}]', False))
    return s


def stack_subs():
    s = [(0x00, 2, 's16', 'count', False), (0x02, 2, 's16', 'a', False), (0x04, 2, 's16', 'b', False)]
    # +0x06..+0x18 u8[19] card[i]; ids 1..18 commodities, 0x13..0x1E calamities; the top card is index count - 1
    for i in range(19):
        s.append((0x06 + i, 1, 'u8', f'card[{i}]', False))
    return s


def tstate_subs():
    # +0 u8 owner (nation of the city; 0 none), +1 u8 round_built (0xFF removed), +2 u8 flags (0x01 city, 0x02 AI site)
    return [(0x00, 1, 'u8', 'owner', False), (0x01, 1, 'u8', 'round_built', False), (0x02, 1, 'u8', 'flags', True)]


def counter_row_subs():
    s = []
    for k in range(2):                                         # +0x00 u8[2] stated
        s.append((0x00 + k, 1, 'u8', f'stated[{k}]', False))
    for k in range(3):                                         # +0x02 u8[3] bluffs
        s.append((0x02 + k, 1, 'u8', f'bluffs[{k}]', False))
    for k in range(3):                                         # +0x05 u8[3] replacements
        s.append((0x05 + k, 1, 'u8', f'replacements[{k}]', False))
    for k in range(5):                                         # +0x08 u8[5] shuffled copy
        s.append((0x08 + k, 1, 'u8', f'shuffled[{k}]', False))
    s += [(0x0D, 1, 's8', 'offerer', False), (0x0E, 1, 's8', 'responder', False),
          (0x0F, 1, 'u8', 'offer_flag', True)]
    return s


def player_subs():
    s = [(0x00, 1, 's8', 'nation', False), (0x01, 1, 'u8', 'start_territory', False)]   # +0x01 (G)
    for k in range(30):                                        # +0x02..+0x1F bytes (G: a PBEM name)
        s.append((0x02 + k, 1, 'u8', f'pbem_name[{k}]', False))
    return s


def response_subs():
    s = []
    for k in range(16):                                        # +0x00 u8[16] copy of a counter row
        s.append((0x00 + k, 1, 'u8', f'row_copy[{k}]', False))
    s += [(0x10, 2, 's16', 'gain', False), (0x12, 1, 's8', 'row', False), (0x13, 1, 's8', 'pbem_pref', False)]
    return s


def trade_log_subs():
    s = [(0x000, 1, 's8', 'count', False)]
    for t in range(32):                                        # +0x001 + 5t + j s8 offerer_cards[t][j]
        for j in range(5):
            s.append((0x001 + 5 * t + j, 1, 's8', f'offerer_cards[{t}][{j}]', False))
    for t in range(32):                                        # +0x0A1 + 5t + j s8 responder_cards[t][j]
        for j in range(5):
            s.append((0x0A1 + 5 * t + j, 1, 's8', f'responder_cards[{t}][{j}]', False))
    for t in range(32):                                        # +0x141 + t s8 offerer[t]
        s.append((0x141 + t, 1, 's8', f'offerer[{t}]', False))
    for t in range(32):                                        # +0x161 + t s8 responder[t]
        s.append((0x161 + t, 1, 's8', f'responder[{t}]', False))
    for t in range(32):                                        # +0x181 + t s8 session[t]
        s.append((0x181 + t, 1, 's8', f'session[{t}]', False))
    return s


# ---- the top-level fields, in file order ---------------------------------------------------------------------

def build():
    # 0x0000 version s16 0x38 (one element)
    F(0x0000, 2, '-', 'version', 1, 2)
    E(0x0000, 2, 'u16', 'version', 'version', True)
    # 0x0002 tag "Advanced Civilization" + NUL (one element)
    F(0x0002, 22, '-', 'tag', 1, 22)
    E(0x0002, 22, 'text', 'tag', 'tag')
    # 0x0018 description, NUL-terminated text (one element)
    F(0x0018, 51, '-', 'description', 1, 51)
    E(0x0018, 51, 'text', 'description', 'description')
    # 0x004B round_start_seed u32 (G: copy of the seed, PBEM code), link 0xF40F4
    F(0x004B, 4, '0xF40F4', 'round_start_seed', 1, 4)
    E(0x004B, 4, 'u32', 'round_start_seed', 'round_start_seed', True)
    # 0x004F setup, link 0xE1620: +0 s8 humans, +1 s8 computers, +2 s8 pbem_viewer, +3 u8 trade settings,
    # +4 u8 flags (0x01 PBEM block, ...), +5 u8 flags (bits 2..3 AI level)
    F(0x004F, 6, '0xE1620', 'setup', 6, 1)
    for i, (typ, hx) in enumerate([('s8', False), ('s8', False), ('s8', False), ('u8', False), ('u8', True), ('u8', True)]):
        E(0x004F + i, 1, typ, f'setup +{i}', 'setup', hx)
    # 0x0055 nation[n], 14 x 199, link 0xF14CC
    F(0x0055, 14 * 199, '0xF14CC', 'nation', 14, 199)
    for n in range(14):
        rec_elems(0x0055 + 199 * n, 'nation', nat_rec('nation', n), nation_subs())
    # 0x0B37 area[r], 161 x 69, link 0xE31B8; record r is territory r - 1, record 0 is none
    F(0x0B37, 161 * 69, '0xE31B8', 'area', 161, 69)
    for r in range(161):
        rec_elems(0x0B37 + 69 * r, 'area', area_rec('area', r), area_subs())
    # 0x369C stack[s], 9 x 25, link 0xF06D0
    F(0x369C, 9 * 25, '0xF06D0', 'stack', 9, 25)
    for s in range(9):
        rec_elems(0x369C + 25 * s, 'stack', f'stack {s}', stack_subs())
    # 0x377D tstate[t], 160 x 3, link 0xE2C18
    F(0x377D, 160 * 3, '0xE2C18', 'tstate', 160, 3)
    for t in range(160):
        rec_elems(0x377D + 3 * t, 'tstate', f'tstate {t}', tstate_subs())
    # 0x395D phase u8, shown with its name, link 0xF4102
    F(0x395D, 1, '0xF4102', 'phase', 1, 1)
    E(0x395D, 1, 'phase', 'phase', 'phase')
    # 0x395E round s16, link 0xF40FC
    F(0x395E, 2, '0xF40FC', 'round', 1, 2)
    E(0x395E, 2, 's16', 'round', 'round')
    # 0x3960 census_count s16, link 0xF0698
    F(0x3960, 2, '0xF0698', 'census_count', 1, 2)
    E(0x3960, 2, 's16', 'census_count', 'census_count')
    # 0x3962 census_order[i], 10 x 2, link 0xF069A
    F(0x3962, 20, '0xF069A', 'census_order', 10, 2)
    for i in range(10):
        E(0x3962 + 2 * i, 2, 's16', f'census_order[{i}]', 'census_order')
    # 0x3976 player_count s16, link 0xF06AE
    F(0x3976, 2, '0xF06AE', 'player_count', 1, 2)
    E(0x3976, 2, 's16', 'player_count', 'player_count')
    # 0x3978 player_order[i], 8 x 2, link 0xF06B0
    F(0x3978, 16, '0xF06B0', 'player_order', 8, 2)
    for i in range(8):
        E(0x3978 + 2 * i, 2, 's16', f'player_order[{i}]', 'player_order')
    # 0x3988 trade_session s16, link 0xF5476
    F(0x3988, 2, '0xF5476', 'trade_session', 1, 2)
    E(0x3988, 2, 's16', 'trade_session', 'trade_session')
    # 0x398A counter_row[i], 56 x 16, link 0x111118
    F(0x398A, 56 * 16, '0x111118', 'counter_row', 56, 16)
    for i in range(56):
        rec_elems(0x398A + 16 * i, 'counter_row', f'counter_row {i}', counter_row_subs())
    # 0x3D0A counter_row_count s16, link 0x112548
    F(0x3D0A, 2, '0x112548', 'counter_row_count', 1, 2)
    E(0x3D0A, 2, 's16', 'counter_row_count', 'counter_row_count')
    # 0x3D0C seed u32 hex, link 0xCAC5C
    F(0x3D0C, 4, '0xCAC5C', 'seed', 1, 4)
    E(0x3D0C, 4, 'u32', 'seed', 'seed', True)
    # 0x3D10 player[p], 8 x 32, link 0xF4104
    F(0x3D10, 8 * 32, '0xF4104', 'player', 8, 32)
    for p in range(8):
        rec_elems(0x3D10 + 32 * p, 'player', f'player {p}', player_subs())
    # 0x3E10 pbem_player s16 (G), link 0xF5474
    F(0x3E10, 2, '0xF5474', 'pbem_player', 1, 2)
    E(0x3E10, 2, 's16', 'pbem_player', 'pbem_player')
    # 0x3E12 pbem_name, text (one element), link 0x109E84
    F(0x3E12, 20, '0x109E84', 'pbem_name', 1, 20)
    E(0x3E12, 20, 'text', 'pbem_name', 'pbem_name')
    # 0x3E26 pbem_goods_flag s16, link 0xF5478
    F(0x3E26, 2, '0xF5478', 'pbem_goods_flag', 1, 2)
    E(0x3E26, 2, 's16', 'pbem_goods_flag', 'pbem_goods_flag')
    # 0x3E28 pbem_trade_flag u8, link 0xF408C
    F(0x3E28, 1, '0xF408C', 'pbem_trade_flag', 1, 1)
    E(0x3E28, 1, 'u8', 'pbem_trade_flag', 'pbem_trade_flag')
    # 0x3E29 another_trade[n], 12 x 1, link 0xF09D4 (1 = n wants another session)
    F(0x3E29, 12, '0xF09D4', 'another_trade', 12, 1)
    for n in range(12):
        E(0x3E29 + n, 1, 'u8', nat_idx('another_trade', n), 'another_trade')
    # 0x3E35 pbem_data_invalid s8, link 0xF40FE
    F(0x3E35, 1, '0xF40FE', 'pbem_data_invalid', 1, 1)
    E(0x3E35, 1, 's8', 'pbem_data_invalid', 'pbem_data_invalid')
    # 0x3E36 discard[s], 9 x 25, link 0xF07B4 (card-stack record, a and b unused)
    F(0x3E36, 9 * 25, '0xF07B4', 'discard', 9, 25)
    for s in range(9):
        rec_elems(0x3E36 + 25 * s, 'discard', f'discard {s}', stack_subs())
    # 0x3F17 discard_pbem[s], 9 x 25, link 0xF0898
    F(0x3F17, 9 * 25, '0xF0898', 'discard_pbem', 9, 25)
    for s in range(9):
        rec_elems(0x3F17 + 25 * s, 'discard_pbem', f'discard_pbem {s}', stack_subs())
    # 0x3FF8 calamity_resolved s16, link 0xF4094
    F(0x3FF8, 2, '0xF4094', 'calamity_resolved', 1, 2)
    E(0x3FF8, 2, 's16', 'calamity_resolved', 'calamity_resolved')
    # 0x3FFA discard_pbem_flag s16, link 0xF4098
    F(0x3FFA, 2, '0xF4098', 'discard_pbem_flag', 1, 2)
    E(0x3FFA, 2, 's16', 'discard_pbem_flag', 'discard_pbem_flag')
    # 0x3FFC grain_vs_famine[n], 10 x 2, link 0xF409A
    F(0x3FFC, 20, '0xF409A', 'grain_vs_famine', 10, 2)
    for n in range(10):
        E(0x3FFC + 2 * n, 2, 's16', nat_idx('grain_vs_famine', n), 'grain_vs_famine')
    # 0x4010 score[n][round], 10 x 45 x 2, link 0xEE9F0; element (n, k) at +90n + 2k
    F(0x4010, 10 * 45 * 2, '0xEE9F0', 'score', 10 * 45, 2)
    for n in range(10):
        for k in range(45):
            E(0x4010 + 90 * n + 2 * k, 2, 's16', nat_idx('score', n, f'[{k}]'), 'score')
    # 0x4394 trade_log, 417 bytes, link 0x110F74
    F(0x4394, 417, '0x110F74', 'trade_log', 417, 1)
    rec_elems(0x4394, 'trade_log', 'trade_log', trade_log_subs())
    # 0x4535 version_stamp, text "1.11B1" (one element), link 0xBC8E4
    F(0x4535, 11, '0xBC8E4', 'version_stamp', 1, 11)
    E(0x4535, 11, 'text', 'version_stamp', 'version_stamp')
    # 0x4540 cfg_word u16 hex, link 0xF0982
    F(0x4540, 2, '0xF0982', 'cfg_word', 1, 2)
    E(0x4540, 2, 'u16', 'cfg_word', 'cfg_word', True)
    # 0x4542 bought_from_stack[n], 10 x 4, link 0xF590C (s32, -1 none)
    F(0x4542, 40, '0xF590C', 'bought_from_stack', 10, 4)
    for n in range(10):
        E(0x4542 + 4 * n, 4, 's32', nat_idx('bought_from_stack', n), 'bought_from_stack')
    # 0x456A unk_1105d4[n], 10 x 4, link 0x1105D4 (?)
    F(0x456A, 40, '0x1105D4', 'unk_1105d4', 10, 4)
    for n in range(10):
        E(0x456A + 4 * n, 4, 's32', nat_idx('unk_1105d4', n), 'unk_1105d4', True)
    # 0x4592 trade_want_count[n], 10 x 4, link 0x1105AC
    F(0x4592, 40, '0x1105AC', 'trade_want_count', 10, 4)
    for n in range(10):
        E(0x4592 + 4 * n, 4, 's32', nat_idx('trade_want_count', n), 'trade_want_count')
    # 0x45BA players_at_start s32, link 0xF40F0
    F(0x45BA, 4, '0xF40F0', 'players_at_start', 1, 4)
    E(0x45BA, 4, 's32', 'players_at_start', 'players_at_start')
    # 0x45BE bpm_flags[n][t], 10 x 160, link 0xF0044 (u8 hex)
    F(0x45BE, 10 * 160, '0xF0044', 'bpm_flags', 10 * 160, 1)
    for n in range(10):
        for t in range(160):
            E(0x45BE + 160 * n + t, 1, 'u8', nat_idx('bpm_flags', n, f'[{t}]'), 'bpm_flags', True)
    # 0x4BFE bpm_priority[n][t], 10 x 160, link 0xEED84 (s8)
    F(0x4BFE, 10 * 160, '0xEED84', 'bpm_priority', 10 * 160, 1)
    for n in range(10):
        for t in range(160):
            E(0x4BFE + 160 * n + t, 1, 's8', nat_idx('bpm_priority', n, f'[{t}]'), 'bpm_priority')
    # 0x523E token_stock s32, link 0xF4898
    F(0x523E, 4, '0xF4898', 'token_stock', 1, 4)
    E(0x523E, 4, 's32', 'token_stock', 'token_stock')
    # 0x5242 pbem_file, text (one element), link 0xF5230
    F(0x5242, 13, '0xF5230', 'pbem_file', 1, 13)
    E(0x5242, 13, 'text', 'pbem_file', 'pbem_file')
    # 0x524F response[i], 7 x 20, link 0x111498
    F(0x524F, 7 * 20, '0x111498', 'response', 7, 20)
    for i in range(7):
        rec_elems(0x524F + 20 * i, 'response', f'response {i}', response_subs())
    # 0x52DB response_count s16, link 0x112568
    F(0x52DB, 2, '0x112568', 'response_count', 1, 2)
    E(0x52DB, 2, 's16', 'response_count', 'response_count')
    # 0x52DD accept_mode s16, link 0xF4100
    F(0x52DD, 2, '0xF4100', 'accept_mode', 1, 2)
    E(0x52DD, 2, 's16', 'accept_mode', 'accept_mode')
    # 0x52DF pbem_trade_detail[i], 9 x 18, link 0xF47D4 (G); 18 elements [i][k]
    F(0x52DF, 9 * 18, '0xF47D4', 'pbem_trade_detail', 9 * 18, 1)
    for i in range(9):
        for k in range(18):
            E(0x52DF + 18 * i + k, 1, 'u8', f'pbem_trade_detail[{i}][{k}]', 'pbem_trade_detail')
    # 0x5381 pbem_detail_flag[i], 9 x 1, link 0xF4878 (G)
    F(0x5381, 9, '0xF4878', 'pbem_detail_flag', 9, 1)
    for i in range(9):
        E(0x5381 + i, 1, 's8', f'pbem_detail_flag[{i}]', 'pbem_detail_flag')
    # 0x538A nation_pbem[n], 14 x 199, link 0xF2A94 (G: PBEM pre-trade copies)
    F(0x538A, 14 * 199, '0xF2A94', 'nation_pbem', 14, 199)
    for n in range(14):
        rec_elems(0x538A + 199 * n, 'nation_pbem', nat_rec('nation_pbem', n), nation_subs())
    # 0x5E6C met[i][j], 9 x 9, link 0x109E24 (nation i+1 met j+1 in a conflict)
    F(0x5E6C, 81, '0x109E24', 'met', 81, 1)
    for i in range(9):
        for j in range(9):
            E(0x5E6C + 9 * i + j, 1, 'u8', f'met[{i}][{j}]', 'met')
    # 0x5EBD surplus_removed[n], 12 x 1, link 0xF5934
    F(0x5EBD, 12, '0xF5934', 'surplus_removed', 12, 1)
    for n in range(12):
        E(0x5EBD + n, 1, 'u8', nat_idx('surplus_removed', n), 'surplus_removed')
    # 0x5EC9 nation_replay[n], 14 x 199, link 0xF1FB0 (REPLAY snapshot after movement)
    F(0x5EC9, 14 * 199, '0xF1FB0', 'nation_replay', 14, 199)
    for n in range(14):
        rec_elems(0x5EC9 + 199 * n, 'nation_replay', nat_rec('nation_replay', n), nation_subs())
    # 0x69AB area_replay[r], 161 x 69, link 0xE5D20
    F(0x69AB, 161 * 69, '0xE5D20', 'area_replay', 161, 69)
    for r in range(161):
        rec_elems(0x69AB + 69 * r, 'area_replay', area_rec('area_replay', r), area_subs())
    # 0x9510 tstate_replay[t], 160 x 3, link 0xE2DF8 (as tstate)
    F(0x9510, 160 * 3, '0xE2DF8', 'tstate_replay', 160, 3)
    for t in range(160):
        rec_elems(0x9510 + 3 * t, 'tstate_replay', f'tstate_replay {t}', tstate_subs())
    # 0x96F0 replay_valid s32, link 0xEDF58
    F(0x96F0, 4, '0xEDF58', 'replay_valid', 1, 4)
    E(0x96F0, 4, 's32', 'replay_valid', 'replay_valid')
    # 0x96F4 replay_round s32, link 0xEDF5C
    F(0x96F4, 4, '0xEDF5C', 'replay_round', 1, 4)
    E(0x96F4, 4, 's32', 'replay_round', 'replay_round')
    # 0x96F8 random_table[i][j], 30 x 10 x 2, link 0x112A94; element at +20i + 2j
    F(0x96F8, 30 * 10 * 2, '0x112A94', 'random_table', 300, 2)
    for i in range(30):
        for j in range(10):
            E(0x96F8 + 20 * i + 2 * j, 2, 's16', f'random_table[{i}][{j}]', 'random_table')
    # 0x9950 bpm_third[n][t], 10 x 160, link 0xEFA04 (? meaning)
    F(0x9950, 10 * 160, '0xEFA04', 'bpm_third', 10 * 160, 1)
    for n in range(10):
        for t in range(160):
            E(0x9950 + 160 * n + t, 1, 'u8', nat_idx('bpm_third', n, f'[{t}]'), 'bpm_third', True)
    # 0x9F90 pbem_buy[n], 10 x 2, link 0xF0684
    F(0x9F90, 20, '0xF0684', 'pbem_buy', 10, 2)
    for n in range(10):
        E(0x9F90 + 2 * n, 2, 's16', nat_idx('pbem_buy', n), 'pbem_buy')
    # 0x9FA4 adv_buy_count[n], 10 x 2, link 0xF6E76
    F(0x9FA4, 20, '0xF6E76', 'adv_buy_count', 10, 2)
    for n in range(10):
        E(0x9FA4 + 2 * n, 2, 's16', nat_idx('adv_buy_count', n), 'adv_buy_count')
    # 0x9FB8 adv_bought[n][k], 10 x 5 x 2, link 0xF6E12 (s16, -1 none)
    F(0x9FB8, 10 * 5 * 2, '0xF6E12', 'adv_bought', 10 * 5, 2)
    for n in range(10):
        for k in range(5):
            E(0x9FB8 + 10 * n + 2 * k, 2, 's16', nat_idx('adv_bought', n, f'[{k}]'), 'adv_bought')
    # 0xA01C adv_cards_spent[n][c], 10 x 19, link 0xF6D54 (s8)
    F(0xA01C, 10 * 19, '0xF6D54', 'adv_cards_spent', 10 * 19, 1)
    for n in range(10):
        for c in range(19):
            E(0xA01C + 19 * n + c, 1, 's8', nat_idx('adv_cards_spent', n, f'[{c}]'), 'adv_cards_spent')
    # 0xA0DA pop_short[n], 10 x 1, link 0xF488C (G)
    F(0xA0DA, 10, '0xF488C', 'pop_short', 10, 1)
    for n in range(10):
        E(0xA0DA + n, 1, 'u8', nat_idx('pop_short', n), 'pop_short')
    # 0xA0E4 ai_territory[n-1][t], 9 x 157 x 2, link 0xF3580 (u16 hex); element at +314(n-1) + 2t
    F(0xA0E4, 9 * 157 * 2, '0xF3580', 'ai_territory', 9 * 157, 2)
    for n in range(1, 10):
        for t in range(157):
            E(0xA0E4 + 314 * (n - 1) + 2 * t, 2, 'u16', nat_idx('ai_territory', n, f'[{t}]'), 'ai_territory', True)
    # 0xABEE trade_session_held s32, link 0xF4230
    F(0xABEE, 4, '0xF4230', 'trade_session_held', 1, 4)
    E(0xABEE, 4, 's32', 'trade_session_held', 'trade_session_held')
    # 0xABF2 unk_1106b0[n], 10 x 4, link 0x1106B0 (?)
    F(0xABF2, 40, '0x1106B0', 'unk_1106b0', 10, 4)
    for n in range(10):
        E(0xABF2 + 4 * n, 4, 's32', nat_idx('unk_1106b0', n), 'unk_1106b0', True)
    # 0xAC1A crc u32 (an end-game file: the first CRC here, the second at 0xAC1E) - never listed as a difference
    F(0xAC1A, 4, '-', 'crc', 1, 4, True)


build()

# ---- coverage: the fields tile the whole file, and every byte of [0, 0xAC1A) belongs to exactly one element --

OFF2ELEM = [None] * CRC_OFF
ELEMS_BY_FIELD = {}


def check_coverage():
    pos = 0
    for (off, size, link, name, count, esize, is_crc) in FIELDS:
        assert off == pos, f'field {name}: offset {off:#x}, expected {pos:#x}'
        pos += size
    assert pos == CRC_OFF + 4, f'fields end at {pos:#x}, expected {CRC_OFF + 4:#x}'
    for idx, (off, size, typ, label, field, hexed) in enumerate(ELEMS):
        for i in range(size):
            o = off + i
            assert 0 <= o < CRC_OFF, f'element {label} outside the compared region'
            assert OFF2ELEM[o] is None, f'byte {o:#x} in two elements: {ELEMS[OFF2ELEM[o]][3]} and {label}'
            OFF2ELEM[o] = idx
        ELEMS_BY_FIELD.setdefault(field, []).append(idx)
    for (off, size, link, name, count, esize, is_crc) in FIELDS:
        if is_crc:
            continue
        idxs = ELEMS_BY_FIELD[name]
        p = off
        for idx in idxs:
            eoff, esize2 = ELEMS[idx][0], ELEMS[idx][1]
            assert eoff == p, f'field {name}: element at {eoff:#x}, expected {p:#x}'
            p += esize2
        assert p == off + size, f'field {name}: elements end at {p:#x}, field ends at {off + size:#x}'


check_coverage()


def fmt_val(typ, raw, hexed):
    if typ == 'text':
        return repr(raw.rstrip(b'\0'))            # trailing NULs removed; a change after a first NUL still shows
    if typ == 'phase':
        v = raw[0]
        return f'{v} {PHASES[v]}' if v < len(PHASES) else f'{v} ?'
    v = struct.unpack('<' + FMT[typ], raw)[0]
    if hexed:
        w = WIDTH[typ]
        return f'0x{v & ((1 << (8 * w)) - 1):0{w}X}'
    return str(v)


def diff_lines(a, b, ignore=()):
    """One line per differing element of the compared regions (both CRC_OFF bytes), in file order."""
    assert len(a) == len(b) == CRC_OFF
    x = int.from_bytes(a, 'little') ^ int.from_bytes(b, 'little')
    if x == 0:
        return []
    seen = set()
    out = []
    while x:
        lsb = x & -x
        o = (lsb.bit_length() - 1) >> 3      # byte index of the lowest differing bit
        x ^= lsb
        e = OFF2ELEM[o]
        if e in seen:
            continue
        seen.add(e)
        off, size, typ, label, field, hexed = ELEMS[e]
        if field in ignore:
            continue
        out.append(f'0x{off:04X}  {label}  {fmt_val(typ, a[off:off + size], hexed)} -> '
                   f'{fmt_val(typ, b[off:off + size], hexed)}')
    return out


def refuse(msg):
    print(f'savediff: {msg}', file=sys.stderr)
    sys.exit(2)


def load(path):
    try:
        d = open(path, 'rb').read()
    except OSError as e:
        refuse(str(e))
    if len(d) not in (SIZE, SIZE_END):
        refuse(f'{path}: {len(d)} bytes, expected {SIZE} or {SIZE_END}')
    ver = struct.unpack_from('<H', d, 0)[0]
    if ver != VERSION:
        refuse(f'{path}: version {ver:#06x}, expected {VERSION:#06x}')
    if d[SETUP_FLAGS] & 0x01:
        refuse(f'{path}: PBEM bit set (setup +4 = {d[SETUP_FLAGS]:#04x}); PBEM saves are refused')
    stored = struct.unpack_from('<I', d, len(d) - 4)[0]
    ok = crc32(d[BODY_OFF:-4]) == stored
    return d, ok


def header(path, d, ok):
    desc = d[DESC_OFF:DESC_OFF + DESC_LEN].split(b'\0')[0].decode('latin-1')
    ph = d[PHASE_OFF]
    phname = PHASES[ph] if ph < len(PHASES) else '?'
    rnd = struct.unpack_from('<h', d, ROUND_OFF)[0]
    print(f"{path}: {len(d)} bytes, desc '{desc}', phase {ph} {phname}, round {rnd}, CRC {'ok' if ok else 'bad'}")


def print_layout(path):
    d = None
    if path:
        d, ok = load(path)
        header(path, d, ok)
    for (off, size, link, name, count, esize, is_crc) in FIELDS:
        print(f'0x{off:04X}  {size:6d} bytes  link {link}  {name}  {esize} x {count}')
        if d is not None and not is_crc:
            for idx in ELEMS_BY_FIELD[name]:
                eoff, esize2, typ, label, field, hexed = ELEMS[idx]
                print(f'    0x{eoff:04X}  {label} = {fmt_val(typ, d[eoff:eoff + esize2], hexed)}')


def main():
    ap = argparse.ArgumentParser(description='name each differing element of two version-0x38 save files')
    ap.add_argument('files', nargs='*', metavar='FILE')
    ap.add_argument('--ignore', action='append', default=[], metavar='NAME', help='drop a whole top-level field')
    ap.add_argument('--max', type=int, default=None, metavar='N', help='stop after N difference lines')
    ap.add_argument('--layout', nargs='?', const='', default=None, metavar='[FILE]',
                    help='print the field table (with FILE, also each element\'s value)')
    a = ap.parse_args()
    if a.layout is not None:
        if a.files:
            ap.error('--layout takes no positional files')
        print_layout(a.layout or None)
        return
    if len(a.files) != 2:
        ap.error('need exactly two files (or --layout [FILE])')
    names = {f[3] for f in FIELDS}
    for ig in a.ignore:
        if ig not in names:
            refuse(f'--ignore: no top-level field named {ig!r}')
    da, oka = load(a.files[0])
    db, okb = load(a.files[1])
    header(a.files[0], da, oka)
    header(a.files[1], db, okb)
    if len(da) == SIZE_END or len(db) == SIZE_END:
        print('end-game file: first CRC at 0xAC1A')
    lines = diff_lines(da[:CRC_OFF], db[:CRC_OFF], set(a.ignore))
    if a.max is not None and len(lines) > a.max:
        for l in lines[:a.max]:
            print(l)
        print(f'... {len(lines) - a.max} more differences')
    else:
        for l in lines:
            print(l)
    sys.exit(1 if lines else 0)


if __name__ == '__main__':
    main()
