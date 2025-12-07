#include "pch.h"
#include "ConsoleWidget.h"
#include "ObjectFactory.h"
#include "GlobalConsole.h"
#include "StatsOverlayD2D.h"
#include "USlateManager.h"
#include <windows.h>
#include <cstdarg>
#include <cctype>
#include <cstring>
#include <algorithm>
#include <mutex>

#include "Source/Runtime/Debug/CrashHandler.h"

using std::max;
using std::min;

IMPLEMENT_CLASS(UConsoleWidget)

UConsoleWidget::UConsoleWidget()
	: UWidget("Console Widget")
	, HistoryPos(-1)
	, AutoScroll(true)
	, ScrollToBottom(false)
{
	memset(InputBuf, 0, sizeof(InputBuf));
}

UConsoleWidget::~UConsoleWidget()
{
	// Clear the GlobalConsole pointer to prevent dangling pointer access
	if (UGlobalConsole::GetConsoleWidget() == this)
	{
		UGlobalConsole::SetConsoleWidget(nullptr);
	}
}

void UConsoleWidget::Initialize()
{
	ClearLog();

	// Help 커맨드를 입력했을 때 콘솔에 표시할 명령어 목록
	HelpCommandList.Add("HELP");
	HelpCommandList.Add("HISTORY");
	HelpCommandList.Add("CLEAR");
	HelpCommandList.Add("CRASH");
	HelpCommandList.Add("CLASSIFY");
	HelpCommandList.Add("STAT");
	HelpCommandList.Add("STAT FPS");
	HelpCommandList.Add("STAT MEMORY");
	HelpCommandList.Add("STAT PICKING");
	HelpCommandList.Add("STAT DECAL");
	HelpCommandList.Add("STAT ALL");
	HelpCommandList.Add("STAT NONE");
	HelpCommandList.Add("STAT LIGHT");
	HelpCommandList.Add("STAT SHADOW");

	// Add welcome messages
	AddLog("=== Console Widget Initialized ===");
	AddLog("Console initialized. Type 'HELP' for available commands.");
	AddLog("Testing console output functionality...");

	// Test UE_LOG after ConsoleWidget is created
	UE_LOG("ConsoleWidget: Successfully Initialized");
	UE_LOG("This message should appear in the console widget");
}

void UConsoleWidget::Update()
{
	std::lock_guard<std::mutex> Lock(PendingLogsMutex);
	if (!PendingLogs.IsEmpty())
	{
		Items.insert(Items.end(), PendingLogs.begin(), PendingLogs.end());
		PendingLogs.clear();
	}
}

void UConsoleWidget::RenderWidget()
{
	// Show basic info at top
	ImGui::Text("Console - %d messages", Items.Num());
	ImGui::Separator();

	// Main console area
	RenderToolbar();
	ImGui::Separator();
	RenderLogOutput();
	ImGui::Separator();
	RenderCommandInput();
}

