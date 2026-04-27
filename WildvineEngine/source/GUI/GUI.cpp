#include "EngineUtilities\GUI\GUI.h"
#include "Viewport.h"
#include "Window.h"
#include "Device.h"
#include "DeviceContext.h"
#include "MeshComponent.h"
#include "ECS\Actor.h"
#include "EngineUtilities\Utilities\Camera.h"
//#include "imgui_internal.h"

static ImGuizmo::OPERATION mCurrentGizmoOperation(ImGuizmo::TRANSLATE);

// --- IMPLEMENTACIÓN DE NUESTRO NUEVO ESTILO ---
void GUI::unrealDarkStyle() {
	ImGuiStyle& style = ImGui::GetStyle();
	ImVec4* colors = style.Colors;

	// Geometría más cuadrada, estilo motor profesional
	style.WindowRounding = 4.0f;
	style.ChildRounding = 4.0f;
	style.FrameRounding = 2.0f;
	style.PopupRounding = 4.0f;
	style.ScrollbarRounding = 2.0f;
	style.TabRounding = 4.0f;
	style.WindowBorderSize = 1.0f;
	style.FrameBorderSize = 1.0f; 

	style.WindowPadding = ImVec2(8, 8);
	style.FramePadding = ImVec2(6, 4);
	style.ItemSpacing = ImVec2(8, 4);

	// Colores base oscuros tipo UE / Motor AAA
	ImVec4 bgDark = ImVec4(0.05f, 0.05f, 0.05f, 1.0f);
	ImVec4 panelColor = ImVec4(0.11f, 0.11f, 0.11f, 1.0f);
	ImVec4 headerColor = ImVec4(0.15f, 0.15f, 0.15f, 1.0f);
	ImVec4 accentColor = ImVec4(0.0f, 0.45f, 0.85f, 1.0f); // Azul técnico
	ImVec4 textLight = ImVec4(0.90f, 0.90f, 0.90f, 1.0f);

	colors[ImGuiCol_Text] = textLight;
	colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.0f);
	
	colors[ImGuiCol_WindowBg] = bgDark;
	colors[ImGuiCol_ChildBg] = panelColor;
	colors[ImGuiCol_PopupBg] = panelColor;
	colors[ImGuiCol_Border] = ImVec4(0.15f, 0.15f, 0.15f, 1.0f);
	colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);

	colors[ImGuiCol_FrameBg] = bgDark;
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.20f, 0.20f, 0.20f, 1.0f);
	colors[ImGuiCol_FrameBgActive] = ImVec4(0.25f, 0.25f, 0.25f, 1.0f);

	colors[ImGuiCol_TitleBg] = panelColor;
	colors[ImGuiCol_TitleBgActive] = panelColor;
	colors[ImGuiCol_TitleBgCollapsed] = bgDark;

	colors[ImGuiCol_MenuBarBg] = panelColor;

	colors[ImGuiCol_ScrollbarBg] = bgDark;
	colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.25f, 0.25f, 0.25f, 1.0f);
	colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.35f, 0.35f, 0.35f, 1.0f);
	colors[ImGuiCol_ScrollbarGrabActive] = accentColor;

	colors[ImGuiCol_CheckMark] = accentColor;
	colors[ImGuiCol_SliderGrab] = accentColor;
	colors[ImGuiCol_SliderGrabActive] = ImVec4(0.0f, 0.55f, 0.95f, 1.0f);

	colors[ImGuiCol_Button] = headerColor;
	colors[ImGuiCol_ButtonHovered] = accentColor;
	colors[ImGuiCol_ButtonActive] = ImVec4(0.0f, 0.35f, 0.75f, 1.0f);

	colors[ImGuiCol_Header] = headerColor;
	colors[ImGuiCol_HeaderHovered] = ImVec4(0.25f, 0.25f, 0.25f, 1.0f);
	colors[ImGuiCol_HeaderActive] = accentColor;

	colors[ImGuiCol_Separator] = ImVec4(0.20f, 0.20f, 0.20f, 1.0f);
	colors[ImGuiCol_SeparatorHovered] = accentColor;
	colors[ImGuiCol_SeparatorActive] = ImVec4(0.0f, 0.55f, 0.95f, 1.0f);

	colors[ImGuiCol_Tab] = bgDark;
	colors[ImGuiCol_TabHovered] = headerColor;
	colors[ImGuiCol_TabActive] = panelColor;
	colors[ImGuiCol_TabUnfocused] = bgDark;
	colors[ImGuiCol_TabUnfocusedActive] = panelColor;
	
	colors[ImGuiCol_DockingPreview] = ImVec4(accentColor.x, accentColor.y, accentColor.z, 0.35f);
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

	// Comentamos el del profe y metemos el nuestro:
	// appleLiquidStyle(0.72f, ImVec4(0.0f, 0.515f, 1.0f, 1.0f));
    unrealDarkStyle();

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
GUI::vec3Control(const std::string& label, float* values, float resetValue, float columnWidth) {
	ImGuiIO& io = ImGui::GetIO();
	auto boldFont = io.Fonts->Fonts[0];

	ImGui::PushID(label.c_str());

	ImGui::Columns(2);
	ImGui::SetColumnWidth(0, columnWidth);
	ImGui::Text(label.c_str());
	ImGui::NextColumn();

	ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

	float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
	ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
	ImGui::PushFont(boldFont);
	if (ImGui::Button("X", buttonSize)) values[0] = resetValue;
	ImGui::PopFont();
	ImGui::PopStyleColor(3);

	ImGui::SameLine();
	ImGui::DragFloat("##X", &values[0], 0.1f, 0.0f, 0.0f, "%.2f");
	ImGui::PopItemWidth();
	ImGui::SameLine();

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
	ImGui::PushFont(boldFont);
	if (ImGui::Button("Y", buttonSize)) values[1] = resetValue;
	ImGui::PopFont();
	ImGui::PopStyleColor(3);

	ImGui::SameLine();
	ImGui::DragFloat("##Y", &values[1], 0.1f, 0.0f, 0.0f, "%.2f");
	ImGui::PopItemWidth();
	ImGui::SameLine();

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
	ImGui::PushFont(boldFont);
	if (ImGui::Button("Z", buttonSize)) values[2] = resetValue;
	ImGui::PopFont();
	ImGui::PopStyleColor(3);

	ImGui::SameLine();
	ImGui::DragFloat("##Z", &values[2], 0.1f, 0.0f, 0.0f, "%.2f");
	ImGui::PopItemWidth();

	ImGui::PopStyleVar();
	ImGui::Columns(1);

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
	// Checkbox para Static
	bool isStatic = false;
	ImGui::Checkbox("##Static", &isStatic);
	ImGui::SameLine();

	// Input text para el nombre del objeto
	char objectName[128] = "Cube";
	ImGui::SetNextItemWidth(ImGui::GetContentRegionAvailWidth() * 0.6f);
	ImGui::InputText("##ObjectName", &actor->getName()[0], IM_ARRAYSIZE(objectName));
	ImGui::SameLine();

	// Icono (este puede ser una imagen, aquí solo como ejemplo de botón)
	if (ImGui::Button("Icon")) {
		// Lógica del botón de icono aquí
	}

	// Separador horizontal
	ImGui::Separator();

	// Dropdown para Tag
	const char* tags[] = { "Untagged", "Player", "Enemy", "Environment" };
	static int currentTag = 0;
	ImGui::SetNextItemWidth(ImGui::GetContentRegionAvailWidth() * 0.5f);
	ImGui::Combo("Tag", &currentTag, tags, IM_ARRAYSIZE(tags));
	ImGui::SameLine();

	// Dropdown para Layer
	const char* layers[] = { "Default", "TransparentFX", "Ignore Raycast", "Water", "UI" };
	static int currentLayer = 0;
	ImGui::SetNextItemWidth(ImGui::GetContentRegionAvailWidth() * 0.5f);
	ImGui::Combo("Layer", &currentLayer, layers, IM_ARRAYSIZE(layers));

	ImGui::Separator();
	if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
		inspectorContainer(actor);
	}
	ImGui::End();
}

