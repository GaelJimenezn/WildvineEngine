/**
 * @file BaseApp.h
 * @brief Declara la clase principal del motor WildvineEngine.
 * @ingroup core
 *
 * BaseApp orquesta el ciclo de vida completo del engine: inicializaciÃ³n de
 * DirectX, loop principal, gestiÃ³n de actores, render pipeline, GUI del editor
 * y escena. Es el punto de entrada de toda la aplicaciÃ³n.
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
#include "EngineUtilities/GUI/GUI.h"
#include "ECS/Actor.h"

#include "SceneGraph/SceneGraph.h"
#include "SceneGraph/Octree.h"

#include "EngineUtilities/Utilities/Camera.h"
#include "EngineUtilities/Utilities/Skybox.h"
#include "EngineUtilities/Utilities/LayoutBuilder.h"
#include "EngineUtilities/Utilities/EditorViewportPass.h"

#include "ECS/LightComponent.h"
#include "ECS/MeshRendererComponent.h"
#include "ECS/ParticleEmitterComponent.h"

#include "Rendering/Material.h"
#include "Rendering/MaterialInstance.h"
#include "Rendering/Mesh.h"
#include "Rendering/RenderPipeline.h"
#include "Utilities/AudioSystem.h"
#include "Rendering/RenderScene.h"

#include "CommandManager.h"

#include <future>
#include <string>

/** @brief Declara o ejecuta ImGui_ImplWin32_WndProcHandler. */
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd,
                                                             UINT msg,
                                                             WPARAM wParam,
                                                             LPARAM lParam);

/**
 * @struct ActorClipboard
 * @brief Almacena los datos de un actor copiado para operaciones de copiar/pegar.
 *
 * Contiene toda la informaciÃ³n necesaria para replicar un actor:
 * nombre, transformaciÃ³n, malla, datos de luz y banderas de tipo.
 */
struct ActorClipboard {
  /** @brief Nombre del actor copiado. */
  std::string name;
  /** @brief PosiciÃ³n en espacio mundo. */
  EU::Vector3 position;
  /** @brief RotaciÃ³n en grados (Pitch, Yaw, Roll). */
  EU::Vector3 rotation;
  /** @brief Escala por eje. */
  EU::Vector3 scale;
  /** @brief Puntero a la malla (no propietario). */
  Mesh *mesh = nullptr;
  /** @brief Datos de luz si el actor es una luz. */
  LightData lightData;
  /** @brief Instancia de material del actor. */
  MaterialInstance materialInstance;
  /** @brief Indica si el actor tiene componente de luz. */
  bool hasLight = false;
  /** @brief Indica si el actor tiene componente de malla. */
  bool hasMesh = false;
};

/**
 * @struct LoadedModel
 * @brief Agrupa todos los recursos GPU asociados a un modelo importado.
 *
 * Incluye la malla, el material PBR y sus mapas de textura (albedo, normal,
 * metallic, roughness, AO). TambiÃ©n guarda el AABB local para picking y fit.
 */
struct LoadedModel {
  Mesh mesh;
  std::vector<std::unique_ptr<Material>> materials;
  std::vector<std::unique_ptr<MaterialInstance>> materialInstances;
  std::vector<std::unique_ptr<Texture>> externalTextures;
  EU::Vector3 localMin = EU::Vector3(0.0f, 0.0f, 0.0f);
  EU::Vector3 localMax = EU::Vector3(0.0f, 0.0f, 0.0f);
};

/** @brief Declara struct PendingModelImport. */
struct PendingModelImport {
  std::string path;
  EU::Vector3 spawnPosition = EU::Vector3(0.0f, 0.0f, 0.0f);
  std::future<bool> job;
  bool active = false;
};

/**
 * @struct GizmoEditState
 * @brief Captura el estado TRS de un actor antes de una ediciÃ³n con Gizmo.
 *
 * Usado por el sistema de Undo/Redo para guardar el estado previo antes de
 * que el usuario mueva, rote o escale un actor con ImGuizmo.
 */
struct GizmoEditState {
  /** @brief PosiciÃ³n antes de editar. */
  EU::Vector3 position;
  /** @brief RotaciÃ³n antes de editar. */
  EU::Vector3 rotation;
  /** @brief Escala antes de editar. */
  EU::Vector3 scale;
};

