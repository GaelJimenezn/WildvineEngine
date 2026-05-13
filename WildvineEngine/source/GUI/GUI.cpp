#include "EngineUtilities\GUI\GUI.h"
#include "Viewport.h"
#include "Window.h"
#include "Device.h"
#include "DeviceContext.h"
#include "MeshComponent.h"
#include "ECS\Actor.h"
#include "ECS\LightComponent.h"
#include "ECS\MeshRendererComponent.h"
#include "Rendering\Mesh.h"
#include "Rendering\Material.h"
#include "Rendering\MaterialInstance.h"
#include "EngineUtilities\Utilities\Camera.h"
//#include "imgui_internal.h"
static ImGuizmo::OPERATION mCurrentGizmoOperation(ImGuizmo::TRANSLATE);
static ImGuizmo::MODE mCurrentGizmoMode(ImGuizmo::LOCAL);

namespace {
const char* GetLightTypeLabel(LightType type);

ImU32 AccentU32(const ImVec4& color) {
	return ImGui::ColorConvertFloat4ToU32(color);
}

float RadToDeg(float radians) {
	return XMConvertToDegrees(radians);
}

float DegToRad(float degrees) {
	return XMConvertToRadians(degrees);
}

void DrawInspectorPill(const char* text, const ImVec4& color) {
	ImGui::PushStyleColor(ImGuiCol_Button, color);
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, color);
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, color);
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 12.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10.0f, 4.0f));
	ImGui::Button(text);
	ImGui::PopStyleVar(2);
	ImGui::PopStyleColor(3);
}

bool BeginInspectorSection(const char* label, bool defaultOpen = true) {
	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_SpanAvailWidth;
	if (defaultOpen) {
		flags |= ImGuiTreeNodeFlags_DefaultOpen;
	}

	ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.16f, 0.18f, 0.22f, 0.95f));
	ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.20f, 0.23f, 0.28f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.22f, 0.26f, 0.32f, 1.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10.0f, 8.0f));
	const bool open = ImGui::CollapsingHeader(label, flags);
	ImGui::PopStyleVar();
	ImGui::PopStyleColor(3);
	return open;
}

bool BeginInspectorPropertyTable(const char* id, float firstColumnWidth = 132.0f) {
	if (!ImGui::BeginTable(id, 2, ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_BordersInnerV)) {
		return false;
	}

	ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, firstColumnWidth);
	ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);
	return true;
}

void DrawPropertyLabel(const char* label) {
	ImGui::TableNextRow();
	ImGui::TableSetColumnIndex(0);
	ImGui::AlignTextToFramePadding();
	ImGui::TextDisabled("%s", label);
	ImGui::TableSetColumnIndex(1);
	ImGui::SetNextItemWidth(-FLT_MIN);
}

void DrawPropertyValueText(const char* label, const char* value) {
	ImGui::TableNextRow();
	ImGui::TableSetColumnIndex(0);
	ImGui::AlignTextToFramePadding();
	ImGui::TextDisabled("%s", label);
	ImGui::TableSetColumnIndex(1);
	ImGui::TextUnformatted(value);
}

void DrawPropertyValueBool(const char* label, bool value) {
	DrawPropertyValueText(label, value ? "Yes" : "No");
}

void DrawPropertyToggle(const char* label, const char* id, bool* value) {
	ImGui::TableNextRow();
	ImGui::TableSetColumnIndex(0);
	ImGui::AlignTextToFramePadding();
	ImGui::TextDisabled("%s", label);
	ImGui::TableSetColumnIndex(1);
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);
	ImGui::Checkbox(id, value);
	ImGui::PopStyleVar();
}

const char* GetActorTypeLabel(EU::TSharedPointer<Actor> actor) {
	if (actor.isNull()) {
		return "Actor";
	}

	auto lightComponent = actor->getComponent<LightComponent>();
	if (!lightComponent.isNull()) {
		return GetLightTypeLabel(lightComponent->getLightData().type);
	}

	if (!actor->getComponent<MeshRendererComponent>().isNull()) {
		return "Static Mesh Actor";
	}

	if (!actor->getComponent<Transform>().isNull()) {
		return "Empty Actor";
	}

	return "Actor";
}

ImVec4 GetActorTypeColor(EU::TSharedPointer<Actor> actor) {
	if (actor.isNull()) {
		return ImVec4(0.45f, 0.47f, 0.52f, 1.0f);
	}

	auto lightComponent = actor->getComponent<LightComponent>();
	if (!lightComponent.isNull()) {
		return ImVec4(0.92f, 0.68f, 0.22f, 1.0f);
	}

	if (!actor->getComponent<MeshRendererComponent>().isNull()) {
		return ImVec4(0.24f, 0.50f, 0.92f, 1.0f);
	}

	return ImVec4(0.36f, 0.72f, 0.46f, 1.0f);
}

void DrawInspectorComponentChips(bool hasTransform, bool hasMeshRenderer, bool hasLight) {
	if (hasTransform) {
		DrawInspectorPill("Transform", ImVec4(0.18f, 0.50f, 0.28f, 1.0f));
	}
	if (hasMeshRenderer) {
		if (hasTransform) {
			ImGui::SameLine();
		}
		DrawInspectorPill("Renderer", ImVec4(0.22f, 0.42f, 0.76f, 1.0f));
	}
	if (hasLight) {
		if (hasTransform || hasMeshRenderer) {
			ImGui::SameLine();
		}
		DrawInspectorPill("Light", ImVec4(0.62f, 0.46f, 0.14f, 1.0f));
	}
}

const char* GetLightTypeLabel(LightType type) {
	switch (type) {
	case LightType::Directional: return "Directional";
	case LightType::Point: return "Point";
	case LightType::Spot: return "Spot";
	default: return "Unknown";
	}
}

const char* GetMaterialDomainLabel(MaterialDomain domain) {
	switch (domain) {
	case MaterialDomain::Opaque: return "Opaque";
	case MaterialDomain::Masked: return "Masked";
	case MaterialDomain::Transparent: return "Transparent";
	default: return "Unknown";
	}
}

