#include "GUI/GUI.h"
#include "Viewport.h"
#include "Window.h"
#include "Device.h"
#include "DeviceContext.h"
#include "MeshComponent.h"
#include "ECS/Actor.h"
#include "ECS/Transform.h" // AÑADIDO PARA ECS
#include "ECS/MeshRendererComponent.h" // AÑADIDO PARA ECS
#include "ECS/LightComponent.h" // AÑADIDO PARA ECS
#include "EngineUtilities/Utilities/Camera.h"
#include <string>
#include <vector>

static ImGuizmo::OPERATION mCurrentGizmoOperation(ImGuizmo::TRANSLATE);

void GUI::awake() {}

void GUI::init(Window& window, Device& device, DeviceContext& deviceContext) {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

	ImGui::StyleColorsDark();

	ImGuiStyle& style = ImGui::GetStyle();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}

	// Estilo Unreal Engine 5 (Gris oscuro y Cian)
	appleLiquidStyle(1.0f, ImVec4(0.0f, 0.44f, 0.87f, 1.0f));

	ImGui_ImplWin32_Init(window.m_hWnd);
	ImGui_ImplDX11_Init(device.m_device, deviceContext.m_deviceContext);

	toolTipData();
	selectedActorIndex = 0;
}

void GUI::appleLiquidStyle(float opacity, ImVec4 accent) {
	ImGuiStyle& style = ImGui::GetStyle();
	ImVec4* colors = style.Colors;

	// Geometría más cuadrada y profesional, estilo Unreal Engine
	style.WindowRounding = 3.0f;
	style.ChildRounding = 3.0f;
	style.FrameRounding = 2.0f;
	style.PopupRounding = 3.0f;
	style.TabRounding = 2.0f;
	style.GrabRounding = 2.0f;
	style.ScrollbarRounding = 2.0f;
	
	style.WindowBorderSize = 1.0f;
	style.FrameBorderSize = 1.0f;
	style.PopupBorderSize = 1.0f;

	style.WindowPadding = ImVec2(10, 10);
	style.FramePadding = ImVec2(8, 6);
	style.ItemSpacing = ImVec2(6, 6);

	// Paleta de grises oscuros profundos (UE5 Theme)
	const ImVec4 bgPanel = ImVec4(0.07f, 0.07f, 0.08f, opacity);       // Fondo de ventanas principales y Ribbon
	const ImVec4 bgContent = ImVec4(0.11f, 0.11f, 0.12f, opacity);     // Fondo de childs/áreas de trabajo (Inspector/Outliner)
	const ImVec4 bgInteract = ImVec4(0.16f, 0.16f, 0.17f, opacity);    // Botones inactivos / frames
	
	// El cian eléctrico/azul característico de Unreal para selecciones
	const ImVec4 ueBlue = ImVec4(0.0f, 0.44f, 0.87f, 1.0f);            
	const ImVec4 ueBlueHover = ImVec4(0.15f, 0.55f, 0.95f, 1.0f);

	colors[ImGuiCol_Text] = ImVec4(0.85f, 0.85f, 0.85f, 1.00f);
	colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
	
	colors[ImGuiCol_WindowBg] = bgPanel;
	colors[ImGuiCol_ChildBg] = bgContent;
	colors[ImGuiCol_PopupBg] = bgPanel;
	
	colors[ImGuiCol_Border] = ImVec4(0.04f, 0.04f, 0.04f, 1.0f);
	colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);

	colors[ImGuiCol_FrameBg] = bgInteract;
	colors[ImGuiCol_FrameBgHovered] = ueBlue;
	colors[ImGuiCol_FrameBgActive] = ueBlueHover;

	colors[ImGuiCol_TitleBg] = bgPanel;
	colors[ImGuiCol_TitleBgActive] = bgPanel;
	colors[ImGuiCol_TitleBgCollapsed] = bgPanel;

	colors[ImGuiCol_MenuBarBg] = bgPanel;

	colors[ImGuiCol_Button] = bgInteract;
	colors[ImGuiCol_ButtonHovered] = ueBlue;
	colors[ImGuiCol_ButtonActive] = ueBlueHover;

	colors[ImGuiCol_Header] = bgInteract;
	colors[ImGuiCol_HeaderHovered] = ueBlueHover;
	colors[ImGuiCol_HeaderActive] = ueBlue;

	colors[ImGuiCol_Separator] = ImVec4(0.14f, 0.14f, 0.15f, 1.0f);
	colors[ImGuiCol_SeparatorHovered] = ueBlueHover;
	colors[ImGuiCol_SeparatorActive] = ueBlue;

	colors[ImGuiCol_Tab] = bgPanel;
	colors[ImGuiCol_TabHovered] = ueBlueHover;
	colors[ImGuiCol_TabActive] = ueBlue;
	colors[ImGuiCol_TabUnfocused] = bgPanel;
	colors[ImGuiCol_TabUnfocusedActive] = bgContent;

	colors[ImGuiCol_CheckMark] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	colors[ImGuiCol_SliderGrab] = ueBlue;
	colors[ImGuiCol_SliderGrabActive] = ueBlueHover;

	colors[ImGuiCol_DockingPreview] = ImVec4(ueBlue.x, ueBlue.y, ueBlue.z, 0.40f);
	colors[ImGuiCol_DockingEmptyBg] = bgPanel;
}