/**
 * @class BaseApp
 * @brief Clase principal del motor WildvineEngine.
 *
 * Gestiona el ciclo de vida completo de la aplicaciÃ³n, incluyendo:
 * - InicializaciÃ³n y destrucciÃ³n de los subsistemas DirectX 11.
 * - Loop principal (update/render).
 * - GestiÃ³n de actores y escena.
 * - Pipeline de renderizado (Forward / Deferred).
 * - GUI del editor (ImGui + ImGuizmo).
 * - SerializaciÃ³n / deserializaciÃ³n de escena.
 * - Sistema de Undo/Redo con CommandManager.
 * - Clipboard y duplicaciÃ³n de actores.
 *
 * @note Solo debe existir una instancia de esta clase por proceso.
 */
class BaseApp {
public:
  /** @brief Constructor por defecto. */
  BaseApp() = default;

  /**
   * @brief Destructor â€” invoca destroy() automÃ¡ticamente al salir de Ã¡mbito.
   */
  ~BaseApp() {
    destroy();
  }

  /**
   * @brief Prepara la ventana y el contexto Win32 antes de init().
   * @return S_OK en Ã©xito, cÃ³digo HRESULT de error en otro caso.
   */
  HRESULT
  awake();

  /**
   * @brief Crea la ventana principal y entra al message loop de Win32.
   * @param hInst    Handle de instancia Win32 (de WinMain).
   * @param nCmdShow ParÃ¡metro de visibilidad de ventana.
   * @return CÃ³digo de salida de la aplicaciÃ³n.
   */
  int run(HINSTANCE hInst, int nCmdShow);

  /**
   * @brief Inicializa todos los subsistemas (D3D11, shaders, GUI, escena por defecto).
   * @return S_OK en Ã©xito, cÃ³digo HRESULT de error en otro caso.
   */
  HRESULT
  init();

  /**
   * @brief Actualiza la lÃ³gica del engine una vez por frame.
   * @param deltaTime Tiempo transcurrido desde el Ãºltimo frame, en segundos.
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
   * @param newW Nuevo ancho en pÃ­xeles.
   * @param newH Nuevo alto en pÃ­xeles.
   */
  void onResize(unsigned int newW, unsigned int newH);

  /**
   * @brief Aplica un resize pendiente del viewport del editor cuando es estable.
   *
   * Se llama cuando el tamaÃ±o del viewport no ha cambiado durante varios frames
   * consecutivos para evitar recreaciones continuas de texturas GBuffer.
   */
  void handleEditorViewportResize();

  /**
   * @brief Serializa la escena actual a un archivo binario en disco.
   * @param path Ruta completa del archivo de destino.
   * @return true si la serializaciÃ³n tuvo Ã©xito.
   */
  bool saveScene(const std::string &path);

  /**
   * @brief Deserializa y carga una escena desde un archivo binario.
   * @param path Ruta completa del archivo fuente.
   * @return true si la carga tuvo Ã©xito.
   */
  bool loadScene(const std::string &path);

  /**
   * @brief Devuelve la ruta al archivo de escena por defecto.
   * @return Cadena con la ruta relativa o absoluta del archivo de escena.
   */
  std::string getDefaultScenePath() const;

  /**
   * @brief Agrega un actor a la lista de actores y al SceneGraph.
   * @param actor Shared pointer al actor que se va a aÃ±adir.
   */
  void addActorToScene(const EU::TSharedPointer<Actor> &actor);

  /**
   * @brief Elimina un actor de la lista de actores y del SceneGraph.
   * @param actor Shared pointer al actor que se va a eliminar.
   */
  void removeActorFromScene(const EU::TSharedPointer<Actor> &actor);

private:
  /**
   * @brief Crea y configura un actor de luz del tipo indicado.
   *
   * Para LightType::Directional solo crea uno (singleton). Los demÃ¡s tipos
   * siempre crean un actor nuevo.
   *
   * @param type     Tipo de luz (Directional, Point, Spot, Rect).
   * @param baseName Nombre base del actor resultante.
   * @return Shared pointer al actor de luz creado.
   */
  EU::TSharedPointer<Actor> createLightActor(LightType type, const std::string &baseName);
  EU::TSharedPointer<Actor> createParticleEmitterActor();

