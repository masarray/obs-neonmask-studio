# SPDX-License-Identifier: GPL-2.0-or-later
"""Cheap structural regressions, not a substitute for libobs shader compilation."""
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]
host = (ROOT / "src" / "neonmask-filter.c").read_text(encoding="utf-8")
safe_fit = (ROOT / "src" / "neonmask-safe-fit.c").read_text(encoding="utf-8")
shader = (ROOT / "shaders" / "neon-mask.effect").read_text(encoding="utf-8")
names = re.findall(r'NM_PARAM\([^,]+,\s*"([a-z_]+)"\)', host)
assert len(names) == len(set(names)) == 49, f"Expected 49 unique bindings: {names!r}"
uniforms = {name: kind for kind, name in re.findall(r"\buniform\s+(float\d?|int|texture2d|float4x4)\s+([A-Za-z_][A-Za-z0-9_]*)\s*;", shader)}
assert len(uniforms) == 51, f"Unexpected number of shader uniforms: {uniforms}"
assert set(names) == set(uniforms) - {"ViewProj", "image"}, (
    "Host/shader uniform drift: " + str(set(names) ^ (set(uniforms) - {"ViewProj", "image"}))
)
assert uniforms["color_a"] == uniforms["color_b"] == "float4"
assert uniforms["color_mode"] == "int"
for key in ("rainbow_phase","rainbow_saturation","rainbow_hue_offset","rainbow_spread"):
    assert uniforms[key] == "float"
assert uniforms["uv_size"] == uniforms["half_size"] == "float2"
assert uniforms["output_size"] == uniforms["input_origin"] == "float2"
assert uniforms["bubble_top"] == uniforms["bubble_bottom"] == uniforms["bubble_tail"] == "float4"
assert uniforms["mask_offset"] == uniforms["subject_pan"] == "float2"
assert uniforms["subject_zoom"] == uniforms["shape_rotation"] == "float"
assert uniforms["polygon_sides"] == "int"
assert uniforms["shape_detail"] == "float"
assert "NM_SHAPE_CHAT_BUBBLE" in host and "NM_SHAPE_ANGLED_CARD" in host
assert "NM_SHAPE_HUD_PANEL" in host and "NM_SHAPE_SQUIRCLE" in host
assert "NM_SHAPE_TECH_HUD" in host and "NM_SHAPE_GAME_UI" in host
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
assert "techHudDistance(" in shader and "shape_id == 13" in shader
assert "gameUiDistance(" in shader and "shape_id == 14" in shader
assert "return sqrt(ds)*(inside ? -1.0 : 1.0)" in shader
assert "Preset.HUDCut" in host and "Preset.Squircle" in host
assert "nm_geometry_visibility(" in host
assert "obs_property_set_visible(round,rounded)" in host
assert "obs_property_set_visible(detail,authored)" in host
assert "shape_id==NM_SHAPE_GAME_UI ||" not in host.split("const bool authored=",1)[1].split(";",1)[0]
assert "obs_property_set_visible(sides,shape_id==NM_SHAPE_POLYGON)" in host
assert "float2 topA=float2(-half_size.x+cut*0.38" in shader
assert "artBarA=saturate(artBarA+trace*art_intensity*0.88)" in shader
assert "float2 v10=float2(-b.x,b.y-cut*0.70)" in shader
assert "float2 v0=float2(-b.x+cut,-b.y)" in shader
assert "float2 v3=float2( b.x-cut, b.y)" in shader
assert "ornament_mode == 3 && shape_id == 13" in shader
assert "bool authoredOverlay = (shape_id == 13 && ornament_mode == 3)" in shader
assert "float thickness=safeLCornerThickness(ornament_width,armX,armY)" in shader
assert "float2 trOuter=float2(hx+gap+thickness," in shader
assert "float2 blOuter=float2(-hx-gap-thickness," in shader
assert "q,trOuter,float2(-1.0,1.0),armX,armY,thickness" in shader
assert "q,blOuter,float2(1.0,-1.0),armX,armY,thickness" in shader
assert "Other corners get only thin diagonal technical accents, never Ls." in shader
assert "float2 tlCorner=" not in shader
assert "coreA*=0.84" in shader and "outerA*=0.52" in shader
assert "float innerD=min(min(segmentDistance(q,inTopA,inTopB)" not in shader
# The same saved Roundness slider affects both Bubble tail and Angled Card
# cut-end fillets. Reject the old UI text claiming it is Angled-only.
en_locale = (ROOT / "data" / "locale" / "en-US.ini").read_text(encoding="utf-8")
id_locale = (ROOT / "data" / "locale" / "id-ID.ini").read_text(encoding="utf-8")
assert 'Roundness="Contour roundness"' in en_locale
assert 'Roundness="Kelengkungan kontur"' in id_locale
assert 'Mask.Detail="Shape detail (cut / squircle)"' in en_locale
assert 'Mask.Detail="Detail bentuk (potongan / squircle)"' in id_locale
assert "return chatBubbleDistance(" in shader and "return angledCardDistance(" in shader
assert "float2 tl=float2(bubble_top.x*b.x,bubble_top.y*b.y);" in shader
assert "float2 br=float2(bubble_bottom.x*b.x,bubble_bottom.y*b.y);" in shader
assert "float startT=clamp(bubble_tail.x,railMin,railMax-minBase);" in shader
assert "float2 outward=normalize(float2(-bottomDir.y,bottomDir.x));" in shader
assert "bubbleTailMaxDepth(tipBase,outward,b)" in shader
assert "bubbleQuadInside(p,tl,tr,br,bl)>=0.0" in shader
assert "NM_PARAM(bubble_top, \"bubble_top\")" in host
assert "NM_PARAM(bubble_bottom, \"bubble_bottom\")" in host
assert "NM_PARAM(bubble_tail, \"bubble_tail\")" in host
assert "gs_effect_set_vec4(f->bubble_top, &bubble_top)" in host
assert "gs_effect_set_vec4(f->bubble_bottom, &bubble_bottom)" in host
assert "gs_effect_set_vec4(f->bubble_tail, &bubble_tail)" in host
for key in ("bubble_tl_x","bubble_tl_y","bubble_tr_x","bubble_tr_y",
            "bubble_br_x","bubble_br_y","bubble_bl_x","bubble_bl_y",
            "bubble_tail_start","bubble_tail_end","bubble_tail_tip_pos","bubble_tail_depth"):
    assert f'obs_data_set_default_double(settings, "{key}"' in host
    assert f'obs_data_set_double(settings, "{key}"' in host or key in ("bubble_tl_x","bubble_tl_y","bubble_tr_x","bubble_tr_y","bubble_br_x","bubble_br_y","bubble_bl_x","bubble_bl_y","bubble_tail_start","bubble_tail_end","bubble_tail_tip_pos")