const char* GetBlendModeLabel(BlendMode blendMode) {
	switch (blendMode) {
	case BlendMode::Opaque: return "Opaque";
	case BlendMode::Alpha: return "Alpha";
	case BlendMode::Additive: return "Additive";
	case BlendMode::PremultipliedAlpha: return "Premultiplied";
	default: return "Unknown";
	}
}
}
void 
GUI::init(Window& window, Device& device, DeviceContext& deviceContext) {
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
	ImGuiStyle& style = ImGui::GetStyle();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}

	appleLiquidStyle(0.72f, ImVec4(0.0f, 0.515f, 1.0f, 1.0f));

	// Setup Platform/Renderer backends
	ImGui_ImplWin32_Init(window.m_hWnd);
	ImGui_ImplDX11_Init(device.m_device, deviceContext.m_deviceContext);

	// Init ToolTips
	toolTipData();

	selectedActorIndex = 0;
}

void
GUI::update(Viewport& viewport, Window& window) {
	// Start the Dear ImGui frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	ImGuizmo::BeginFrame();
	ImGuiIO& io = ImGui::GetIO();
	if (io.KeyCtrl && ImGui::IsKeyPressed('S', false)) {
		m_requestSaveScene = true;
	}
	ImGuizmo::SetOrthographic(false);
	//ImGuizmo::SetRect(0, 0, (float)window.m_width, (float)window.m_height);

	// In Program always
	drawStudioTopRibbon();
	drawEditorDockspace();
	closeApp();
	drawGizmoToolbar();
}

void
GUI::render() {
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	ImGuiIO& io = ImGui::GetIO();
	// Update and Render additional Platform Windows
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}
}

void
GUI::destroy() {
	// Cleanup
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void 
GUI::vec3Control(const std::string& label, float* values, float resetValue, float columnWidth, bool displayAsDegrees) {
	ImGuiIO& io = ImGui::GetIO();
	auto boldFont = io.Fonts->Fonts[0];
	float displayValues[3] = { values[0], values[1], values[2] };
	if (displayAsDegrees) {
		displayValues[0] = RadToDeg(values[0]);
		displayValues[1] = RadToDeg(values[1]);
		displayValues[2] = RadToDeg(values[2]);
	}

	ImGui::PushID(label.c_str());
	if (!ImGui::BeginTable(("##Vec3Table" + label).c_str(), 2, ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_BordersInnerV)) {
		ImGui::PopID();
		return;
	}

	ImGui::TableSetupColumn("Label", ImGuiTableColumnFlags_WidthFixed, columnWidth);
	ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);
	ImGui::TableNextRow();
	ImGui::TableSetColumnIndex(0);
	ImGui::AlignTextToFramePadding();
	ImGui::TextDisabled("%s", label.c_str());
	ImGui::TableSetColumnIndex(1);
	ImGui::PushItemWidth(-1.0f);

	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 3.0f, 4.0f });
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
	float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
	ImVec2 buttonSize = { lineHeight, lineHeight };
	const float spacing = ImGui::GetStyle().ItemSpacing.x;
	const float availableWidth = ImGui::GetContentRegionAvail().x;
	const float dragWidth = (availableWidth - (buttonSize.x * 3.0f) - (spacing * 5.0f)) / 3.0f;
	const float safeDragWidth = dragWidth > 24.0f ? dragWidth : 24.0f;
	const float dragSpeed = displayAsDegrees ? 1.0f : 0.1f;
	const char* dragFormat = displayAsDegrees ? "%.1f deg" : "%.2f";

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
	ImGui::PushFont(boldFont);
	if (ImGui::Button("X", buttonSize)) {
		values[0] = resetValue;
		displayValues[0] = displayAsDegrees ? RadToDeg(resetValue) : resetValue;
	}
	ImGui::PopFont();
	ImGui::PopStyleColor(3);

	ImGui::SameLine();
	ImGui::SetNextItemWidth(safeDragWidth);
	if (ImGui::DragFloat("##X", &displayValues[0], dragSpeed, 0.0f, 0.0f, dragFormat)) {
		values[0] = displayAsDegrees ? DegToRad(displayValues[0]) : displayValues[0];
	}
	ImGui::SameLine();

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
	ImGui::PushFont(boldFont);
	if (ImGui::Button("Y", buttonSize)) {
		values[1] = resetValue;
		displayValues[1] = displayAsDegrees ? RadToDeg(resetValue) : resetValue;
	}
	ImGui::PopFont();
	ImGui::PopStyleColor(3);

	ImGui::SameLine();
	ImGui::SetNextItemWidth(safeDragWidth);
	if (ImGui::DragFloat("##Y", &displayValues[1], dragSpeed, 0.0f, 0.0f, dragFormat)) {
		values[1] = displayAsDegrees ? DegToRad(displayValues[1]) : displayValues[1];
	}
	ImGui::SameLine();

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
	ImGui::PushFont(boldFont);
	if (ImGui::Button("Z", buttonSize)) {
		values[2] = resetValue;
		displayValues[2] = displayAsDegrees ? RadToDeg(resetValue) : resetValue;
	}
	ImGui::PopFont();
	ImGui::PopStyleColor(3);

	ImGui::SameLine();
	ImGui::SetNextItemWidth(safeDragWidth);
	if (ImGui::DragFloat("##Z", &displayValues[2], dragSpeed, 0.0f, 0.0f, dragFormat)) {
		values[2] = displayAsDegrees ? DegToRad(displayValues[2]) : displayValues[2];
	}

	ImGui::PopStyleVar(2);
	ImGui::PopItemWidth();
	ImGui::EndTable();

	ImGui::PopID();
}

void 
GUI::toolTipData() {
}