  /**
   * @brief Asegura que el layout de docking del editor sea el predeterminado.
   */
  void enforceDefaultSceneLayout();

  /**
   * @brief Encuadra la cÃ¡mara del editor para mostrar la escena completa al inicio.
   */
  void frameDefaultSceneCamera();

  /**
   * @brief Marca un actor como seleccionado y actualiza el Ã­ndice en la GUI.
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
   * @return S_OK si la textura se cargÃ³ correctamente.
   */
  HRESULT
  loadPbrTexture(Texture &texture, const std::string &path, const char *debugName);

  /** @brief Declara o ejecuta buildEditorGridMesh. */
  HRESULT
  buildEditorGridMesh();

  /** @brief Declara o ejecuta addEditorGridToRenderScene. */
  void addEditorGridToRenderScene();

  /** @brief Declara o ejecuta rebuildEditorSelectionMesh. */
  HRESULT
  rebuildEditorSelectionMesh(const EU::Vector3 &localMin, const EU::Vector3 &localMax);

  /** @brief Declara o ejecuta addEditorSelectionToRenderScene. */
  void addEditorSelectionToRenderScene();

  /**
   * @brief Procedimiento estÃ¡tico de ventana Win32.
   * @param hWnd    Handle de la ventana que recibe el mensaje.
   * @param message Identificador del mensaje Win32.
   * @param wParam  ParÃ¡metro W dependiente del mensaje.
   * @param lParam  ParÃ¡metro L dependiente del mensaje.
   * @return Resultado del procesamiento del mensaje.
   */
  static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

private:
  // -------------------------------------------------------------------------
  // Subsistemas DirectX 11
  // -------------------------------------------------------------------------
  /** @brief Ventana Win32 principal. */
  Window m_window;
  /** @brief Dispositivo D3D11. */
  Device m_device;
  /** @brief Contexto inmediato D3D11. */
  DeviceContext m_deviceContext;
  /** @brief Swap chain de presentaciÃ³n. */
  SwapChain m_swapChain;
  /** @brief Textura del back buffer. */
  Texture m_backBuffer;
  /** @brief RTV del back buffer. */
  RenderTargetView m_renderTargetView;
  /** @brief Textura de profundidad/stencil. */
  Texture m_depthStencil;
  /** @brief DSV principal del back buffer. */
  DepthStencilView m_depthStencilView;
  /** @brief Viewport de pantalla completa. */
  Viewport m_viewport;
  /** @brief Shader program base (forward legacy). */
  ShaderProgram m_shaderProgram;

  /** @brief true cuando D3D11 estÃ¡ listo para render. */
  bool m_d3dReady = false;
  /** @brief true cuando ImGui estÃ¡ inicializado. */
  bool m_guiInitialized = false;

  /** @brief Constant buffer principal (CBMain). */
  Buffer m_constantBuffer;
  /** @brief Datos del constant buffer principal. */
  CBMain m_constantBufferStruct;

  // -------------------------------------------------------------------------
  // Estado inicial (botÃ³n Reset)
  // -------------------------------------------------------------------------
  /**
   * @struct InitialTransform
   * @brief Guarda la transformacion TRS original de un actor.
   */
  struct InitialTransform {
    /** @brief PosiciÃ³n original del actor. */
    EU::Vector3 position;
    /** @brief RotaciÃ³n original del actor. */
    EU::Vector3 rotation;
    /** @brief Escala original del actor. */
    EU::Vector3 scale;
  };

  /** @brief true si el estado inicial ya fue capturado. */
  bool m_initialStateCaptured = false;
  /** @brief DirecciÃ³n de luz al inicio. */
  EU::Vector3 m_initialLightDir;
  /** @brief Color de luz al inicio. */
  EU::Vector3 m_initialLightColor;
  /** @brief PosiciÃ³n de cÃ¡mara al inicio. */
  EU::Vector3 m_initialCameraPos;
  /** @brief Transformaciones originales de todos los actores. */
  std::vector<InitialTransform> m_initialTransforms;

