#!/usr/bin/env python3
"""Nescartdb XML -> JSON converter for Breaknes.

Converts the tepples NesCartDB export (XML) into the JSON files consumed by the
emulator (see Nescartdb/Readme.md for the schema):

  - nescarts.json  - the full database (games -> cartridges -> boards)
  - index.json     - flat lookup index: prg_crc/chr_crc -> board type

Deterministic: the same XML always produces the same JSON.

Usage:
  python convert.py --input "NesCarts (2017-08-21).utf8.xml" --output-dir Nescartdb

Notes:
  - sha1 attributes are intentionally omitted (identification is CRC-only).
  - sizes like "256k" are normalized to bytes.
"""

import argparse
import json
import sys
import xml.etree.ElementTree as ET
from datetime import datetime, timezone


def parse_size(value):
    """'256k' -> 262144, '8k' -> 8192, '512' -> 512."""
    if value is None:
        return None
    s = str(value).strip().lower()
    if not s:
        return None
    if s.endswith('k'):
        return int(s[:-1]) * 1024
    if s.endswith('m'):
        return int(s[:-1]) * 1024 * 1024
    return int(s)


def parse_bool(value):
    """'1'/'true' -> True, '0'/'false' -> False, None -> None."""
    if value is None:
        return None
    s = str(value).strip().lower()
    if s in ('1', 'true', 'yes'):
        return True
    if s in ('0', 'false', 'no'):
        return False
    return None


def parse_int(value):
    if value is None:
        return None
    try:
        return int(str(value).strip())
    except ValueError:
        return None


def convert_game(game_el):
    game = {}
    for attr in ('name', 'altname', 'class', 'subclass', 'catalog', 'publisher',
                 'developer', 'portdeveloper', 'region', 'date'):
        val = game_el.get(attr)
        if val is not None:
            game[attr] = val
    players = parse_int(game_el.get('players'))
    if players is not None:
        game['players'] = players

    cartridges = []
    for cart_el in game_el.findall('cartridge'):
        cartridges.append(convert_cartridge(cart_el))
    if cartridges:
        game['cartridges'] = cartridges

    return game


def convert_cartridge(cart_el):
    cart = {}
    for attr in ('system', 'revision', 'crc', 'dump', 'dumper', 'datedumped'):
        val = cart_el.get(attr)
        if val is not None:
            cart[attr] = val
    prototype = parse_bool(cart_el.get('prototype'))
    if prototype is not None:
        cart['prototype'] = prototype

    board_el = cart_el.find('board')
    if board_el is not None:
        cart['board'] = convert_board(board_el)

    return cart


def convert_mem(el, attr_name):
    """<prg>/<chr>: name, id, size (bytes), crc."""
    if el is None:
        return None
    mem = {}
    for attr in ('name', 'id', 'crc'):
        val = el.get(attr)
        if val is not None:
            mem[attr] = val
    size = parse_size(el.get('size'))
    if size is not None:
        mem['size'] = size
    return mem


