# Generacion De Documentacion

La documentacion HTML de Wildvine Engine se genera con Doxygen usando el archivo `docs/doxygen/Doxyfile`.

## Generacion Local

1. Instala `doxygen`.
2. Instala `graphviz` si quieres soporte completo para grafos.
3. Ejecuta:

```bash
doxygen docs/doxygen/Doxyfile
```

La salida HTML quedara en `docs/generated/html`.

## Publicacion En GitHub Pages

El workflow `/.github/workflows/doxygen-pages.yml`:

- instala Doxygen en Ubuntu,
- genera el HTML,
- sube el artefacto de Pages,
- publica el sitio con GitHub Pages.

## Nota

La configuracion actual publica automaticamente en `push` a las ramas `main`, `master` y `ForwardRendering+Shadows`, ademas de permitir ejecucion manual con `workflow_dispatch`.
