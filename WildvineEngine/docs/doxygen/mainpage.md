# Wildvine Engine

Wildvine Engine es un motor experimental de renderizado en **Direct3D 11** con herramientas de editor basadas en **ImGui**, una capa ECS ligera y un pipeline forward con soporte para materiales PBR, skybox y depuracion visual.

Esta documentacion fue organizada para que GitHub Pages pueda publicar una referencia navegable del codigo propio del proyecto.

## Contenido

- **Core**: ciclo de vida de la aplicacion, ventana, dispositivo, swap chain y recursos base.
- **Rendering**: pipeline forward, materiales, tipos de render y recoleccion de escena.
- **ECS**: actores, componentes y transformaciones.
- **SceneGraph**: jerarquia y actualizacion espacial.
- **GUI**: herramientas del editor e integracion con ImGui.
- **Utilities**: camara, skybox, layout de vertex input y pases auxiliares.
- **Math**: vectores, matrices y quaternions.
- **Memory**: smart pointers del proyecto.
- **Structures**: contenedores auxiliares.

## Convenciones

- La documentacion prioriza el **codigo propio del motor**.
- Las dependencias externas como `Imgui`, `DXUT`, `FBX SDK` y `stb_image` se excluyen de la salida de Doxygen para mantener la referencia enfocada.
- Los archivos fuente y cabeceras del proyecto incluyen anotaciones `@file` y agrupacion por modulo para facilitar la navegacion.

## Punto De Entrada

La ejecucion del motor inicia en `WildvineEngine.cpp`, que crea la instancia principal de `BaseApp` y delega el ciclo de vida de inicializacion, actualizacion, render y destruccion.