  /** @brief Estado de ediciÃ³n conservado durante la ejecuciÃ³n Play. */
  std::vector<InitialTransform> m_runtimeTransforms;
  /** @brief PosiciÃ³n de cÃ¡mara guardada al iniciar Play. */
  EU::Vector3 m_runtimeCameraPosition;
  /** @brief DirecciÃ³n de cÃ¡mara guardada al iniciar Play. */
  EU::Vector3 m_runtimeCameraForward;
  /** @brief Vector arriba de cÃ¡mara guardado al iniciar Play. */
  EU::Vector3 m_runtimeCameraUp;
  /** @brief true mientras los comportamientos de runtime estÃ¡n activos. */
  bool m_isPlaying = false;

  /** @brief Captura el estado TRS de todos los actores para el botÃ³n Reset. */
  void captureInitialState();

  /** @brief Restaura la escena al estado guardado por captureInitialState(). */
  void resetSceneToDefaults();

  /** @brief Captura los transforms de ediciÃ³n antes de iniciar Play. */
  void captureRuntimeState();

  /** @brief Inicia los componentes de comportamiento y entra al modo Play. */
  void startPlayMode();

  /** @brief Detiene los comportamientos y restaura el estado de ediciÃ³n. */
  void stopPlayMode();

  /**
   * @brief Activa o desactiva todos los comportamientos de runtime.
   * @param running true para ejecutar comportamientos; false para detenerlos.
   */
  void setRuntimeBehaviorsRunning(bool running);

  /**
   * @brief Inicia o detiene las fuentes de audio registradas en la escena.
   * @param playing true para reproducir; false para detener cada fuente.
   */
  void setRuntimeAudioPlaying(bool playing);

  /**
   * @brief Mueve la cÃ¡mara para encuadrar el actor indicado (tecla F).
   * @param actor Actor objetivo del encuadre.
   */
  void focusCameraOnActor(const EU::TSharedPointer<Actor> &actor);

  /** @brief Ajusta la cÃ¡mara para que todos los objetos de la escena sean visibles. */
  void fitCameraToScene();

  /** @brief NÃºmero de draw calls ejecutados en el Ãºltimo frame. */
  unsigned int m_lastDrawCalls = 0;
  CullingStats m_cullingStats;
  Octree m_sceneOctree;

  // -------------------------------------------------------------------------
  // Picking
  // -------------------------------------------------------------------------
  /** @brief Esquina mÃ­nima del AABB local del modelo principal. */
  EU::Vector3 m_modelLocalMin;
  /** @brief Esquina mÃ¡xima del AABB local del modelo principal. */
  EU::Vector3 m_modelLocalMax;

  /** @brief Lanza un ray desde el mouse y selecciona el actor mas cercano. */
  void pickActorFromMouse();

  // -------------------------------------------------------------------------
  // Undo / Redo
  // -------------------------------------------------------------------------
  /** @brief Gestor de historial de comandos (Undo/Redo). */
  CommandManager m_commands;
  /** @brief Estado del gizmo en el frame anterior. */
  bool m_prevGizmoUsing = false;
  /** @brief true mientras el usuario edita con el gizmo. */
  bool m_gizmoEditing = false;
  /** @brief Ãndice en m_actors del actor bajo ediciÃ³n gizmo. */
  int m_gizmoEditActorIndex = -1;
  /** @brief Estado TRS capturado antes de la ediciÃ³n con gizmo. */
  GizmoEditState m_gizmoBefore;

  /**
   * @brief Captura el estado TRS de un actor dado su Ã­ndice.
   * @param index Ãndice del actor en m_actors.
   * @param out   Estado capturado (salida).
   * @return true si el actor existe y el estado fue capturado.
   */
  bool captureGizmoState(int index, GizmoEditState &out);

  // -------------------------------------------------------------------------
  // Clipboard (Copiar/Pegar actores)
  // -------------------------------------------------------------------------
  /** @brief Actor almacenado en el clipboard. */
  ActorClipboard m_clipboard;
  /** @brief true si hay un actor en el clipboard. */
  bool m_hasClipboard = false;

  /**
   * @brief Crea un actor de pistola en la posiciÃ³n, rotaciÃ³n y escala indicadas.
   * @param name  Nombre del nuevo actor.
   * @param pos   PosiciÃ³n en espacio mundo.
   * @param rot   RotaciÃ³n en grados (Euler).
   * @param scale Escala por eje.
   * @return Shared pointer al actor creado.
   */
  EU::TSharedPointer<Actor> spawnPistol(const std::string &name,
                                        const EU::Vector3 &pos,
                                        const EU::Vector3 &rot,
                                        const EU::Vector3 &scale);