void
GUI::appleLiquidStyle(float opacity, ImVec4 accent) {
	ImGuiStyle& style = ImGui::GetStyle();
	ImVec4* colors = style.Colors;

	// Geometría suave tipo macOS
	style.WindowRounding = 14.0f;
	style.ChildRounding = 14.0f;
	style.PopupRounding = 14.0f;
	style.FrameRounding = 10.0f;
	style.GrabRounding = 10.0f;
	style.ScrollbarRounding = 12.0f;
	style.TabRounding = 10.0f;

	style.WindowBorderSize = 1.0f;
	style.FrameBorderSize = 0.0f;
	style.PopupBorderSize = 1.0f;
	style.TabBorderSize = 0.0f;

	style.WindowPadding = ImVec2(14, 12);
	style.FramePadding = ImVec2(12, 8);
	style.ItemSpacing = ImVec2(8, 8);
	style.ItemInnerSpacing = ImVec2(8, 6);

	const float o = opacity;                 // opacidad del “cristal”
	const ImVec4 txt = ImVec4(1, 1, 1, 0.95f);     // texto claro
	const ImVec4 pane = ImVec4(0.16f, 0.16f, 0.18f, o); // panel “vidrioso” oscuro
	const ImVec4 paneHi = ImVec4(0.20f, 0.20f, 0.22f, o);
	const ImVec4 paneLo = ImVec4(0.13f, 0.13f, 0.15f, o * 0.85f);

	// Colores base “glass”
	colors[ImGuiCol_Text] = txt;
	colors[ImGuiCol_TextDisabled] = ImVec4(1, 1, 1, 0.45f);
	colors[ImGuiCol_WindowBg] = pane;     // importante: con alpha
	colors[ImGuiCol_ChildBg] = paneLo;
	colors[ImGuiCol_PopupBg] = paneHi;
	colors[ImGuiCol_Border] = ImVec4(1, 1, 1, 0.10f);
	colors[ImGuiCol_BorderShadow] = ImVec4(0, 0, 0, 0.0f);

	colors[ImGuiCol_FrameBg] = paneLo;
	colors[ImGuiCol_FrameBgHovered] = pane;
	colors[ImGuiCol_FrameBgActive] = paneHi;

	colors[ImGuiCol_TitleBg] = pane;
	colors[ImGuiCol_TitleBgActive] = paneHi;
	colors[ImGuiCol_TitleBgCollapsed] = paneLo;

	colors[ImGuiCol_MenuBarBg] = pane;

	colors[ImGuiCol_ScrollbarBg] = ImVec4(0, 0, 0, 0.0f);
	colors[ImGuiCol_ScrollbarGrab] = ImVec4(1, 1, 1, 0.10f);
	colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(1, 1, 1, 0.18f);
	colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(1, 1, 1, 0.26f);

	// Acento tipo macOS (azul #0A84FF por defecto)
	colors[ImGuiCol_CheckMark] = accent;
	colors[ImGuiCol_SliderGrab] = accent;
	colors[ImGuiCol_SliderGrabActive] = ImVec4(accent.x, accent.y, accent.z, 1.0f);

	colors[ImGuiCol_Button] = paneLo;
	colors[ImGuiCol_ButtonHovered] = pane;
	colors[ImGuiCol_ButtonActive] = paneHi;

	colors[ImGuiCol_Header] = paneLo;
	colors[ImGuiCol_HeaderHovered] = pane;
	colors[ImGuiCol_HeaderActive] = paneHi;

	colors[ImGuiCol_Separator] = ImVec4(1, 1, 1, 0.10f);
	colors[ImGuiCol_SeparatorHovered] = ImVec4(1, 1, 1, 0.18f);
	colors[ImGuiCol_SeparatorActive] = ImVec4(1, 1, 1, 0.30f);

	colors[ImGuiCol_Tab] = paneLo;
	colors[ImGuiCol_TabHovered] = pane;
	colors[ImGuiCol_TabActive] = paneHi;
	colors[ImGuiCol_TabUnfocused] = paneLo;
	colors[ImGuiCol_TabUnfocusedActive] = pane;

	colors[ImGuiCol_DockingPreview] = ImVec4(accent.x, accent.y, accent.z, 0.35f);
	colors[ImGuiCol_DockingEmptyBg] = ImVec4(0, 0, 0, 0.0f);

	colors[ImGuiCol_TableHeaderBg] = pane;
	colors[ImGuiCol_TableBorderStrong] = ImVec4(1, 1, 1, 0.08f);
	colors[ImGuiCol_TableBorderLight] = ImVec4(1, 1, 1, 0.04f);
	colors[ImGuiCol_TableRowBg] = ImVec4(1, 1, 1, 0.03f);
	colors[ImGuiCol_TableRowBgAlt] = ImVec4(1, 1, 1, 0.06f);

	colors[ImGuiCol_TextSelectedBg] = ImVec4(accent.x, accent.y, accent.z, 0.35f);
	colors[ImGuiCol_NavHighlight] = ImVec4(accent.x, accent.y, accent.z, 0.50f);
	colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1, 1, 1, 0.30f);
	colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0, 0, 0, 0.20f);
	colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0, 0, 0, 0.35f);
}


void
GUI::ToolBar() {
	if (ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("New")) {
				// Acción para "New"
			}
			if (ImGui::MenuItem("Open")) {
				// Acción para "Open"
			}
			if (ImGui::MenuItem("Save")) {
				// Acción para "Save"
			}
			if (ImGui::MenuItem("Exit")) {
				// Acción para "Exit"
				show_exit_popup = true;
				ImGui::OpenPopup("Exit?");
				//closeApp();
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Edit")) {
			if (ImGui::MenuItem("Undo")) {
				// Acción para "Undo"
			}
			if (ImGui::MenuItem("Redo")) {
				// Acción para "Redo"
			}
			if (ImGui::MenuItem("Cut")) {
				// Acción para "Cut"
			}
			if (ImGui::MenuItem("Copy")) {
				// Acción para "Copy"
			}
			if (ImGui::MenuItem("Paste")) {
				// Acción para "Paste"
			}
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Tools")) {
			if (ImGui::MenuItem("Options")) {
				// Acción para "Options"
			}
			if (ImGui::MenuItem("Settings")) {
				// Acción para "Settings"
			}
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}
}

