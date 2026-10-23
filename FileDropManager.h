#pragma once

#include <string>
#include <vector>
#include <utility>

#include <unordered_map>

enum class FileType {

	kModel,
	kJson,

	kTypeCount
};

class FileDropManager
{
public:


	// パス追加
	static void AddFilePath(std::wstring path);

	// パスのリストを取り出す（毎回リセット）
	static std::vector<std::wstring> TakeFilePaths(FileType type);


	// Resources/ 基準のstring型のパスに変換
	static std::string GetRelativePath(std::wstring path);

	// ファイルの拡張子抜いたのを返す
	static std::string GetFileNameWithoutExtension(std::wstring path);

	// wstring <-> string 変換
	static std::string ToUTF8(const std::wstring& wstr);
	static std::wstring ToWide(const std::string& str);
private:

	// 対応表
	static std::unordered_map<std::wstring, FileType> typeToExtension_;

	//ファイルタイプ分のパスのリストの、配列
	static std::vector<std::wstring> droppedFilesList_[static_cast<size_t>(FileType::kTypeCount)];


};

