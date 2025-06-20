#pragma once
#include <Windows.h>
#include<strsafe.h>
// Debug用のあれこれを使えるようにする
#include <dbghelp.h>
#pragma comment (lib,"Dbghelp.lib")
#include <filesystem>

class Dump {
public:
    static void Initialize();
    static LONG WINAPI ExportDump(EXCEPTION_POINTERS* exception);
};
