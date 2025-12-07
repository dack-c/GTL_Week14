#pragma once
#include <mutex>
#include "Widget.h"
#include "Vector.h"
#include "ImGui/imgui.h"

/**
 * @brief Console Widget for displaying log messages and executing commands
 * Based on ImGuiConsole but restructured to fit our Widget system
 */
class UConsoleWidget : public UWidget
{
public:
	DECLARE_CLASS(UConsoleWidget, UWidget)

	void Initialize() override;
	void Update() override;
	void RenderWidget() override;

	// Console specific methods
	void AddLog(const char* fmt, ...);
	void VAddLog(const char* fmt, va_list args);
	void ClearLog();
	void ExecCommand(const char* command_line);

	// Special Member Function
	UConsoleWidget();
	~UConsoleWidget() override;

private:
	// Console data
	char InputBuf[256];
	TArray<FString> Items;
	TArray<FString> PendingLogs;
	mutable std::mutex PendingLogsMutex;
	TArray<FString> HelpCommandList;
	TArray<FString> History;
	int32 HistoryPos;

	// UI state
	bool AutoScroll;
	bool ScrollToBottom;
	ImGuiTextFilter Filter;

	// Text selection state
	struct FTextSelection
	{
		int32 StartLine = -1;
		int32 EndLine = -1;

		bool IsActive() const { return StartLine >= 0 && EndLine >= 0; }
		void Clear() { StartLine = -1; EndLine = -1; }
	};

	FTextSelection TextSelection;
	bool bIsDragging = false;

	// Helper methods
	static int TextEditCallbackStub(ImGuiInputTextCallbackData* data);
	int TextEditCallback(ImGuiInputTextCallbackData* data);

	// String utilities
	static int Stricmp(const char* s1, const char* s2);
	static int Strnicmp(const char* s1, const char* s2, int n);
	static void Strtrim(char* s);

	// Rendering helpers
	void RenderLogOutput();
	void RenderCommandInput();
	void RenderToolbar();
};