void
GUI::closeApp() {
	if (show_exit_popup) {
		ImGui::OpenPopup("Exit?");
		show_exit_popup = false; // Reset the flag
	}
	// Centrar el popup en la pantalla
	ImVec2 center = ImGui::GetMainViewport()->GetCenter();
	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

	if (ImGui::BeginPopupModal("Exit?", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
		ImGui::Text("Estas a punto de salir de la aplicacion.\nEstas seguro?\n\n");
		ImGui::Separator();

		if (ImGui::Button("OK", ImVec2(120, 0))) {
			exit(0); // Salir de la aplicación
			ImGui::CloseCurrentPopup();
		}
		ImGui::SetItemDefaultFocus();
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(120, 0))) {
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}

void
GUI::inspectorGeneral(EU::TSharedPointer<Actor> actor) {
	ImGui::Begin("Inspector");
	if (actor.isNull()) {
		ImGui::Dummy(ImVec2(0.0f, 12.0f));
		ImGui::TextDisabled("No actor selected");
		ImGui::TextWrapped("Select an actor in the Hierarchy to inspect transforms, materials, lights and renderer data.");
		ImGui::End();
		return;
	}

	static char objectName[128] = {};
	static Actor* cachedActor = nullptr;
	if (cachedActor != actor.get()) {
		cachedActor = actor.get();
		strncpy_s(objectName, actor->getName().c_str(), _TRUNCATE);
	}

	auto meshRenderer = actor->getComponent<MeshRendererComponent>();
	auto lightComponent = actor->getComponent<LightComponent>();
	auto transform = actor->getComponent<Transform>();
	const bool hasMeshRenderer = !meshRenderer.isNull();
	const bool hasLightComponent = !lightComponent.isNull();
	const bool hasTransform = !transform.isNull();
	const ImVec4 accentColor = GetActorTypeColor(actor);
	const char* actorTypeLabel = GetActorTypeLabel(actor);

	ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 6.0f);
	ImGui::BeginChild("##InspectorHeader", ImVec2(0.0f, 104.0f), true);
	ImDrawList* drawList = ImGui::GetWindowDrawList();
	ImVec2 headerMin = ImGui::GetWindowPos();
	ImVec2 headerMax = ImVec2(headerMin.x + ImGui::GetWindowSize().x, headerMin.y + ImGui::GetWindowSize().y);
	drawList->AddRectFilled(headerMin, ImVec2(headerMax.x, headerMin.y + 4.0f), AccentU32(accentColor), 6.0f, ImDrawFlags_RoundCornersTop);

	ImGui::TextDisabled("Details");
	ImGui::Text("Selected Actor");
	ImGui::SameLine();
	ImGui::TextDisabled("| %s", actorTypeLabel);
	ImGui::SetNextItemWidth(-1.0f);
	if (ImGui::InputText("##ObjectName", objectName, IM_ARRAYSIZE(objectName))) {
		actor->setName(objectName);
	}
	ImGui::Spacing();
	DrawInspectorComponentChips(hasTransform, hasMeshRenderer, hasLightComponent);
	ImGui::Spacing();
	ImGui::TextDisabled("Component details, rendering data and editable properties");
	ImGui::EndChild();
	ImGui::PopStyleVar();

	ImGui::Spacing();
	if (BeginInspectorSection("Identity")) {
		if (BeginInspectorPropertyTable("##IdentityProperties")) {
			DrawPropertyValueText("Name", actor->getName().c_str());
			DrawPropertyValueText("Type", actorTypeLabel);
			DrawPropertyValueBool("Transform", hasTransform);
			DrawPropertyValueBool("Renderer", hasMeshRenderer);
			DrawPropertyValueBool("Light", hasLightComponent);
			ImGui::EndTable();
		}
	}

	ImGui::Spacing();
	if (hasTransform && BeginInspectorSection("Transform")) {
		inspectorContainer(actor);
	}

	if (hasMeshRenderer) {
		const std::vector<MaterialInstance*>& materialInstances = meshRenderer->getMaterialInstances();
		Mesh* mesh = meshRenderer->getMesh();

		if (BeginInspectorSection("Renderer")) {
			const int submeshCount = mesh ? static_cast<int>(mesh->getSubmeshes().size()) : 0;
			const int materialCount = static_cast<int>(materialInstances.size());
			if (BeginInspectorPropertyTable("##RendererProperties")) {
				bool isVisible = meshRenderer->isVisible();
				DrawPropertyToggle("Visible", "##RendererVisible", &isVisible);
				meshRenderer->setVisible(isVisible);

				bool castShadow = meshRenderer->canCastShadow();
				DrawPropertyToggle("Cast Shadow", "##RendererCastShadow", &castShadow);
				meshRenderer->setCastShadow(castShadow);

				char countBuffer[32] = {};
				sprintf_s(countBuffer, "%d", submeshCount);
				DrawPropertyValueText("Submeshes", countBuffer);

				sprintf_s(countBuffer, "%d", materialCount);
				DrawPropertyValueText("Material Slots", countBuffer);
				ImGui::EndTable();
			}
		}

		if (!materialInstances.empty() && BeginInspectorSection("Materials")) {
			for (size_t i = 0; i < materialInstances.size(); ++i) {
				MaterialInstance* materialInstance = materialInstances[i];
				if (!materialInstance) {
					continue;
				}

				MaterialParams& params = materialInstance->getParams();
				Material* material = materialInstance->getMaterial();
				std::string header = "Material Slot " + std::to_string(i);
				if (ImGui::TreeNodeEx(header.c_str(), ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth)) {
					if (material) {
						if (BeginInspectorPropertyTable(("##MaterialMeta" + std::to_string(i)).c_str())) {
							DrawPropertyValueText("Domain", GetMaterialDomainLabel(material->getDomain()));
							if (material->getDomain() == MaterialDomain::Transparent) {
								DrawPropertyValueText("Blend", GetBlendModeLabel(material->getBlendMode()));
							}
							ImGui::EndTable();
						}

						static const char* kMaterialDomains[] = { "Opaque", "Masked", "Transparent" };
						int currentDomain = static_cast<int>(material->getDomain());
						if (BeginInspectorPropertyTable(("##MaterialEditor" + std::to_string(i)).c_str())) {
							DrawPropertyLabel("Domain");
							if (ImGui::Combo(("##Domain" + std::to_string(i)).c_str(), &currentDomain, kMaterialDomains, IM_ARRAYSIZE(kMaterialDomains))) {
								material->setDomain(static_cast<MaterialDomain>(currentDomain));
							}

							if (material->getDomain() == MaterialDomain::Transparent) {
								static const char* kBlendModes[] = { "Opaque", "Alpha", "Additive", "Premultiplied" };
								int currentBlendMode = static_cast<int>(material->getBlendMode());
								DrawPropertyLabel("Blend Mode");
								if (ImGui::Combo(("##BlendMode" + std::to_string(i)).c_str(), &currentBlendMode, kBlendModes, IM_ARRAYSIZE(kBlendModes))) {
									material->setBlendMode(static_cast<BlendMode>(currentBlendMode));
								}
							}

							DrawPropertyLabel("Base Color");
							ImGui::ColorEdit4(("##BaseColor" + std::to_string(i)).c_str(), &params.baseColor.x);
							DrawPropertyLabel("Metallic");
							ImGui::SliderFloat(("##Metallic" + std::to_string(i)).c_str(), &params.metallic, 0.0f, 1.0f);
							DrawPropertyLabel("Roughness");
							ImGui::SliderFloat(("##Roughness" + std::to_string(i)).c_str(), &params.roughness, 0.0f, 1.0f);
							DrawPropertyLabel("Ambient Occlusion");
							ImGui::SliderFloat(("##AO" + std::to_string(i)).c_str(), &params.ao, 0.0f, 1.0f);
							DrawPropertyLabel("Normal Scale");
							ImGui::SliderFloat(("##NormalScale" + std::to_string(i)).c_str(), &params.normalScale, 0.0f, 2.0f);
							if (materialInstance->getEmissive()) {
								DrawPropertyLabel("Emissive Strength");
								ImGui::SliderFloat(("##EmissiveStrength" + std::to_string(i)).c_str(), &params.emissiveStrength, 0.0f, 8.0f);
							}
							if (material->getDomain() == MaterialDomain::Masked) {
								DrawPropertyLabel("Alpha Cutoff");
								ImGui::SliderFloat(("##AlphaCutoff" + std::to_string(i)).c_str(), &params.alphaCutoff, 0.0f, 1.0f);
							}
							ImGui::EndTable();
						}
					}
					ImGui::TreePop();
				}
			}
		}
	}

	if (hasLightComponent && BeginInspectorSection("Light")) {
		LightData& light = lightComponent->getLightData();
		if (BeginInspectorPropertyTable("##LightProperties")) {
			DrawPropertyValueText("Type", GetLightTypeLabel(light.type));
			bool castShadow = lightComponent->canCastShadow();
			DrawPropertyToggle("Cast Shadow", "##LightCastShadow", &castShadow);
			lightComponent->setCastShadow(castShadow);
			DrawPropertyLabel("Color");
			ImGui::ColorEdit3("##LightColor", &light.color.x);
			DrawPropertyLabel("Intensity");
			ImGui::SliderFloat("##LightIntensity", &light.intensity, 0.0f, 10.0f);
			if (light.type == LightType::Directional || light.type == LightType::Spot) {
				DrawPropertyLabel("Direction");
				ImGui::SliderFloat3("##LightDirection", &light.direction.x, -1.0f, 1.0f);
			}
			if (light.type == LightType::Point || light.type == LightType::Spot) {
				DrawPropertyLabel("Range");
				ImGui::SliderFloat("##LightRange", &light.range, 0.0f, 100.0f);
			}
			ImGui::EndTable();
		}
	}
	ImGui::End();
}