assert "schema < 3u" in host and "bottom_y = 1.0f - 1.8f*depth" in host
assert '"bubble_tl_x","bubble_tl_y","bubble_tr_x","bubble_tr_y"' in host
assert "bubble_body_controls" in host and "bubble_tail_controls" in host
assert 'obs_property_set_visible(body,shape_id==NM_SHAPE_CHAT_BUBBLE)' in host
assert 'obs_property_set_visible(tail,shape_id==NM_SHAPE_CHAT_BUBBLE)' in host
assert "nm_context_visibility(" in host
assert 'obs_properties_add_text(art_group,"art_clip_hint"' in host
assert 'obs_property_set_visible(p,tech || game || ring)' in host
assert 'obs_property_set_visible(p,ornament!=NM_ORNAMENT_NONE && !ring)' in host
assert 'obs_properties_get(arts,"ring_outer_width_pct")' in host
assert 'obs_properties_get(arts,"ring_inner_width_pct")' in host
assert 'obs_properties_get(arts,"ring_inner_offset_pct")' in host
assert 'obs_properties_get(arts,"ring_spacing_pct")' in host
assert 'obs_property_set_visible(p,tech || game);' in host
assert 'obs_property_set_visible(p,game);' in host
assert "NM_SHOW(\"speed\",border && animation!=NM_ANIM_STATIC)" in host
assert "NM_SHOW(\"glow_radius\",border && glow)" in host
assert 'Bubble.TLX="Top-left X"' in en_locale
assert 'Shape.TechHUD="Tech HUD Advanced — cut corners + brackets"' in en_locale
assert 'Art.Width="Outer ornament width (px)"' in en_locale
assert 'Art.ClipHint="Outer ornaments are auto-fitted inside the source when expansion is off' in en_locale
assert 'may shrink the mask' in en_locale
assert 'Art.Gap="Outer ornament gap from frame (px)"' in en_locale
assert 'Art.RingInnerOffset="Inner orbit center distance from frame (px)"' in en_locale
assert 'Art.RingSpacing="Inner ↔ outer orbit center spacing (px)"' in en_locale
assert 'Art.RingGeometryHint="Dual Ring is mask-relative:' in en_locale
assert 'Border.Width="Base neon border width (px)"' in en_locale
assert 'Preset.TechHUD="Tech HUD Advanced — Cyan + Amber"' in en_locale
assert 'Shape.GameUI="Game UI — corner brackets + inner rail"' in en_locale
assert 'Preset.GameUI="Game UI — Neon Green"' in en_locale
assert 'Preset.GradientRainbow="Gradient Rainbow — Rotating Spectrum"' in en_locale
assert 'Preset.RotatingRing="Rotating Ring — Counter Orbit"' in en_locale
assert 'Art.DualRing="Dual Ring — counter-rotating 2/3 arcs"' in en_locale
assert 'Art.RingOuterWidthPct="Outer orbit width (% mask radius)"' in en_locale
assert 'Art.RingInnerWidthPct="Inner orbit width (% mask radius)"' in en_locale
assert 'Art.RingInnerOffsetPct="Frame → inner orbit center (% mask radius)"' in en_locale
assert 'Art.RingSpacingPct="Inner ↔ outer center spacing (% mask radius)"' in en_locale
assert 'Art.RingOuterWidthPct="Ketebalan orbit luar (% radius mask)"' in id_locale

