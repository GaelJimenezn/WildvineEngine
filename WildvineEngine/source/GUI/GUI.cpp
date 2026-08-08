/**
 * @file GUI.cpp
 * @brief Implementa la logica de GUI dentro del subsistema GUI.
 * @ingroup gui
 */
#include "EngineUtilities\GUI\GUI.h"
#include <commdlg.h>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <wrl/client.h>
#include <vector>
#include "Viewport.h"
#include "Window.h"
#include "Device.h"
#include "DeviceContext.h"
#include "MeshComponent.h"
#include "ECS\Actor.h"
#include "ECS\LightComponent.h"
#include "ECS\AudioSourceComponent.h"
#include "ECS\MeshRendererComponent.h"
#include "Rendering\Mesh.h"
#include "Rendering\Material.h"
#include "Rendering\MaterialInstance.h"
#include "EngineUtilities\Utilities\Camera.h"
#include "imgui_internal.h"
static ImGuizmo::OPERATION mCurrentGizmoOperation(ImGuizmo::TRANSLATE);
static ImGuizmo::MODE mCurrentGizmoMode(ImGuizmo::LOCAL);
static bool mGizmoEnabled = true;
static bool mEditingAudioRange = false;

namespace {
const char* GetLightTypeLabel(LightType type);

namespace fs = std::filesystem;

bool IsModelFile(const fs::path& path) {
	std::string extension = path.extension().string();
	for (char& character : extension) {
		if (character >= 'A' && character <= 'Z') {
			character = static_cast<char>(character + 32);
		}
	}
	return extension == ".fbx" || extension == ".obj" ||
		extension == ".glb" || extension == ".gltf";
}

bool IsImageFile(const fs::path& path) {
	std::string extension = path.extension().string();
	for (char& character : extension) {
		if (character >= 'A' && character <= 'Z') {
			character = static_cast<char>(character + 32);
		}
	}
	return extension == ".png" || extension == ".jpg" ||
		extension == ".jpeg" || extension == ".tga" ||
		extension == ".dds";
}

bool IsAudioFile(const fs::path& path) {
	std::string extension = path.extension().string();
	for (char& character : extension) {
		if (character >= 'A' && character <= 'Z') {
			character = static_cast<char>(character + 32);
		}
	}
	return extension == ".wav" || extension == ".mp3" || extension == ".flac";
}

bool IsPlayableAudioFile(const fs::path& path) {
	std::string extension = path.extension().string();
	for (char& character : extension) {
		if (character >= 'A' && character <= 'Z') {
			character = static_cast<char>(character + 32);
		}
	}
	return extension == ".wav";
}

#pragma pack(push, 1)
struct WaveHeader {
	char riff[4] = { 'R', 'I', 'F', 'F' };
	uint32_t fileSize = 0;
	char wave[4] = { 'W', 'A', 'V', 'E' };
	char format[4] = { 'f', 'm', 't', ' ' };
	uint32_t formatSize = 16;
	uint16_t audioFormat = 1;
	uint16_t channels = 0;
	uint32_t sampleRate = 0;
	uint32_t byteRate = 0;
	uint16_t blockAlign = 0;
	uint16_t bitsPerSample = 0;
	char data[4] = { 'd', 'a', 't', 'a' };
	uint32_t dataSize = 0;
};
#pragma pack(pop)

bool ConvertAudioToWave(const fs::path& sourcePath, const fs::path& outputPath) {
	using Microsoft::WRL::ComPtr;
	static bool mediaFoundationReady = SUCCEEDED(MFStartup(MF_VERSION));
	if (!mediaFoundationReady) return false;

	ComPtr<IMFSourceReader> reader;
	if (FAILED(MFCreateSourceReaderFromURL(sourcePath.c_str(), nullptr,
		reader.GetAddressOf()))) return false;

	ComPtr<IMFMediaType> outputType;
	if (FAILED(MFCreateMediaType(outputType.GetAddressOf())) ||
		FAILED(outputType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio)) ||
		FAILED(outputType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM)) ||
		FAILED(reader->SetCurrentMediaType(
			static_cast<DWORD>(MF_SOURCE_READER_FIRST_AUDIO_STREAM), nullptr,
			outputType.Get()))) {
		return false;
	}

	ComPtr<IMFMediaType> currentType;
	if (FAILED(reader->GetCurrentMediaType(
		static_cast<DWORD>(MF_SOURCE_READER_FIRST_AUDIO_STREAM),
		currentType.GetAddressOf()))) {
		return false;
	}

	WaveHeader header;
	const UINT32 channels = MFGetAttributeUINT32(
		currentType.Get(), MF_MT_AUDIO_NUM_CHANNELS, 0);
	const UINT32 sampleRate = MFGetAttributeUINT32(
		currentType.Get(), MF_MT_AUDIO_SAMPLES_PER_SECOND, 0);
	const UINT32 bitsPerSample = MFGetAttributeUINT32(
		currentType.Get(), MF_MT_AUDIO_BITS_PER_SAMPLE, 0);
	if (channels == 0 || sampleRate == 0 || bitsPerSample == 0) {
		return false;
	}
	header.channels = static_cast<uint16_t>(channels);
	header.sampleRate = sampleRate;
	header.bitsPerSample = static_cast<uint16_t>(bitsPerSample);
	header.blockAlign = static_cast<uint16_t>(
		header.channels * (header.bitsPerSample / 8));
	header.byteRate = header.sampleRate * header.blockAlign;

	std::vector<uint8_t> samples;
	for (;;) {
		DWORD flags = 0;
		ComPtr<IMFSample> sample;
		if (FAILED(reader->ReadSample(
			static_cast<DWORD>(MF_SOURCE_READER_FIRST_AUDIO_STREAM), 0,
			nullptr, &flags, nullptr, sample.GetAddressOf()))) return false;
		if (sample) {
			ComPtr<IMFMediaBuffer> buffer;
			if (FAILED(sample->ConvertToContiguousBuffer(buffer.GetAddressOf()))) {
				return false;
			}
			BYTE* bytes = nullptr;
			DWORD length = 0;
			if (FAILED(buffer->Lock(&bytes, nullptr, &length))) return false;
			samples.insert(samples.end(), bytes, bytes + length);
			buffer->Unlock();
		}
		if (flags & MF_SOURCE_READERF_ENDOFSTREAM) break;
	}

	header.dataSize = static_cast<uint32_t>(samples.size());
	header.fileSize = header.dataSize + sizeof(WaveHeader) - 8;
	std::ofstream output(outputPath, std::ios::binary);
	if (!output.is_open()) return false;
	output.write(reinterpret_cast<const char*>(&header), sizeof(header));
	output.write(reinterpret_cast<const char*>(samples.data()), samples.size());
	return output.good();
}

std::string ToContentPath(const fs::path& path) {
	return path.generic_string();
}

bool ImportModelWithTextures(const fs::path& sourcePath) {
	const fs::path modelDirectory =
		fs::path("Assets") / "Models" / sourcePath.stem();
	const fs::path destinationModel = modelDirectory / sourcePath.filename();
	const fs::path sourceDirectory = sourcePath.parent_path();
	std::error_code error;
	fs::create_directories(modelDirectory / "Textures", error);
	if (error) return false;

	fs::copy_file(sourcePath, destinationModel,
		fs::copy_options::overwrite_existing, error);
	if (error) return false;

	for (const fs::directory_entry& entry :
		fs::recursive_directory_iterator(sourceDirectory, error)) {
		if (error) return false;
		if (!entry.is_regular_file() || !IsImageFile(entry.path())) continue;

		const fs::path destinationTexture =
			modelDirectory / "Textures" / entry.path().filename();
		fs::create_directories(destinationTexture.parent_path(), error);
		if (error) return false;
		fs::copy_file(entry.path(), destinationTexture,
			fs::copy_options::overwrite_existing, error);
		if (error) return false;
	}

	return true;
}

struct DebugTextureItem {
	const char* label;
	const char* channels;
	ID3D11ShaderResourceView* srv;
};

ImU32 AccentU32(const ImVec4& color) {
	return ImGui::ColorConvertFloat4ToU32(color);
}

void PushEditorPopupStyle() {
	ImGui::PushStyleVar(ImGuiStyleVar_PopupRounding, 6.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(12.0f, 10.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8.0f, 6.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8.0f, 5.0f));
	ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.075f, 0.080f, 0.095f, 0.98f));
	ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.16f, 0.32f, 0.72f, 0.85f));
	ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.42f, 0.16f, 0.86f, 0.95f));
}

void PopEditorPopupStyle() {
	ImGui::PopStyleColor(3);
	ImGui::PopStyleVar(4);
}

float RadToDeg(float radians) {
	return XMConvertToDegrees(radians);
}

float DegToRad(float degrees) {
	return XMConvertToRadians(degrees);
}

