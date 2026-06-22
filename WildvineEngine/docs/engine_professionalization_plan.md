# Wildvine Engine Professionalization Plan

## SRV/Debug Views

### Connected now
- Scene Final: editor viewport color target.
- Pre-Shadow: scene render without shadow resolve.
- Shadow Map: directional light depth map.
- Material SRVs: Albedo, Normal, Metallic, Roughness, AO.

### Existing but not active in BaseApp
- GBuffer Albedo + Metallic from DeferredRenderer.
- GBuffer Normal + Roughness from DeferredRenderer.
- GBuffer World + AO from DeferredRenderer.
- GBuffer Emissive + Alpha from DeferredRenderer.

### Missing for a professional renderer
- Depth buffer visualization SRV.
- Linear depth debug SRV.
- World normals debug SRV in forward mode.
- Roughness/metallic/AO combined inspector.
- Motion vectors SRV.
- SSAO buffer SRV.
- Bloom threshold SRV.
- Bloom blur chain SRVs.
- Tonemapping input/output SRVs.
- HDR scene color SRV.
- Reflection/environment prefilter SRVs.
- BRDF LUT SRV.
- Irradiance map SRV.
- Per-light shadow atlas SRV.
- Object ID / picking buffer SRV.
- GPU timing/debug counters.

## Neon Blue/Purple Palette

| Token | Hex | Use |
| --- | --- | --- |
| `bg_0` | `#0E0F11` | deepest viewport/panel background |
| `bg_1` | `#131417` | standard panel background |
| `bg_2` | `#1B1C20` | frames/buttons/cards |
| `bg_3` | `#25272C` | hover/active dark surface |
| `neon_blue` | `#0D57D1` | selected toolbar state |
| `electric_blue` | `#1294FF` | highlights/navigation |
| `neon_purple` | `#6B29DB` | active separators/docking |
| `electric_purple` | `#A342FF` | hovered active controls |
| `cyan_glow` | `#00E0FF` | checkmarks/focus accents |
| `text_main` | `#E0E6F0` | primary text |
| `text_muted` | `#7A828F` | secondary labels |

## BaseApp Cleanup Map

### Split next
- `RendererBootstrap`: device, swap chain, viewport, resize-safe resources.
- `SceneBootstrap`: default actor, default light, scene graph registration.
- `AssetBootstrap`: model and PBR texture loading.
- `EditorController`: GUI requests, save/load, panel toggles.
- `SceneSerializer`: WVSCENE read/write.

### Immediate optimizations
- Cache texture/model paths in one data structure.
- Load PBR texture set through one helper instead of five repeated blocks.
- Add a resource cache so repeated texture/model requests reuse existing GPU resources.
- Move default scene creation out of `BaseApp::init`.
- Keep resize code centralized and unbind SRVs before destroying render targets.

### Renderer upgrades
- Let `BaseApp` use `RenderPipeline` instead of hardcoded `ForwardRenderer`.
- Add runtime switch between Forward and Deferred.
- Pipe DeferredRenderer GBuffer SRVs into GUI when deferred is active.
- Add post-process chain: HDR, tonemap, bloom, color grading.
- Add object picking buffer for editor selection.
