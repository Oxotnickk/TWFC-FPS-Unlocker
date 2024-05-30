#include <unordered_map>
#include <Shlwapi.h>
#include <iostream>

#pragma comment(lib, "Shlwapi.lib")

struct KeySettings
{
    WORD keyF1;
    WORD keyF2;
};

std::unordered_map<std::string, WORD> keyMap =
{
    {"F1", VK_F1},
    {"F2", VK_F2},
    {"F3", VK_F3},
    {"F4", VK_F4},
    {"F5", VK_F5},
    {"F6", VK_F6},
    {"F7", VK_F7},
    {"F8", VK_F8},
    {"F9", VK_F9},
    {"F10", VK_F10},
    {"F11", VK_F11},
    {"F12", VK_F12},
};