void DrawDebugTextureEntry(const DebugTextureItem& item, int index, int& selectedView, float thumbnailHeight) {
	ImGui::PushID(index);
	if (ImGui::Selectable(item.label, selectedView == index, 0, ImVec2(0.0f, 20.0f))) {
		selectedView = index;
	}

	if (item.channels != nullptr && item.channels[0] != '\0') {
		ImGui::TextDisabled("%s", item.channels);
	}

	if (item.srv) {
		const float thumbnailWidth = thumbnailHeight * 1.6f;
		ImGui::Image((ImTextureID)item.srv, ImVec2(thumbnailWidth, thumbnailHeight));
	}
	else {
		ImGui::Dummy(ImVec2(thumbnailHeight * 1.6f, thumbnailHeight));
		ImGui::SameLine(0.0f, 0.0f);
		ImGui::TextDisabled("Unavailable");
	}

	ImGui::PopID();
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

	ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.10f, 0.12f, 0.15f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.13f, 0.16f, 0.20f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.14f, 0.24f, 0.38f, 1.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8.0f, 6.0f));
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
	case LightType::Rect: return "Rect";
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

	appleLiquidStyle(0.76f, ImVec4(0.95f, 0.42f, 0.08f, 1.0f));

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
	(void)opacity;
	(void)accent;
	ImGuiStyle& style = ImGui::GetStyle();
	ImVec4* colors = style.Colors;

	style.WindowRounding = 0.0f;
	style.ChildRounding = 3.0f;
	style.PopupRounding = 3.0f;
	style.FrameRounding = 3.0f;
	style.GrabRounding = 3.0f;
	style.ScrollbarRounding = 3.0f;
	style.TabRounding = 3.0f;

	style.WindowBorderSize = 1.0f;
	style.FrameBorderSize = 1.0f;
	style.PopupBorderSize = 1.0f;
	style.TabBorderSize = 0.0f;

	style.WindowPadding = ImVec2(8, 8);
	style.FramePadding = ImVec2(8, 5);
	style.ItemSpacing = ImVec2(6, 6);
	style.ItemInnerSpacing = ImVec2(6, 4);
	style.ScrollbarSize = 12.0f;

	const ImVec4 bg0 = ImVec4(0.055f, 0.058f, 0.066f, 1.0f);
	const ImVec4 bg1 = ImVec4(0.075f, 0.079f, 0.090f, 1.0f);
	const ImVec4 bg2 = ImVec4(0.105f, 0.110f, 0.125f, 1.0f);
	const ImVec4 bg3 = ImVec4(0.145f, 0.153f, 0.172f, 1.0f);
	const ImVec4 blue = ImVec4(0.05f, 0.34f, 0.82f, 1.0f);
	const ImVec4 blueHi = ImVec4(0.07f, 0.58f, 1.00f, 1.0f);
	const ImVec4 purple = ImVec4(0.42f, 0.16f, 0.86f, 1.0f);
	const ImVec4 purpleHi = ImVec4(0.64f, 0.26f, 1.00f, 1.0f);
	const ImVec4 cyan = ImVec4(0.00f, 0.88f, 1.00f, 1.0f);

	colors[ImGuiCol_Text] = ImVec4(0.88f, 0.90f, 0.94f, 1.0f);
	colors[ImGuiCol_TextDisabled] = ImVec4(0.48f, 0.51f, 0.56f, 1.0f);
	colors[ImGuiCol_WindowBg] = bg1;
	colors[ImGuiCol_ChildBg] = bg0;
	colors[ImGuiCol_PopupBg] = bg2;
	colors[ImGuiCol_Border] = ImVec4(0.22f, 0.23f, 0.26f, 1.0f);
	colors[ImGuiCol_BorderShadow] = ImVec4(0, 0, 0, 0.0f);
	colors[ImGuiCol_FrameBg] = bg0;
	colors[ImGuiCol_FrameBgHovered] = bg2;
	colors[ImGuiCol_FrameBgActive] = bg3;
	colors[ImGuiCol_TitleBg] = bg0;
	colors[ImGuiCol_TitleBgActive] = bg2;
	colors[ImGuiCol_TitleBgCollapsed] = bg0;
	colors[ImGuiCol_MenuBarBg] = ImVec4(0.032f, 0.034f, 0.040f, 1.0f);
	colors[ImGuiCol_ScrollbarBg] = bg0;
	colors[ImGuiCol_ScrollbarGrab] = bg3;
	colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.24f, 0.25f, 0.28f, 1.0f);
	colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.30f, 0.32f, 0.36f, 1.0f);
	colors[ImGuiCol_CheckMark] = cyan;
	colors[ImGuiCol_SliderGrab] = purpleHi;
	colors[ImGuiCol_SliderGrabActive] = cyan;
	colors[ImGuiCol_Button] = bg2;
	colors[ImGuiCol_ButtonHovered] = bg3;
	colors[ImGuiCol_ButtonActive] = purple;
	colors[ImGuiCol_Header] = bg2;
	colors[ImGuiCol_HeaderHovered] = bg3;
	colors[ImGuiCol_HeaderActive] = purple;
	colors[ImGuiCol_Separator] = ImVec4(0.23f, 0.24f, 0.27f, 1.0f);
	colors[ImGuiCol_SeparatorHovered] = purpleHi;
	colors[ImGuiCol_SeparatorActive] = cyan;
	colors[ImGuiCol_Tab] = bg0;
	colors[ImGuiCol_TabHovered] = bg3;
	colors[ImGuiCol_TabActive] = bg2;
	colors[ImGuiCol_TabUnfocused] = bg0;
	colors[ImGuiCol_TabUnfocusedActive] = bg1;
	colors[ImGuiCol_DockingPreview] = ImVec4(0.42f, 0.16f, 0.86f, 0.55f);
	colors[ImGuiCol_DockingEmptyBg] = bg0;
	colors[ImGuiCol_TableHeaderBg] = bg2;
	colors[ImGuiCol_TableBorderStrong] = ImVec4(0.25f, 0.26f, 0.30f, 1.0f);
	colors[ImGuiCol_TableBorderLight] = ImVec4(0.17f, 0.18f, 0.20f, 1.0f);
	colors[ImGuiCol_TableRowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
	colors[ImGuiCol_TableRowBgAlt] = ImVec4(1, 1, 1, 0.025f);
	colors[ImGuiCol_TextSelectedBg] = ImVec4(0.42f, 0.16f, 0.86f, 0.45f);
	colors[ImGuiCol_NavHighlight] = cyan;
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
	if (!ImGui::Begin("Details", &m_showInspector)) {
		ImGui::End();
		return;
	}
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
	auto audioSource = actor->getComponent<AudioSourceComponent>();
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
			if (light.type == LightType::Spot) {
				DrawPropertyLabel("Spot Angle");
				ImGui::SliderFloat("##LightSpotAngle", &light.spotAngle, 0.0f, 90.0f);
			}
			if (light.type == LightType::Rect) {
				DrawPropertyLabel("Width");
				ImGui::SliderFloat("##LightWidth", &light.width, 0.1f, 100.0f);
				DrawPropertyLabel("Height");
				ImGui::SliderFloat("##LightHeight", &light.height, 0.1f, 100.0f);
			}
			ImGui::EndTable();
		}
	}

	if (!audioSource.isNull() && BeginInspectorSection("Audio Source")) {
		char audioPath[260] = {};
		strncpy_s(
			audioPath,
			audioSource->getAudioPath().c_str(),
			_TRUNCATE);
		if (ImGui::InputText("WAV Path", audioPath, IM_ARRAYSIZE(audioPath))) {
			audioSource->setAudioPath(audioPath);
		}

		if (ImGui::BeginDragDropTarget()) {
			const ImGuiPayload* payload =
				ImGui::AcceptDragDropPayload("DND_AUDIO_PATH");
			if (payload) {
				const char* path = static_cast<const char*>(payload->Data);
				audioSource->setAudioPath(path);
			}
			ImGui::EndDragDropTarget();
		}

		float volumePercent = audioSource->getVolume() * 100.0f;
		if (ImGui::SliderFloat("Volume", &volumePercent, 0.0f, 100.0f,
			"%.0f%%")) {
			audioSource->setVolume(volumePercent / 100.0f);
		}

		float pitch = audioSource->getPitch();
		if (ImGui::SliderFloat("Pitch", &pitch, -1.0f, 1.0f, "%.2f")) {
			audioSource->setPitch(pitch);
		}

		bool autoActivate = audioSource->isAutoActivate();
		if (ImGui::Checkbox("Auto Activate", &autoActivate)) {
			audioSource->setAutoActivate(autoActivate);
		}
		ImGui::SameLine();
		bool loop = audioSource->isLooping();
		if (ImGui::Checkbox("Loop", &loop)) {
			audioSource->setLoop(loop);
		}
		ImGui::SameLine();
		bool muted = audioSource->isMuted();
		if (ImGui::Checkbox("Mute", &muted)) {
			audioSource->setMuted(muted);
		}

		bool spatial = audioSource->isSpatial();
		if (ImGui::Checkbox("Spatial 3D", &spatial)) {
			audioSource->setSpatial(spatial);
		}
		if (spatial) {
			ImGui::Checkbox("Edit Max Range Gizmo", &mEditingAudioRange);
			float minDistance = audioSource->getMinDistance();
			float maxDistance = audioSource->getMaxDistance();
			if (ImGui::DragFloat("Min Distance", &minDistance, 0.1f,
				0.0f, 1000.0f, "%.1f")) {
				audioSource->setAttenuation(minDistance, maxDistance);
			}
			if (ImGui::DragFloat("Max Distance", &maxDistance, 0.1f,
				0.1f, 1000.0f, "%.1f")) {
				audioSource->setAttenuation(minDistance, maxDistance);
			}
		}

		if (ImGui::Button("Play Audio")) {
			audioSource->play();
		}
		ImGui::SameLine();
		if (ImGui::Button("Stop Audio")) {
			audioSource->stop();
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
	if (!ImGui::Begin("World Outliner", &m_showOutliner)) {
		ImGui::End();
		return;
	}

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
	if (!mGizmoEnabled) {
		m_isUsingGizmo = false;
		return;
	}

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

	auto audioSource = actor->getComponent<AudioSourceComponent>();
	if (mEditingAudioRange && !audioSource.isNull() &&
		audioSource->isSpatial()) {
		const float maxDistance = audioSource->getMaxDistance();
		float rangePosition[3] = { pos[0], pos[1], pos[2] };
		float rangeRotation[3] = { 0.0f, 0.0f, 0.0f };
		float rangeScale[3] = { maxDistance, maxDistance, maxDistance };
		float rangeMatrix[16];
		ImGuizmo::RecomposeMatrixFromComponents(
			rangePosition, rangeRotation, rangeScale, rangeMatrix);
		ImGuizmo::SetID(1);
		ImGuizmo::Manipulate(
			vArr,
			pArr,
			ImGuizmo::SCALE,
			ImGuizmo::LOCAL,
			rangeMatrix);
		m_isUsingGizmo = ImGuizmo::IsUsing();
		if (m_isUsingGizmo) {
			float newPosition[3], newRotation[3], newScale[3];
			ImGuizmo::DecomposeMatrixToComponents(
				rangeMatrix, newPosition, newRotation, newScale);
			const float newMaxDistance = fmaxf(fabsf(newScale[0]),
				fmaxf(fabsf(newScale[1]), fabsf(newScale[2])));
			audioSource->setAttenuation(
				audioSource->getMinDistance(), newMaxDistance);
		}
		return;
	}

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
				bool isActive = (mGizmoEnabled && mCurrentGizmoOperation == op);
				if (isActive)
					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.4f, 0.8f, 1.0f));

				if (ImGui::Button(label)) {
					mGizmoEnabled = true;
					mCurrentGizmoOperation = op;
				}

				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("%s (%s)", label, shortcut);

				if (isActive) ImGui::PopStyleColor();
				ImGui::SameLine();
			};

		buttonMode("T", ImGuizmo::TRANSLATE, "W");
		buttonMode("R", ImGuizmo::ROTATE, "E");
		buttonMode("S", ImGuizmo::SCALE, "R");

		const bool worldLocalSupported = (mGizmoEnabled && mCurrentGizmoOperation != ImGuizmo::SCALE);
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
				ImGui::SetTooltip(mGizmoEnabled
					? "Scale uses local orientation. World/Local affects Move and Rotate."
					: "Select mode disables gizmo orientation.");
			}
		}
	}
	ImGui::End();

	ImGui::PopStyleVar();
}