void GUI::update(Viewport& viewport, Window& window) {
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	ImGuizmo::BeginFrame();
	ImGuiIO& io = ImGui::GetIO();
	
	if (io.KeyCtrl && ImGui::IsKeyPressed('S', false)) {
		m_requestSaveScene = true;
	}
	
	ImGuizmo::SetOrthographic(false);

	drawStudioTopRibbon();
	drawEditorDockspace();
	closeApp();
	
	// ELIMINADO: drawGizmoToolbar(); para quitar la ventana molesta que sobraba en el viewport
}

void GUI::render() {
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	ImGuiIO& io = ImGui::GetIO();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}
}

void GUI::destroy() {
	if (ImGui::GetCurrentContext() == nullptr) return;
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}


void GUI::vec3Control(const std::string& label, float* values, float resetValue, float columnWidth, bool displayAsDegrees) {
	ImGuiIO& io = ImGui::GetIO();
	auto boldFont = io.Fonts->Fonts[0]; 

	ImGui::PushID(label.c_str());

	ImGui::Columns(2);
	ImGui::SetColumnWidth(0, columnWidth);
	ImGui::Text("%s", label.c_str());
	ImGui::NextColumn();

	ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 2.0f, 0.0f }); 

	float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
	ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

	// BOTÓN X (Rojo)
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.7f, 0.2f, 0.2f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.8f, 0.3f, 0.3f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.6f, 0.1f, 0.1f, 1.0f });
	ImGui::PushFont(boldFont);
	if (ImGui::Button("X", buttonSize)) values[0] = resetValue;
	ImGui::PopFont();
	ImGui::PopStyleColor(3);
	ImGui::SameLine();
	const float dragSpeed = displayAsDegrees ? 1.0f : 0.1f;
	const char* valueFormat = "%.2f";

	ImGui::DragFloat("##X", &values[0], dragSpeed, 0.0f, 0.0f, valueFormat);
	ImGui::PopItemWidth();
	ImGui::SameLine();

	// BOTÓN Y (Verde)
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.3f, 0.6f, 0.3f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.4f, 0.7f, 0.4f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.5f, 0.2f, 1.0f });
	ImGui::PushFont(boldFont);
	if (ImGui::Button("Y", buttonSize)) values[1] = resetValue;
	ImGui::PopFont();
	ImGui::PopStyleColor(3);
	ImGui::SameLine();
	ImGui::DragFloat("##Y", &values[1], dragSpeed, 0.0f, 0.0f, valueFormat);
	ImGui::PopItemWidth();
	ImGui::SameLine();

	// BOTÓN Z (Azul)
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.4f, 0.8f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.5f, 0.9f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.3f, 0.7f, 1.0f });
	ImGui::PushFont(boldFont);
	if (ImGui::Button("Z", buttonSize)) values[2] = resetValue;
	ImGui::PopFont();
	ImGui::PopStyleColor(3);
	ImGui::SameLine();
	ImGui::DragFloat("##Z", &values[2], dragSpeed, 0.0f, 0.0f, valueFormat);
	ImGui::PopItemWidth();

	ImGui::PopStyleVar();
	ImGui::Columns(1);
	ImGui::PopID();
}

void GUI::toolTipData() {}

void GUI::ToolBar() {}