void UConsoleWidget::RenderToolbar()
{
	if (ImGui::SmallButton("Add Debug Text"))
	{
		AddLog("%d some text", Items.Num());
		AddLog("some more text");
		AddLog("display very important message here!");
	}
	ImGui::SameLine();

	if (ImGui::SmallButton("Add Debug Error"))
	{
		AddLog("[error] something went wrong");
	}
	ImGui::SameLine();

	if (ImGui::SmallButton("Clear"))
	{
		ClearLog();
	}
	ImGui::SameLine();

	if (ImGui::SmallButton("Copy"))
	{
		FString clipboard_text;

		// 선택된 텍스트가 있으면 선택 영역만 복사
		if (TextSelection.IsActive())
		{
			int32 Start = std::min(TextSelection.StartLine, TextSelection.EndLine);
			int32 End = std::max(TextSelection.StartLine, TextSelection.EndLine);

			if (Start >= 0 && End >= 0 && Start < Items.Num() && End < Items.Num())
			{
				int32 idx = 0;
				for (const FString& item : Items)
				{
					if (idx >= Start && idx <= End)
					{
						clipboard_text += item + "\n";
					}
					++idx;
				}
			}
		}
		else
		{
			// 선택 영역이 없으면 전체 복사
			for (const FString& item : Items)
			{
				clipboard_text += item + "\n";
			}
		}

		if (!clipboard_text.empty())
		{
			ImGui::SetClipboardText(clipboard_text.c_str());
			if (TextSelection.IsActive())
			{
				int32 LineCount = std::abs(TextSelection.EndLine - TextSelection.StartLine) + 1;
				AddLog("[info] Copied %d selected line(s) to clipboard", LineCount);
			}
			else
			{
				AddLog("[info] Copied %d lines to clipboard", Items.Num());
			}
		}
	}

	// Options menu
	ImGui::SameLine();
	if (ImGui::Button("Options"))
		ImGui::OpenPopup("Options");

	if (ImGui::BeginPopup("Options"))
	{
		ImGui::Checkbox("Auto-scroll", &AutoScroll);
		ImGui::EndPopup();
	}

	ImGui::SameLine();
	Filter.Draw("Filter", 180);
}