void GUI::drawStudioTopRibbon()
{
	ImGuiViewport* viewport = ImGui::GetMainViewport();

	const float menuBarHeight = 24.0f;
	const float toolbarHeight = 42.0f;

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
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.0f, 3.0f));
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.032f, 0.034f, 0.040f, 1.0f));

	if (ImGui::Begin("##UnrealMenuBar", nullptr, menuFlags))
	{
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("New Level")) {
					m_requestNewScene = true;
				}
				if (ImGui::MenuItem("Open Level")) {
					OPENFILENAMEA dialog{};
					char filePath[MAX_PATH] = {};
					dialog.lStructSize = sizeof(dialog);
					dialog.lpstrFile = filePath;
					dialog.nMaxFile = MAX_PATH;
					dialog.lpstrFilter = "Wildvine Level\0*.wvscene\0All Files\0*.*\0";
					dialog.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
					if (GetOpenFileNameA(&dialog) == TRUE) {
						m_openScenePath = filePath;
						m_requestOpenScene = true;
					}
				}
				if (ImGui::MenuItem("Save Current", "Ctrl+S")) {
					m_requestSaveScene = true;
				}
				ImGui::Separator();
				if (ImGui::MenuItem("Exit")) {
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
			if (ImGui::BeginMenu("Window"))
			{
				ImGui::MenuItem("World Outliner", nullptr, &m_showOutliner);
				ImGui::MenuItem("Details", nullptr, &m_showInspector);
				ImGui::MenuItem("Content Browser", nullptr, &m_showToolbox);
				ImGui::Separator();
				ImGui::MenuItem("Render Diagnostics", nullptr, &m_showRenderDebug);
				ImGui::MenuItem("GBuffer Viewer", nullptr, &m_showGBufferDebug);
				ImGui::MenuItem("Material SRV Inspector", nullptr, &m_showMaterialSRVDebug);
				ImGui::MenuItem("Audio", nullptr, &m_showAudioPanel);
				if (ImGui::MenuItem("Reset Editor Layout")) {
					m_showOutliner = true;
					m_showInspector = true;
					m_showToolbox = false;
					m_showRenderDebug = false;
					m_showGBufferDebug = false;
					m_showMaterialSRVDebug = false;
				}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Create"))
			{
				if (ImGui::MenuItem("Directional Light")) {
					m_requestedLightType = LightType::Directional;
					m_requestCreateLight = true;
				}
				ImGui::MenuItem("Static Mesh", nullptr, false, false);
				ImGui::MenuItem("Material", nullptr, false, false);
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Renderer"))
			{
				ImGui::MenuItem("Render Diagnostics", nullptr, &m_showRenderDebug);
				ImGui::MenuItem("GBuffer Viewer", nullptr, &m_showGBufferDebug);
				ImGui::MenuItem("Material SRV Inspector", nullptr, &m_showMaterialSRVDebug);
				ImGui::MenuItem("Visualize Shadow Factor", nullptr, &m_visualizeDeferredShadowFactor);
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Help"))
			{
				ImGui::MenuItem("Documentation");
				ImGui::MenuItem("About Wildvine Engine");
				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}
	}
	ImGui::End();
	ImGui::PopStyleColor();
	ImGui::PopStyleVar(2);

	ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, viewport->Pos.y + menuBarHeight), ImGuiCond_Always);
	ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, toolbarHeight), ImGuiCond_Always);

	ImGuiWindowFlags toolbarFlags =
		ImGuiWindowFlags_NoDecoration |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoScrollWithMouse |
		ImGuiWindowFlags_NoScrollbar;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.0f, 6.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(5.0f, 4.0f));
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(9.0f, 5.0f));
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.060f, 0.064f, 0.074f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.105f, 0.112f, 0.130f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.155f, 0.170f, 0.205f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.42f, 0.16f, 0.86f, 1.0f));

	if (ImGui::Begin("##CompactEditorToolbar", nullptr, toolbarFlags))
	{
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.095f, 0.100f, 0.118f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.145f, 0.160f, 0.200f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.08f, 0.36f, 0.86f, 1.0f));

		if (ImGui::Button("Save", ImVec2(54.0f, 28.0f))) {
			m_requestSaveScene = true;
		}
		ImGui::SameLine();
		if (!m_isRuntimePlaying) {
			ImGui::PushStyleColor(
				ImGuiCol_Button,
				ImVec4(0.10f, 0.38f, 0.13f, 1.0f));
			if (ImGui::Button("Play", ImVec2(54.0f, 28.0f))) {
				m_requestPlay = true;
			}
			ImGui::PopStyleColor();
		}
		else {
			ImGui::PushStyleColor(
				ImGuiCol_Button,
				ImVec4(0.60f, 0.12f, 0.14f, 1.0f));
			if (ImGui::Button("Stop", ImVec2(54.0f, 28.0f))) {
				m_requestStop = true;
			}
			ImGui::PopStyleColor();
		}
		ImGui::SameLine();
		if (ImGui::Button("Content", ImVec2(76.0f, 28.0f))) {
			m_showToolbox = true;
		}
		ImGui::SameLine();
		ImGui::Dummy(ImVec2(10.0f, 1.0f));
		ImGui::SameLine();

		if (ImGui::Button("Selection Mode  v", ImVec2(148.0f, 28.0f))) {
			ImGui::OpenPopup("##SelectionModeMenu");
		}

		PushEditorPopupStyle();
		ImGui::SetNextWindowSize(ImVec2(260.0f, 0.0f), ImGuiCond_Appearing);
		if (ImGui::BeginPopup("##SelectionModeMenu")) {
			if (ImGui::MenuItem("Selection", "Shift+1", true)) {
				mGizmoEnabled = false;
			}
			ImGui::MenuItem("Landscape", "Shift+2", false, false);
			ImGui::MenuItem("Foliage", "Shift+3", false, false);
			ImGui::MenuItem("Mesh Paint", "Shift+4", false, false);
			ImGui::MenuItem("Modeling", "Shift+5", false, false);
			ImGui::MenuItem("Fracture", "Shift+6", false, false);
			ImGui::MenuItem("Brush Editing", "Shift+7", false, false);
			ImGui::MenuItem("Animation", "Shift+8", false, false);
			ImGui::EndPopup();
		}
		PopEditorPopupStyle();

		ImGui::SameLine();
		ImGui::Dummy(ImVec2(10.0f, 1.0f));
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.10f, 0.38f, 0.13f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.16f, 0.55f, 0.20f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.08f, 0.68f, 0.24f, 1.0f));
		if (ImGui::Button("+  v", ImVec2(46.0f, 28.0f))) {
			ImGui::OpenPopup("##CreateActorMenu");
		}
		ImGui::PopStyleColor(3);

		PushEditorPopupStyle();
		ImGui::SetNextWindowSize(ImVec2(285.0f, 0.0f), ImGuiCond_Appearing);
		if (ImGui::BeginPopup("##CreateActorMenu")) {
			ImGui::SetNextItemWidth(230.0f);
			static char createSearchBuffer[64] = {};
			ImGui::InputTextWithHint("##CreateSearch", "Start typing to search", createSearchBuffer, IM_ARRAYSIZE(createSearchBuffer));
			ImGui::Separator();

			ImGui::TextDisabled("GET CONTENT");
			ImGui::MenuItem("Import Content...", nullptr, false, false);
			ImGui::MenuItem("Quixel Bridge", nullptr, false, false);
			if (ImGui::MenuItem("Content Browser", nullptr, m_showToolbox)) {
				m_showToolbox = true;
			}
			ImGui::Separator();

			ImGui::TextDisabled("PLACE ACTORS");
			if (ImGui::BeginMenu("Basic", false)) {
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Lights")) {
				if (ImGui::MenuItem("Directional Light")) {
					m_requestedLightType = LightType::Directional;
					m_requestCreateLight = true;
				}
				if (ImGui::MenuItem("Point Light")) {
					m_requestedLightType = LightType::Point;
					m_requestCreateLight = true;
				}
				if (ImGui::MenuItem("Spot Light")) {
					m_requestedLightType = LightType::Spot;
					m_requestCreateLight = true;
				}
				if (ImGui::MenuItem("Rect Light")) {
					m_requestedLightType = LightType::Rect;
					m_requestCreateLight = true;
				}
				ImGui::EndMenu();
			}
			ImGui::MenuItem("Shapes", nullptr, false, false);
			ImGui::MenuItem("Cinematic", nullptr, false, false);
			ImGui::MenuItem("Media Plate", nullptr, false, false);
			ImGui::MenuItem("Visual Effects", nullptr, false, false);
			ImGui::MenuItem("Geometry", nullptr, false, false);
			ImGui::MenuItem("Volumes", nullptr, false, false);
			ImGui::MenuItem("Place Actors Panel", nullptr, false, false);
			ImGui::Separator();

			ImGui::TextDisabled("RECENT");
			if (ImGui::MenuItem("Directional Light")) {
				m_requestedLightType = LightType::Directional;
				m_requestCreateLight = true;
			}
			if (ImGui::MenuItem("Point Light")) {
				m_requestedLightType = LightType::Point;
				m_requestCreateLight = true;
			}
			if (ImGui::MenuItem("Spot Light")) {
				m_requestedLightType = LightType::Spot;
				m_requestCreateLight = true;
			}
			if (ImGui::MenuItem("Rect Light")) {
				m_requestedLightType = LightType::Rect;
				m_requestCreateLight = true;
			}
			ImGui::EndPopup();
		}
		PopEditorPopupStyle();

		ImGui::PopStyleColor(3);
	}
	ImGui::End();

	ImGui::PopStyleColor(4);
	ImGui::PopStyleVar(4);
}
void GUI::drawViewportPanel(ID3D11ShaderResourceView* viewportSRV)
{
	ImGuiWindowFlags flags =
		ImGuiWindowFlags_NoScrollbar |
		ImGuiWindowFlags_NoScrollWithMouse | 
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoCollapse |
		(m_isRuntimePlaying ? 0 : ImGuiWindowFlags_MenuBar);

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

	if (ImGui::Begin(m_isRuntimePlaying ? "Gameplay" : "DefaultScene",
		nullptr, flags))
	{
		m_viewportDrawList = ImGui::GetWindowDrawList();

		PushEditorPopupStyle();
		if (!m_isRuntimePlaying && ImGui::BeginMenuBar()) {
			ImGui::SetNextWindowSize(ImVec2(285.0f, 0.0f), ImGuiCond_Appearing);
			if (ImGui::BeginMenu("Menu")) {
				ImGui::TextDisabled("TRANSFORM TOOLS");
				if (ImGui::MenuItem("Select Mode", "Q", !mGizmoEnabled)) {
					mGizmoEnabled = false;
				}
				if (ImGui::MenuItem("Translate Mode", "W", mGizmoEnabled && mCurrentGizmoOperation == ImGuizmo::TRANSLATE)) {
					mGizmoEnabled = true;
					mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
				}
				if (ImGui::MenuItem("Rotate Mode", "E", mGizmoEnabled && mCurrentGizmoOperation == ImGuizmo::ROTATE)) {
					mGizmoEnabled = true;
					mCurrentGizmoOperation = ImGuizmo::ROTATE;
				}
				if (ImGui::MenuItem("Scale Mode", "R", mGizmoEnabled && mCurrentGizmoOperation == ImGuizmo::SCALE)) {
					mGizmoEnabled = true;
					mCurrentGizmoOperation = ImGuizmo::SCALE;
				}
				ImGui::Separator();
				if (ImGui::MenuItem("Coordinate System", mCurrentGizmoMode == ImGuizmo::WORLD ? "World" : "Local")) {
					mCurrentGizmoMode = (mCurrentGizmoMode == ImGuizmo::WORLD) ? ImGuizmo::LOCAL : ImGuizmo::WORLD;
				}
				ImGui::Separator();
				if (ImGui::BeginMenu("Show")) {
					ImGui::TextDisabled("EDITOR PANELS");
					ImGui::MenuItem("World Outliner", nullptr, &m_showOutliner);
					ImGui::MenuItem("Details", nullptr, &m_showInspector);
					ImGui::MenuItem("Content Browser", nullptr, &m_showToolbox);
					ImGui::Separator();
					ImGui::TextDisabled("DEBUG PANELS");
					ImGui::MenuItem("Render Diagnostics", nullptr, &m_showRenderDebug);
					ImGui::MenuItem("GBuffer Viewer", nullptr, &m_showGBufferDebug);
					ImGui::MenuItem("Material SRV Inspector", nullptr, &m_showMaterialSRVDebug);
					ImGui::MenuItem("Visualize Shadow Factor", nullptr, &m_visualizeDeferredShadowFactor);
					ImGui::EndMenu();
				}
				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}
		PopEditorPopupStyle();
		if (m_isRuntimePlaying) {
			ImGui::SetCursorPos(ImVec2(14.0f, 12.0f));
			ImGui::TextDisabled("PLAY MODE  |  WASD move  |  RMB look  |  Shift sprint");
		}

		ImVec2 panelMin = ImGui::GetCursorScreenPos();
		ImVec2 panelSize = ImGui::GetContentRegionAvail();

		if (panelSize.x < 1.0f) panelSize.x = 1.0f;
		if (panelSize.y < 1.0f) panelSize.y = 1.0f;

		if (viewportSRV)
		{
			// Mantener aspect ratio del render target sin deformar
			float panelAspect = panelSize.x / panelSize.y;
			float renderAspect = panelSize.x / panelSize.y; // Se usa el panel para el resize
			// La imagen se dibuja al tamano del panel (el render target se adapta al panel)
			ImVec2 imageSize = panelSize;
			ImVec2 imageOffset(0.0f, 0.0f);

			ImGui::SetCursorScreenPos(ImVec2(panelMin.x + imageOffset.x, panelMin.y + imageOffset.y));
			ImGui::Image((ImTextureID)viewportSRV, imageSize);
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

		if (ImGui::BeginDragDropTarget()) {
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_MODEL_PATH")) {
				const char* path = (const char*)payload->Data;
				m_assetSpawnPath = path;
				m_assetSpawnRequested = true;
			}
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_TEXTURE_PATH")) {
				const char* path = (const char*)payload->Data;
				m_textureDropPath = path;
				m_textureDropRequested = true;
			}
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_AUDIO_PATH")) {
				const char* path = static_cast<const char*>(payload->Data);
				m_audioSpawnPath = path;
				m_audioSpawnRequested = true;
			}
			ImGui::EndDragDropTarget();
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

void GUI::drawRenderDebugPanel(ID3D11ShaderResourceView* preShadowSRV,
	ID3D11ShaderResourceView* finalViewportSRV,
	ID3D11ShaderResourceView* shadowMapSRV)
{
	if (!ImGui::Begin("Render Diagnostics", &m_showRenderDebug)) {
		ImGui::End();
		return;
	}

	DebugTextureItem items[] = {
		{ "Scene Final", "Viewport color target", finalViewportSRV },
		{ "Pre-Shadow", "Scene before shadow resolve", preShadowSRV },
		{ "Shadow Map", "Directional light depth", shadowMapSRV }
	};

	static int selectedView = 0;
	if (selectedView >= IM_ARRAYSIZE(items)) {
		selectedView = 0;
	}

	int availableCount = 0;
	for (int i = 0; i < IM_ARRAYSIZE(items); ++i) {
		if (items[i].srv) {
			availableCount++;
		}
	}

	ImGui::TextDisabled("Renderer");
	ImGui::SameLine();
	ImGui::Text("Forward Renderer");
	ImGui::SameLine();
	ImGui::TextDisabled("| Passes: %d/%d", availableCount, IM_ARRAYSIZE(items));
	ImGui::Separator();

	const float leftWidth = 230.0f;
	ImGui::BeginChild("##RenderPassList", ImVec2(leftWidth, 0.0f), true);
	ImGui::TextDisabled("Pass Browser");
	ImGui::Separator();

	for (int i = 0; i < IM_ARRAYSIZE(items); ++i) {
		ImGui::PushID(i);
		const bool selected = (selectedView == i);
		if (selected) {
			ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.08f, 0.36f, 0.86f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.52f, 0.20f, 1.00f, 1.0f));
		}

		if (ImGui::Selectable("##RenderPassRow", selected, ImGuiSelectableFlags_SpanAvailWidth, ImVec2(0.0f, 132.0f))) {
			selectedView = i;
		}

		ImVec2 rowMin = ImGui::GetItemRectMin();
		ImVec2 rowMax = ImGui::GetItemRectMax();
		ImDrawList* drawList = ImGui::GetWindowDrawList();
		drawList->AddText(ImVec2(rowMin.x + 8.0f, rowMin.y + 7.0f), ImGui::GetColorU32(ImGuiCol_Text), items[i].label);
		drawList->AddText(ImVec2(rowMin.x + 8.0f, rowMin.y + 25.0f), ImGui::GetColorU32(ImGuiCol_TextDisabled), items[i].channels);

		ImVec2 thumbMin(rowMin.x + 8.0f, rowMin.y + 48.0f);
		ImVec2 thumbMax(rowMax.x - 8.0f, rowMax.y - 8.0f);
		if (items[i].srv) {
			drawList->AddRectFilled(thumbMin, thumbMax, IM_COL32(10, 11, 13, 255), 3.0f);
			ImGui::SetCursorScreenPos(thumbMin);
			ImGui::Image((ImTextureID)items[i].srv, ImVec2(thumbMax.x - thumbMin.x, thumbMax.y - thumbMin.y));
		}
		else {
			drawList->AddRectFilled(thumbMin, thumbMax, IM_COL32(18, 20, 23, 255), 3.0f);
			drawList->AddRect(thumbMin, thumbMax, IM_COL32(58, 62, 70, 255), 3.0f);
			drawList->AddText(ImVec2(thumbMin.x + 8.0f, thumbMin.y + 8.0f), ImGui::GetColorU32(ImGuiCol_TextDisabled), "Unavailable");
		}
		ImGui::SetCursorScreenPos(ImVec2(rowMin.x, rowMax.y));

		if (selected) {
			ImGui::PopStyleColor(2);
		}
		ImGui::PopID();
	}
	ImGui::EndChild();

	ImGui::SameLine();
	ImGui::BeginChild("##RenderPassPreview", ImVec2(0.0f, 0.0f), true);
	ImGui::Text("%s", items[selectedView].label);
	ImGui::SameLine();
	ImGui::TextDisabled("%s", items[selectedView].channels);
	ImGui::Separator();

	ImVec2 previewSize = ImGui::GetContentRegionAvail();
	if (previewSize.x < 1.0f) previewSize.x = 1.0f;
	if (previewSize.y < 1.0f) previewSize.y = 1.0f;
	if (items[selectedView].srv) {
		ImGui::Image((ImTextureID)items[selectedView].srv, previewSize);
	}
	else {
		ImVec2 min = ImGui::GetCursorScreenPos();
		ImVec2 maxPt(min.x + previewSize.x, min.y + previewSize.y);
		ImGui::InvisibleButton("##RenderPassMissing", previewSize);
		ImDrawList* drawList = ImGui::GetWindowDrawList();
		drawList->AddRectFilled(min, maxPt, IM_COL32(18, 20, 23, 255));
		drawList->AddText(ImVec2(min.x + 16.0f, min.y + 16.0f), ImGui::GetColorU32(ImGuiCol_TextDisabled), "No SRV bound for this pass");
	}
	ImGui::EndChild();

	ImGui::End();
}
void GUI::drawGBufferDebugPanel(ID3D11ShaderResourceView* albedoMetallicSRV,
	ID3D11ShaderResourceView* normalRoughnessSRV,
	ID3D11ShaderResourceView* worldAoSRV,
	ID3D11ShaderResourceView* emissiveAlphaSRV)
{
	DebugTextureItem items[] = {
		{ "GB: Albedo + Metallic", "RGB Albedo | A Metallic", albedoMetallicSRV },
		{ "GB: Normal + Roughness", "RGB Normal | A Roughness", normalRoughnessSRV },
		{ "GB: World + AO", "RGB World | A AO", worldAoSRV },
		{ "GB: Emissive + Alpha", "RGB Emissive | A Alpha", emissiveAlphaSRV }
	};

	static int selectedView = 0;
	if (selectedView >= IM_ARRAYSIZE(items)) {
		selectedView = 0;
	}

	bool hasAnyTexture = false;
	for (int i = 0; i < IM_ARRAYSIZE(items); ++i) {
		if (items[i].srv != nullptr) {
			hasAnyTexture = true;
			break;
		}
	}

	if (!ImGui::Begin("GBuffer Viewer", &m_showGBufferDebug)) {
		ImGui::End();
		return;
	}
	ImGui::TextDisabled("Texture Combination Preview");
	ImGui::SameLine();
	ImGui::Text("%s", hasAnyTexture ? "PBR fallback from loaded SRVs" : "Deferred attachments unavailable");
	ImGui::Checkbox("Visualize Shadow Factor", &m_visualizeDeferredShadowFactor);
	ImGui::Separator();

	const float leftWidth = 245.0f;
	ImGui::BeginChild("##GBufferList", ImVec2(leftWidth, 0.0f), true);
	for (int i = 0; i < IM_ARRAYSIZE(items); ++i) {
		ImGui::PushID(i);
		if (ImGui::Selectable(items[i].label, selectedView == i, ImGuiSelectableFlags_SpanAvailWidth, ImVec2(0.0f, 24.0f))) {
			selectedView = i;
		}
		ImGui::TextDisabled("%s", items[i].channels);
		ImVec2 thumbSize(ImGui::GetContentRegionAvail().x, 92.0f);
		if (items[i].srv) {
			ImGui::Image((ImTextureID)items[i].srv, thumbSize);
		}
		else {
			ImVec2 min = ImGui::GetCursorScreenPos();
			ImVec2 maxPt(min.x + thumbSize.x, min.y + thumbSize.y);
			ImGui::InvisibleButton("##MissingGBufferThumb", thumbSize);
			ImDrawList* drawList = ImGui::GetWindowDrawList();
			drawList->AddRectFilled(min, maxPt, IM_COL32(18, 20, 23, 255), 3.0f);
			drawList->AddRect(min, maxPt, IM_COL32(58, 62, 70, 255), 3.0f);
			drawList->AddText(ImVec2(min.x + 8.0f, min.y + 8.0f), ImGui::GetColorU32(ImGuiCol_TextDisabled), "No preview SRV bound");
		}
		ImGui::Separator();
		ImGui::PopID();
	}
	ImGui::EndChild();

	ImGui::SameLine();
	ImGui::BeginChild("##GBufferPreview", ImVec2(0.0f, 0.0f), true);
	ImGui::Text("%s", items[selectedView].label);
	ImGui::SameLine();
	ImGui::TextDisabled("%s", items[selectedView].channels);
	ImGui::Separator();
	ImVec2 previewSize = ImGui::GetContentRegionAvail();
	if (previewSize.x < 1.0f) previewSize.x = 1.0f;
	if (previewSize.y < 1.0f) previewSize.y = 1.0f;
	if (items[selectedView].srv) {
		ImGui::Image((ImTextureID)items[selectedView].srv, previewSize);
	}
	else {
		ImVec2 min = ImGui::GetCursorScreenPos();
		ImVec2 maxPt(min.x + previewSize.x, min.y + previewSize.y);
		ImGui::InvisibleButton("##MissingGBufferPreview", previewSize);
		ImDrawList* drawList = ImGui::GetWindowDrawList();
		drawList->AddRectFilled(min, maxPt, IM_COL32(18, 20, 23, 255));
		drawList->AddText(ImVec2(min.x + 16.0f, min.y + 16.0f), ImGui::GetColorU32(ImGuiCol_TextDisabled), "Open a loaded material or deferred renderer output to populate this preview");
	}
	ImGui::EndChild();

	ImGui::End();
}

