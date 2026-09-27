# SPDX-License-Identifier: GPL-2.0-or-later
"""Cheap structural regressions, not a substitute for libobs shader compilation."""
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]
host = (ROOT / "src" / "neonmask-filter.c").read_text(encoding="utf-8")
shader = (ROOT / "shaders" / "neon-mask.effect").read_text(encoding="utf-8")
names = re.findall(r'NM_PARAM\([^,]+,\s*"([a-z_]+)"\)', host)
assert len(names) == len(set(names)) == 33, f"Expected 33 unique bindings: {names!r}"
uniforms = {name: kind for kind, name in re.findall(r"\buniform\s+(float\d?|int|texture2d|float4x4)\s+([A-Za-z_][A-Za-z0-9_]*)\s*;", shader)}
assert len(uniforms) == 35, f"Unexpected number of shader uniforms: {uniforms}"
assert set(names) == set(uniforms) - {"ViewProj", "image"}, (
    "Host/shader uniform drift: " + str(set(names) ^ (set(uniforms) - {"ViewProj", "image"}))
)
assert uniforms["color_a"] == uniforms["color_b"] == "float4"
assert uniforms["uv_size"] == uniforms["half_size"] == "float2"
assert uniforms["mask_offset"] == uniforms["subject_pan"] == "float2"
assert uniforms["subject_zoom"] == uniforms["shape_rotation"] == "float"
assert uniforms["polygon_sides"] == "int"
assert uniforms["shape_detail"] == "float"
assert "NM_SHAPE_CHAT_BUBBLE" in host and "NM_SHAPE_ANGLED_CARD" in host
assert "NM_SHAPE_HUD_PANEL" in host and "NM_SHAPE_SQUIRCLE" in host
assert "NM_SHAPE_SVG_PATH" in host and "svgPathDistance(" in shader
assert "nm_svg_read_local(" in host and "nm_svg_raster_sdf(" in host
assert "shape_id == 12 && svg_ready == 0" in shader
assert "outside>0.0 ? outside+max(d,0.0) : d" in shader
assert "return d+length(max(abs(q)-half_size,0.0))" not in shader
assert "gs_effect_set_texture(f->svg_sdf" in host
assert "NM_PARAM(svg_sdf, \"svg_sdf\")" in host
assert "NM_PARAM(svg_ready, \"svg_ready\")" in host
assert "obs_properties_add_path(mask_group, \"svg_path\"" in host
assert "obs_enter_graphics();" in host and "gs_texture_destroy(f->svg_texture)" in host
assert "hudPanelDistance(" in shader and "squircleDistance(" in shader
assert "return sqrt(ds)*(inside ? -1.0 : 1.0)" in shader
assert "Preset.HUDCut" in host and "Preset.Squircle" in host
assert "nm_geometry_visibility(" in host
assert "obs_property_set_visible(round,rounded)" in host
assert "obs_property_set_visible(detail,authored)" in host
assert "obs_property_set_visible(sides,shape_id==NM_SHAPE_POLYGON)" in host
assert "float2 topA=float2(-half_size.x+cut*0.38" in shader
assert "artBarA=saturate(artBarA+trace*art_intensity*0.88)" in shader
assert "float2 v10=float2(-b.x,b.y-cut*0.70)" in shader
# The same saved Roundness slider affects both Bubble tail and Angled Card
# cut-end fillets. Reject the old UI text claiming it is Angled-only.
en_locale = (ROOT / "data" / "locale" / "en-US.ini").read_text(encoding="utf-8")
id_locale = (ROOT / "data" / "locale" / "id-ID.ini").read_text(encoding="utf-8")
assert 'Roundness="Contour roundness"' in en_locale
assert 'Roundness="Kelengkungan kontur"' in id_locale
assert 'Mask.Detail="Shape detail (tail / cut / squircle)"' in en_locale
assert 'Mask.Detail="Detail bentuk (ekor / potongan / squircle)"' in id_locale
assert "return chatBubbleDistance(" in shader and "return angledCardDistance(" in shader
# A primitive min is an occupancy union, NOT the exposed contour distance:
# its hidden body bottom/tail base caused a phantom horizontal neon seam.
assert "float ds = edgeDistanceSquared(p,topLeft,topRight)" in shader
assert "edgeDistanceSquared(p,bottomRight,right)" in shader
assert "edgeDistanceSquared(p,left,bottomLeft)" in shader
assert "arcDistanceSquared(p,rightTangent,leftTangent,tipCenter,tipR)" in shader
assert "return sqrt(ds)*(inside && !removed" in shader
# Maximum Angled Card roundness forms a shared-center quarter circle.
assert "float r = amount*cut*(1.0+INV_ROOT2)" in shader
assert "cut*1.20" not in shader
assert "return min(body,wedge)" not in shader
assert 'obs_data_has_user_value(settings, "shape_detail")' in host
assert 'gs_effect_set_float(f->shape_detail, f->config.shape_detail)' in host
assert uniforms["style_id"] == uniforms["ornament_mode"] == "int"
assert uniforms["art_intensity"] == uniforms["art_gap"] == "float"
assert "roundedContourTurn" in shader
assert "float gapGate" in shader and "float hbar" in shader and "float vbar" in shader
assert "artBarA" in shader and "artBarGlowA" in shader
assert "NM_ORNAMENT_CYBER" in host and "NM_ORNAMENT_NONE" in host
assert "NM_ORNAMENT_REACTOR" in host and "NM_ORNAMENT_TECH_HUD" in host and "NM_ORNAMENT_STREAMER" in host
for mode in (2, 3, 4):
    assert f"ornament_mode == {mode}" in shader
