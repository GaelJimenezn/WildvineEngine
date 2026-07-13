/**
 * @file BaseApp.h
 * @brief Declara la clase principal del motor WildvineEngine.
 * @ingroup core
 *
 * BaseApp orquesta el ciclo de vida completo del engine: inicialización de
 * DirectX, loop principal, gestión de actores, render pipeline, GUI del editor
 * y escena. Es el punto de entrada de toda la aplicación.
 */
#pragma once
#include "Prerequisites.h"
#include "Window.h"
#include "Device.h"
#include "DeviceContext.h"
#include "SwapChain.h"
#include "Texture.h"
#include "RenderTargetView.h"
#include "DepthStencilView.h"
#include "Viewport.h"
#include "ShaderProgram.h"
#include "MeshComponent.h"
#include "Buffer.h"
#include "SamplerState.h"

#include "Model3D.h"
#include "GUI.h"
#include "ECS/Actor.h"

#include "SceneGraph/SceneGraph.h"

#include "EngineUtilities/Utilities/Camera.h"
#include "EngineUtilities/Utilities/Skybox.h"
#include "EngineUtilities/Utilities/LayoutBuilder.h"
#include "EngineUtilities/Utilities/EditorViewportPass.h"

#include "ECS/LightComponent.h"
#include "ECS/MeshRendererComponent.h"

#include "Rendering/Material.h"
#include "Rendering/MaterialInstance.h"
#include "Rendering/Mesh.h"
#include "Rendering/RenderPipeline.h"
#include "Rendering/RenderScene.h"

#include "CommandManager.h"

#include <string>

extern IMGUI_IMPL_API
LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

/**
 * @struct ActorClipboard
 * @brief Almacena los datos de un actor copiado para operaciones de copiar/pegar.
 *
 * Contiene toda la información necesaria para replicar un actor:
 * nombre, transformación, malla, datos de luz y banderas de tipo.
 */
struct ActorClipboard {
  std::string name;                    /**< @brief Nombre del actor copiado. */
  EU::Vector3 position;                /**< @brief Posición en espacio mundo. */
  EU::Vector3 rotation;                /**< @brief Rotación en grados (Pitch, Yaw, Roll). */
  EU::Vector3 scale;                   /**< @brief Escala por eje. */
  Mesh* mesh = nullptr;                /**< @brief Puntero a la malla (no propietario). */
  LightData lightData;                 /**< @brief Datos de luz si el actor es una luz. */
  MaterialInstance materialInstance;   /**< @brief Instancia de material del actor. */
  bool hasLight = false;               /**< @brief Indica si el actor tiene componente de luz. */
  bool hasMesh  = false;               /**< @brief Indica si el actor tiene componente de malla. */
};

/**
 * @struct LoadedModel
 * @brief Agrupa todos los recursos GPU asociados a un modelo importado.
 *
 * Incluye la malla, el material PBR y sus mapas de textura (albedo, normal,
 * metallic, roughness, AO). También guarda el AABB local para picking y fit.
 */
struct LoadedModel {
  Mesh                                           mesh;
  std::vector<std::unique_ptr<Material>>         materials;
  std::vector<std::unique_ptr<MaterialInstance>> materialInstances;
  std::vector<std::unique_ptr<Texture>>          externalTextures;
  EU::Vector3                                    localMin = EU::Vector3(0.0f, 0.0f, 0.0f);
  EU::Vector3                                    localMax = EU::Vector3(0.0f, 0.0f, 0.0f);
};

/**
 * @struct GizmoEditState
 * @brief Captura el estado TRS de un actor antes de una edición con Gizmo.
 *
 * Usado por el sistema de Undo/Redo para guardar el estado previo antes de
 * que el usuario mueva, rote o escale un actor con ImGuizmo.
 */
struct GizmoEditState {
  EU::Vector3 position; /**< @brief Posición antes de editar. */
  EU::Vector3 rotation; /**< @brief Rotación antes de editar. */
  EU::Vector3 scale;    /**< @brief Escala antes de editar. */
};