void UConsoleWidget::RenderLogOutput()
{
	// Reserve space for input at bottom
	const float footer_height_to_reserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();

	if (ImGui::BeginChild("ScrollingRegion", ImVec2(0, -footer_height_to_reserve),
		ImGuiChildFlags_NavFlattened, ImGuiWindowFlags_HorizontalScrollbar))
	{
		if (ImGui::BeginPopupContextWindow())
		{
			if (ImGui::Selectable("Clear")) ClearLog();
			ImGui::EndPopup();
		}

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1)); // Tighten spacing

		ImDrawList* DrawList = ImGui::GetWindowDrawList();
		float LineHeight = ImGui::GetTextLineHeight();

		// 마우스 입력 처리
		ImVec2 MousePos = ImGui::GetMousePos();
		bool bIsWindowHovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows);

		// ESC 또는 다른 곳 클릭 시 선택 해제
		if (ImGui::IsKeyPressed(ImGuiKey_Escape))
		{
			TextSelection.Clear();
			bIsDragging = false;
		}

		// 윈도우 밖 클릭 시 선택 해제
		if (!bIsWindowHovered && ImGui::IsMouseClicked(0))
		{
			TextSelection.Clear();
			bIsDragging = false;
		}

		// 선택 영역 계산
		int32 StartLine = TextSelection.IsActive() ? std::min(TextSelection.StartLine, TextSelection.EndLine) : -1;
		int32 EndLine = TextSelection.IsActive() ? std::max(TextSelection.StartLine, TextSelection.EndLine) : -1;

		// 1단계: 로그 렌더링하면서 위치 정보 수집
		struct FRenderedLine
		{
			ImVec2 Min;
			ImVec2 Max;
			int32 Index;
		};
		TArray<FRenderedLine> RenderedLines;
		RenderedLines.Reserve(Items.Num());

		int32 LogIndex = 0;
		for (const FString& item : Items)
		{
			if (!Filter.PassFilter(item.c_str()))
			{
				++LogIndex;
				continue;
			}

			// Color coding for different log levels
			ImVec4 color;
			bool has_color = false;

			if (item.find("[error]") != std::string::npos)
			{
				color = ImVec4(1.0f, 0.4f, 0.4f, 1.0f);
				has_color = true;
			}
			else if (item.find("[warning]") != std::string::npos)
			{
				color = ImVec4(1.0f, 0.8f, 0.0f, 1.0f);
				has_color = true;
			}
			else if (item.find("[info]") != std::string::npos)
			{
				color = ImVec4(0.0f, 0.8f, 1.0f, 1.0f);
				has_color = true;
			}

			if (has_color)
				ImGui::PushStyleColor(ImGuiCol_Text, color);

			ImVec2 LineMin = ImGui::GetCursorScreenPos();
			ImGui::TextUnformatted(item.c_str());
			ImVec2 LineMax = ImGui::GetCursorScreenPos();

			if (has_color)
				ImGui::PopStyleColor();

			// 렌더링된 라인 정보 저장
			ImVec2 TextSize = ImGui::CalcTextSize(item.c_str());
			RenderedLines.Add({
				LineMin,
				ImVec2(LineMin.x + TextSize.x, LineMax.y),
				LogIndex
			});

			++LogIndex;
		}

		// 마우스 드래그 처리
		static int32 ClickedLineOnPress = -1;
		static bool bWasSingleLineSelected = false;
		static int32 PreviouslySelectedLine = -1;

		if (bIsWindowHovered && ImGui::IsMouseClicked(0))
		{
			// 클릭 전 선택 상태 저장
			bWasSingleLineSelected = (TextSelection.IsActive() && TextSelection.StartLine == TextSelection.EndLine);
			PreviouslySelectedLine = bWasSingleLineSelected ? TextSelection.StartLine : -1;

			// 클릭한 라인 찾기
			ClickedLineOnPress = -1;
			for (const FRenderedLine& Line : RenderedLines)
			{
				if (MousePos.y >= Line.Min.y && MousePos.y < Line.Min.y + LineHeight)
				{
					ClickedLineOnPress = Line.Index;
					break;
				}
			}

			// 다중 라인 선택 상태에서 클릭 시 선택 해제만
			if (TextSelection.IsActive() && !bWasSingleLineSelected)
			{
				TextSelection.Clear();
				bIsDragging = false;
			}
			// 단일 라인 또는 선택 없는 상태에서 클릭 시 새로운 선택 시작
			else if (ClickedLineOnPress >= 0)
			{
				bIsDragging = true;
				TextSelection.StartLine = ClickedLineOnPress;
				TextSelection.EndLine = ClickedLineOnPress;
			}
		}

		if (bIsDragging && ImGui::IsMouseDown(0))
		{
			// 드래그 중 - EndLine 업데이트
			for (const FRenderedLine& Line : RenderedLines)
			{
				if (MousePos.y >= Line.Min.y && MousePos.y < Line.Min.y + LineHeight)
				{
					TextSelection.EndLine = Line.Index;
					break;
				}
			}
		}

		if (ImGui::IsMouseReleased(0))
		{
			// 단일 라인 토글: 드래그 없이 같은 라인을 다시 클릭한 경우
			if (bIsDragging && bWasSingleLineSelected &&
				TextSelection.IsActive() &&
				TextSelection.StartLine == TextSelection.EndLine &&
				PreviouslySelectedLine == ClickedLineOnPress &&
				!ImGui::IsMouseDragging(0, 1.0f))
			{
				TextSelection.Clear();
			}

			bIsDragging = false;
		}

		// 선택 영역 하이라이트
		if (TextSelection.IsActive())
		{
			const ImU32 HighlightColor = IM_COL32(70, 100, 200, 128);

			if (bIsDragging)
			{
				// 드래그 중: 화면 밖 선택 영역도 표시
				ImVec2 WindowPos = ImGui::GetWindowPos();
				float VirtualY = WindowPos.y + ImGui::GetStyle().WindowPadding.y - ImGui::GetScrollY();

				for (int32 idx = 0; idx < Items.Num(); ++idx)
				{
					if (idx < StartLine || idx > EndLine) continue;

					// 화면에 렌더링된 라인 찾기
					bool bFound = false;
					for (const FRenderedLine& Line : RenderedLines)
					{
						if (Line.Index == idx)
						{
							DrawList->AddRectFilled(Line.Min, ImVec2(Line.Max.x, Line.Min.y + LineHeight), HighlightColor);
							bFound = true;
							break;
						}
					}

					// 화면 밖 라인은 가상 위치에 표시
					if (!bFound)
					{
						ImVec2 SelMin = ImVec2(WindowPos.x, VirtualY + idx * LineHeight);
						ImVec2 SelMax = ImVec2(SelMin.x + 1000.0f, SelMin.y + LineHeight);
						DrawList->AddRectFilled(SelMin, SelMax, HighlightColor);
					}
				}
			}
			else
			{
				// 드래그 완료: 화면에 보이는 것만 하이라이트
				for (const FRenderedLine& Line : RenderedLines)
				{
					if (Line.Index >= StartLine && Line.Index <= EndLine)
					{
						DrawList->AddRectFilled(Line.Min, ImVec2(Line.Max.x, Line.Min.y + LineHeight), HighlightColor);
					}
				}
			}
		}

		// Ctrl+C로 복사
		if (TextSelection.IsActive() && ImGui::IsKeyDown(ImGuiKey_LeftCtrl) && ImGui::IsKeyPressed(ImGuiKey_C))
		{
			FString SelectedText;
			int32 Start = std::min(TextSelection.StartLine, TextSelection.EndLine);
			int32 End = std::max(TextSelection.StartLine, TextSelection.EndLine);

			if (Start >= 0 && End >= 0 && Start < Items.Num() && End < Items.Num())
			{
				int32 idx = 0;
				for (const FString& item : Items)
				{
					if (idx >= Start && idx <= End)
					{
						SelectedText += item + "\n";
					}
					++idx;
				}

				ImGui::SetClipboardText(SelectedText.c_str());
			}
		}

		// Auto scroll to bottom
		if (ScrollToBottom || (AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()))
			ImGui::SetScrollHereY(1.0f);
		ScrollToBottom = false;

		ImGui::PopStyleVar();
	}
	ImGui::EndChild();
}

