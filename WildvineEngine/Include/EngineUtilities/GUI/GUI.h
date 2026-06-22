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

class Viewport;
class Window;
class Device;
class DeviceContext;
class Actor;
class Camera;

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
  bool m_showRenderDebug = true;
  bool m_showGBufferDebug = true;
  ImDrawList* m_viewportDrawList = nullptr;
  bool m_viewportActive = false;

public:
  bool m_isUsingGizmo = false;               ///< Indica si el gizmo esta capturando entrada del usuario.
  bool m_visualizeDeferredShadowFactor = false; ///< Muestra el factor de sombra diferido en escala de grises.
  int selectedActorIndex = -1;               ///< Indice del actor seleccionado en el outliner.
  ImVec2 m_viewportPos = ImVec2(0.0f, 0.0f); ///< Posicion del panel de viewport en pantalla.
  ImVec2 m_viewportSize = ImVec2(0.0f, 0.0f);///< Tamano actual del viewport del editor.
  bool m_viewportHovered = false;            ///< Indica si el cursor esta sobre el viewport.
  bool m_viewportFocused = false;            ///< Indica si el viewport tiene foco de entrada.
};