/**
 * @class BaseApp
 * @brief Clase principal del motor WildvineEngine.
 *
 * Gestiona el ciclo de vida completo de la aplicación, incluyendo:
 * - Inicialización y destrucción de los subsistemas DirectX 11.
 * - Loop principal (update/render).
 * - Gestión de actores y escena.
 * - Pipeline de renderizado (Forward / Deferred).
 * - GUI del editor (ImGui + ImGuizmo).
 * - Serialización / deserialización de escena.
 * - Sistema de Undo/Redo con CommandManager.
 * - Clipboard y duplicación de actores.
 *
 * @note Solo debe existir una instancia de esta clase por proceso.
 */
class BaseApp {
public:
  /** @brief Constructor por defecto. */
  BaseApp() = default;

  /**
   * @brief Destructor — invoca destroy() automáticamente al salir de ámbito.
   */
  ~BaseApp() { destroy(); }

  /**
   * @brief Prepara la ventana y el contexto Win32 antes de init().
   * @return S_OK en éxito, código HRESULT de error en otro caso.
   */
  HRESULT awake();

  /**
   * @brief Crea la ventana principal y entra al message loop de Win32.
   * @param hInst    Handle de instancia Win32 (de WinMain).
   * @param nCmdShow Parámetro de visibilidad de ventana.
   * @return Código de salida de la aplicación.
   */
  int run(HINSTANCE hInst, int nCmdShow);

  /**
   * @brief Inicializa todos los subsistemas (D3D11, shaders, GUI, escena por defecto).
   * @return S_OK en éxito, código HRESULT de error en otro caso.
   */
  HRESULT init();

  /**
   * @brief Actualiza la lógica del engine una vez por frame.
   * @param deltaTime Tiempo transcurrido desde el último frame, en segundos.
   */
  void update(float deltaTime);

  /**
   * @brief Ejecuta el pipeline de render del frame actual.
   *
   * Recopila la escena, actualiza el per-frame constant buffer y
   * delega en RenderPipeline::render().
   */
  void render();

  /**
   * @brief Libera todos los recursos (GPU, GUI, actores, texturas, etc.).
   */
  void destroy();

  /**
   * @brief Callback de redimensionamiento de ventana principal.
   * @param newW Nuevo ancho en píxeles.
   * @param newH Nuevo alto en píxeles.
   */
  void onResize(unsigned int newW, unsigned int newH);

  /**
   * @brief Aplica un resize pendiente del viewport del editor cuando es estable.
   *
   * Se llama cuando el tamaño del viewport no ha cambiado durante varios frames
   * consecutivos para evitar recreaciones continuas de texturas GBuffer.
   */
  void handleEditorViewportResize();

  /**
   * @brief Serializa la escena actual a un archivo JSON en disco.
   * @param path Ruta completa del archivo de destino.
   * @return true si la serialización tuvo éxito.
   */
  bool saveScene(const std::string& path);

  /**
   * @brief Deserializa y carga una escena desde un archivo JSON.
   * @param path Ruta completa del archivo fuente.
   * @return true si la carga tuvo éxito.
   */
  bool loadScene(const std::string& path);

  /**
   * @brief Devuelve la ruta al archivo de escena por defecto.
   * @return Cadena con la ruta relativa o absoluta del archivo de escena.
   */
  std::string getDefaultScenePath() const;

  /**
   * @brief Agrega un actor a la lista de actores y al SceneGraph.
   * @param actor Shared pointer al actor que se va a añadir.
   */
  void addActorToScene(const EU::TSharedPointer<Actor>& actor);

  /**
   * @brief Elimina un actor de la lista de actores y del SceneGraph.
   * @param actor Shared pointer al actor que se va a eliminar.
   */
  void removeActorFromScene(const EU::TSharedPointer<Actor>& actor);

private:
  /**
   * @brief Crea y configura un actor de luz del tipo indicado.
   *
   * Para LightType::Directional solo crea uno (singleton). Los demás tipos
   * siempre crean un actor nuevo.
   *
   * @param type     Tipo de luz (Directional, Point, Spot, Rect).
   * @param baseName Nombre base del actor resultante.
   * @return Shared pointer al actor de luz creado.
   */
  EU::TSharedPointer<Actor> createLightActor(LightType type, const std::string& baseName);

  /**
   * @brief Asegura que el layout de docking del editor sea el predeterminado.
   */
  void enforceDefaultSceneLayout();

  /**
   * @brief Encuadra la cámara del editor para mostrar la escena completa al inicio.
   */
  void frameDefaultSceneCamera();