void GUI::closeApp() {
	if (show_exit_popup) {
		ImGui::OpenPopup("Exit?");
		show_exit_popup = false;
	}
	ImVec2 center = ImGui::GetMainViewport()->GetCenter();
	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

	if (ImGui::BeginPopupModal("Exit?", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
		ImGui::Text("Estas a punto de salir de MinerEngine.\n¿Estas seguro?\n\n");
		ImGui::Separator();

		if (ImGui::Button("OK", ImVec2(120, 0))) {
			exit(0);
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


void GUI::inspectorGeneral(EU::TSharedPointer<Actor> actor) {
	ImGui::Begin("Inspector");

	if (!actor) {
		ImGui::TextDisabled("No actor selected");
		ImGui::End();
		return;
	}

	ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.11f, 0.11f, 0.12f, 1.0f)); // UE5 bgContent
	ImGui::BeginChild("HeaderRegion", ImVec2(0, 95), true);

	bool isStatic = false;
	ImGui::Checkbox("##Static", &isStatic);
	ImGui::SameLine();

	char objectName[128];
	strcpy_s(objectName, sizeof(objectName), actor->getName().c_str());
	ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 40.0f);
	if (ImGui::InputText("##ObjectName", objectName, IM_ARRAYSIZE(objectName))) {
		actor->setName(std::string(objectName));
	}

	ImGui::SameLine();
	ImGui::Button("Icon", ImVec2(30, 0));

	ImGui::Spacing();

	const char* tags[] = { "Untagged", "Player", "Enemy", "Environment" };
	static int currentTag = 0;
	ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.45f);
	ImGui::Combo("Tag", &currentTag, tags, IM_ARRAYSIZE(tags));
	ImGui::SameLine();

	const char* layers[] = { "Default", "TransparentFX", "Ignore Raycast", "Water", "UI" };
	static int currentLayer = 0;
	ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
	ImGui::Combo("Layer", &currentLayer, layers, IM_ARRAYSIZE(layers));

	ImGui::EndChild();
	ImGui::PopStyleColor();

	ImGui::Spacing();

	if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
		inspectorContainer(actor);
	}

	ImGui::Spacing();

	if (ImGui::CollapsingHeader("Rendering", ImGuiTreeNodeFlags_DefaultOpen)) {
		ImGui::Indent(10.0f);
        
        auto meshRenderer = actor->getComponent<MeshRendererComponent>();
        auto lightComp = actor->getComponent<LightComponent>();
        bool castShadow = false;

        if (!meshRenderer.isNull()) castShadow = meshRenderer->canCastShadow();
        else if (!lightComp.isNull()) castShadow = lightComp->canCastShadow();

		if (ImGui::Checkbox("Cast Shadows", &castShadow)) {
            if (!meshRenderer.isNull()) meshRenderer->setCastShadow(castShadow);
            if (!lightComp.isNull()) lightComp->setCastShadow(castShadow);
		}

		ImGui::TextDisabled("Configuraciones de luz y material...");
		ImGui::Unindent(10.0f);
	}

	ImGui::End();
}

void GUI::inspectorContainer(EU::TSharedPointer<Actor> actor) {
	if (!actor) return;
	auto transform = actor->getComponent<Transform>();
	if (!transform) return;

    EU::Vector3 pos = transform->getPosition();
    float p[3] = { pos.x, pos.y, pos.z };
    vec3Control("Position", p, 0.0f, 75.0f);
    transform->setPosition(EU::Vector3(p[0], p[1], p[2]));

    EU::Vector3 rot = transform->getRotation();
    float r[3] = { rot.x, rot.y, rot.z };
    vec3Control("Rotation", r, 0.0f, 75.0f, true);
    transform->setRotation(EU::Vector3(r[0], r[1], r[2]));

    EU::Vector3 sca = transform->getScale();
    float s[3] = { sca.x, sca.y, sca.z };
    vec3Control("Scale", s, 1.0f, 75.0f);
    transform->setScale(EU::Vector3(s[0], s[1], s[2]));
}