assert 'Color.Mode.Rainbow="Rainbow gradient"' in en_locale
assert 'Rainbow.Spread="Gradient spread"' in en_locale
assert 'Color.Mode.Rainbow="Gradien pelangi"' in id_locale
assert 'Rainbow.Spread="Sebaran gradien"' in id_locale
assert 'Bubble.TLX="Sudut kiri-atas X"' in id_locale
assert 'Shape.TechHUD="Tech HUD Advanced — sudut potong + bracket"' in id_locale
assert 'Art.Width="Ketebalan ornamen luar (px)"' in id_locale
assert 'Art.ClipHint="Ornamen luar otomatis disesuaikan agar tetap masuk batas sumber saat ekspansi mati' in id_locale
assert 'dapat mengecilkan mask' in id_locale
# A primitive min is an occupancy union, NOT the exposed contour distance:
# its hidden body bottom/tail base caused a phantom horizontal neon seam.
assert "float ds=edgeDistanceSquared(p,tlN,trP)" in shader
assert "edgeDistanceSquared(p,brN,right)" in shader
assert "edgeDistanceSquared(p,left,blP)" in shader
assert "arcDistanceSquared(p,rightTangent,leftTangent,tipCenter,tipR)" in shader
assert "return sqrt(ds)*((bodyInside || tailInside) && !removed" in shader
# Maximum Angled Card roundness forms a shared-center quarter circle.
assert "float r = amount*cut*(1.0+INV_ROOT2)" in shader
assert "cut*1.20" not in shader
assert "return min(body,wedge)" not in shader
assert 'obs_data_has_user_value(settings, "shape_detail")' in host
assert 'gs_effect_set_float(f->shape_detail, f->config.shape_detail)' in host
assert uniforms["style_id"] == uniforms["ornament_mode"] == "int"
assert uniforms["art_intensity"] == uniforms["art_gap"] == "float"
for key in ("ornament_width","ornament_length_x","ornament_length_y",
            "inner_rail_width","ring_inner_offset","ring_spacing"):
    assert uniforms[key] == "float"
    assert f'NM_PARAM({key}, "{key}")' in host
assert "roundedContourTurn" in shader
assert "float3 hsvToRgb(" in shader and "float rainbowContourTurn(" in shader
assert "if(color_mode == 1)" in shader and "else if(color_mode == 2)" in shader
assert "contourTurn*rainbow_spread" in shader
assert "rainbow_hue_offset+rainbow_phase" in shader
assert "float3 outerColor = neon;" in shader
assert "float gapGate" in shader
assert "float connectedLCornerDistance(" in shader
assert "float safeLCornerThickness(" in shader
assert "float2 elbowCenter=outerCorner+" in shader
assert "float elbow=roundedBoxDistance(p-elbowCenter,float2(h,h),0.0)" in shader
assert "return min(elbow,min(hBar,vBar))" in shader
assert "artBarA" in shader and "artBarGlowA" in shader
assert "NM_ORNAMENT_CYBER" in host and "NM_ORNAMENT_NONE" in host
assert "NM_ORNAMENT_REACTOR" in host and "NM_ORNAMENT_TECH_HUD" in host and "NM_ORNAMENT_STREAMER" in host
assert "NM_ORNAMENT_GAME_UI" in host and "NM_ORNAMENT_DUAL_RING" in host
for mode in (2, 3, 4, 5, 6):
    assert f"ornament_mode == {mode}" in shader