  /** @brief Crea una copia del actor seleccionado y la aÃ±ade a la escena. */
  void duplicateSelected();
  /** @brief Elimina el actor seleccionado de la escena y destruye sus recursos. */
  void deleteSelected();
  /** @brief Copia los datos del actor seleccionado al clipboard interno. */
  void copySelected();
  /** @brief Crea un actor nuevo a partir de los datos del clipboard. */
  void pasteClipboard();
  /** @brief Exporta el actor seleccionado como archivo prefab binario. */
  void savePrefabSelected();
  /** @brief Importa un prefab binario y lo instancia en la escena. */
  void loadPrefab();

  // -------------------------------------------------------------------------
  // Recursos de modelos importados dinÃ¡micamente
  // -------------------------------------------------------------------------
  /** @brief Modelos cargados por el Content Browser. */
  std::vector<std::unique_ptr<LoadedModel>> m_loadedModels;
  /** @brief Instancias de material creadas dinÃ¡micamente. */
  std::vector<std::unique_ptr<MaterialInstance>> m_dynamicMaterials;
  /** @brief Texturas cargadas dinÃ¡micamente. */
  std::vector<std::unique_ptr<Texture>> m_dynamicTextures;
  /** @brief Texturas de preview para el Content Browser. */
  std::vector<Texture> m_thumbTextures;
  /** @brief Datos de thumbnail por asset. */
  std::vector<AssetThumb> m_thumbnails;
  PendingModelImport m_pendingModelImport;
  bool m_queuedModelSpawnActive = false;
  std::string m_queuedModelSpawnPath;
  EU::Vector3 m_queuedModelSpawnPosition = EU::Vector3(0.0f, 0.0f, 0.0f);

  /**
   * @brief Carga un archivo de modelo (FBX/GLB/OBJ) y crea un actor con Ã©l en la escena.
   * @param modelPath Ruta completa al archivo de modelo.
   * @param spawnPosition
   * PosiciÃ³n mundial donde se crea el actor.
   * @return Shared pointer al actor
   * generado.
   */
  EU::TSharedPointer<Actor> loadModelActor(const std::string &modelPath,
                                           const EU::Vector3 &spawnPosition);

  /** @brief Declara o ejecuta getViewportMouseGroundPosition. */
  bool getViewportMouseGroundPosition(EU::Vector3 &outPosition) const;

  /**
   * @brief Busca y carga las texturas PBR de un modelo desde una carpeta.
   * @param lm     Referencia al LoadedModel donde se cargarÃ¡n las texturas.
   * @param folder Ruta de la carpeta donde buscar las texturas.
   */
  void loadModelTextures(LoadedModel &lm, const std::string &folder);

  /** @brief Genera miniaturas de las texturas importadas. */
  void buildTextureThumbnails();

  /**
   * @brief Calcula el AABB (axis-aligned bounding box) de un actor.
   * @param actor  Actor a consultar.
   * @param outMin Esquina mÃ­nima del AABB en espacio local.
   * @param outMax Esquina mÃ¡xima del AABB en espacio local.
   * @return true si el AABB pudo calcularse.
   */
  bool getActorAABB(const EU::TSharedPointer<Actor> &actor,
                    EU::Vector3 &outMin,
                    EU::Vector3 &outMax);

  // -------------------------------------------------------------------------
  // Texturas PBR del modelo principal (CyberGun)
  // -------------------------------------------------------------------------
  /** @brief SRV de la textura Albedo (color base). */
  Texture m_AlbedoSRV;
  /** @brief SRV de la textura de metalicidad. */
  Texture m_MetallicSRV;
  /** @brief SRV de la textura de rugosidad. */
  Texture m_RoughnessSRV;
  /** @brief SRV de la textura de oclusiÃ³n ambiental. */
  Texture m_AOSRV;
  /** @brief SRV del mapa de normales tangentes. */
  Texture m_NormalSRV;
  /** @brief SRV de la textura de emisiÃ³n. */
  Texture m_EmissiveSRV;
  /** @brief Textura blanca 1x1 usada como fallback PBR. */
  Texture m_defaultWhiteSRV;
  /** @brief Textura negra 1x1 usada como fallback PBR. */
  Texture m_defaultBlackSRV;
  /** @brief Normal plana 1x1 usada como fallback PBR. */
  Texture m_defaultFlatNormalSRV;