void
GUI::inspectorContainer(EU::TSharedPointer<Actor> actor) {
	//ImGui::Begin("Transform");
	// Draw the structure
	vec3Control("Position", const_cast<float*>(actor->getComponent<Transform>()->getPosition().data()), 0.0f, 78.0f, false);
	vec3Control("Rotation", const_cast<float*>(actor->getComponent<Transform>()->getRotation().data()), 0.0f, 78.0f, true);
	vec3Control("Scale", const_cast<float*>(actor->getComponent<Transform>()->getScale().data()), 1.0f, 78.0f, false);

	//ImGui::End();
}

void 
GUI::outliner(const std::vector<EU::TSharedPointer<Actor>>& actors) {
	ImGui::Begin("Hierarchy");

	ImGui::TextDisabled("Scene");
	static ImGuiTextFilter filter;
	filter.Draw("Search...", -1.0f);

	ImGui::Separator();

	if (selectedActorIndex >= static_cast<int>(actors.size())) {
		selectedActorIndex = actors.empty() ? -1 : static_cast<int>(actors.size()) - 1;
	}

	for (int i = 0; i < static_cast<int>(actors.size()); ++i) {
		const auto& actor = actors[i];
		std::string actorName = actor ? actor->getName() : "Actor";
		const char* actorTypeLabel = GetActorTypeLabel(actor);
		ImVec4 actorTypeColor = GetActorTypeColor(actor);
		std::string filterLabel = actorName + " " + actorTypeLabel;
		if (!filter.PassFilter(filterLabel.c_str())) {
			continue;
		}

		auto meshRenderer = actor ? actor->getComponent<MeshRendererComponent>() : EU::TSharedPointer<MeshRendererComponent>();
		auto lightComponent = actor ? actor->getComponent<LightComponent>() : EU::TSharedPointer<LightComponent>();
		const bool hasMeshRenderer = !meshRenderer.isNull();
		const bool hasLightComponent = !lightComponent.isNull();

		ImGui::PushID(i);
		const bool isSelected = (selectedActorIndex == i);
		if (isSelected) {
			ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.18f, 0.32f, 0.58f, 0.70f));
			ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.22f, 0.38f, 0.66f, 0.85f));
			ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.24f, 0.42f, 0.72f, 0.95f));
		}

		ImVec2 rowSize(ImGui::GetContentRegionAvail().x, 42.0f);
		if (ImGui::Selectable("##actorRow", isSelected, ImGuiSelectableFlags_SpanAvailWidth, rowSize)) {
			selectedActorIndex = i;
		}

		ImVec2 min = ImGui::GetItemRectMin();
		ImVec2 max = ImGui::GetItemRectMax();
		ImDrawList* drawList = ImGui::GetWindowDrawList();
		drawList->AddText(ImVec2(min.x + 12.0f, min.y + 6.0f), ImGui::GetColorU32(ImGuiCol_Text), actorName.c_str());
		drawList->AddText(ImVec2(min.x + 12.0f, min.y + 23.0f), AccentU32(actorTypeColor), actorTypeLabel);

		float badgeX = max.x - 84.0f;
		if (hasMeshRenderer) {
			drawList->AddRectFilled(ImVec2(badgeX, min.y + 12.0f), ImVec2(badgeX + 28.0f, min.y + 30.0f), IM_COL32(68, 118, 180, 180), 6.0f);
			drawList->AddText(ImVec2(badgeX + 9.0f, min.y + 14.0f), IM_COL32(240, 244, 255, 255), "M");
			badgeX += 40.0f;
		}
		if (hasLightComponent) {
			drawList->AddRectFilled(ImVec2(badgeX, min.y + 12.0f), ImVec2(badgeX + 28.0f, min.y + 30.0f), IM_COL32(180, 142, 52, 180), 6.0f);
			drawList->AddText(ImVec2(badgeX + 9.0f, min.y + 14.0f), IM_COL32(255, 248, 232, 255), "L");
		}

		if (isSelected) {
			ImGui::PopStyleColor(3);
		}
		ImGui::PopID();
	}

	ImGui::End();
}

