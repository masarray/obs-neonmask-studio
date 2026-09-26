# SPDX-License-Identifier: GPL-2.0-or-later
"""Static shader/host parameter contract; not a GPU compiler or OBS runtime test."""
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]
HOST = (ROOT / "src" / "neonmask-filter.c").read_text(encoding="utf-8")
SHADER = (ROOT / "shaders" / "neon-mask.effect").read_text(encoding="utf-8")
NAMES = re.findall(r'NM_PARAM\([^,]+,\s*"([a-z_]+)"\)', HOST)
assert len(NAMES) == 16, f"Expected 16 bound uniforms, got {NAMES!r}"
for name in NAMES:
    assert re.search(r"\buniform\s+(?:float\d?|int)\s+" + re.escape(name) + r"\s*;", SHADER), name
assert SHADER.count("technique Draw") == 1
assert SHADER.count("float4 drawNeonMask(") == 1
assert SHADER.count("{") == SHADER.count("}")
print("PASS: 16 host/shader uniforms, Draw technique and balanced shader blocks")