assert "float segmentDistance(" in shader
assert "ornament_mode == 5 && shape_id == 14" in shader
assert "float minorHalf=max(0.25,inner_rail_width*0.5)" in shader
assert "float2 outerCorner=float2(hx+gap+thickness," in shader
assert "connectedLCornerDistance(" in shader
assert "float innerD=abs(d+innerGap)" in shader
assert "corners*0.99+innerRail*0.58" in shader
assert "ornament_mode == 6 && shape_id == 1" in shader
assert "ringAnglePoint=float2(1.0,0.0)" in shader
assert "dot(ringAnglePoint,ringAnglePoint)<0.0001" in shader
assert "outerRel=frac(t-phase+1.0)" in shader
assert "float innerStart=frac(0.50-phase+1.0)" in shader
assert "float innerRel=frac(t-innerStart+1.0)" in shader
assert "const float span=0.4400000000" in shader
assert "const float span=0.6666666667" not in shader
assert "const float tipFade=0.055" in shader
assert "float innerOffset=halfCore+ring_inner_offset" in shader
assert "float outerOffset=innerOffset+ring_spacing" in shader
assert "float locatorGap=max(5.0,art_gap*0.65)" not in shader
assert "float orbitGap=max(7.0,art_gap*0.90)" not in shader
assert "innerOffset+innerHalf+orbitGap+outerHalf" not in shader
assert "float outerD=abs(d-outerOffset)" in shader
assert "float innerD=abs(d-innerOffset)" in shader
assert "outerHalf*lerp(0.08,1.0,outerTaper)" in shader
assert "innerHalf*lerp(0.08,1.0,innerTaper)" in shader
assert "float dualOuterA = 0.0" in shader
assert "float dualInnerA = 0.0" in shader
assert "float dualOuterCoreA = 0.0" in shader
assert "float dualInnerCoreA = 0.0" in shader
assert "outerHalf*0.30" in shader and "innerHalf*0.28" in shader
assert "art_intensity*0.40*outerCore" in shader
assert "art_intensity*0.35*innerCore" in shader
assert "baseRadius*0.032*glowScale" in shader

assert "dualOuterColor=lerp(color_a.rgb,color_b.rgb,outerMix*0.80)" in shader
assert "dualInnerColor=lerp(color_b.rgb,color_a.rgb,innerMix*0.80)" in shader
assert "dualOuterGlowA=saturate(art_intensity*glow_strength*0.30" in shader
assert "dualInnerGlowA=saturate(art_intensity*glow_strength*0.24" in shader
assert "lerp(dualOuterColor,float3(1.0,1.0,1.0),0.22)" in shader
assert "lerp(dualInnerColor,float3(1.0,1.0,1.0),0.18)" in shader

assert "dualOuterHotA" not in shader and "dualInnerHotA" not in shader
assert "outerStartCap" not in shader and "innerStartCap" not in shader
assert "outerEndCap" not in shader and "innerEndCap" not in shader
assert "coreA*=0.0" in shader and "outerA*=0.0" in shader
assert "fineA*=0.0" in shader and "rimA*=0.0" in shader
assert "coreA*=0.80" in shader and "outerA*=0.40" in shader
assert "float nodeD=" not in shader
for key in ("art_intensity", "art_gap"):
    assert f'obs_data_has_user_value(settings, "{key}")' in host
    assert f'obs_data_set_double(settings, "{key}", next.{key})' in host
for key in ("ornament_width","ornament_length_x","ornament_length_y",
            "inner_rail_width","ring_inner_offset","ring_spacing"):
    assert f'obs_data_set_default_double(settings, "{key}"' in host
assert 'obs_data_has_user_value(settings,d4f_keys[i])' in host
assert 'obs_data_has_user_value(settings, "ring_inner_offset")' in host
assert 'obs_data_has_user_value(settings, "ring_spacing")' in host
for key in ("ring_outer_width_pct","ring_inner_width_pct",
            "ring_inner_offset_pct","ring_spacing_pct"):
    assert f'obs_data_set_default_double(settings, "{key}"' in host
    assert key in host
