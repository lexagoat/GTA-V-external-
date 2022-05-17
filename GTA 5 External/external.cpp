#include<Windows.h>
#include<iostream>
#include<TlHelp32.h>
#include<vector>

uintptr_t GetPID(const wchar_t* procName) {
    PROCESSENTRY32 entry;
    entry.dwSize = sizeof(PROCESSENTRY32);
    uintptr_t pID = NULL;
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
    if (Process32First(snapshot, &entry)) {
        do {
            if (_wcsicmp(entry.szExeFile, procName) == 0) {
                pID = entry.th32ProcessID;
                break;
            }
        } while (Process32Next(snapshot, &entry));
    }
    CloseHandle(snapshot);
    return pID;
}

uintptr_t GetModuleBaseAddress(DWORD procId, const wchar_t* modName)
{
    uintptr_t modBaseAddr = 0;
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, procId);
    if (hSnap != INVALID_HANDLE_VALUE)
    {
        MODULEENTRY32 modEntry;
        modEntry.dwSize = sizeof(modEntry);
        if (Module32First(hSnap, &modEntry))
        {
            do
            {
                if (!_wcsicmp(modEntry.szModule, modName))
                {
                    modBaseAddr = (uintptr_t)modEntry.modBaseAddr;
                    break;
                }
            } while (Module32Next(hSnap, &modEntry));
        }
    }
    CloseHandle(hSnap);
    return modBaseAddr;
}

uintptr_t FindPTR(HANDLE hproc, uintptr_t ptr, std::vector<unsigned int>offsets)
{
    uintptr_t addr = ptr;
    for (unsigned int i = 0; i < offsets.size(); ++i)
    {
        ReadProcessMemory(hproc, (BYTE*)addr, &addr, sizeof(addr), 0);
        addr += offsets[i];
    }
    return addr;
}

uintptr_t HealthPTR = 0x02597248;
uintptr_t WantedLevel_Addr = 0x0252B218;

uintptr_t HealthBasePTR;
uintptr_t WantedLevelBasePTR;

HWND Game;
HDC Game_window;
HFONT Font;
COLORREF TextColor;

float ScreenX = GetSystemMetrics(SM_CXSCREEN); 
float ScreenY = GetSystemMetrics(SM_CYSCREEN); 

void DrawStringOnScreen(int ScreenX, int ScreenY, COLORREF color, const char* text)
{
    SetTextAlign(Game_window, TA_CENTER | TA_NOUPDATECP);
    SetBkColor(Game_window, RGB(0, 0, 0));
    SetBkMode(Game_window, TRANSPARENT);
    SetTextColor(Game_window, color);
    SelectObject(Game_window, Font);
    TextOutA(Game_window, ScreenX, ScreenY, text, strlen(text));
    DeleteObject(Font);
}

int main()
{
    SetConsoleTitleA("GTA-V 1.48: External");

    uintptr_t Procid = GetPID(L"GTA5.exe");
    uintptr_t ModuleBaseAddress = GetModuleBaseAddress(Procid, L"GTA5.exe");

    HANDLE hProc = 0;
    hProc = OpenProcess(PROCESS_ALL_ACCESS, false, Procid);

    while (Procid == NULL)
    {
        std::cout << "Waiting For Game";
        std::cout << ".";
        Sleep(200);
        std::cout << ".";
        Sleep(200);
        std::cout << ".";
        system("cls");
        Procid = GetPID(L"GTA5.exe");
    }

    if (hProc == NULL)
    {
        std::cout << "Failed To Obtain Handle" << "\n" << std::endl;
        std::cout << "Restart The Hack.";
        getchar();
        exit(0);
    }

    Game = FindWindowA(0, "Grand Theft Auto V");
    Game_window = GetDC(Game);

    HealthBasePTR = ModuleBaseAddress + HealthPTR;
    std::vector<unsigned int> HealthOffsets = { 0x78, 0x690, 0xF0, 0x0, 0x280 };
    uintptr_t Health = FindPTR(hProc, HealthBasePTR, HealthOffsets);

    WantedLevelBasePTR = ModuleBaseAddress + WantedLevel_Addr;
    std::vector<unsigned int> WantedLevelOffset = { 0x5D8 };
    uintptr_t WantedLevel = FindPTR(hProc, WantedLevelBasePTR, WantedLevelOffset);

    bool GodMode = false;
    bool update_console = true;
    bool Draw = false;
    float Health_Left = 0;

    while (true)
    {
        if (update_console)
        {
            system("cls");
            std::cout << "[INS] - GodMode: " << (GodMode ? "ON" : "OFF") << std::endl;
            std::cout << "[DEL] - Remove Wanted Level" << std::endl;
            std::cout << "[END] - Draw Health on Screen: " << (Draw ? "ON" : "OFF") << std::endl;
            update_console = false;
        }
        if (GetAsyncKeyState(VK_INSERT) & 1)
        {
            GodMode = !GodMode;
            update_console = !update_console;
            if (GodMode)
            {
                float GodmodeValue = 90000;

                WriteProcessMemory(hProc, (LPVOID)Health, &GodmodeValue, sizeof(GodmodeValue), nullptr);
            }
            else
            {
                !GodMode;
                !update_console;
                float GodmodeOFF = 328;
                WriteProcessMemory(hProc, (LPVOID)Health, &GodmodeOFF, sizeof(GodmodeOFF), nullptr);
            }
        }
        if (GetAsyncKeyState(VK_DELETE) & 1)
        {
            update_console = !update_console;
            BYTE RemoveWanted = 0;
            WriteProcessMemory(hProc, (BYTE*)WantedLevel, &RemoveWanted, sizeof(RemoveWanted), nullptr);
        }
        if (GetAsyncKeyState(VK_END) & 1)
        {
            update_console = !update_console;
            Draw = !Draw;
            if (Draw)
            {
                while (Draw)
                {
                    if (Health_Left >= 150)
                        TextColor = RGB(0, 255, 0);
                    else
                        TextColor = RGB(255, 0, 0);

                    char HealthBuffer[255];
                    ReadProcessMemory(hProc, (LPVOID)Health, &Health_Left, sizeof(Health_Left), nullptr);
                    sprintf_s(HealthBuffer, sizeof(HealthBuffer), "Health: %i", (int)Health_Left);
                    DrawStringOnScreen(ScreenX / 3.7, ScreenY - 190, TextColor, HealthBuffer);

                    if (GetAsyncKeyState(VK_END) & 1)
                    {
                        !update_console;
                        !Draw;
                        break;
                    }
                }
            }
        }
    }
    CloseHandle(hProc);
}