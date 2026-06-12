#pragma once
#include "Prerequisites.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include <imgui_internal.h>
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include "ImGuizmo.h"
#include <string>
#include <vector>

// Forward declarations de la arquitectura base
class Viewport;
class Window;
class Device;
class DeviceContext;
class Actor;
class Camera;

// Forward declarations de tus componentes
class Transform;
class LightComponent;
class MeshRendererComponent;

/**
 * @class GUI
 * @brief Owns the ImGui/ImGuizmo editor interface and viewport interaction state.
 *
 * GUI builds the editor docking layout, top ribbon, viewport panel, outliner, inspector,
 * transform controls, modal flows, and save-request signaling used by @c BaseApp.
 */
class
	GUI {
public:
    /** @brief Creates an inert editor UI object. */
    GUI() = default;
    /**
     * @brief Does not automatically shut down ImGui; call destroy() in application
     * shutdown.
     */
    ~GUI() = default;

    /** @brief Reserved pre-initialization hook for editor state. */
    void
    	awake();
    /** @brief Creates ImGui context and binds Win32/D3D11 backends. */
    void
    	init(Window& window, Device& device, DeviceContext& deviceContext);
    /**
     * @brief Starts a frame and builds the editor UI for the current viewport/window
     * state.
     */
    void
    	update(Viewport& viewport, Window& window);
    /** @brief Submits ImGui draw data to the D3D11 backend. */
    void
    	render();
    /** @brief Shuts down ImGui backends and releases editor UI resources. */
    void
    	destroy();

    /** @brief Legacy toolbar entry point retained for older editor code paths. */
    void
    	ToolBar();
    /** @brief Draws/processes the close-application confirmation flow. */
    void
    	closeApp();
    /** @brief Registers tooltip metadata used by editor widgets. */
    void
    	toolTipData();

    /** @brief Applies the editor visual style palette and spacing. */
    void
    	appleLiquidStyle(float opacity, ImVec4 accent);

    // Firma con todos los parámetros
    /**
     * @brief Draws a labeled three-float control with reset and optional degree display.
     */
    void
    	vec3Control(
    		const std::string& label,
    		float* values,
    		float resetValue = 0.0f,
    		float columnWidth = 100.0f,
    		bool displayAsDegrees = false
    	);

    /** @brief Draws general selected-actor data in the inspector. */
    void
    	inspectorGeneral(EU::TSharedPointer<Actor> actor);
    /** @brief Draws transform/component sections for the selected actor. */
    void
    	inspectorContainer(EU::TSharedPointer<Actor> actor);
    /** @brief Draws the scene outliner and updates actor selection state. */
    void
    	outliner(const std::vector<EU::TSharedPointer<Actor>>& actors);
    /**
     * @brief Applies ImGuizmo transform editing for the selected actor in the viewport.
     */
    void
    	editTransform(Camera& cam, Window& window, EU::TSharedPointer<Actor> actor);

    /** @brief Draws the transform-gizmo mode toolbar. */
    void
    	drawGizmoToolbar();

    /** @brief Stores a DirectX matrix into a contiguous float[16] array for ImGuizmo. */
    void
    	ToFloatArray(const XMMATRIX& mat, float* dest) {
        XMFLOAT4X4 temp;
        XMStoreFloat4x4(&temp, mat);
        memcpy(dest, &temp, sizeof(float) * 16);
    }

    /** @brief Draws the fixed top ribbon with editor actions and mode controls. */
    void
    	drawStudioTopRibbon();
    /** @brief Draws the editor viewport panel using the provided shader-resource view. */
    void
    	drawViewportPanel(ID3D11ShaderResourceView* viewportSRV);
    /** @brief Creates the editor docking host and layout region. */
    void
    	drawEditorDockspace();

    /** @brief Returns and clears a pending Ctrl+S/editor save request. */
    bool
    	consumeSaveSceneRequest() {
        const bool requested = m_requestSaveScene;
        m_requestSaveScene = false;
        return requested;
    }

private:
    /** @brief Tracks whether the exit confirmation popup should be visible. */
    bool show_exit_popup = false;
    /** @brief One-shot scene-save request consumed by @c BaseApp. */
    bool m_requestSaveScene = false;
    /** @brief Draw list for viewport overlays and gizmo helpers. */
    ImDrawList* m_viewportDrawList = nullptr;
    /** @brief True while the viewport panel is active for editor interaction. */
    bool m_viewportActive = false;

public:
    /** @brief True while ImGuizmo is actively manipulating an actor transform. */
    bool m_isUsingGizmo = false;
    /** @brief Index of the selected actor in the editor actor list, or -1 for none. */
    int selectedActorIndex = -1;
    /** @brief Top-left screen position of the editor viewport panel. */
    ImVec2 m_viewportPos = ImVec2(0.0f, 0.0f);
    /** @brief Current editor viewport panel size in pixels. */
    ImVec2 m_viewportSize = ImVec2(0.0f, 0.0f);
    /** @brief True when the mouse is hovering the editor viewport. */
    bool m_viewportHovered = false;
    /** @brief True when the editor viewport has keyboard/input focus. */
    bool m_viewportFocused = false;
};