void GUI::editTransform(Camera& cam, Window& window, EU::TSharedPointer<Actor> actor)
{
	if (actor.isNull()) return;
	auto transform = actor->getComponent<Transform>();
	if (transform.isNull()) return;

	float rectX = m_viewportPos.x;
	float rectY = m_viewportPos.y;
	float rectW = m_viewportSize.x;
	float rectH = m_viewportSize.y;

	if (rectW < 64.0f || rectH < 64.0f)
	{
		m_isUsingGizmo = false;
		return;
	}

	float* pos = const_cast<float*>(transform->getPosition().data());
	float* rot = const_cast<float*>(transform->getRotation().data());
	float* sca = const_cast<float*>(transform->getScale().data());
	float gizmoRotation[3] = {
		RadToDeg(rot[0]),
		RadToDeg(rot[1]),
		RadToDeg(rot[2])
	};

	float mArr[16];
	ImGuizmo::RecomposeMatrixFromComponents(pos, gizmoRotation, sca, mArr);

	float vArr[16], pArr[16];
	ToFloatArray(cam.getView(), vArr);
	ToFloatArray(cam.getProj(), pArr);

	ImGuizmo::SetOrthographic(false);

	// MUY IMPORTANTE: usar el drawlist del viewport, no el actual
	if (m_viewportDrawList)
		ImGuizmo::SetDrawlist(m_viewportDrawList);
	else
		ImGuizmo::SetDrawlist(ImGui::GetForegroundDrawList());

	ImGuizmo::SetID(0);
	ImGuizmo::SetGizmoSizeClipSpace(0.12f);
	ImGuizmo::AllowAxisFlip(true);
	ImGuizmo::SetRect(rectX, rectY, rectW, rectH);

	float snapValue = 25.0f;
	if (mCurrentGizmoOperation == ImGuizmo::ROTATE)    snapValue = 1.0f;
	if (mCurrentGizmoOperation == ImGuizmo::TRANSLATE) snapValue = 0.5f;

	float snap[3] = { snapValue, snapValue, snapValue };
	bool useSnap = ImGui::GetIO().KeyCtrl;
	ImGuizmo::MODE activeGizmoMode = mCurrentGizmoMode;
	if (mCurrentGizmoOperation == ImGuizmo::SCALE) {
		activeGizmoMode = ImGuizmo::LOCAL;
	}

	ImGuizmo::Manipulate(
		vArr,
		pArr,
		mCurrentGizmoOperation,
		activeGizmoMode,
		mArr,
		nullptr,
		useSnap ? snap : nullptr
	);

	m_isUsingGizmo = ImGuizmo::IsUsing();

	if (m_isUsingGizmo)
	{
		float newPos[3], newRot[3], newSca[3];
		ImGuizmo::DecomposeMatrixToComponents(mArr, newPos, newRot, newSca);

		transform->setPosition(EU::Vector3(newPos[0], newPos[1], newPos[2]));
		transform->setRotation(EU::Vector3(DegToRad(newRot[0]), DegToRad(newRot[1]), DegToRad(newRot[2])));
		transform->setScale(EU::Vector3(newSca[0], newSca[1], newSca[2]));
	}
}