void UConsoleWidget::RenderCommandInput()
{
	// Command input
	bool reclaim_focus = false;
	ImGuiInputTextFlags input_text_flags = ImGuiInputTextFlags_EnterReturnsTrue |
		ImGuiInputTextFlags_EscapeClearsAll |
		ImGuiInputTextFlags_CallbackCompletion |
		ImGuiInputTextFlags_CallbackHistory;

	if (ImGui::InputText("Input", InputBuf, sizeof(InputBuf), input_text_flags, &TextEditCallbackStub, (void*)this))
	{
		char* s = InputBuf;
		Strtrim(s);
		if (s[0])
			ExecCommand(s);
		strcpy_s(InputBuf, sizeof(InputBuf), "");
		reclaim_focus = true;
	}

	// Auto-focus
	ImGui::SetItemDefaultFocus();
	if (reclaim_focus)
		ImGui::SetKeyboardFocusHere(-1);
}

void UConsoleWidget::AddLog(const char* fmt, ...)
{
	char buf[1024];
	va_list args;
	va_start(args, fmt);
	vsnprintf_s(buf, sizeof(buf), fmt, args);
	buf[sizeof(buf) - 1] = 0;
	va_end(args);

	if (strstr(buf, "[error]") != nullptr)
	{
		USlateManager::GetInstance().ForceOpenConsole();
	}

	{
		std::lock_guard<std::mutex> Lock(PendingLogsMutex);
		PendingLogs.Add(FString(buf));
	}
	ScrollToBottom = true;
}

void UConsoleWidget::VAddLog(const char* fmt, va_list args)
{
	char buf[1024];
	vsnprintf_s(buf, sizeof(buf), fmt, args);
	buf[sizeof(buf) - 1] = 0;

	if (strstr(buf, "[error]") != nullptr)
	{
		USlateManager::GetInstance().ForceOpenConsole();
	}

	{
		std::lock_guard<std::mutex> Lock(PendingLogsMutex);
		PendingLogs.Add(FString(buf));
	}
	ScrollToBottom = true;
}