  /**
   * @brief Marca un actor como seleccionado y actualiza el índice en la GUI.
   * @param actor Actor a seleccionar.
   */
  void selectActor(EU::TSharedPointer<Actor> actor);

  /**
   * @brief Sincroniza los actores de luz con el SceneGraph.
   */
  void syncLightActors();

  /**
   * @brief Carga una textura PBR desde disco y la sube a la GPU.
   * @param texture   Referencia a la instancia Texture de destino.
   * @param path      Ruta del archivo de imagen (PNG, JPG, TGA, etc.).
   * @param debugName Nombre descriptivo para mensajes de error y debug.
   * @return S_OK si la textura se cargó correctamente.
   */
  HRESULT loadPbrTexture(Texture& texture, const std::string& path, const char* debugName);

  /**
   * @brief Procedimiento estático de ventana Win32.
   * @param hWnd    Handle de la ventana que recibe el mensaje.
   * @param message Identificador del mensaje Win32.
   * @param wParam  Parámetro W dependiente del mensaje.
   * @param lParam  Parámetro L dependiente del mensaje.
   * @return Resultado del procesamiento del mensaje.
   */
  static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

private:
  // -------------------------------------------------------------------------
  // Subsistemas DirectX 11
  // -------------------------------------------------------------------------
  Window            m_window;           /**< @brief Ventana Win32 principal. */
  Device            m_device;           /**< @brief Dispositivo D3D11. */
  DeviceContext     m_deviceContext;    /**< @brief Contexto inmediato D3D11. */
  SwapChain         m_swapChain;        /**< @brief Swap chain de presentación. */
  Texture           m_backBuffer;       /**< @brief Textura del back buffer. */
  RenderTargetView  m_renderTargetView; /**< @brief RTV del back buffer. */
  Texture           m_depthStencil;     /**< @brief Textura de profundidad/stencil. */
  DepthStencilView  m_depthStencilView; /**< @brief DSV principal del back buffer. */
  Viewport          m_viewport;         /**< @brief Viewport de pantalla completa. */
  ShaderProgram     m_shaderProgram;    /**< @brief Shader program base (forward legacy). */

  bool              m_d3dReady      = false; /**< @brief true cuando D3D11 está listo para render. */
  bool              m_guiInitialized = false; /**< @brief true cuando ImGui está inicializado. */

  Buffer            m_constantBuffer;       /**< @brief Constant buffer principal (CBMain). */
  CBMain            m_constantBufferStruct; /**< @brief Datos del constant buffer principal. */

  // -------------------------------------------------------------------------
  // Estado inicial (botón Reset)
  // -------------------------------------------------------------------------
  /**
   * @struct InitialTransform
   * @brief Guarda la transformación TRS original de un actor para restaurarla al hacer Reset.
   */
  struct InitialTransform {
    EU::Vector3 position; /**< @brief Posición original del actor. */
    EU::Vector3 rotation; /**< @brief Rotación original del actor. */
    EU::Vector3 scale;    /**< @brief Escala original del actor. */
  };

  bool                          m_initialStateCaptured = false; /**< @brief true si el estado inicial ya fue capturado. */
  EU::Vector3                   m_initialLightDir;              /**< @brief Dirección de luz al inicio. */
  EU::Vector3                   m_initialLightColor;            /**< @brief Color de luz al inicio. */
  EU::Vector3                   m_initialCameraPos;             /**< @brief Posición de cámara al inicio. */
  std::vector<InitialTransform> m_initialTransforms;            /**< @brief Transformaciones originales de todos los actores. */

  /** @brief Captura el estado TRS de todos los actores para el botón Reset. */
  void captureInitialState();

  /** @brief Restaura la escena al estado guardado por captureInitialState(). */
  void resetSceneToDefaults();

  /**
   * @brief Mueve la cámara para encuadrar el actor indicado (tecla F).
   * @param actor Actor objetivo del encuadre.
   */
  void focusCameraOnActor(const EU::TSharedPointer<Actor>& actor);

  /** @brief Ajusta la cámara para que todos los objetos de la escena sean visibles. */
  void fitCameraToScene();

  unsigned int m_lastDrawCalls = 0; /**< @brief Número de draw calls ejecutados en el último frame. */

  // -------------------------------------------------------------------------
  // Picking
  // -------------------------------------------------------------------------
  EU::Vector3 m_modelLocalMin; /**< @brief Esquina mínima del AABB local del modelo principal. */
  EU::Vector3 m_modelLocalMax; /**< @brief Esquina máxima del AABB local del modelo principal. */