void GUI::drawMaterialSRVDebugPanel(ID3D11ShaderResourceView* albedoSRV,
	ID3D11ShaderResourceView* normalSRV,
	ID3D11ShaderResourceView* metallicSRV,
	ID3D11ShaderResourceView* roughnessSRV,
	ID3D11ShaderResourceView* aoSRV)
{
	if (!m_showMaterialSRVDebug) {
		return;
	}

	DebugTextureItem items[] = {
		{ "PBR Albedo", "Slot t0 | RGB base color", albedoSRV },
		{ "PBR Normal", "Slot t1 | Tangent-space normal", normalSRV },
		{ "PBR Metallic", "Slot t2 | Metal mask", metallicSRV },
		{ "PBR Roughness", "Slot t3 | Micro-surface roughness", roughnessSRV },
		{ "PBR Ambient Occlusion", "Slot t4 | Occlusion mask", aoSRV }
	};

	static int selectedView = 0;
	if (selectedView >= IM_ARRAYSIZE(items)) {
		selectedView = 0;
	}

	int availableCount = 0;
	for (int i = 0; i < IM_ARRAYSIZE(items); ++i) {
		if (items[i].srv) {
			availableCount++;
		}
	}

	if (!ImGui::Begin("Material SRV Inspector", &m_showMaterialSRVDebug)) {
		ImGui::End();
		return;
	}
	ImGui::TextDisabled("Bound PBR Textures");
	ImGui::SameLine();
	ImGui::Text("%d/%d SRVs", availableCount, IM_ARRAYSIZE(items));
	ImGui::Separator();

	const float leftWidth = 235.0f;
	ImGui::BeginChild("##MaterialSRVList", ImVec2(leftWidth, 0.0f), true);
	for (int i = 0; i < IM_ARRAYSIZE(items); ++i) {
		ImGui::PushID(i);
		if (ImGui::Selectable(items[i].label, selectedView == i, ImGuiSelectableFlags_SpanAvailWidth, ImVec2(0.0f, 24.0f))) {
			selectedView = i;
		}
		ImGui::TextDisabled("%s", items[i].channels);
		ImVec2 thumbSize(ImGui::GetContentRegionAvail().x, 96.0f);
		if (items[i].srv) {
			ImGui::Image((ImTextureID)items[i].srv, thumbSize);
		}
		else {
			ImVec2 min = ImGui::GetCursorScreenPos();
			ImVec2 maxPt(min.x + thumbSize.x, min.y + thumbSize.y);
			ImGui::InvisibleButton("##MissingMaterialSRVThumb", thumbSize);
			ImDrawList* drawList = ImGui::GetWindowDrawList();
			drawList->AddRectFilled(min, maxPt, IM_COL32(18, 20, 28, 255), 3.0f);
			drawList->AddRect(min, maxPt, IM_COL32(68, 72, 86, 255), 3.0f);
			drawList->AddText(ImVec2(min.x + 8.0f, min.y + 8.0f), ImGui::GetColorU32(ImGuiCol_TextDisabled), "SRV missing");
		}
		ImGui::Separator();
		ImGui::PopID();
	}
	ImGui::EndChild();

	ImGui::SameLine();
	ImGui::BeginChild("##MaterialSRVPreview", ImVec2(0.0f, 0.0f), true);
	ImGui::Text("%s", items[selectedView].label);
	ImGui::SameLine();
	ImGui::TextDisabled("%s", items[selectedView].channels);
	ImGui::Separator();
	ImVec2 previewSize = ImGui::GetContentRegionAvail();
	if (previewSize.x < 1.0f) previewSize.x = 1.0f;
	if (previewSize.y < 1.0f) previewSize.y = 1.0f;
	if (items[selectedView].srv) {
		ImGui::Image((ImTextureID)items[selectedView].srv, previewSize);
	}
	else {
		ImVec2 min = ImGui::GetCursorScreenPos();
		ImVec2 maxPt(min.x + previewSize.x, min.y + previewSize.y);
		ImGui::InvisibleButton("##MissingMaterialSRVPreview", previewSize);
		ImDrawList* drawList = ImGui::GetWindowDrawList();
		drawList->AddRectFilled(min, maxPt, IM_COL32(18, 20, 28, 255));
		drawList->AddText(ImVec2(min.x + 16.0f, min.y + 16.0f), ImGui::GetColorU32(ImGuiCol_TextDisabled), "This material slot has no shader resource view");
	}
	ImGui::EndChild();

	ImGui::End();
}

