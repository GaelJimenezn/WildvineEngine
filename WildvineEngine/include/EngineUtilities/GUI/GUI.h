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

class Viewport;
class Window;
class Device;
class DeviceContext;
class Actor;
class Camera;

struct AssetThumb {
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
  
  void 
  destroy();

  void 
  ToolBar();

  
  void 
  closeApp();

  void
  toolTipData();

  void
  appleLiquidStyle(float opacity /*0..1f*/, ImVec4 accent /*=#0A84FF*/);

  void
  vec3Control(const std::string& label,
              float* values,
              float resetValues = 0.0f,
              float columnWidth = 100.0f,
              bool displayAsDegrees = false);

  void
  inspectorGeneral(EU::TSharedPointer<Actor> actor);

  void
  inspectorContainer(EU::TSharedPointer<Actor> actor);

  void
  outliner(const std::vector<EU::TSharedPointer<Actor>>& actors);

  void 
  editTransform(Camera& cam, Window& window, EU::TSharedPointer<Actor> actor);

  void 
  drawGizmoToolbar();

  void ToFloatArray(const XMMATRIX& mat, float* dest) {
    XMFLOAT4X4 temp;
    XMStoreFloat4x4(&temp, mat);
    memcpy(dest, &temp, sizeof(float) * 16);
  }

  void
  drawStudioTopRibbon();

  void drawViewportPanel(ID3D11ShaderResourceView* viewportSRV);

  void drawRenderDebugPanel(ID3D11ShaderResourceView* preShadowSRV,
                            ID3D11ShaderResourceView* finalViewportSRV,
                            ID3D11ShaderResourceView* shadowMapSRV);

  void drawGBufferDebugPanel(ID3D11ShaderResourceView* albedoMetallicSRV,
                             ID3D11ShaderResourceView* normalRoughnessSRV,
                             ID3D11ShaderResourceView* worldAoSRV,
                             ID3D11ShaderResourceView* emissiveAlphaSRV);

  void drawMaterialSRVDebugPanel(ID3D11ShaderResourceView* albedoSRV,
                                 ID3D11ShaderResourceView* normalSRV,
                                 ID3D11ShaderResourceView* metallicSRV,
                                 ID3D11ShaderResourceView* roughnessSRV,
                                 ID3D11ShaderResourceView* aoSRV);

  void drawLightingPanel(float* lightDir, float* lightColor);
  void drawStatsPanel(float deltaTime, unsigned int drawCalls);
  void drawConsolePanel();
  void drawTexturePreview();
  void drawContentBrowser(const std::vector<AssetThumb>& textureThumbs);
  void drawSelectionOutline(Camera& cam, const EU::Vector3& localMin, const EU::Vector3& localMax, const XMMATRIX& world);
  void drawViewportGrid(Camera& cam);

  void drawToolboxPanel();

  void drawEditorDockspace();

  bool shouldShowOutliner() const { return m_showOutliner; }
  bool shouldShowInspector() const { return m_showInspector; }
  bool shouldShowToolbox() const { return m_showToolbox; }
  bool shouldShowRenderDebug() const { return m_showRenderDebug; }
  bool shouldShowGBufferDebug() const { return m_showGBufferDebug; }

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

  bool
  consumeCreateLightRequest(LightType& outType) {
    if (!m_requestCreateLight) {
      return false;
    }
    outType = m_requestedLightType;
    m_requestCreateLight = false;
    return true;
  }

  bool consumeResetRequest() { bool r = m_resetRequested; m_resetRequested = false; return r; }
  bool consumeFocusRequest() { bool r = m_focusRequested; m_focusRequested = false; return r; }
  bool consumeFitRequest()   { bool r = m_fitRequested;   m_fitRequested = false; return r; }
  bool consumeUndoRequest()  { bool r = m_undoRequested;  m_undoRequested = false; return r; }
  bool consumeRedoRequest()  { bool r = m_redoRequested;  m_redoRequested = false; return r; }

private:

  bool checkboxValue = true;
  bool checkboxValue2 = false;
  std::vector<const char*> m_objectsNames;
  std::vector<const char*> m_tooltips;

  bool show_exit_popup = false; // Variable de estado para el popup
  bool m_requestSaveScene = false;
  bool m_requestCreateLight = false;
  LightType m_requestedLightType = LightType::Directional;
  bool m_showOutliner = true;
  bool m_showInspector = true;
  bool m_showToolbox = false;
  bool m_showRenderDebug = false;
  bool m_showGBufferDebug = false;
  bool m_showMaterialSRVDebug = false;
  ImDrawList* m_viewportDrawList = nullptr;
  bool m_viewportActive = false;

public:
  bool m_isUsingGizmo = false;               ///< Indica si el gizmo esta capturando entrada del usuario.
  bool m_visualizeDeferredShadowFactor = false; ///< Muestra el factor de sombra diferido en escala de grises.
  int  m_deferredDebugViewMode = 0;          ///< Modo de debug deferred (0=Final, 1=Shadow, etc.)
  int selectedActorIndex = -1;               ///< Indice del actor seleccionado en el outliner.
  ImVec2 m_viewportPos = ImVec2(0.0f, 0.0f); ///< Posicion del panel de viewport en pantalla.
  ImVec2 m_viewportSize = ImVec2(0.0f, 0.0f);///< Tamano actual del viewport del editor.
  bool m_viewportHovered = false;            ///< Indica si el cursor esta sobre el viewport.
  bool m_viewportFocused = false;            ///< Indica si el viewport tiene foco de entrada.

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