void
GUI::inspectorContainer(EU::TSharedPointer<Actor> actor) {
	//ImGui::Begin("Transform");
	// Draw the structure
	vec3Control("Position", const_cast<float*>(actor->getComponent<Transform>()->getPosition().data()));
	vec3Control("Rotation", const_cast<float*>(actor->getComponent<Transform>()->getRotation().data()));
	vec3Control("Scale", const_cast<float*>(actor->getComponent<Transform>()->getScale().data()));

	//ImGui::End();
}

void 
GUI::outliner(const std::vector<EU::TSharedPointer<Actor>>& actors) {
	ImGui::Begin("Hierarchy");

	// Barra de búsqueda
	static ImGuiTextFilter filter;
	filter.Draw("Search...", 180.0f); // Barra de búsqueda con ancho ajustable

	ImGui::Separator();

	// Recorrer y mostrar cada actor que pase el filtro de búsqueda
	for (int i = 0; i < actors.size(); ++i) {
		const auto& actor = actors[i];

		// Obtener el nombre del actor o asignar un nombre genérico
		std::string actorName = actor ? actor->getName() : "Actor";

		// Verificar si el actor pasa el filtro de búsqueda
		if (!filter.PassFilter(actorName.c_str())) {
			continue; // Saltar actores que no coincidan con el filtro
		}

		// Si el actor es seleccionable
		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
		if (selectedActorIndex == i)
			flags |= ImGuiTreeNodeFlags_Selected;

		// Crear un nodo de árbol para cada actor
		bool nodeOpen = ImGui::TreeNodeEx((void*)(intptr_t)i, flags, "%s", actorName.c_str());

		// Selección de actor
		if (ImGui::IsItemClicked()) {
			selectedActorIndex = i;
			// Aquí puedes llamar a alguna función para mostrar los detalles del actor en otra ventana
		}

		// Mostrar nodos hijos si el nodo está abierto
		if (nodeOpen) {
			ImGui::Text("Position: %.2f, %.2f, %.2f", 
				actor->getComponent<Transform>().get()->getPosition().x, 
				actor->getComponent<Transform>().get()->getPosition().y, 
				actor->getComponent<Transform>().get()->getPosition().z);
			ImGui::TreePop();
		}
	}

	ImGui::End();
}