void GUI::drawToolboxPanel()
{
	if (!ImGui::Begin("Content Browser", &m_showToolbox)) {
		ImGui::End();
		return;
	}

	ImGui::TextDisabled("Transform");
	if (ImGui::Button("Select")) {
		mGizmoEnabled = false;
	}
	ImGui::SameLine();
	if (ImGui::Button("Move")) {
		mGizmoEnabled = true;
		mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
	}
	ImGui::SameLine();
	if (ImGui::Button("Rotate")) {
		mGizmoEnabled = true;
		mCurrentGizmoOperation = ImGuizmo::ROTATE;
	}
	ImGui::SameLine();
	if (ImGui::Button("Scale")) {
		mGizmoEnabled = true;
		mCurrentGizmoOperation = ImGuizmo::SCALE;
	}

	ImGui::Separator();
	ImGui::TextDisabled("Panels");
	ImGui::Checkbox("Explorer", &m_showOutliner);
	ImGui::Checkbox("Properties", &m_showInspector);
	ImGui::Checkbox("Render Debug", &m_showRenderDebug);
	ImGui::Checkbox("GBuffer Debug", &m_showGBufferDebug);
	ImGui::Checkbox("Material SRV Debug", &m_showMaterialSRVDebug);

	ImGui::Separator();
	ImGui::TextDisabled("Renderer");
	ImGui::Checkbox("Visualize Shadow Factor", &m_visualizeDeferredShadowFactor);

	ImGui::Separator();
	ImGui::TextDisabled("Create");
	if (ImGui::Button("Directional Light")) {
		m_requestedLightType = LightType::Directional;
		m_requestCreateLight = true;
	}

	ImGui::End();
}