void UConsoleWidget::ClearLog()
{
	Items.Empty();

	std::lock_guard<std::mutex> Lock(PendingLogsMutex);
	PendingLogs.Empty();
}

void UConsoleWidget::ExecCommand(const char* command_line)
{
	AddLog("# %s", command_line);

	// Add to history
	HistoryPos = -1;
	for (int32 i = History.Num() - 1; i >= 0; i--)
	{
		if (Stricmp(History[i].c_str(), command_line) == 0)
		{
			History.RemoveAt(i);
			break;
		}
	}
	History.Add(FString(command_line));

	// Process command
	if (Stricmp(command_line, "CLEAR") == 0)
	{
		ClearLog();
	}
	else if (Stricmp(command_line, "CRASH") == 0)
	{
		FCrashHandler::InjectCrash();
	}
	else if (Stricmp(command_line, "HELP") == 0)
	{
		AddLog("Commands:");
		for (const FString& cmd : HelpCommandList)
			AddLog("- %s", cmd.c_str());
	}
	else if (Stricmp(command_line, "HISTORY") == 0)
	{
		int32 first = History.Num() - 10;
		for (int32 i = max(0, first); i < History.Num(); i++)
			AddLog("%3d: %s", i, History[i].c_str());
	}
	else if (Stricmp(command_line, "CLASSIFY") == 0)
	{
		AddLog("This is a classification test command.");
	}
	else if (Stricmp(command_line, "STAT") == 0)
	{
		AddLog("STAT commands:");
		AddLog("- STAT FPS");
		AddLog("- STAT MEMORY");
		AddLog("- STAT PICKING");
		AddLog("- STAT DECAL");
		AddLog("- STAT ALL");
		AddLog("- STAT LIGHT");
		AddLog("- STAT NONE");
	}
	else if (Stricmp(command_line, "STAT FPS") == 0)
	{
		UStatsOverlayD2D::Get().ToggleFPS();
		AddLog("STAT FPS TOGGLED");
	}
	else if (Stricmp(command_line, "STAT MEMORY") == 0)
	{
		UStatsOverlayD2D::Get().ToggleMemory();
		AddLog("STAT MEMORY TOGGLED");
	}
	else if (Stricmp(command_line, "STAT PICKING") == 0)
	{
		UStatsOverlayD2D::Get().TogglePicking();
		AddLog("STAT PICKING TOGGLED");
	}
	else if (Stricmp(command_line, "STAT DECAL") == 0)
	{
		UStatsOverlayD2D::Get().ToggleDecal();
		AddLog("STAT DECAL TOGGLED");
	}
	else if (Stricmp(command_line, "STAT LIGHT") == 0)
	{
		UStatsOverlayD2D::Get().ToggleTileCulling();
		AddLog("STAT LIGHT TOGGLED");
	}
	else if (Stricmp(command_line, "STAT ALL") == 0)
	{
		UStatsOverlayD2D::Get().SetShowFPS(true);
		UStatsOverlayD2D::Get().SetShowMemory(true);
		UStatsOverlayD2D::Get().SetShowPicking(true);
		UStatsOverlayD2D::Get().SetShowDecal(true);
		UStatsOverlayD2D::Get().SetShowTileCulling(true);
		AddLog("STAT: ON");
	}
	else if (Stricmp(command_line, "STAT NONE") == 0)
	{
		UStatsOverlayD2D::Get().SetShowFPS(false);
		UStatsOverlayD2D::Get().SetShowMemory(false);
		UStatsOverlayD2D::Get().SetShowPicking(false);
		UStatsOverlayD2D::Get().SetShowDecal(false);
		UStatsOverlayD2D::Get().SetShowTileCulling(false);
		AddLog("STAT: OFF");
	}
	else
	{
		AddLog("Unknown command: '%s'", command_line);
	}

	ScrollToBottom = true;
}