void GUI::outliner(const std::vector<EU::TSharedPointer<Actor>>& actors) {
	ImGui::Begin("Hierarchy");

	static ImGuiTextFilter filter;
	filter.Draw("Search...", ImGui::GetContentRegionAvail().x);

	ImGui::Separator();

	for (int i = 0; i < (int)actors.size(); ++i) {
		const auto& actor = actors[i];
		if (!actor) continue;

		std::string actorName = actor->getName();
		if (!filter.PassFilter(actorName.c_str())) continue;

		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;
		if (selectedActorIndex == i) flags |= ImGuiTreeNodeFlags_Selected;

		bool nodeOpen = ImGui::TreeNodeEx((void*)(intptr_t)i, flags, "%s", actorName.c_str());

		if (ImGui::IsItemClicked(ImGuiMouseButton_Left)) {
			selectedActorIndex = i;
		}

		if (ImGui::BeginPopupContextItem()) {
			selectedActorIndex = i; 
			ImGui::TextDisabled("Actor Options");
			ImGui::Separator();
			if (ImGui::MenuItem("Rename...")) {  }
			if (ImGui::MenuItem("Duplicate", "Ctrl+D")) {  }
			ImGui::Separator();
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.4f, 0.4f, 1.0f));
			if (ImGui::MenuItem("Delete", "Del")) {  }
			ImGui::PopStyleColor();
			ImGui::EndPopup();
		}

		if (nodeOpen) {
			auto transform = actor->getComponent<Transform>();
			if (transform) {
				ImGui::TextDisabled(" Pos: %.1f, %.1f, %.1f",
					transform->getPosition().x,
					transform->getPosition().y,
					transform->getPosition().z);
			}
			ImGui::TreePop();
		}
	}

	if (ImGui::BeginPopupContextWindow(nullptr, ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems)) {
		if (ImGui::MenuItem("Create Empty Actor")) {  }
		if (ImGui::MenuItem("Create 3D Object")) {  }
		ImGui::EndPopup();
	}

	ImGui::End();
}

void GUI::editTransform(Camera& cam, Window& window, EU::TSharedPointer<Actor> actor) {
	if (!actor) return;
	static ImGuizmo::MODE mCurrentGizmoMode = ImGuizmo::WORLD;
	auto transform = actor->getComponent<Transform>();
	if (!transform) return;

	float rectX = m_viewportPos.x;
	float rectY = m_viewportPos.y;
	float rectW = m_viewportSize.x;
	float rectH = m_viewportSize.y;

	if (rectW < 64.0f || rectH < 64.0f) {
		m_isUsingGizmo = false;
		return;
	}

    EU::Vector3 pos = transform->getPosition();
    EU::Vector3 rot = transform->getRotation();
    EU::Vector3 sca = transform->getScale();
    float posArr[3] = { pos.x, pos.y, pos.z };
    float rotArr[3] = { rot.x, rot.y, rot.z };
    float scaArr[3] = { sca.x, sca.y, sca.z };

	float mArr[16];
	ImGuizmo::RecomposeMatrixFromComponents(posArr, rotArr, scaArr, mArr);

	float vArr[16], pArr[16];
	ToFloatArray(cam.getView(), vArr);
	ToFloatArray(cam.getProj(), pArr);

	ImGuizmo::SetOrthographic(false);

	if (m_viewportDrawList) ImGuizmo::SetDrawlist(m_viewportDrawList);
	else ImGuizmo::SetDrawlist(ImGui::GetForegroundDrawList());

	ImGuizmo::SetID(0);
	ImGuizmo::SetGizmoSizeClipSpace(0.15f);
	ImGuizmo::AllowAxisFlip(false);
	ImGuizmo::SetRect(rectX, rectY, rectW, rectH);

	float snapValue = 25.0f;
	if (mCurrentGizmoOperation == ImGuizmo::ROTATE)    snapValue = 5.0f;
	if (mCurrentGizmoOperation == ImGuizmo::TRANSLATE) snapValue = 0.5f;

	float snap[3] = { snapValue, snapValue, snapValue };
	bool useSnap = ImGui::GetIO().KeyCtrl;
	bool canManipulate = m_viewportHovered || m_viewportActive || m_isUsingGizmo;

	if (canManipulate) {
		ImGuizmo::Manipulate(vArr, pArr, mCurrentGizmoOperation, mCurrentGizmoMode, mArr, nullptr, useSnap ? snap : nullptr);
	}

	m_isUsingGizmo = ImGuizmo::IsUsing();

	if (m_isUsingGizmo) {
		float newPos[3], newRot[3], newSca[3];
		ImGuizmo::DecomposeMatrixToComponents(mArr, newPos, newRot, newSca);
		transform->setPosition(EU::Vector3(newPos[0], newPos[1], newPos[2]));
		transform->setRotation(EU::Vector3(newRot[0], newRot[1], newRot[2]));
		transform->setScale(EU::Vector3(newSca[0], newSca[1], newSca[2]));
	}
}