void GUI::drawEditorDockspace()
{
	ImGuiViewport* mainViewport = ImGui::GetMainViewport();

	// Debe coincidir con la altura total que ocupa tu ribbon superior
	const float topOffset = 66.0f; // 24 menu + 42 compact toolbar

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

	static bool s_defaultLayoutBuilt = false;
	if (!s_defaultLayoutBuilt || ImGui::DockBuilderGetNode(dockspace_id) == nullptr) {
		s_defaultLayoutBuilt = true;

		ImGui::DockBuilderRemoveNode(dockspace_id);
		ImGui::DockBuilderAddNode(dockspace_id, dockspace_flags | ImGuiDockNodeFlags_DockSpace);
		ImGui::DockBuilderSetNodeSize(dockspace_id, dockSize);

		ImGuiID dockMain = dockspace_id;
		ImGuiID dockRight = ImGui::DockBuilderSplitNode(dockMain, ImGuiDir_Right, 0.22f, nullptr, &dockMain);
		ImGuiID dockBottom = ImGui::DockBuilderSplitNode(dockMain, ImGuiDir_Down, 0.26f, nullptr, &dockMain);
		ImGuiID dockLeft = ImGui::DockBuilderSplitNode(dockMain, ImGuiDir_Left, 0.18f, nullptr, &dockMain);
		ImGuiID dockRightBottom = ImGui::DockBuilderSplitNode(dockRight, ImGuiDir_Down, 0.50f, nullptr, &dockRight);

		// Centro: Viewport — DefaultScene va al último para quedar activo
		ImGui::DockBuilderDockWindow("GBuffer Viewer", dockMain);
		ImGui::DockBuilderDockWindow("Material SRV Inspector", dockMain);
		ImGui::DockBuilderDockWindow("Render Diagnostics", dockMain);
		ImGui::DockBuilderDockWindow("Shader Viewer", dockMain);
		ImGui::DockBuilderDockWindow("DefaultScene", dockMain);

		// Izquierda: Lighting primero, World Outliner al último para quedar activo
		ImGui::DockBuilderDockWindow("Lighting", dockLeft);
		ImGui::DockBuilderDockWindow("World Outliner", dockLeft);

		// Derecha arriba: Details
		ImGui::DockBuilderDockWindow("Details", dockRight);

		// Derecha abajo: G-Buffer debug
		ImGui::DockBuilderDockWindow("G-Buffer", dockRightBottom);
		ImGui::DockBuilderDockWindow("Performance", dockRightBottom);

		// Abajo: Content, Console
		ImGui::DockBuilderDockWindow("Content", dockBottom);
		ImGui::DockBuilderDockWindow("Console", dockBottom);
		ImGui::DockBuilderDockWindow("Content Browser", dockBottom);

		ImGui::DockBuilderFinish(dockspace_id);
	}

	ImGui::End();

	ImGui::PopStyleVar(3);
}









