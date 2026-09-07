#include "DebugConsole.h"
#include "windows.h"
#include "cstdio"
#include "iostream"

namespace {
	bool g_consoleOpen = false;
}

void OpenDebugConsole() {
	if (g_consoleOpen) {
		return;
	}

	if (!AllocConsole()) {
		return;
	}

	FILE* filePtr = nullptr;
	freopen_s(&filePtr, "CONOUT$", "w", stdout);
	freopen_s(&filePtr, "CONOUT$", "w", stderr);
	freopen_s(&filePtr, "CONIN$", "r", stdin);

	// Sync new console entries
	std::ios::sync_with_stdio(true);
	std::cout.clear();
	std::cerr.clear();
	std::cin.clear();

	SetConsoleTitleA("LCS - Debug Console");
	g_consoleOpen = true;
}

void CloseDebugConsole() {
	if (!g_consoleOpen) {
		return;
	}

	// NULL all stdouts
	FILE* filePtr = nullptr;
	freopen_s(&filePtr, "NUL", "w", stdout);
	freopen_s(&filePtr, "NUL", "w", stderr);
	freopen_s(&filePtr, "NUL", "r", stdin);

	FreeConsole();
	g_consoleOpen = false;
}

bool IsDebugConsoleOpen() {
	return g_consoleOpen;
}