// Se mantiene la declaración de la función para no romper el .h, pero ya no se llama en el Viewport
void GUI::drawGizmoToolbar() {
	ImGui::SetNextWindowBgAlpha(0.0f);
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

	if (ImGui::Begin("GizmoToolBar", nullptr, window_flags)) {
		auto buttonMode = [&](const char* label, ImGuizmo::OPERATION op, const char* shortcut) {
			bool isActive = (mCurrentGizmoOperation == op);
			if (isActive) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.44f, 0.87f, 1.0f)); // UE5 Blue
			if (ImGui::Button(label)) mCurrentGizmoOperation = op;
			if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s (%s)", label, shortcut);
			if (isActive) ImGui::PopStyleColor();
			ImGui::SameLine();
			};

		buttonMode("T", ImGuizmo::TRANSLATE, "W");
		buttonMode("R", ImGuizmo::ROTATE, "E");
		buttonMode("S", ImGuizmo::SCALE, "R");

		static ImGuizmo::MODE mCurrentGizmoMode = ImGuizmo::WORLD;
		if (ImGui::Button(mCurrentGizmoMode == ImGuizmo::WORLD ? "Global" : "Local")) {
			mCurrentGizmoMode = (mCurrentGizmoMode == ImGuizmo::WORLD) ? ImGuizmo::LOCAL : ImGuizmo::WORLD;
		}
	}
	ImGui::End();
	ImGui::PopStyleVar();
}

void GUI::drawStudioTopRibbon() {
	ImGuiViewport* viewport = ImGui::GetMainViewport();
	const float menuBarHeight = 24.0f;
	const float ribbonHeight = 72.0f;

	ImGui::SetNextWindowPos(viewport->Pos, ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, menuBarHeight), ImGuiCond_Always);

	ImGuiWindowFlags menuFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_MenuBar;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.0f, 4.0f));
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.07f, 0.07f, 0.08f, 1.0f)); // UE5 bgPanel

	if (ImGui::Begin("##StudioMenuBar", nullptr, menuFlags)) {
		if (ImGui::BeginMenuBar()) {
			if (ImGui::BeginMenu("File")) {
				ImGui::MenuItem("New Scene");
				ImGui::MenuItem("Open Scene...");
				ImGui::MenuItem("Save");
				ImGui::Separator();
				if (ImGui::MenuItem("Exit MinerEngine")) show_exit_popup = true;
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Edit")) {
				ImGui::MenuItem("Undo");
				ImGui::MenuItem("Redo");
				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}
	}
	ImGui::End();
	ImGui::PopStyleColor();
	ImGui::PopStyleVar(2);

	ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, viewport->Pos.y + menuBarHeight), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, ribbonHeight), ImGuiCond_Always);

	ImGuiWindowFlags ribbonFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.0f, 10.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8.0f, 6.0f));
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.11f, 0.11f, 0.12f, 1.0f)); // UE5 bgContent

	if (ImGui::Begin("##StudioRibbon", nullptr, ribbonFlags)) {
		auto ribbonButton = [&](const char* id, const char* topText, const char* bottomText, ImVec2 size, bool active = false) -> bool {
			if (active) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.44f, 0.87f, 1.0f)); // UE5 Blue
			bool pressed = ImGui::Button(id, size);
			ImVec2 min = ImGui::GetItemRectMin();
			ImVec2 max = ImGui::GetItemRectMax();
			ImDrawList* drawList = ImGui::GetWindowDrawList();
			ImVec2 topSize = ImGui::CalcTextSize(topText);
			ImVec2 bottomSize = ImGui::CalcTextSize(bottomText);
			float centerX = (min.x + max.x) * 0.5f;
			drawList->AddText(ImVec2(centerX - topSize.x * 0.5f, min.y + 10.0f), ImGui::GetColorU32(ImGuiCol_Text), topText);
			drawList->AddText(ImVec2(centerX - bottomSize.x * 0.5f, min.y + 34.0f), ImGui::GetColorU32(ImGuiCol_TextDisabled), bottomText);
			if (active) ImGui::PopStyleColor();
			return pressed;
			};

		auto separatorGroup = [&]() {
			ImGui::SameLine();
			ImGui::Dummy(ImVec2(6.0f, 1.0f));
			ImGui::SameLine();
			ImVec2 p = ImGui::GetCursorScreenPos();
			ImDrawList* draw = ImGui::GetWindowDrawList();
			draw->AddLine(ImVec2(p.x, p.y), ImVec2(p.x, p.y + 48.0f), IM_COL32(40, 40, 45, 255), 1.0f);
			ImGui::Dummy(ImVec2(8.0f, 48.0f));
			ImGui::SameLine();
			};

		const ImVec2 btnSize(72.0f, 52.0f);

		ribbonButton("##Select", "Select", "Cursor", btnSize, false); ImGui::SameLine();
		if (ribbonButton("##Move", "Move", "W", btnSize, mCurrentGizmoOperation == ImGuizmo::TRANSLATE)) mCurrentGizmoOperation = ImGuizmo::TRANSLATE; ImGui::SameLine();
		if (ribbonButton("##Rotate", "Rotate", "E", btnSize, mCurrentGizmoOperation == ImGuizmo::ROTATE)) mCurrentGizmoOperation = ImGuizmo::ROTATE; ImGui::SameLine();
		if (ribbonButton("##Scale", "Scale", "R", btnSize, mCurrentGizmoOperation == ImGuizmo::SCALE)) mCurrentGizmoOperation = ImGuizmo::SCALE;

		separatorGroup();

		ribbonButton("##Part", "3D Object", "Mesh", btnSize, false); ImGui::SameLine();
		ribbonButton("##Light", "Light", "Point", btnSize, false); ImGui::SameLine();
		ribbonButton("##Material", "Material", "Editor", btnSize, false);

		separatorGroup();
		ribbonButton("##Play", "Play", "Game", btnSize, false);
	}
	ImGui::End();

	ImGui::PopStyleColor(1);
	ImGui::PopStyleVar(3);
}

