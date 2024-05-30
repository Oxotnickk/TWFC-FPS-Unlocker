#include <Windows.h>
#include <TlHelp32.h>
#include <iostream>
#include <fstream>
#include <chrono>
#include <ctime>
#include <string>
#include <locale>
#include <codecvt>
#include <sstream>
#include "Keys.h"

bool bToggle1 = false;
bool bToggle2 = false;
bool firstRun = true;

std::string GetExecutablePath()
{
    wchar_t buffer[MAX_PATH];
    GetModuleFileName(NULL, buffer, MAX_PATH);
    std::wstring wideString(buffer);
    std::string::size_type pos = wideString.find_last_of(L"\\/");
    return std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t>().to_bytes(wideString.substr(0, pos));
}

void CreateINIFile(const KeySettings& settings) 
{
    std::string exePath = GetExecutablePath();
    std::string iniFilePath = exePath + "\\settings.ini";

    std::ofstream iniFile(iniFilePath);
    if (iniFile.is_open())
    {
        iniFile << "[Hotkeys]" << std::endl;
        std::string key1String = "F1";
        std::string key2String = "F2";

        if (PathFileExistsA(iniFilePath.c_str()))
        {
            char buffer[256];
            GetPrivateProfileStringA("Hotkeys", "Key1", "F1", buffer, sizeof(buffer), iniFilePath.c_str());
            key1String = buffer;
            GetPrivateProfileStringA("Hotkeys", "Key2", "F2", buffer, sizeof(buffer), iniFilePath.c_str());
            key2String = buffer;
        }

        iniFile << "Key1=" << key1String << std::endl;
        iniFile << "Key2=" << key2String << std::endl;
        iniFile.close();
    }
}

KeySettings ReadINIFile() 
{
    KeySettings settings;
    std::string exePath = GetExecutablePath();
    std::string iniFilePath = exePath + "\\settings.ini";

    if (!PathFileExistsA(iniFilePath.c_str())) 
    {
        settings.keyF1 = VK_F1;
        settings.keyF2 = VK_F2;
        CreateINIFile(settings);
    }
    else 
    {
        char buffer[256];
        GetPrivateProfileStringA("Hotkeys", "Key1", "F1", buffer, sizeof(buffer), iniFilePath.c_str());
        settings.keyF1 = keyMap[buffer];

        GetPrivateProfileStringA("Hotkeys", "Key2", "F2", buffer, sizeof(buffer), iniFilePath.c_str());
        settings.keyF2 = keyMap[buffer];
    }

    return settings;
}

void WriteToLog(const std::string& message)
{
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    struct tm logTime;

    if (localtime_s(&logTime, &now_time) == 0)
    {
        std::string exePath = GetExecutablePath();
        std::string logFilePath = exePath + "\\Engine.log";

        std::ofstream logFile(logFilePath, firstRun ? std::ios::trunc : std::ios::app);

        if (logFile.is_open())
        {
            logFile << "[" << logTime.tm_hour << ":" << logTime.tm_min << ":" << logTime.tm_sec << "] " << message << std::endl;
            logFile.close();
        }

        if (firstRun) 
        {
            firstRun = false;
        }
    }
}

DWORD_PTR GetBaseAddress(const wchar_t* moduleName)
{
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, GetCurrentProcessId());
    MODULEENTRY32 moduleEntry;
    moduleEntry.dwSize = sizeof(moduleEntry);

    if (Module32First(hSnapshot, &moduleEntry))
    {
        do
        {
            if (wcscmp(moduleEntry.szModule, moduleName) == 0)
            {
                CloseHandle(hSnapshot);
                return reinterpret_cast<DWORD_PTR>(moduleEntry.modBaseAddr);
            }
        } while (Module32Next(hSnapshot, &moduleEntry));
    }

    CloseHandle(hSnapshot);
    return 0;
}

DWORD WINAPI MainThread(HMODULE hModule)
{
    WriteToLog("[INFO] Module found");

    WriteToLog("[INFO] Offset found");

    WriteToLog("[INFO] Successfull");

    const wchar_t* moduleName = L"TWFC.exe";

    DWORD offset = 0x2103F30;

    DWORD_PTR baseAddress = GetBaseAddress(moduleName);
    if (baseAddress == 0)
    {
        WriteToLog("[ERROR] Failed to get the base address of the module");
        return 0;
    }

    DWORD address = baseAddress + offset;
    float* floatValue = reinterpret_cast<float*>(address);

    KeySettings settings = ReadINIFile();

    bool bPressed[2] = { false, false };

    while (true)
    {
        for (int i = 0; i < 2; ++i)
        {
            WORD currentKey = (i == 0) ? settings.keyF1 : settings.keyF2;
            if (GetAsyncKeyState(currentKey) & 0x8000)
            {
                if (!bPressed[i])
                {
                    if (i == 0)
                    {
                        bToggle1 = !bToggle1;
                    }
                    else
                    {
                        bToggle2 = !bToggle2;
                    }
                    bPressed[i] = true;
                }
            }
            else
            {
                bPressed[i] = false;
            }

            if (bToggle1 && floatValue)
            {
                *floatValue = 60.0f;
            }
            else if (bToggle2 && floatValue)
            {
                *floatValue = 80.0f;
            }
            else if (floatValue)
            {
                *floatValue = 30.0f;
            }

            Sleep(100);
        }
    }

    FreeLibraryAndExitThread(hModule, 0);
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        CloseHandle(CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)MainThread, hModule, 0, nullptr));
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}