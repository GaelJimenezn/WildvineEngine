#pragma once
#include "Prerequisites.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include <imgui_internal.h>
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include "ImGuizmo.h"

class Viewport;
class Window;
class Device;
class DeviceContext;
class Actor;
class Camera;

/**
 * @class GUI
 * @brief Gestiona y renderiza la interfaz gráfica de usuario del motor/editor.
 * * Utiliza la biblioteca Dear ImGui para crear ventanas, paneles, botones y controles
 * interactivos. También integra ImGuizmo para la manipulación de transformaciones (traslación,
 * rotación, escala) de los objetos dentro de la escena directamente desde el viewport.
 */
class 
GUI {
public:
  GUI()  = default;
  ~GUI() = default;

  // --- Ciclo de Vida ---
  
  /**
   * @brief Despierta o prepara el sistema GUI antes de la inicialización principal.
   */
  void 
  awake();

  /**
   * @brief Inicializa los contextos de ImGui y los enlaza con la ventana y DirectX 11.
   * @param window Referencia a la ventana de la aplicación.
   * @param device Referencia al dispositivo gráfico de DirectX.
   * @param deviceContext Referencia al contexto del dispositivo de DirectX.
   */
  void 
  init(Window& window, Device& device, DeviceContext& deviceContext);

  /**
   * @brief Actualiza la lógica de la interfaz en cada frame.
   * @param viewport Referencia al viewport principal.
   * @param window Referencia a la ventana de la aplicación.
   */
  void 
  update(Viewport& viewport, Window& window);
  
  /**
   * @brief Envía los comandos de dibujo de ImGui al pipeline gráfico para ser renderizados.
   */
  void 
  render();
  
  /**
   * @brief Libera los recursos de ImGui y apaga sus contextos.
   */
  void 
  destroy();

  // --- Acciones y Popups ---
  
  /**
   * @brief Dibuja la barra de herramientas principal del editor.
   */
  void 
  ToolBar();

  /**
   * @brief Muestra un popup o modal de confirmación para cerrar la aplicación.
   */
  void 
  closeApp();

  /**
   * @brief Gestiona y muestra la información en formato "tooltip" (ventanas emergentes al pasar el ratón).
   */
  void 
  toolTipData();

  // --- Estilos Visuales ---
  
  /**
   * @brief Aplica un estilo visual personalizado tipo "Apple Liquid" a los componentes de ImGui.
   * @param opacity Nivel de opacidad general de las ventanas.
   * @param accent Color de acento para elementos activos/seleccionados.
   */
  void
  appleLiquidStyle(float opacity, ImVec4 accent);

  // --- Controles de Usuario ---
  
  /**
   * @brief Crea un control personalizado para editar vectores de 3 dimensiones (X, Y, Z).
   * @param label Etiqueta descriptiva del control.
   * @param values Puntero a un arreglo de 3 flotantes que se modificarán.
   * @param resetValues Valor por defecto al que regresan los campos si se reinician.
   * @param columnWidth Ancho de la columna para alinear los controles.
   */
  void 
  vec3Control(const std::string& label,
                   float* values,
                   float resetValues = 0.0f,
                   float columnWidth = 100.0f);

  // --- Paneles del Editor ---
  
  /**
   * @brief Dibuja el panel del inspector general para el actor seleccionado (transformaciones básicas).
   * @param actor Puntero inteligente al actor actualmente seleccionado.
   */
  void 
  inspectorGeneral(EU::TSharedPointer<Actor> actor);

  /**
   * @brief Dibuja el panel de componentes para el actor seleccionado (scripts, colliders, mallas, etc.).
   * @param actor Puntero inteligente al actor actualmente seleccionado.
   */
  void 
  inspectorContainer(EU::TSharedPointer<Actor> actor);

  /**
   * @brief Dibuja la jerarquía de la escena (Outliner) listando todos los actores disponibles.
   * @param actors Vector que contiene referencias a todos los actores en la escena.
   */
  void 
  outliner(const std::vector<EU::TSharedPointer<Actor>>& actors);

  /**
   * @brief Dibuja la cinta (ribbon) superior del estudio/editor con menús desplegables.
   */
  void 
  drawStudioTopRibbon();

  /**
   * @brief Dibuja la ventana del Viewport donde se renderiza la escena del juego.
   * @param viewportSRV Shader Resource View que contiene la imagen renderizada de la escena.
   */
  void 
  drawViewportPanel(ID3D11ShaderResourceView* viewportSRV);

  /**
   * @brief Configura y dibuja el sistema de Docking (acoplamiento) para organizar los paneles del editor.
   */
  void 
  drawEditorDockspace();

  // --- Transformacion y Gizmos ---
  
  /**
   * @brief Utiliza ImGuizmo para mostrar y manejar los ejes de transformación gráfica sobre un actor.
   * @param cam Cámara actual del editor utilizada para proyectar los gizmos.
   * @param window Ventana de la aplicación para calcular ratios y entradas.
   * @param actor Actor que se va a transformar con el gizmo.
   */
  void 
  editTransform(Camera& cam, Window& window, 
                     EU::TSharedPointer<Actor> actor);

  /**
   * @brief Dibuja una barra de herramientas específica para seleccionar el modo de gizmo (Mover, Rotar, Escalar).
   */
  void 
  drawGizmoToolbar();

  /**
   * @brief Convierte una matriz XMMATRIX de DirectX Math a un arreglo lineal de 16 flotantes (requerido por ImGuizmo).
   * @param mat Matriz de transformación de entrada.
   * @param dest Puntero al arreglo destino donde se copiarán los 16 flotantes.
   */
  void
  ToFloatArray(const XMMATRIX& mat, float* dest) {
    XMFLOAT4X4 temp;
    XMStoreFloat4x4(&temp, mat);
    memcpy(dest, &temp, sizeof(float) * 16);
  }

private:
  bool checkboxValue = true;                                  /**< Valor de prueba o estado genérico para un checkbox. */
  bool checkboxValue2 = false;                                /**< Valor de prueba o estado genérico secundario para un checkbox. */
  std::vector<const char*> m_objectsNames;                    /**< Lista temporal o caché de nombres de objetos en escena. */
  std::vector<const char*> m_tooltips;                        /**< Lista de textos para tooltips a mostrar. */

  bool show_exit_popup = false;                               /**< Bandera que indica si el popup de salida está visible. */
  ImDrawList* m_viewportDrawList = nullptr;                   /**< Lista de dibujado de ImGui específica para superponer cosas en el viewport. */
  bool m_viewportActive = false;                              /**< Indica si el panel del viewport está actualmente activo. */

public:
  bool m_isUsingGizmo = false;                                /**< Indica si el usuario está interactuando (arrastrando) actualmente un gizmo. */
  int selectedActorIndex = -1;                                /**< Índice del actor seleccionado actualmente en el outliner (-1 si no hay selección). */
  ImVec2 m_viewportPos = ImVec2(0.0f, 0.0f);                  /**< Posición en pantalla de la esquina superior izquierda de la ventana del viewport. */
  ImVec2 m_viewportSize = ImVec2(0.0f, 0.0f);                 /**< Tamaño actual (ancho x alto) de la ventana del viewport de ImGui. */
  bool m_viewportHovered = false;                             /**< Indica si el cursor del ratón está sobre la ventana del viewport. */
  bool m_viewportFocused = false;                             /**< Indica si la ventana del viewport tiene el foco del teclado/ratón. */
};