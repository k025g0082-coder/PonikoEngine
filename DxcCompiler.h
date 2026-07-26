#pragma once

#include <string>
#include <cassert>

#include <d3d12.h>

#include <dxcapi.h>
#pragma comment(lib, "dxcompiler.lib")

#include <wrl/client.h>      // ComPtr


#include <windows.h>

class DxcCompiler
{
public:

	// ---CompileShader関数---
	IDxcBlob* CompileShader(
		const std::wstring& filePath, // compileするファイルのパス
		const wchar_t* profile,		  // compileするprofile
		IDxcUtils* dxcUtils,		  // 初期化で生成する３つ
		IDxcCompiler3* dxcCompiler,
		IDxcIncludeHandler* includeHandler
	);

	// 初期化、兼Dxcの初期化
	void InitializeDxc();

	// げったー。piplineの設定の際に必要
	IDxcUtils* GetIDxcUtils() { return dxcUtils;}
	IDxcCompiler3* GetDxcCompiler() { return dxcCompiler; }
	IDxcIncludeHandler* GetIncludeHandler() { return includeHandler; }

private:

	

	IDxcUtils* dxcUtils = nullptr;
	IDxcCompiler3* dxcCompiler = nullptr;
	IDxcIncludeHandler* includeHandler = nullptr;
};

