#include "DxcCompiler.h"
#include "DirectXUtility.h"

/*=== Dxcの初期化 === */
void DxcCompiler::InitializeDxc() {

	// dxcompilerを使うための初期化

	HRESULT hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils));

	assert(SUCCEEDED(hr));

	hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler));

	assert(SUCCEEDED(hr));

	// 現時点でincludeはしないが、includeもできるようにするためのインクルーダーを作成
	hr = dxcUtils->CreateDefaultIncludeHandler(&includeHandler);
	assert(SUCCEEDED(hr));
};

IDxcBlob* DxcCompiler::CompileShader(
	const std::wstring& filePath, // compileするファイルのパス
	const wchar_t* profile,		  // compileするprofile
	IDxcUtils* dxcUtils,		  // 初期化で生成する３つ
	IDxcCompiler3* dxcCompiler,
	IDxcIncludeHandler* includeHandler,
	const wchar_t* entryPoint
) {
	OutputDebugStringW(profile);
	OutputDebugStringW(L"\n");
	//--- 1. ファイルの読み込み

	//Log(ConvertString(std::format("Begin CompileShader, path{},profile:{}\n", filePath,profile)));

	// hlslファイルを読む
	IDxcBlobEncoding* shaderSource = nullptr;
	HRESULT hr = dxcUtils->LoadFile(filePath.c_str(), nullptr, &shaderSource);

	assert(SUCCEEDED(hr));

	// 読み込んだファイルの内容を設定
	DxcBuffer shaderSourceBuffer;
	shaderSourceBuffer.Ptr = shaderSource->GetBufferPointer();
	shaderSourceBuffer.Size = shaderSource->GetBufferSize();
	shaderSourceBuffer.Encoding = DXC_CP_UTF8; // UTF-8でエンコードされていると指定

	//--- 2. Compileする
	LPCWSTR arguments[] = {
		L"-E", entryPoint, // entry pointを指定
		L"-T", profile, // profileを指定
		L"-Zi",			// デバッグ情報を生成
		L"-Qembed_debug", // デバッグ情報をシェーダバイナリに埋め込む
		L"-Od",			// 最適化しない
		L"-Zpr",		// メモリレイアウトは行優先
	};

	// 実際にShaderをCompileする
	IDxcResult* shaderResult = nullptr;
	hr = dxcCompiler->Compile(
		&shaderSourceBuffer,
		arguments,
		_countof(arguments),
		includeHandler,
		IID_PPV_ARGS(&shaderResult)
	);
	assert(SUCCEEDED(hr));

	//--- 3. Compile結果の確認
	IDxcBlobUtf8* errorMessage = nullptr;
	shaderResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errorMessage), nullptr);
	if (errorMessage != nullptr && errorMessage->GetStringLength() != 0) {

		DirectXUtility::Log(errorMessage->GetStringPointer());

		assert(false);
	}

	//--- 4. CompileされたShaderを取得して返す
	IDxcBlob* shaderBlob = nullptr;
	hr = shaderResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBlob), nullptr);
	assert(SUCCEEDED(hr));

	// 成功したログを出す
	//Log(ConvertString(std::format("Begin CompileShader, path{},profile:{}\n", filePath, profile)));

	// もう使わないリソースサヨナラ
	errorMessage->Release();
	shaderResult->Release();

	// バイナリ開放
	return shaderBlob;

}