/**
 * @file GUI.h
 * @brief Declara la API de GUI dentro del subsistema GUI.
 * @ingroup gui
 */
#pragma once
#include "Prerequisites.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include <imgui_internal.h>
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include "ImGuizmo.h"
#include "Rendering/RenderTypes.h"
#include "Logger.h"

/** @brief Declara class Viewport. */
class
Viewport;
/** @brief Declara class Window. */
class
Window;
/** @brief Declara class Device. */
class
Device;
/** @brief Declara class DeviceContext. */
class
DeviceContext;
/** @brief Declara class Actor. */
class
Actor;
/** @brief Declara class Camera. */
class
Camera;

/** @brief Declara struct AssetThumb. */
struct
AssetThumb {
  std::string name;
  ID3D11ShaderResourceView* srv = nullptr;
};

/**
 * @class GUI
 * @brief Centraliza la interfaz del editor construida sobre ImGui e ImGuizmo.
 *
 * La clase expone paneles de viewport, depuracion de render, outliner e inspector.
 * Tambien recopila interacciones del usuario que despues consume `BaseApp`.
 */
class 
GUI {
public:
	GUI()  = default;
	~GUI() = default;

  /**
   * @brief Inicializa estado interno previo a la integracion con ImGui.
   */
  void 
  awake();

  /**
   * @brief Configura los backends de ImGui para Win32 y Direct3D 11.
   */
	void 
  init(Window& window, Device& device, DeviceContext& deviceContext);

  /**
   * @brief Actualiza el frame de ImGui y el estado de la ventana del editor.
   */
  void 
  update(Viewport& viewport, Window& window);
  
  /**
   * @brief Renderiza todos los paneles activos del editor.
   */
  void 
  render();
  
  /** @brief Declara o ejecuta destroy. */
  void 
  destroy();

  /** @brief Declara o ejecuta ToolBar. */
  void 
  ToolBar();

  
  /** @brief Declara o ejecuta closeApp. */
  void 
  closeApp();

  /** @brief Declara o ejecuta toolTipData. */
  void
  toolTipData();

  /** @brief Declara o ejecuta appleLiquidStyle. */
  void
  appleLiquidStyle(float opacity /*0..1f*/, ImVec4 accent /*=#0A84FF*/);

  /** @brief Declara o ejecuta vec3Control. */
  void
  vec3Control(const std::string& label,
              float* values,
              float resetValues = 0.0f,
              float columnWidth = 100.0f,
              bool displayAsDegrees = false);

  /** @brief Declara o ejecuta inspectorGeneral. */
  void
  inspectorGeneral(EU::TSharedPointer<Actor> actor);

  /** @brief Declara o ejecuta inspectorContainer. */
  void
  inspectorContainer(EU::TSharedPointer<Actor> actor);

  /** @brief Declara o ejecuta outliner. */
  void
  outliner(const std::vector<EU::TSharedPointer<Actor>>& actors);

  /** @brief Declara o ejecuta editTransform. */
  void 
  editTransform(Camera& cam, Window& window, EU::TSharedPointer<Actor> actor);

  /** @brief Declara o ejecuta drawGizmoToolbar. */
  void 
  drawGizmoToolbar();

  /** @brief Declara o ejecuta ToFloatArray. */
  void
  ToFloatArray(const XMMATRIX& mat, float* dest) {
    XMFLOAT4X4 temp;
    XMStoreFloat4x4(&temp, mat);
    memcpy(dest, &temp, sizeof(float) * 16);
  }

  /** @brief Declara o ejecuta drawStudioTopRibbon. */
  void
  drawStudioTopRibbon();

  /** @brief Declara o ejecuta drawViewportPanel. */
  void
  drawViewportPanel(ID3D11ShaderResourceView* viewportSRV);

  /** @brief Declara o ejecuta drawRenderDebugPanel. */
  void
  drawRenderDebugPanel(ID3D11ShaderResourceView* preShadowSRV,
                            ID3D11ShaderResourceView* finalViewportSRV,
                            ID3D11ShaderResourceView* shadowMapSRV);

  /** @brief Declara o ejecuta drawGBufferDebugPanel. */
  void
  drawGBufferDebugPanel(ID3D11ShaderResourceView* albedoMetallicSRV,
                             ID3D11ShaderResourceView* normalRoughnessSRV,
                             ID3D11ShaderResourceView* worldAoSRV,
                             ID3D11ShaderResourceView* emissiveAlphaSRV);

  /** @brief Declara o ejecuta drawMaterialSRVDebugPanel. */
  void
  drawMaterialSRVDebugPanel(ID3D11ShaderResourceView* albedoSRV,
                                 ID3D11ShaderResourceView* normalSRV,
                                 ID3D11ShaderResourceView* metallicSRV,
                                 ID3D11ShaderResourceView* roughnessSRV,
                                 ID3D11ShaderResourceView* aoSRV);

  /** @brief Declara o ejecuta drawLightingPanel. */
  void
  drawLightingPanel(float* lightDir, float* lightColor);
  /** @brief Dibuja los controles globales del sistema de audio. */
  void
  drawAudioPanel();
  /** @brief Declara o ejecuta drawStatsPanel. */
  void
  drawStatsPanel(float deltaTime, unsigned int drawCalls,
    unsigned int submittedObjects, unsigned int visibleObjects,
    unsigned int culledObjects, unsigned int octreeNodes);
  /** @brief Declara o ejecuta drawConsolePanel. */
  void
  drawConsolePanel();
  /** @brief Declara o ejecuta drawTexturePreview. */
  void
  drawTexturePreview();
  /** @brief Declara o ejecuta drawContentBrowser. */
  void
  drawContentBrowser(const std::vector<AssetThumb>& textureThumbs);
  /** @brief Declara o ejecuta drawSelectionOutline. */
  void
  drawSelectionOutline(
    Camera& cam,
    const EU::Vector3& localMin,
    const EU::Vector3& localMax,
    const XMMATRIX& world
  );
  /** @brief Declara o ejecuta drawViewportGrid. */
  void
  drawViewportGrid(Camera& cam);

  /** @brief Declara o ejecuta drawToolboxPanel. */
  void
  drawToolboxPanel();

  /** @brief Declara o ejecuta drawEditorDockspace. */
  void
  drawEditorDockspace();

  /** @brief Declara o ejecuta shouldShowOutliner. */
  bool
  shouldShowOutliner() const { return m_showOutliner; }
  /** @brief Declara o ejecuta shouldShowInspector. */
  bool
  shouldShowInspector() const { return m_showInspector; }
  /** @brief Declara o ejecuta shouldShowToolbox. */
  bool
  shouldShowToolbox() const { return m_showToolbox; }
  /** @brief Declara o ejecuta shouldShowRenderDebug. */
  bool
  shouldShowRenderDebug() const { return m_showRenderDebug; }
  /** @brief Declara o ejecuta shouldShowGBufferDebug. */
  bool
  shouldShowGBufferDebug() const { return m_showGBufferDebug; }
  /** @brief Indica si el panel de controles de audio está visible. */
  bool
  shouldShowAudioPanel() const { return m_showAudioPanel; }

  /**
   * @brief Consume de forma atomica la solicitud de guardado emitida desde la UI.
   * @return `true` una sola vez por peticion de guardado.
   */
  bool
  consumeSaveSceneRequest() {
    const bool requested = m_requestSaveScene;
    m_requestSaveScene = false;
    return requested;
  }

  /** @brief Consume la solicitud de crear un nivel vacío. */
  bool
  consumeNewSceneRequest() {
    const bool requested = m_requestNewScene;
    m_requestNewScene = false;
    return requested;
  }

  /** @brief Consume la ruta de nivel elegida mediante Open Level. */
  bool
  consumeOpenSceneRequest(std::string& outPath) {
    if (!m_requestOpenScene) return false;
    outPath = m_openScenePath;
    m_openScenePath.clear();
    m_requestOpenScene = false;
    return true;
  }

  /**
   * @brief Consume la solicitud de entrada al modo Play.
   * @return true una sola vez después de pulsar el botón Play.
   */
  bool
  consumePlayRequest() {
    const bool requested = m_requestPlay;
    m_requestPlay = false;
    return requested;
  }

  /**
   * @brief Consume la solicitud de detener el modo Play.
   * @return true una sola vez después de pulsar el botón Stop.
   */
  bool
  consumeStopRequest() {
    const bool requested = m_requestStop;
    m_requestStop = false;
    return requested;
  }

  /**
   * @brief Actualiza el estado visual de los controles Play y Stop.
   * @param isPlaying true mientras el motor ejecuta comportamientos runtime.
   */
  void
  setRuntimePlaying(bool isPlaying) {
    m_isRuntimePlaying = isPlaying;
  }

  /**
   * @brief Consume el último cambio de volumen maestro solicitado.
   * @param outVolume Recibe el nuevo volumen lineal entre 0.0 y 1.0.
   * @return true si el usuario cambió el control desde el último frame.
   */
  bool
  consumeAudioMasterVolumeChange(float& outVolume) {
    if (!m_audioMasterVolumeChanged) {
      return false;
    }

    outVolume = m_audioMasterVolume;
    m_audioMasterVolumeChanged = false;
    return true;
  }

  /** @brief Consume la solicitud de pausar todas las fuentes de audio. */
  bool
  consumeAudioPauseRequest() {
    const bool requested = m_audioPauseRequested;
    m_audioPauseRequested = false;
    return requested;
  }

  /** @brief Consume la solicitud de reanudar todas las fuentes de audio. */
  bool
  consumeAudioResumeRequest() {
    const bool requested = m_audioResumeRequested;
    m_audioResumeRequested = false;
    return requested;
  }

  /**
   * @brief Actualiza el estado visual de pausa del panel de audio.
   * @param paused true cuando AudioSystem está suspendido.
   */
  void
  setAudioPaused(bool paused) {
    m_audioPaused = paused;
  }

  /** @brief Declara o ejecuta consumeCreateLightRequest. */
  bool
  consumeCreateLightRequest(LightType& outType) {
    if (!m_requestCreateLight) {
      return false;
    }
    outType = m_requestedLightType;
    m_requestCreateLight = false;
    return true;
  }

  /** @brief Declara o ejecuta consumeResetRequest. */
  bool
  consumeResetRequest() { bool r = m_resetRequested; m_resetRequested = false; return r; }
  /** @brief Declara o ejecuta consumeFocusRequest. */
  bool
  consumeFocusRequest() { bool r = m_focusRequested; m_focusRequested = false; return r; }
  /** @brief Declara o ejecuta consumeFitRequest. */
  bool
  consumeFitRequest()   { bool r = m_fitRequested;   m_fitRequested = false; return r; }
  /** @brief Declara o ejecuta consumeUndoRequest. */
  bool
  consumeUndoRequest()  { bool r = m_undoRequested;  m_undoRequested = false; return r; }
  /** @brief Declara o ejecuta consumeRedoRequest. */
  bool
  consumeRedoRequest()  { bool r = m_redoRequested;  m_redoRequested = false; return r; }

private:

  bool checkboxValue = true;
  bool checkboxValue2 = false;
  std::vector<const char*> m_objectsNames;
  std::vector<const char*> m_tooltips;

  bool show_exit_popup = false; // Variable de estado para el popup
  bool m_requestSaveScene = false;
  bool m_requestNewScene = false;
  bool m_requestOpenScene = false;
  std::string m_openScenePath;
  bool m_requestPlay = false;
  bool m_requestStop = false;
  bool m_isRuntimePlaying = false;
  bool m_requestCreateLight = false;
  LightType m_requestedLightType = LightType::Directional;
  bool m_showOutliner = true;
  bool m_showInspector = true;
  bool m_showToolbox = false;
  bool m_showRenderDebug = false;
  bool m_showGBufferDebug = false;
  bool m_showMaterialSRVDebug = false;
  bool m_showAudioPanel = true;
  float m_audioMasterVolume = 1.0f;
  bool m_audioMasterVolumeChanged = false;
  bool m_audioPaused = false;
  bool m_audioPauseRequested = false;
  bool m_audioResumeRequested = false;
  ImDrawList* m_viewportDrawList = nullptr;
  bool m_viewportActive = false;

public:
  /** @brief Indica si el gizmo esta capturando entrada del usuario.. */
  bool m_isUsingGizmo = false;
  /** @brief Muestra el factor de sombra diferido en escala de grises.. */
  bool m_visualizeDeferredShadowFactor = false;
  /** @brief Modo de debug deferred (0=Final, 1=Shadow, etc.). */
  int  m_deferredDebugViewMode = 0;
  /** @brief Indice del actor seleccionado en el outliner.. */
  int selectedActorIndex = -1;
  /** @brief Posicion del panel de viewport en pantalla.. */
  ImVec2 m_viewportPos = ImVec2(0.0f, 0.0f);
  ImVec2 m_viewportSize = ImVec2(0.0f, 0.0f);///< Tamano actual del viewport del editor.
  /** @brief Indica si el cursor esta sobre el viewport.. */
  bool m_viewportHovered = false;
  /** @brief Indica si el viewport tiene foco de entrada.. */
  bool m_viewportFocused = false;

  // Logger / Console
  bool m_logShowInfo = true;
  bool m_logShowWarning = true;
  bool m_logShowError = true;
  bool m_logAutoScroll = true;
  ImGuiTextFilter m_logFilter;

  // Reset / Focus / Fit
  bool m_resetRequested = false;
  bool m_focusRequested = false;
  bool m_fitRequested = false;

  // Undo/Redo
  bool m_undoRequested = false;
  bool m_redoRequested = false;

  // Actor operations
  bool m_duplicateRequested = false;
  bool m_deleteRequested = false;
  bool m_copyRequested = false;
  bool m_pasteRequested = false;
  bool m_savePrefabRequested = false;
  bool m_loadPrefabRequested = false;

  // Content Browser spawn
  std::string m_assetSpawnPath;
  bool m_assetSpawnRequested = false;
  std::string m_assetDeletePath;
  bool m_assetDeleteRequested = false;
  std::string m_audioSpawnPath;
  bool m_audioSpawnRequested = false;
  
  // Texture drop
  std::string m_textureDropPath;
  bool m_textureDropRequested = false;
  
  // Import request
  bool m_importContentRequested = false;

  // Texture preview
  ID3D11ShaderResourceView* m_previewSRV = nullptr;
  std::string m_previewLabel;
  bool m_showPreview = false;

  // Grid / Snap
  bool  m_showGrid = true;
  float m_gridSize = 100.0f;
  bool  m_snapEnabled = false;
  float m_snapTranslate = 0.5f;
  float m_snapRotate = 15.0f;
  float m_snapScale = 0.1f;
};