void GUI::drawGizmoToolbar()
{
	//ImGui::SetNextWindowPos(ImVec2(300, 150), ImGuiCond_Always);
	ImGui::SetNextWindowBgAlpha(0.0f); // 0 = transparente total

	ImGuiWindowFlags window_flags =
		ImGuiWindowFlags_NoDecoration |
		ImGuiWindowFlags_AlwaysAutoResize |/*
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoSavedSettings |*/
		ImGuiWindowFlags_NoFocusOnAppearing |
		ImGuiWindowFlags_NoNav;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

	if (ImGui::Begin("GizmoToolBar", nullptr, window_flags))
	{
		auto buttonMode = [&](const char* label, ImGuizmo::OPERATION op, const char* shortcut)
			{
				bool isActive = (mCurrentGizmoOperation == op);
				if (isActive)
					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.4f, 0.8f, 1.0f));

				if (ImGui::Button(label))
					mCurrentGizmoOperation = op;

				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("%s (%s)", label, shortcut);

				if (isActive) ImGui::PopStyleColor();
				ImGui::SameLine();
			};

		buttonMode("T", ImGuizmo::TRANSLATE, "W");
		buttonMode("R", ImGuizmo::ROTATE, "E");
		buttonMode("S", ImGuizmo::SCALE, "R");

		const bool worldLocalSupported = (mCurrentGizmoOperation != ImGuizmo::SCALE);
		if (!worldLocalSupported) {
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		}
		if (ImGui::Button(mCurrentGizmoMode == ImGuizmo::WORLD ? "Global" : "Local"))
			mCurrentGizmoMode = (mCurrentGizmoMode == ImGuizmo::WORLD) ? ImGuizmo::LOCAL : ImGuizmo::WORLD;
		if (!worldLocalSupported) {
			ImGui::PopStyleVar();
			ImGui::PopItemFlag();
			if (ImGui::IsItemHovered()) {
				ImGui::SetTooltip("Scale uses local orientation. World/Local affects Move and Rotate.");
			}
		}
	}
	ImGui::End();

	ImGui::PopStyleVar();
}

void GUI::drawStudioTopRibbon()
{
	// =========================================================
	// CONFIGURACION GENERAL DE LA BARRA SUPERIOR TIPO STUDIO
	// =========================================================
	ImGuiViewport* viewport = ImGui::GetMainViewport();

	const float menuBarHeight = 24.0f;
	const float ribbonHeight = 72.0f;
	const float totalHeight = menuBarHeight + ribbonHeight;

	// -----------------------------
	// 1) MENU SUPERIOR
	// -----------------------------
	ImGui::SetNextWindowPos(viewport->Pos, ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, menuBarHeight), ImGuiCond_Always);

	ImGuiWindowFlags menuFlags =
		ImGuiWindowFlags_NoDecoration |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoScrollWithMouse |
		ImGuiWindowFlags_NoScrollbar |
		ImGuiWindowFlags_MenuBar;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.0f, 4.0f));
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.10f, 0.11f, 0.14f, 1.0f));

	if (ImGui::Begin("##StudioMenuBar", nullptr, menuFlags))
	{
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				ImGui::MenuItem("New Place");
				ImGui::MenuItem("Open Place");
				if (ImGui::MenuItem("Save", "Ctrl+S"))
				{
					m_requestSaveScene = true;
				}
				ImGui::Separator();
				if (ImGui::MenuItem("Exit"))
				{
					show_exit_popup = true;
				}
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Edit"))
			{
				ImGui::MenuItem("Undo");
				ImGui::MenuItem("Redo");
				ImGui::Separator();
				ImGui::MenuItem("Cut");
				ImGui::MenuItem("Copy");
				ImGui::MenuItem("Paste");
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("View"))
			{
				ImGui::MenuItem("Explorer");
				ImGui::MenuItem("Properties");
				ImGui::MenuItem("Toolbox");
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Plugins"))
			{
				ImGui::MenuItem("Manage Plugins");
				ImGui::MenuItem("Plugin Folder");
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Test"))
			{
				ImGui::MenuItem("Play");
				ImGui::MenuItem("Pause");
				ImGui::MenuItem("Stop");
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Window"))
			{
				ImGui::MenuItem("Reset Layout");
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Help"))
			{
				ImGui::MenuItem("Documentation");
				ImGui::MenuItem("About");
				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}
	}
	ImGui::End();

	ImGui::PopStyleColor();
	ImGui::PopStyleVar(2);

	// -----------------------------
	// 2) RIBBON PRINCIPAL
	// -----------------------------
	ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, viewport->Pos.y + menuBarHeight), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, ribbonHeight), ImGuiCond_Always);

	ImGuiWindowFlags ribbonFlags =
		ImGuiWindowFlags_NoDecoration |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoScrollWithMouse |
		ImGuiWindowFlags_NoScrollbar;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.0f, 10.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8.0f, 6.0f));
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.08f, 0.09f, 0.12f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.14f, 0.15f, 0.19f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.20f, 0.22f, 0.28f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.24f, 0.26f, 0.34f, 1.0f));

	if (ImGui::Begin("##StudioRibbon", nullptr, ribbonFlags))
	{
		auto ribbonButton = [&](const char* id, const char* topText, const char* bottomText, ImVec2 size, bool active = false) -> bool
			{
				if (active)
					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.24f, 0.34f, 0.58f, 1.0f));

				bool pressed = ImGui::Button(id, size);

				ImVec2 min = ImGui::GetItemRectMin();
				ImVec2 max = ImGui::GetItemRectMax();

				// Texto centrado manualmente dentro del boton
				ImDrawList* drawList = ImGui::GetWindowDrawList();

				ImVec2 topSize = ImGui::CalcTextSize(topText);
				ImVec2 bottomSize = ImGui::CalcTextSize(bottomText);

				float centerX = (min.x + max.x) * 0.5f;

				drawList->AddText(
					ImVec2(centerX - topSize.x * 0.5f, min.y + 10.0f),
					ImGui::GetColorU32(ImGuiCol_Text),
					topText
				);

				drawList->AddText(
					ImVec2(centerX - bottomSize.x * 0.5f, min.y + 34.0f),
					ImGui::GetColorU32(ImGuiCol_TextDisabled),
					bottomText
				);

				if (active)
					ImGui::PopStyleColor();

				return pressed;
			};

		auto separatorGroup = [&]()
			{
				ImGui::SameLine();
				ImGui::Dummy(ImVec2(6.0f, 1.0f));
				ImGui::SameLine();

				ImVec2 p = ImGui::GetCursorScreenPos();
				ImDrawList* draw = ImGui::GetWindowDrawList();
				draw->AddLine(
					ImVec2(p.x, p.y),
					ImVec2(p.x, p.y + 48.0f),
					IM_COL32(80, 80, 90, 255),
					1.0f
				);

				ImGui::Dummy(ImVec2(8.0f, 48.0f));
				ImGui::SameLine();
			};

		const ImVec2 btnSize(72.0f, 52.0f);

		// Herramientas de transformacion
		if (ribbonButton("##Select", "Select", "Cursor", btnSize, false))
		{
			// modo seleccion
		}
		ImGui::SameLine();

		if (ribbonButton("##Move", "Move", "W", btnSize, mCurrentGizmoOperation == ImGuizmo::TRANSLATE))
		{
			mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
		}
		ImGui::SameLine();

		if (ribbonButton("##Scale", "Scale", "R", btnSize, mCurrentGizmoOperation == ImGuizmo::SCALE))
		{
			mCurrentGizmoOperation = ImGuizmo::SCALE;
		}
		ImGui::SameLine();

		if (ribbonButton("##Rotate", "Rotate", "E", btnSize, mCurrentGizmoOperation == ImGuizmo::ROTATE))
		{
			mCurrentGizmoOperation = ImGuizmo::ROTATE;
		}
		ImGui::SameLine();

		if (ribbonButton("##Transform", "Transform", "Tool", btnSize, false))
		{
			// herramienta extra
		}

		separatorGroup();

		// Creacion / escena
		if (ribbonButton("##Part", "Part", "Mesh", btnSize, false))
		{
			// crear parte
		}
		ImGui::SameLine();

		if (ribbonButton("##Terrain", "Terrain", "Edit", btnSize, false))
		{
			// abrir terrain tools
		}
		ImGui::SameLine();

		if (ribbonButton("##Material", "Material", "Editor", btnSize, false))
		{
			// material editor
		}
		ImGui::SameLine();

		if (ribbonButton("##Color", "Color", "Picker", btnSize, false))
		{
			// color picker
		}

		separatorGroup();

		// Ventanas / paneles
		if (ribbonButton("##Explorer", "Explorer", "Panel", btnSize, false))
		{
			// toggle explorer
		}
		ImGui::SameLine();

		if (ribbonButton("##Properties", "Properties", "Panel", btnSize, false))
		{
			// toggle properties
		}
		ImGui::SameLine();

		if (ribbonButton("##Toolbox", "Toolbox", "Assets", btnSize, false))
		{
			// toggle toolbox
		}
	}
	ImGui::End();

	ImGui::PopStyleColor(4);
	ImGui::PopStyleVar(3);
}