def convert_board(board_el):
    board = {}
    for attr in ('type', 'pcb'):
        val = board_el.get(attr)
        if val is not None:
            board[attr] = val
    mapper = parse_int(board_el.get('mapper'))
    if mapper is not None:
        board['mapper'] = mapper

    prg = board_el.find('prg')
    chr = board_el.find('chr')
    wram = board_el.find('wram')
    vram = board_el.find('vram')

    if prg is not None:
        board['prg'] = convert_mem(prg, 'prg')
    if chr is not None:
        board['chr'] = convert_mem(chr, 'chr')

    if wram is not None:
        item = {}
        size = parse_size(wram.get('size'))
        if size is not None:
            item['size'] = size
        battery = parse_bool(wram.get('battery'))
        if battery is not None:
            item['battery'] = battery
        if wram.get('id') is not None:
            item['id'] = wram.get('id')
        if item:
            board['wram'] = item

    if vram is not None:
        item = {}
        size = parse_size(vram.get('size'))
        if size is not None:
            item['size'] = size
        if vram.get('id') is not None:
            item['id'] = vram.get('id')
        if item:
            board['vram'] = item

    chips = []
    for chip_el in board_el.findall('chip'):
        item = {}
        if chip_el.get('type') is not None:
            item['type'] = chip_el.get('type')
        battery = parse_bool(chip_el.get('battery'))
        if battery is not None:
            item['battery'] = battery
        if item:
            chips.append(item)
    if chips:
        board['chips'] = chips

    cic = []
    for cic_el in board_el.findall('cic'):
        if cic_el.get('type') is not None:
            cic.append({'type': cic_el.get('type')})
    if cic:
        board['cic'] = cic

    pad = board_el.find('pad')
    if pad is not None:
        pad_item = {}
        h = parse_int(pad.get('h'))
        v = parse_int(pad.get('v'))
        if h is not None:
            pad_item['h'] = h
        if v is not None:
            pad_item['v'] = v
        if pad_item:
            board['pad'] = pad_item

    peripherals = []
    for dev_el in board_el.findall('peripherals/device'):
        dev = {}
        if dev_el.get('name') is not None:
            dev['name'] = dev_el.get('name')
        if dev_el.get('type') is not None:
            dev['type'] = dev_el.get('type')
        pins = []
        for pin_el in dev_el.findall('pin'):
            pin = {}
            if pin_el.get('number') is not None:
                pin['number'] = pin_el.get('number')
            if pin_el.get('function') is not None:
                pin['function'] = pin_el.get('function')
            if pin:
                pins.append(pin)
        if pins:
            dev['pins'] = pins
        if dev:
            peripherals.append(dev)
    if peripherals:
        board['peripherals'] = peripherals

    return board


def build_index(games):
    index = []
    for game in games:
        for cart in game.get('cartridges', []):
            board = cart.get('board')
            if not board:
                continue
            prg = board.get('prg') or {}
            chr = board.get('chr') or {}
            prg_crc = prg.get('crc')
            chr_crc = chr.get('crc')
            if not prg_crc or not chr_crc:
                continue
            record = {
                'prg_crc': prg_crc,
                'chr_crc': chr_crc,
                'system': cart.get('system'),
                'type': board.get('type'),
                'pcb': board.get('pcb'),
            }
            if board.get('mapper') is not None:
                record['mapper'] = board['mapper']
            index.append(record)
    return index


def main():
    parser = argparse.ArgumentParser(description='Nescartdb XML -> JSON converter')
    parser.add_argument('--input', required=True, help='Path to the tepples NesCartDB XML export')
    parser.add_argument('--output-dir', required=True, help='Directory to write nescarts.json and index.json')
    args = parser.parse_args()

    with open(args.input, 'rb') as f:
        raw = f.read()

    # The export declares encoding="utf-16" but is often served as UTF-8 (with BOM).
    # Detect the real encoding from the BOM / content.
    if raw.startswith(b'\xff\xfe') or raw.startswith(b'\xfe\xff'):
        text = raw.decode('utf-16')
    else:
        text = raw.decode('utf-8-sig')

    root = ET.fromstring(text)

    games = [convert_game(g) for g in root.findall('game')]

    source = {
        'name': 'NesCartDB',
        'agent': root.get('agent'),
        'author': root.get('author'),
        'export': 'forum.nesdev.org - tepples/NesCarts (2017-08-21).utf8.xml',
        'converted_at': datetime.now(timezone.utc).strftime('%Y-%m-%dT%H:%M:%SZ'),
        'converter': 'Tools/NescartdbConvert/convert.py',
    }

    nescarts = {
        'source': source,
        'games': games,
    }

    index = build_index(games)

    with open(args.output_dir + '/nescarts.json', 'w', encoding='utf-8') as f:
        json.dump(nescarts, f, ensure_ascii=False, indent=1)
        f.write('\n')

    with open(args.output_dir + '/index.json', 'w', encoding='utf-8') as f:
        json.dump(index, f, ensure_ascii=False, indent=1)
        f.write('\n')

    print('Games: %d, cartridges in index: %d' % (len(games), len(index)))


if __name__ == '__main__':
    main()
