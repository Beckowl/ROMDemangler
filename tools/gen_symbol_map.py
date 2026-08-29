#!/usr/bin/env python3

import json
import re
import sys

HEX_RE = re.compile(r"^[+-]?(0[xX])?[0-9a-fA-F]+$")

map_path = sys.argv[1]
out_path = sys.argv[2] if len(sys.argv) > 2 else "symbolMap.json"

symbols = {}
with open(map_path, "r", encoding="utf-8", errors="replace") as f:
    for line in f:
        tokens = line.split()
        if len(tokens) != 2:
            continue

        label = tokens[-1]
        for token in tokens:
            if not HEX_RE.match(token):
                continue
            addr = int(token, 16) & 0xFFFFFFFF
            symbols[f"0x{addr:08x}"] = label

symbols = dict(sorted(symbols.items(), key=lambda kv: int(kv[0], 16)))

with open(out_path, "w", encoding="utf-8") as f:
    json.dump(symbols, f, indent="\t")
    f.write("\n")

print(f"Wrote {len(symbols)} symbols to {out_path}")