void GUI::drawViewportPanel(ID3D11ShaderResourceView* viewportSRV) {
	ImGuiWindowFlags flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoCollapse;
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

	if (ImGui::Begin("Viewport", nullptr, flags)) {
		m_viewportDrawList = ImGui::GetWindowDrawList();
		ImVec2 panelMin = ImGui::GetCursorScreenPos();
		ImVec2 panelSize = ImGui::GetContentRegionAvail();

		if (panelSize.x < 1.0f) panelSize.x = 1.0f;
		if (panelSize.y < 1.0f) panelSize.y = 1.0f;

		m_viewportPos = panelMin;
		m_viewportSize = panelSize;

		if (viewportSRV) {
			ImGui::Image((ImTextureID)viewportSRV, panelSize);
		}
		else {
			ImDrawList* drawList = ImGui::GetWindowDrawList();
			ImVec2 panelMax(panelMin.x + panelSize.x, panelMin.y + panelSize.y);
			drawList->AddRectFilled(panelMin, panelMax, IM_COL32(20, 20, 22, 255)); // Gris oscuro neutral
			drawList->AddText(ImVec2(panelMin.x + 12.0f, panelMin.y + 12.0f), IM_COL32(220, 220, 240, 255), "Viewport no renderizado");
		}

		m_viewportHovered = ImGui::IsItemHovered();
		m_viewportActive = ImGui::IsItemActive();
		m_viewportFocused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);
	}
	ImGui::End();
	ImGui::PopStyleVar();
}

void GUI::drawEditorDockspace() {
	ImGuiViewport* mainViewport = ImGui::GetMainViewport();
	const float topOffset = 96.0f;
	ImVec2 dockPos = ImVec2(mainViewport->Pos.x, mainViewport->Pos.y + topOffset);
	ImVec2 dockSize = ImVec2(mainViewport->Size.x, mainViewport->Size.y - topOffset);

	// ELIMINADO: ImGuiWindowFlags_MenuBar para quitar el rectángulo vacío en el Dockspace
	ImGuiWindowFlags window_flags = 
		ImGuiWindowFlags_NoTitleBar | 
		ImGuiWindowFlags_NoCollapse | 
		ImGuiWindowFlags_NoResize | 
		ImGuiWindowFlags_NoMove | 
		ImGuiWindowFlags_NoBringToFrontOnFocus | 
		ImGuiWindowFlags_NoNavFocus | 
		ImGuiWindowFlags_NoBackground | 
		ImGuiWindowFlags_NoDecoration | 
		ImGuiWindowFlags_NoSavedSettings;

	ImGui::SetNextWindowPos(dockPos, ImGuiCond_Always);
	ImGui::SetNextWindowSize(dockSize, ImGuiCond_Always);
	ImGui::SetNextWindowViewport(mainViewport->ID);

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

	ImGui::Begin("##MainEditorDockspace", nullptr, window_flags);
	ImGuiID dockspace_id = ImGui::GetID("##EditorDockspace");
	ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);
	ImGui::End();

	ImGui::PopStyleVar(3);
}