assert 'nm_config_migrate_ring_v3(&next)' in host
assert 'nm_config_ring_geometry_px(&f->config,rx' in host
assert 'schema < 4u' in host

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
assert "outerA *= 1.0-smoothstep(2.30*outerRadius,2.50*outerRadius,gap)" in shader
assert "float3 outerColor = neon;" in shader
assert "if(outA <= 0.0005) return float4(0.0,0.0,0.0,0.0);" in shader
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
assert 'obs_data_set_default_bool(settings, "safe_fit", cfg.safe_fit)' in host
assert 'obs_properties_add_bool(mask_group, "safe_fit"' in host
assert 'nm_safe_fit_calculate(&f->config, width, height, &fit)' in host
assert "const float bx=fit.half_width;" in host and "const float by=fit.half_height;" in host
assert "const float half_width = fit.half_width;" in host
assert "const float half_height = fit.half_height;" in host
assert 'obs_data_set_default_bool(settings, "expand_canvas", cfg.expand_canvas)' in host
assert 'obs_properties_add_bool(mask_group,"expand_canvas"' in host
assert "obs_property_set_modified_callback(expand,nm_context_changed)" in host
assert 'Mask.ExpandCanvas="Expand output for glow' in en_locale
assert 'Mask.ExpandCanvas="Perluas output untuk glow / ornamen luar' in id_locale
assert ".get_width = nm_get_width" in host and ".get_height = nm_get_height" in host
assert "return horizontal ? fit.output_width : fit.output_height;" in host
assert "vec2_set(&origin, (float)fit.pad_left, (float)fit.pad_top);" in host
assert "vec2_set(&output_dimensions, (float)fit.output_width, (float)fit.output_height);" in host
assert "fit.output_width, fit.output_height, \"Draw\"" in host
assert "v_in.uv * output_size - input_origin - uv_size * 0.5" in shader
assert "v_in.uv * uv_size - uv_size * 0.5" not in shader
assert "sourceUV = (samplePos + uv_size * 0.5) / uv_size" in shader
assert "gs_effect_set_vec2(f->output_size, &output_dimensions)" in host
assert "gs_effect_set_vec2(f->input_origin, &origin)" in host
assert 'Mask.SafeFit="Keep glow / outer ornaments inside source (may shrink mask)"' in en_locale
assert 'Mask.SafeFit="Jaga glow / ornamen luar di dalam sumber (mask dapat mengecil)"' in id_locale
assert "const bool outer_authored =" in safe_fit
assert "const bool fit_inside_source = cfg->safe_fit || outer_authored;" in safe_fit
assert "if (!fit_inside_source && !cfg->expand_canvas)" in safe_fit
assert 'obs_data_set_bool(settings,"safe_fit",true)' not in host
assert "obs_property_set_visible(p,!expanded && !outer_authored)" in host
assert "if (!valid)" in host, "Missing shader parameters must disable effect gracefully"
# No shader / zero-size rendering must fail closed, not display the webcam.
render = host.split("static void nm_render(", 1)[1].split("struct obs_source_info neonmask_filter_info", 1)[0]
assert "if (!target || !f->effect)" in render
assert "obs_source_skip_video_filter(" not in render, "Unmasked bypass in render callback"
assert "filter will render transparent" in host

assert "nm_custom_changed" in host, "User edits should reset preset status to Custom"
assert 'obs_properties_add_list(props,"color_mode"' in host
assert 'obs_properties_add_group(props,"rainbow_color"' in host
assert 'NM_SHOW("secondary",border && color_mode == NM_COLOR_DUAL)' in host
assert 'NM_SHOW("rainbow_color",border && color_mode == NM_COLOR_RAINBOW)' in host
for key in ("color_mode","rainbow_phase","rainbow_saturation","rainbow_hue_offset","rainbow_spread"):
    assert f'NM_PARAM({key}, "{key}")' in host
assert "nm_rainbow_tick(&f->motion" in host
assert "sourcePos - mask_offset" in shader
assert "sourcePos - subject_pan" in shader and "/ max(subject_zoom" in shader
assert "insideSource" in shader and "float4(0.0, 0.0, 0.0, 0.0)" in shader
assert "NM_SHAPE_RECTANGLE" in host and "NM_SHAPE_TRIANGLE" in host and "NM_SHAPE_POLYGON" in host
print("PASS: shader bindings, alpha/color contracts, dimensions and fallback guards")