assert "float segmentDistance(" in shader
for key in ("art_intensity", "art_gap"):
    assert f'obs_data_has_user_value(settings, "{key}")' in host
    assert f'obs_data_set_double(settings, "{key}", next.{key})' in host
assert 'obs_data_has_user_value(settings, "ornament_mode")' in host
assert 'obs_data_set_int(settings, "ornament_mode", next.ornament_mode)' in host
for key in ("mid_glow_strength", "bloom_strength", "hotspot_strength", "hotspot_size"):
    assert uniforms[key] == "float", f"Missing premium light uniform {key}"
    assert f'NM_PARAM({key}, "{key}")' in host
    assert f'gs_effect_set_float(f->{key}, f->config.{key})' in host
assert "float midA =" in shader and "float outerA =" in shader
assert "float fineA =" in shader and "float hotspotA =" in shader
assert "float breath =" in shader and "animation_id == 1" in shader
assert "float turnDelta =" in shader and "animation_id == 2" in shader
assert "premul = outerColor * outerA" in shader
assert "premul = midColor * midA" in shader
assert "premul = fineColor * fineA" in shader
assert "premul = hotspotColor * hotspotA" in shader
for key in ("mid_glow_strength", "bloom_strength", "hotspot_strength", "hotspot_size"):
    assert f'obs_data_has_user_value(settings, "{key}")' in host
    assert f'obs_data_set_double(settings, "{key}", next.{key})' in host
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
# No shader / zero-size rendering must fail closed, not display the webcam.
render = host.split("static void nm_render(", 1)[1].split("struct obs_source_info neonmask_filter_info", 1)[0]
assert "if (!target || !f->effect)" in render
assert "obs_source_skip_video_filter(" not in render, "Unmasked bypass in render callback"
assert "filter will render transparent" in host

assert "nm_custom_changed" in host, "User edits should reset preset status to Custom"
assert "sourcePos - mask_offset" in shader
assert "sourcePos - subject_pan" in shader and "/ max(subject_zoom" in shader
assert "insideSource" in shader and "float4(0.0, 0.0, 0.0, 0.0)" in shader
assert "NM_SHAPE_RECTANGLE" in host and "NM_SHAPE_TRIANGLE" in host and "NM_SHAPE_POLYGON" in host
print("PASS: shader bindings, alpha/color contracts, dimensions and fallback guards")