void GUI::drawViewportGrid(Camera& cam) {
	if (!m_showGrid) return;
	if (m_viewportSize.x < 16.0f || m_viewportSize.y < 16.0f) return;
	float view[16], proj[16], identity[16];
	ToFloatArray(cam.getView(), view);
	ToFloatArray(cam.getProj(), proj);
	ToFloatArray(XMMatrixRotationX(XM_PIDIV2), identity);
	if (m_viewportDrawList) ImGuizmo::SetDrawlist(m_viewportDrawList);
	ImGuizmo::SetRect(m_viewportPos.x, m_viewportPos.y, m_viewportSize.x, m_viewportSize.y);
	if (m_viewportDrawList) {
		m_viewportDrawList->PushClipRect(
			m_viewportPos,
			ImVec2(m_viewportPos.x + m_viewportSize.x, m_viewportPos.y + m_viewportSize.y),
			true);
	}

	ImGuizmo::DrawGrid(view, proj, identity, m_gridSize);
	if (m_viewportDrawList) {
		m_viewportDrawList->PopClipRect();
	}
}

void GUI::drawLightingPanel(float* lightDir, float* lightColor) {
	ImGui::Begin("Lighting");
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.72f, 0.28f, 0.40f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.85f, 0.36f, 0.48f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.60f, 0.20f, 0.32f, 1.0f));
	if (ImGui::Button("  Reset Scene  ")) {
		m_resetRequested = true;
		m_deferredDebugViewMode = 0;
		m_visualizeDeferredShadowFactor = false;
	}
	ImGui::PopStyleColor(3);
	if (ImGui::IsItemHovered()) ImGui::SetTooltip("Restaura transforms, luz y camara a sus valores originales");
	ImGui::Separator();
	ImGui::TextDisabled("Luz direccional principal");
	ImGui::Spacing();
	if (lightDir) {
		ImGui::Text("Direccion");
		ImGui::SliderFloat3("##LightingDir", lightDir, -1.0f, 1.0f);
	}
	if (lightColor) {
		ImGui::Text("Color");
		ImGui::ColorEdit3("##LightingColor", lightColor);
	}
	ImGui::End();
}

void
GUI::drawAudioPanel() {
	if (!ImGui::Begin("Audio", &m_showAudioPanel)) {
		ImGui::End();
		return;
	}

	ImGui::TextDisabled("GLOBAL AUDIO MIXER");
	float masterVolumePercent = m_audioMasterVolume * 100.0f;
	if (ImGui::SliderFloat(
		"Master Volume",
		&masterVolumePercent,
		0.0f,
		100.0f,
		"%.0f%%")) {
		m_audioMasterVolume = masterVolumePercent / 100.0f;
		m_audioMasterVolumeChanged = true;
	}

	ImGui::Separator();
	ImGui::TextDisabled("TRANSPORT");
	if (!m_audioPaused) {
		if (ImGui::Button("Pause All", ImVec2(110.0f, 0.0f))) {
			m_audioPauseRequested = true;
		}
	}
	else {
		if (ImGui::Button("Resume All", ImVec2(110.0f, 0.0f))) {
			m_audioResumeRequested = true;
		}
	}

	ImGui::SameLine();
	ImGui::TextDisabled(
		m_audioPaused ? "All sources paused" : "All sources active");
	ImGui::Spacing();
	ImGui::TextWrapped(
		"Configure each WAV path, source volume, and playback in Details.");
	ImGui::End();
}

void GUI::drawStatsPanel(float deltaTime, unsigned int drawCalls,
  unsigned int submittedObjects, unsigned int visibleObjects,
  unsigned int culledObjects, unsigned int octreeNodes) {
	ImGui::Begin("Performance");
	static float history[120] = {};
	static int idx = 0;
	static float accum = 0.0f; static int frames = 0;
	static float fps = 0.0f; static float ms = 0.0f;

	float dtMs = deltaTime * 1000.0f;
	history[idx] = dtMs; idx = (idx + 1) % IM_ARRAYSIZE(history);
	accum += deltaTime; frames++;
	if (accum >= 0.25f) { fps = frames / accum; ms = (accum / frames) * 1000.0f; accum = 0.0f; frames = 0; }

	ImGui::SetWindowFontScale(1.7f);
	ImGui::Text("%.0f FPS", fps);
	ImGui::SetWindowFontScale(1.0f);
	ImGui::SameLine();
	ImGui::TextDisabled("  %.2f ms", ms);

	ImGui::Spacing();
	ImGui::PlotLines("##frametimes", history, IM_ARRAYSIZE(history), idx,
		"Frame time (ms)", 0.0f, 33.3f, ImVec2(ImGui::GetContentRegionAvail().x, 80.0f));

	ImGui::Spacing(); ImGui::Separator();
	ImGui::Text("Draw calls:"); ImGui::SameLine(); ImGui::Text("%u", drawCalls);
	ImGui::Text("Objects: %u visible / %u total", visibleObjects,
		submittedObjects);
	ImGui::Text("Frustum culled: %u  |  Octree nodes: %u", culledObjects,
		octreeNodes);
	ImGui::TextDisabled("Viewport: %.0f x %.0f", m_viewportSize.x, m_viewportSize.y);
	ImGui::End();
}