// Static helper methods
int UConsoleWidget::Stricmp(const char* s1, const char* s2)
{
	int d;
	while ((d = toupper(*s2) - toupper(*s1)) == 0 && *s1)
	{
		s1++;
		s2++;
	}
	return d;
}

int UConsoleWidget::Strnicmp(const char* s1, const char* s2, int n)
{
	int d = 0;
	while (n > 0 && (d = toupper(*s2) - toupper(*s1)) == 0 && *s1)
	{
		s1++;
		s2++;
		n--;
	}
	return d;
}

void UConsoleWidget::Strtrim(char* s)
{
	char* str_end = s + strlen(s);
	while (str_end > s && str_end[-1] == ' ')
		str_end--;
	*str_end = 0;
}

int UConsoleWidget::TextEditCallbackStub(ImGuiInputTextCallbackData* data)
{
	UConsoleWidget* console = (UConsoleWidget*)data->UserData;
	return console->TextEditCallback(data);
}

int UConsoleWidget::TextEditCallback(ImGuiInputTextCallbackData* data)
{
	switch (data->EventFlag)
	{
	case ImGuiInputTextFlags_CallbackCompletion:
	{
		// Find word boundaries
		const char* word_end = data->Buf + data->CursorPos;
		const char* word_start = word_end;
		while (word_start > data->Buf)
		{
			const char c = word_start[-1];
			if (c == ' ' || c == '\t' || c == ',' || c == ';')
				break;
			word_start--;
		}

		// Build candidates list
		TArray<FString> candidates;
		for (const FString& cmd : HelpCommandList)
		{
			if (Strnicmp(cmd.c_str(), word_start, (int)(word_end - word_start)) == 0)
				candidates.Add(cmd);
		}

		if (candidates.Num() == 0)
		{
			AddLog("No match for \"%.*s\"!", (int)(word_end - word_start), word_start);
		}
		else if (candidates.Num() == 1)
		{
			// Single match - complete it
			data->DeleteChars((int)(word_start - data->Buf), (int)(word_end - word_start));
			data->InsertChars(data->CursorPos, candidates[0].c_str());
			data->InsertChars(data->CursorPos, " ");
		}
		else
		{
			// Multiple matches - show them
			int match_len = (int)(word_end - word_start);
			for (;;)
			{
				int c = 0;
				bool all_candidates_matches = true;
				for (int32 i = 0; i < candidates.Num() && all_candidates_matches; i++)
				{
					if (i == 0)
						c = toupper(candidates[i][match_len]);
					else if (c == 0 || c != toupper(candidates[i][match_len]))
						all_candidates_matches = false;
				}
				if (!all_candidates_matches)
					break;
				match_len++;
			}

			if (match_len > 0)
			{
				data->DeleteChars((int)(word_start - data->Buf), (int)(word_end - word_start));
				data->InsertChars(data->CursorPos, candidates[0].c_str(), candidates[0].c_str() + match_len);
			}

			AddLog("Possible matches:");
			for (const FString& candidate : candidates)
				AddLog("- %s", candidate.c_str());
		}
		break;
	}
	case ImGuiInputTextFlags_CallbackHistory:
	{
		const int32 prev_history_pos = HistoryPos;
		if (data->EventKey == ImGuiKey_UpArrow)
		{
			if (HistoryPos == -1)
				HistoryPos = History.Num() - 1;
			else if (HistoryPos > 0)
				HistoryPos--;
		}
		else if (data->EventKey == ImGuiKey_DownArrow)
		{
			if (HistoryPos != -1)
				if (++HistoryPos >= History.Num())
					HistoryPos = -1;
		}

		if (prev_history_pos != HistoryPos)
		{
			const char* history_str = (HistoryPos >= 0) ? History[HistoryPos].c_str() : "";
			data->DeleteChars(0, data->BufTextLen);
			data->InsertChars(0, history_str);
		}
		break;
	}
	}
	return 0;
}
