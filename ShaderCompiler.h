#pragma once

///DXC
#include <dxcapi.h>
#pragma comment(lib,"dxcompiler.lib")
#include<string>
#include <cassert>
#include <format>

class ShaderCompiler
{


private:
    ///DXCCompilerの初期化に必要な変数
    IDxcUtils* dxcUtils_ = nullptr;
    IDxcCompiler3* dxcCompiler_ = nullptr;
    IDxcIncludeHandler* includeHandler_ = nullptr;  //includeに対応するための設定

public:
    ShaderCompiler();
    ~ShaderCompiler();

    IDxcBlob* Compile(const std::wstring& filePath, const wchar_t* profile);
};

