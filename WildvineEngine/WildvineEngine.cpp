#include "BaseApp.h"

namespace {
bool
DirectoryExists(const std::string& path) {
	const DWORD attributes = GetFileAttributesA(path.c_str());
	return attributes != INVALID_FILE_ATTRIBUTES &&
		(attributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
}

void
ConfigureWorkingDirectory() {
	char modulePath[MAX_PATH] = {};
	const DWORD length = GetModuleFileNameA(nullptr, modulePath, MAX_PATH);
	if (length == 0 || length == MAX_PATH) {
		return;
	}

	std::string executableDir(modulePath);
	const size_t lastSlash = executableDir.find_last_of("\\/");
	if (lastSlash == std::string::npos) {
		return;
	}
	executableDir.resize(lastSlash);

	if (DirectoryExists(executableDir + "\\Assets") &&
		DirectoryExists(executableDir + "\\Skybox")) {
		SetCurrentDirectoryA(executableDir.c_str());
		return;
	}

	const size_t parentSlash = executableDir.find_last_of("\\/");
	if (parentSlash == std::string::npos) {
		return;
	}

	std::string parentDir = executableDir.substr(0, parentSlash);
	if (DirectoryExists(parentDir + "\\Assets") &&
		DirectoryExists(parentDir + "\\Skybox")) {
		SetCurrentDirectoryA(parentDir.c_str());
	}
}
}

//--------------------------------------------------------------------------------------
// Entry point to the program. Initializes everything and goes into a message processing 
// loop. Idle time is used to render the scene.
//--------------------------------------------------------------------------------------
int WINAPI
wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow) {
	ConfigureWorkingDirectory();
	BaseApp app;
	return app.run(hInstance, nCmdShow);
}
