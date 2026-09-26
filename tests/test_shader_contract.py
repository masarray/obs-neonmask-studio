# SPDX-License-Identifier: GPL-2.0-or-later
"""Cheap structural regressions, not a substitute for libobs shader compilation."""
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]
host = (ROOT / "src" / "neonmask-filter.c").read_text(encoding="utf-8")
shader = (ROOT / "shaders" / "neon-mask.effect").read_text(encoding="utf-8")
names = re.findall(r'NM_PARAM\([^,]+,\s*"([a-z_]+)"\)', host)
assert len(names) == len(set(names)) == 23, f"Expected 23 unique bindings: {names!r}"
uniforms = {name: kind for kind, name in re.findall(r"\buniform\s+(float\d?|int|texture2d|float4x4)\s+([A-Za-z_][A-Za-z0-9_]*)\s*;", shader)}
assert len(uniforms) == 25, f"Unexpected number of shader uniforms: {uniforms}"
assert set(names) == set(uniforms) - {"ViewProj", "image"}, (
    "Host/shader uniform drift: " + str(set(names) ^ (set(uniforms) - {"ViewProj", "image"}))
)
assert uniforms["color_a"] == uniforms["color_b"] == "float4"
assert uniforms["uv_size"] == uniforms["half_size"] == uniforms["mask_offset"] == uniforms["subject_pan"] == "float2"
assert uniforms["subject_zoom"] == uniforms["polygon_rotation"] == "float"
assert uniforms["polygon_sides"] == "int"
assert "float2 srcUV" in shader and "srcUV.x >= 0.0" in shader
assert "nm_make_geometry_framed" in host and "nm_reset_framing" in host
assert "obs_properties_add_group" in host
assert uniforms["style_id"] == "int"
assert uniforms["color_phase"] == uniforms["pulse_phase"] == uniforms["flow_phase"] == "float"
assert "elapsed_time" not in uniforms and "animation_speed" not in uniforms
assert "fmodf(f->time" not in host
assert "nm_custom_changed" in host
assert shader.count("technique Draw") == 1
assert shader.count("float4 drawNeonMask(") == 1
assert "VertData mainTransform(VertData v_in)" in shader
assert "float4 drawNeonMask(VertData v_in)" in shader
assert "vertex_shader = mainTransform(v_in)" in shader
assert "pixel_shader = drawNeonMask(v_in)" in shader
assert shader.count("{") == shader.count("}")
assert "OBS_NO_DIRECT_RENDERING" in host, "Premultiplied input contract requires capture"
assert "OBS_ALLOW_DIRECT_RENDERING" not in host
assert "vec4_from_rgba_srgb" in host and "gs_effect_set_vec4" in host
assert not re.search(r"\bgs_effect_set_color\s*\(", host), "OBS color property data is RGBA, not BGRA"
assert "src.rgb * mask" in shader
assert "src.rgb * baseA" not in shader, "Would double-multiply alpha at edges"
assert "obs_source_get_base_width" in host and "obs_source_get_base_height" in host
assert "if (!valid)" in host, "Missing shader parameters must disable effect gracefully"
assert "nm_custom_changed" in host, "User edits should reset preset status to Custom"
print("PASS: shader bindings, alpha/color contracts, dimensions and fallback guards")