  /** @brief Lanza un ray desde el mouse hacia la escena y selecciona el actor más cercano. */
  void pickActorFromMouse();

  // -------------------------------------------------------------------------
  // Undo / Redo
  // -------------------------------------------------------------------------
  CommandManager m_commands;               /**< @brief Gestor de historial de comandos (Undo/Redo). */
  bool           m_prevGizmoUsing  = false; /**< @brief Estado del gizmo en el frame anterior. */
  bool           m_gizmoEditing    = false; /**< @brief true mientras el usuario edita con el gizmo. */
  int            m_gizmoEditActorIndex = -1; /**< @brief Índice en m_actors del actor bajo edición gizmo. */
  GizmoEditState m_gizmoBefore;             /**< @brief Estado TRS capturado antes de la edición con gizmo. */

  /**
   * @brief Captura el estado TRS de un actor dado su índice.
   * @param index Índice del actor en m_actors.
   * @param out   Estado capturado (salida).
   * @return true si el actor existe y el estado fue capturado.
   */
  bool captureGizmoState(int index, GizmoEditState& out);

  // -------------------------------------------------------------------------
  // Clipboard (Copiar/Pegar actores)
  // -------------------------------------------------------------------------
  ActorClipboard m_clipboard;           /**< @brief Actor almacenado en el clipboard. */
  bool           m_hasClipboard = false; /**< @brief true si hay un actor en el clipboard. */

  /**
   * @brief Crea un actor de pistola en la posición, rotación y escala indicadas.
   * @param name  Nombre del nuevo actor.
   * @param pos   Posición en espacio mundo.
   * @param rot   Rotación en grados (Euler).
   * @param scale Escala por eje.
   * @return Shared pointer al actor creado.
   */
  EU::TSharedPointer<Actor> spawnPistol(const std::string& name,
    const EU::Vector3& pos, const EU::Vector3& rot, const EU::Vector3& scale);

  /** @brief Crea una copia del actor seleccionado y la añade a la escena. */
  void duplicateSelected();
  /** @brief Elimina el actor seleccionado de la escena y destruye sus recursos. */
  void deleteSelected();
  /** @brief Copia los datos del actor seleccionado al clipboard interno. */
  void copySelected();
  /** @brief Crea un actor nuevo a partir de los datos del clipboard. */
  void pasteClipboard();
  /** @brief Exporta el actor seleccionado como archivo prefab JSON. */
  void savePrefabSelected();
  /** @brief Importa un prefab JSON y lo instancia en la escena. */
  void loadPrefab();

  // -------------------------------------------------------------------------
  // Recursos de modelos importados dinámicamente
  // -------------------------------------------------------------------------
  std::vector<std::unique_ptr<LoadedModel>>      m_loadedModels;     /**< @brief Modelos cargados por el Content Browser. */
  std::vector<std::unique_ptr<MaterialInstance>> m_dynamicMaterials; /**< @brief Instancias de material creadas dinámicamente. */
  std::vector<std::unique_ptr<Texture>>          m_dynamicTextures;  /**< @brief Texturas cargadas dinámicamente. */
  std::vector<Texture>                           m_thumbTextures;    /**< @brief Texturas de preview para el Content Browser. */
  std::vector<AssetThumb>                        m_thumbnails;       /**< @brief Datos de thumbnail por asset. */

  /**
   * @brief Carga un archivo de modelo (FBX/GLB/OBJ) y crea un actor con él en la escena.
   * @param modelPath Ruta completa al archivo de modelo.
   * @return Shared pointer al actor generado.
   */
  EU::TSharedPointer<Actor> loadModelActor(const std::string& modelPath);

  /**
   * @brief Busca y carga las texturas PBR de un modelo desde una carpeta.
   * @param lm     Referencia al LoadedModel donde se cargarán las texturas.
   * @param folder Ruta de la carpeta donde buscar las texturas.
   */
  void loadModelTextures(LoadedModel& lm, const std::string& folder);

  /** @brief Genera miniaturas de todas las texturas importadas para el Content Browser. */
  void buildTextureThumbnails();