  // -------------------------------------------------------------------------
  // Recursos globales de escena
  // -------------------------------------------------------------------------
  /** @brief Textura cubemap del Skybox. */
  Texture m_skyboxTex;
  /** @brief CÃ¡mara fly-through del editor. */
  Camera m_camera;
  /** @brief Ãrbol de escena con jerarquÃ­a de entidades. */
  SceneGraph m_sceneGraph;

  // -------------------------------------------------------------------------
  // Actores de la escena
  // -------------------------------------------------------------------------
  /** @brief Lista de todos los actores de la escena. */
  std::vector<EU::TSharedPointer<Actor>> m_actors;
  /** @brief Actor del modelo principal (CyberGun). */
  EU::TSharedPointer<Actor> m_cyberGun;
  /** @brief Luz direccional principal (mÃ¡ximo una). */
  EU::TSharedPointer<Actor> m_directionalLightActor;

  /** @brief Modelo 3D activo cargado desde disco (puede ser nullptr). */
  Model3D *m_model = nullptr;

  // -------------------------------------------------------------------------
  // GUI del editor
  // -------------------------------------------------------------------------
  /** @brief Instancia de la GUI del editor (ImGui + paneles personalizados). */
  GUI m_gui;

  // -------------------------------------------------------------------------
  // Pipeline de renderizado
  // -------------------------------------------------------------------------
  /** @brief Skybox de fondo de la escena. */
  Skybox m_skybox;
  /** @brief Estado rasterizador estÃ¡ndar (fill solid, cull back). */
  RasterizerState m_defaultRasterizer;
  /** @brief Estado depth/stencil con escritura y prueba habilitadas. */
  DepthStencilState m_defaultDepthStencil;
  /** @brief Sampler anisÃ³tropo por defecto para todas las texturas. */
  SamplerState m_defaultSampler;

  /** @brief Malla GPU del actor CyberGun. */
  Mesh m_cyberGunRenderMesh;
  /** @brief Material PBR opaco base. */
  Material m_pbrMaterial;
  /** @brief Material PBR para superficies transparentes. */
  Material m_transparentPbrMaterial;
  /** @brief Instancia de material del CyberGun con sus texturas. */
  MaterialInstance m_cyberGunMaterial;

  Mesh m_editorGridMesh;
  Mesh m_editorSelectionMesh;
  Material m_editorGridMaterial;
  MaterialInstance m_editorGridMaterialInstance;
  MaterialInstance m_editorSelectionMaterialInstance;
  bool m_editorGridReady = false;
  bool m_editorSelectionReady = false;
  int m_editorSelectionCachedActorIndex = -1;
  EU::Vector3 m_editorSelectionCachedMin = EU::Vector3(0.0f, 0.0f, 0.0f);
  EU::Vector3 m_editorSelectionCachedMax = EU::Vector3(0.0f, 0.0f, 0.0f);

  /** @brief Pass de render del viewport flotante del editor. */
  EditorViewportPass m_editorViewportPass;
  /** @brief Orquestador Forward/Deferred renderer. */
  RenderPipeline m_renderPipeline;
  /** @brief Administrador de audio 3D basado en DirectXTK. */
  AudioSystem m_audioSystem;
  /** @brief Contenedor de datos de escena para un frame de render. */
  RenderScene m_renderScene;

  // -------------------------------------------------------------------------
  // Resize diferido del viewport del editor
  // -------------------------------------------------------------------------
  /** @brief true si hay un resize pendiente sin aplicar. */
  bool m_editorViewportResizePending = false;
  /** @brief Nuevo ancho pendiente del viewport. */
  unsigned int m_pendingViewportWidth = 1;
  /** @brief Nuevo alto pendiente del viewport. */
  unsigned int m_pendingViewportHeight = 1;

  /** @brief Ãšltimo ancho solicitado por el usuario. */
  unsigned int m_lastRequestedViewportWidth = 1;
  /** @brief Ãšltimo alto solicitado por el usuario. */
  unsigned int m_lastRequestedViewportHeight = 1;
  /** @brief Contador de frames consecutivos sin cambio de tamaÃ±o. */
  int m_viewportResizeStableFrames = 0;
};
