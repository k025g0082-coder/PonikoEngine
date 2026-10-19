#include "FileDropManager.h"
#include <filesystem>
#include <algorithm>

#include <Windows.h>

// 各拡張子とその対応タイプ
std::unordered_map<std::wstring, FileType> FileDropManager::typeToExtension_ = {
    { L".obj", FileType::kModel },
};
std::vector<std::wstring> FileDropManager::droppedFilesList_[static_cast<size_t>(FileType::kTypeCount)]{};

void FileDropManager::AddFilePath(std::wstring path) {

    // obj の部分だけを抜き取るらしい
    std::filesystem::path filePath(path);

    std::wstring extension = filePath.extension().wstring();

    std::transform(
        extension.begin(),
        extension.end(),
        extension.begin(),
        towlower
    );

    // 拡張子のリストから FileType を検索
    auto it = typeToExtension_.find(extension);

    if (it != typeToExtension_.end()) {

        // 対応する種類のリストに登録
        droppedFilesList_[static_cast<size_t>(it->second)]
            .push_back(path);
    }
    
}

std::string FileDropManager::GetRelativePath(std::wstring path)
{
    // resources探す
    std::wstring marker = L"resources\\";

    size_t pos = path.find(marker);

    if (pos != std::wstring::npos) {

        std::wstring relativePath = path.substr(pos);

        // Windowsパスの \ を / に統一
        std::replace(
            relativePath.begin(),
            relativePath.end(),
            L'\\',
            L'/'
        );

        // stringに変換して返す
        return ToUTF8(relativePath);
    }


    return std::string();
}

std::vector<std::wstring> FileDropManager::TakeFilePaths(FileType type) {


    auto& paths = droppedFilesList_[static_cast<size_t>(type)];

    // リザルトにコピー
    std::vector<std::wstring> result = std::move(paths);

    // 元のリストは削除
    paths.clear();

    return result;
}

// ファイルの拡張子抜いたのを返す
std::string FileDropManager::GetFileNameWithoutExtension(std::wstring path)
{
    static std::filesystem::path filePath(path);

    // stem()でできるらしい
    return ToUTF8(filePath.stem().wstring());
}

std::string FileDropManager::ToUTF8(const std::wstring& wstr) {
    if (wstr.empty()) {
        return {};
    }

    int size = WideCharToMultiByte(
        CP_UTF8, 0,
        wstr.data(), static_cast<int>(wstr.size()),
        nullptr, 0, nullptr, nullptr
    );

    std::string result(size, '\0');

    WideCharToMultiByte(
        CP_UTF8, 0,
        wstr.data(), static_cast<int>(wstr.size()),
        result.data(), size, nullptr, nullptr
    );

    return result;
}