  /**
   * @brief Calcula el AABB (axis-aligned bounding box) de un actor.
   * @param actor  Actor a consultar.
   * @param outMin Esquina mínima del AABB en espacio local.
   * @param outMax Esquina máxima del AABB en espacio local.
   * @return true si el AABB pudo calcularse.
   */
  bool getActorAABB(const EU::TSharedPointer<Actor>& actor, EU::Vector3& outMin, EU::Vector3& outMax);

  // -------------------------------------------------------------------------
  // Texturas PBR del modelo principal (CyberGun)
  // -------------------------------------------------------------------------
  Texture m_AlbedoSRV;    /**< @brief SRV de la textura Albedo (color base). */
  Texture m_MetallicSRV;  /**< @brief SRV de la textura de metalicidad. */
  Texture m_RoughnessSRV; /**< @brief SRV de la textura de rugosidad. */
  Texture m_AOSRV;        /**< @brief SRV de la textura de oclusión ambiental. */
  Texture m_NormalSRV;    /**< @brief SRV del mapa de normales tangentes. */
  Texture m_EmissiveSRV;  /**< @brief SRV de la textura de emisión. */

  // -------------------------------------------------------------------------
  // Recursos globales de escena
  // -------------------------------------------------------------------------
  Texture     m_skyboxTex;  /**< @brief Textura cubemap del Skybox. */
  Camera      m_camera;     /**< @brief Cámara fly-through del editor. */
  SceneGraph  m_sceneGraph; /**< @brief Árbol de escena con jerarquía de entidades. */

  // -------------------------------------------------------------------------
  // Actores de la escena
  // -------------------------------------------------------------------------
  std::vector<EU::TSharedPointer<Actor>> m_actors;              /**< @brief Lista de todos los actores de la escena. */
  EU::TSharedPointer<Actor>              m_cyberGun;            /**< @brief Actor del modelo principal (CyberGun). */
  EU::TSharedPointer<Actor>              m_directionalLightActor; /**< @brief Luz direccional principal (máximo una). */

  Model3D* m_model = nullptr; /**< @brief Modelo 3D activo cargado desde disco (puede ser nullptr). */

  // -------------------------------------------------------------------------
  // GUI del editor
  // -------------------------------------------------------------------------
  GUI m_gui; /**< @brief Instancia de la GUI del editor (ImGui + paneles personalizados). */

  // -------------------------------------------------------------------------
  // Pipeline de renderizado
  // -------------------------------------------------------------------------
  Skybox            m_skybox;              /**< @brief Skybox de fondo de la escena. */
  RasterizerState   m_defaultRasterizer;   /**< @brief Estado rasterizador estándar (fill solid, cull back). */
  DepthStencilState m_defaultDepthStencil; /**< @brief Estado depth/stencil con escritura y prueba habilitadas. */
  SamplerState      m_defaultSampler;      /**< @brief Sampler anisótropo por defecto para todas las texturas. */

  Mesh             m_cyberGunRenderMesh;     /**< @brief Malla GPU del actor CyberGun. */
  Material         m_pbrMaterial;            /**< @brief Material PBR opaco base. */
  Material         m_transparentPbrMaterial; /**< @brief Material PBR para superficies transparentes. */
  MaterialInstance m_cyberGunMaterial;       /**< @brief Instancia de material del CyberGun con sus texturas. */

  EditorViewportPass m_editorViewportPass; /**< @brief Pass de render del viewport flotante del editor. */
  RenderPipeline     m_renderPipeline;     /**< @brief Orquestador Forward/Deferred renderer. */
  RenderScene        m_renderScene;        /**< @brief Contenedor de datos de escena para un frame de render. */

  // -------------------------------------------------------------------------
  // Resize diferido del viewport del editor
  // -------------------------------------------------------------------------
  bool         m_editorViewportResizePending = false; /**< @brief true si hay un resize pendiente sin aplicar. */
  unsigned int m_pendingViewportWidth  = 1;           /**< @brief Nuevo ancho pendiente del viewport. */
  unsigned int m_pendingViewportHeight = 1;           /**< @brief Nuevo alto pendiente del viewport. */

  unsigned int m_lastRequestedViewportWidth  = 1; /**< @brief Último ancho solicitado por el usuario. */
  unsigned int m_lastRequestedViewportHeight = 1; /**< @brief Último alto solicitado por el usuario. */
  int          m_viewportResizeStableFrames  = 0; /**< @brief Contador de frames consecutivos sin cambio de tamaño. */
};