void GUI::drawViewportPanel(ID3D11ShaderResourceView* viewportSRV)
{
	ImGuiWindowFlags flags =
		ImGuiWindowFlags_NoScrollbar |
		ImGuiWindowFlags_NoScrollWithMouse | 

		ImGuiWindowFlags_NoCollapse;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

	if (ImGui::Begin("Viewport", nullptr, flags))
	{
		m_viewportDrawList = ImGui::GetWindowDrawList();

		ImVec2 panelMin = ImGui::GetCursorScreenPos();
		ImVec2 panelSize = ImGui::GetContentRegionAvail();

		if (panelSize.x < 1.0f) panelSize.x = 1.0f;
		if (panelSize.y < 1.0f) panelSize.y = 1.0f;

		if (viewportSRV)
		{
			ImGui::Image((ImTextureID)viewportSRV, panelSize);
		}
		else
		{
			ImGui::InvisibleButton("##ViewportSurface", panelSize);
			ImVec2 itemMin = ImGui::GetItemRectMin();
			ImVec2 itemMax = ImGui::GetItemRectMax();
			ImDrawList* drawList = ImGui::GetWindowDrawList();

			drawList->AddRectFilled(itemMin, itemMax, IM_COL32(20, 20, 25, 255));
			drawList->AddText(
				ImVec2(itemMin.x + 12.0f, itemMin.y + 12.0f),
				IM_COL32(220, 220, 220, 255),
				"Viewport sin textura"
			);
		}

		ImVec2 itemMin = ImGui::GetItemRectMin();
		ImVec2 itemMax = ImGui::GetItemRectMax();
		m_viewportPos = itemMin;
		m_viewportSize = ImVec2(itemMax.x - itemMin.x, itemMax.y - itemMin.y);

		// IMPORTANTE: el hover/active del item imagen
		m_viewportHovered = ImGui::IsItemHovered();
		m_viewportActive = ImGui::IsItemActive();
		m_viewportFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);
	}
	ImGui::End();

	ImGui::PopStyleVar();
}

void GUI::drawEditorDockspace()
{
	ImGuiViewport* mainViewport = ImGui::GetMainViewport();

	// Debe coincidir con la altura total que ocupa tu ribbon superior
	const float topOffset = 96.0f; // 24 menu + 72 ribbon

	ImVec2 dockPos = ImVec2(mainViewport->Pos.x, mainViewport->Pos.y + topOffset);
	ImVec2 dockSize = ImVec2(mainViewport->Size.x, mainViewport->Size.y - topOffset);

	ImGuiWindowFlags window_flags =
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoBringToFrontOnFocus |
		ImGuiWindowFlags_NoNavFocus |
		ImGuiWindowFlags_NoBackground |
		ImGuiWindowFlags_NoDecoration |
		ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_MenuBar;

	ImGui::SetNextWindowPos(dockPos, ImGuiCond_Always);
	ImGui::SetNextWindowSize(dockSize, ImGuiCond_Always);
	ImGui::SetNextWindowViewport(mainViewport->ID);

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

	ImGui::Begin("##MainEditorDockspace", nullptr, window_flags);

	ImGuiID dockspace_id = ImGui::GetID("##EditorDockspace");
	ImGuiDockNodeFlags dockspace_flags =
		ImGuiDockNodeFlags_None |
		ImGuiDockNodeFlags_PassthruCentralNode;

	ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);

	ImGui::End();

	ImGui::PopStyleVar(3);
}