void GUI::editTransform(Camera& cam, Window& window, EU::TSharedPointer<Actor> actor)
{
	if (!actor) return;

	static ImGuizmo::MODE mCurrentGizmoMode = ImGuizmo::WORLD;
	auto transform = actor->getComponent<Transform>();
	if (!transform) return;

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

	float mArr[16];
	ImGuizmo::RecomposeMatrixFromComponents(pos, rot, sca, mArr);

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
	ImGuizmo::SetGizmoSizeClipSpace(0.15f);
	ImGuizmo::AllowAxisFlip(false);
	ImGuizmo::SetRect(rectX, rectY, rectW, rectH);

	float snapValue = 25.0f;
	if (mCurrentGizmoOperation == ImGuizmo::ROTATE)    snapValue = 5.0f;
	if (mCurrentGizmoOperation == ImGuizmo::TRANSLATE) snapValue = 0.5f;

	float snap[3] = { snapValue, snapValue, snapValue };
	bool useSnap = ImGui::GetIO().KeyCtrl;

	bool canManipulate = m_viewportHovered || m_viewportActive || m_isUsingGizmo;

	if (canManipulate)
	{
		ImGuizmo::Manipulate(
			vArr,
			pArr,
			mCurrentGizmoOperation,
			mCurrentGizmoMode,
			mArr,
			nullptr,
			useSnap ? snap : nullptr
		);
	}

	m_isUsingGizmo = ImGuizmo::IsUsing();

	if (m_isUsingGizmo)
	{
		float newPos[3], newRot[3], newSca[3];
		ImGuizmo::DecomposeMatrixToComponents(mArr, newPos, newRot, newSca);

		transform->setPosition(EU::Vector3(newPos[0], newPos[1], newPos[2]));
		transform->setRotation(EU::Vector3(newRot[0], newRot[1], newRot[2]));
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

		static ImGuizmo::MODE mCurrentGizmoMode = ImGuizmo::WORLD;
		if (ImGui::Button(mCurrentGizmoMode == ImGuizmo::WORLD ? "Global" : "Local"))
			mCurrentGizmoMode = (mCurrentGizmoMode == ImGuizmo::WORLD) ? ImGuizmo::LOCAL : ImGuizmo::WORLD;
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
				ImGui::MenuItem("Save");
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
	// 2) RIBBON PRINCIPAL (CON NUEVOS COLORES)
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
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.08f, 0.08f, 0.08f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.12f, 0.12f, 0.12f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.0f, 0.45f, 0.85f, 1.0f));

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

		m_viewportPos = panelMin;
		m_viewportSize = panelSize;

		if (viewportSRV)
		{
			ImGui::Image((ImTextureID)viewportSRV, panelSize);
		}
		else
		{
			ImDrawList* drawList = ImGui::GetWindowDrawList();
			ImVec2 panelMax(panelMin.x + panelSize.x, panelMin.y + panelSize.y);

			drawList->AddRectFilled(panelMin, panelMax, IM_COL32(20, 20, 25, 255));
			drawList->AddText(
				ImVec2(panelMin.x + 12.0f, panelMin.y + 12.0f),
				IM_COL32(220, 220, 220, 255),
				"Viewport sin textura"
			);
		}


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