void GUI::drawConsolePanel() {
	ImGui::Begin("Console");
	if (ImGui::Button("Clear")) Logger::get().clear();
	ImGui::SameLine();
	ImGui::Checkbox("Info", &m_logShowInfo); ImGui::SameLine();
	ImGui::Checkbox("Warning", &m_logShowWarning); ImGui::SameLine();
	ImGui::Checkbox("Error", &m_logShowError); ImGui::SameLine();
	ImGui::Checkbox("Auto-scroll", &m_logAutoScroll); ImGui::SameLine();
	m_logFilter.Draw("Filter", 160.0f);
	ImGui::Separator();
	ImGui::BeginChild("ConsoleScroll", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
	std::vector<LogEntry> entries = Logger::get().snapshot();
	for (const LogEntry& e : entries) {
		if (e.level == LogLevel::Info && !m_logShowInfo) continue;
		if (e.level == LogLevel::Warning && !m_logShowWarning) continue;
		if (e.level == LogLevel::Error && !m_logShowError) continue;
		if (!m_logFilter.PassFilter(e.message.c_str())) continue;
		ImVec4 col; const char* tag;
		switch (e.level) {
		case LogLevel::Error:   col = ImVec4(0.95f, 0.40f, 0.40f, 1.0f); tag = "[ERROR] "; break;
		case LogLevel::Warning: col = ImVec4(0.95f, 0.78f, 0.30f, 1.0f); tag = "[WARN]  "; break;
		default:                col = ImVec4(0.80f, 0.78f, 0.90f, 1.0f); tag = "[INFO]  "; break;
		}
		ImGui::PushStyleColor(ImGuiCol_Text, col);
		ImGui::TextUnformatted(tag); ImGui::SameLine();
		ImGui::TextUnformatted(e.message.c_str());
		ImGui::PopStyleColor();
	}
	if (m_logAutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - 1.0f) ImGui::SetScrollHereY(1.0f);
	ImGui::EndChild();
	ImGui::End();
}

void GUI::drawTexturePreview() {
	if (!m_showPreview) return;
	ImGui::SetNextWindowSize(ImVec2(720.0f, 480.0f), ImGuiCond_FirstUseEver);
	if (ImGui::Begin("Texture Preview", &m_showPreview)) {
		ImGui::TextUnformatted(m_previewLabel.c_str());
		ImGui::Separator();
		if (m_previewSRV) {
			ImVec2 avail = ImGui::GetContentRegionAvail();
			if (avail.x < 16.0f) avail.x = 16.0f;
			if (avail.y < 16.0f) avail.y = 16.0f;
			ImGui::Image((ImTextureID)m_previewSRV, avail);
		}
	}
	ImGui::End();
}

void GUI::drawContentBrowser(const std::vector<AssetThumb>& textureThumbs) {
	ImGui::Begin("Content");
	ImGui::TextDisabled("Root: %s", fs::absolute("Assets").string().c_str());

	if (ImGui::Button("Create Folder...")) {
		ImGui::OpenPopup("Create Content Folder");
	}
	ImGui::SameLine();
	if (ImGui::Button("Import Content...")) {
		OPENFILENAMEA ofn;
		char szFile[260] = {0};
		ZeroMemory(&ofn, sizeof(ofn));
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = NULL;
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		ofn.lpstrFilter = "All Supported\0*.fbx;*.obj;*.glb;*.gltf;*.png;*.jpg;*.tga;*.dds;*.wav;*.mp3;*.flac\0Models (*.fbx;*.obj;*.glb;*.gltf)\0*.fbx;*.obj;*.glb;*.gltf\0Textures (*.png;*.jpg;*.tga;*.dds)\0*.png;*.jpg;*.tga;*.dds\0Audio (*.wav;*.mp3;*.flac)\0*.wav;*.mp3;*.flac\0All\0*.*\0";
		ofn.nFilterIndex = 1;
		ofn.lpstrFileTitle = NULL;
		ofn.nMaxFileTitle = 0;
		ofn.lpstrInitialDir = NULL;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
		if (GetOpenFileNameA(&ofn) == TRUE) {
			const fs::path sourcePath(ofn.lpstrFile);
			std::error_code error;
			bool imported = false;
			if (IsModelFile(sourcePath)) {
				imported = ImportModelWithTextures(sourcePath);
			}
			else if (IsAudioFile(sourcePath)) {
				const fs::path audioDirectory = fs::path("Assets") / "Audio";
				fs::create_directories(audioDirectory, error);
				if (!error && IsPlayableAudioFile(sourcePath)) {
					fs::copy_file(sourcePath, audioDirectory / sourcePath.filename(),
						fs::copy_options::overwrite_existing, error);
					imported = !error;
				}
				else if (!error) {
					const fs::path wavePath =
						audioDirectory / (sourcePath.stem().string() + ".wav");
					imported = ConvertAudioToWave(sourcePath, wavePath);
				}
			}
			else if (IsImageFile(sourcePath)) {
				const fs::path destination =
					fs::path("Assets") / "Textures" / sourcePath.filename();
				fs::create_directories(destination.parent_path(), error);
				if (!error) {
					fs::copy_file(sourcePath, destination,
						fs::copy_options::overwrite_existing, error);
					imported = !error;
				}
			}
			if (imported) m_importContentRequested = true;
		}
	}

	if (ImGui::BeginPopupModal("Create Content Folder", nullptr,
		ImGuiWindowFlags_AlwaysAutoResize)) {
		static char folderPath[260] = "";
		ImGui::TextUnformatted("Relative to Assets/ (e.g. Audio/Music)");
		ImGui::InputText("Folder", folderPath, IM_ARRAYSIZE(folderPath));
		if (ImGui::Button("Create")) {
			const std::string relativePath(folderPath);
			if (!relativePath.empty() &&
				relativePath.find("..") == std::string::npos) {
				std::error_code error;
				fs::create_directories(fs::path("Assets") / relativePath, error);
				if (!error) {
					folderPath[0] = '\0';
					ImGui::CloseCurrentPopup();
				}
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel")) ImGui::CloseCurrentPopup();
		ImGui::EndPopup();
	}
	ImGui::Separator();
	
	if (ImGui::BeginTabBar("##ContentTabs")) {
		if (ImGui::BeginTabItem("Models")) {
			std::vector<std::string> models;
			std::error_code error;
			const fs::path modelsRoot("Assets/Models");
			for (const fs::directory_entry& entry :
				fs::recursive_directory_iterator(modelsRoot, error)) {
				if (!error && entry.is_regular_file() && IsModelFile(entry.path())) {
					models.push_back(ToContentPath(
						entry.path().lexically_relative(modelsRoot)));
				}
			}
			if (models.empty()) ImGui::TextDisabled("No hay modelos en Assets/Models");
			const float cell = 90.0f;
			float availW = ImGui::GetContentRegionAvail().x;
			int perRow = (int)(availW / (cell + 10.0f)); if (perRow < 1) perRow = 1;
			int col = 0;
			for (const std::string& m : models) {
				ImGui::PushID(m.c_str());
				ImGui::BeginGroup();
				ImGui::Button("3D Model", ImVec2(cell, cell));

				if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
					std::string fullPath = "Assets/Models/" + m;
					ImGui::SetDragDropPayload("DND_MODEL_PATH", fullPath.c_str(), fullPath.size() + 1);
					ImGui::Text("Spawning %s", m.c_str());
					ImGui::EndDragDropSource();
				}

				if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s (arrastrar al Viewport o doble clic)", m.c_str());
				if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
					m_assetSpawnPath = "Assets/Models/" + m;
					m_assetSpawnRequested = true;
				}
				ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + cell);
				ImGui::TextWrapped("%s", m.c_str());
				ImGui::PopTextWrapPos();
				if (ImGui::SmallButton("Spawn")) {
					m_assetSpawnPath = "Assets/Models/" + m;
					m_assetSpawnRequested = true;
				}
				ImGui::SameLine();
				if (ImGui::SmallButton("Remove")) {
					m_assetDeletePath = "Assets/Models/" + m;
					m_assetDeleteRequested = true;
				}
				ImGui::EndGroup();
				ImGui::PopID();
				if (++col < perRow) ImGui::SameLine(); else col = 0;
			}
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Textures")) {
			if (textureThumbs.empty()) ImGui::TextDisabled("No hay texturas cargadas");
			const float cell = 84.0f;
			float availW = ImGui::GetContentRegionAvail().x;
			int perRow = (int)(availW / (cell + 10.0f)); if (perRow < 1) perRow = 1;
			int col = 0;
			for (const AssetThumb& t : textureThumbs) {
				ImGui::PushID(t.name.c_str());
				ImGui::BeginGroup();
				if (t.srv) ImGui::Image((ImTextureID)t.srv, ImVec2(cell, cell));
				else       ImGui::Dummy(ImVec2(cell, cell));
				
				if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
					std::string fullPath = "Assets/Textures/" + t.name;
					ImGui::SetDragDropPayload("DND_TEXTURE_PATH", fullPath.c_str(), fullPath.size() + 1);
					ImGui::Text("Applying %s", t.name.c_str());
					ImGui::EndDragDropSource();
				}
				
				if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", t.name.c_str());
				ImGui::EndGroup();
				ImGui::PopID();
				if (++col < perRow) ImGui::SameLine(); else col = 0;
			}
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Audio")) {
			std::vector<std::string> sounds;
			std::error_code error;
			const fs::path audioRoot("Assets/Audio");
			for (const fs::directory_entry& entry :
				fs::recursive_directory_iterator(audioRoot, error)) {
				if (!error && entry.is_regular_file() &&
					IsPlayableAudioFile(entry.path())) {
					sounds.push_back(ToContentPath(
						entry.path().lexically_relative(audioRoot)));
				}
			}
			if (sounds.empty()) {
				ImGui::TextDisabled("No hay audio WAV en Assets/Audio");
			}
			for (const std::string& sound : sounds) {
				ImGui::PushID(sound.c_str());
				ImGui::Button("WAV", ImVec2(72.0f, 36.0f));
				if (ImGui::BeginDragDropSource(
					ImGuiDragDropFlags_SourceAllowNullID)) {
					const std::string fullPath = "Assets/Audio/" + sound;
					ImGui::SetDragDropPayload("DND_AUDIO_PATH", fullPath.c_str(),
						fullPath.size() + 1);
					ImGui::Text("Assign %s", sound.c_str());
					ImGui::EndDragDropSource();
				}
				ImGui::SameLine();
				ImGui::TextUnformatted(sound.c_str());
				ImGui::PopID();
			}
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
	ImGui::End();
}

void GUI::drawSelectionOutline(Camera& cam, const EU::Vector3& mn, const EU::Vector3& mx, const XMMATRIX& world) {
	if (!m_viewportDrawList) return;
	if (m_viewportSize.x < 16.0f || m_viewportSize.y < 16.0f) return;

	m_viewportDrawList->PushClipRect(m_viewportPos, ImVec2(m_viewportPos.x + m_viewportSize.x, m_viewportPos.y + m_viewportSize.y), true);

	XMMATRIX vp = cam.getView() * cam.getProj();

	ImVec2 pts[8];
	bool valid[8];
	for (int c = 0; c < 8; ++c) {
		float cx = (c & 1) ? mx.x : mn.x;
		float cy = (c & 2) ? mx.y : mn.y;
		float cz = (c & 4) ? mx.z : mn.z;
		XMVECTOR worldC = XMVector3TransformCoord(XMVectorSet(cx, cy, cz, 1.0f), world);
		XMVECTOR clip = XMVector4Transform(
			XMVectorSet(XMVectorGetX(worldC), XMVectorGetY(worldC), XMVectorGetZ(worldC), 1.0f), vp);
		float w = XMVectorGetW(clip);
		if (w <= 0.0001f) { valid[c] = false; pts[c] = ImVec2(0, 0); continue; }
		float ndcx = XMVectorGetX(clip) / w;
		float ndcy = XMVectorGetY(clip) / w;
		float sx = m_viewportPos.x + (ndcx * 0.5f + 0.5f) * m_viewportSize.x;
		float sy = m_viewportPos.y + (1.0f - (ndcy * 0.5f + 0.5f)) * m_viewportSize.y;
		pts[c] = ImVec2(sx, sy);
		valid[c] = true;
	}

	static const int edges[12][2] = {
		{0,1},{2,3},{4,5},{6,7},
		{0,2},{1,3},{4,6},{5,7},
		{0,4},{1,5},{2,6},{3,7}
	};

	for (int e = 0; e < 12; ++e) {
		int a = edges[e][0], b = edges[e][1];
		if (valid[a] && valid[b]) {
			m_viewportDrawList->AddLine(pts[a], pts[b], IM_COL32(0, 0, 0, 160), 4.0f);
			m_viewportDrawList->AddLine(pts[a], pts[b], IM_COL32(190, 140, 255, 240), 2.0f);
		}
	}
	
	m_viewportDrawList->PopClipRect();
}
