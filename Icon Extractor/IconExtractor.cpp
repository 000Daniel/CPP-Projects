// IconExtractor.cpp
// Extracts every RT_GROUP_ICON resource from one or more Windows .exe files
// and writes each icon group as a multi-image .ico file.

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

#include <cstdint>
#include <cwchar>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <shellapi.h>

#pragma pack(push, 1)
struct IconDirHeader
{
    WORD reserved;
    WORD type;
    WORD count;
};

#pragma pack(push, 1)
// Entry layout inside an executable's RT_GROUP_ICON resource.
struct GroupIconEntry
{
    BYTE width;
    BYTE height;
    BYTE colorCount;
    BYTE reserved;
    WORD planes;
    WORD bitCount;
    DWORD bytesInResource;
    WORD resourceId;
};
#pragma pack(pop)
// Entry layout written to an .ico file.
struct IconFileEntry
{
    BYTE width;
    BYTE height;
    BYTE colorCount;
    BYTE reserved;
    WORD planes;
    WORD bitCount;
    DWORD bytesInResource;
    DWORD imageOffset;
};
#pragma pack(pop)

static_assert(sizeof(IconDirHeader) == 6, "Unexpected icon directory header size");
static_assert(sizeof(GroupIconEntry) == 14, "Unexpected group icon entry size");
static_assert(sizeof(IconFileEntry) == 16, "Unexpected icon file entry size");

struct ExtractContext
{
    std::filesystem::path inputPath;
    std::size_t groupsSeen = 0;
    std::size_t filesWritten = 0;
    bool hadError = false;
};

std::filesystem::path MakeOutputPath(const std::filesystem::path& inputPath,
    std::size_t groupNumber)
{
    const std::wstring stem = inputPath.stem().wstring();
    const std::wstring groupSuffix = (groupNumber == 0)
        ? L""
        : L"_" + std::to_wstring(groupNumber + 1);

    std::wstring fileName = stem + groupSuffix + L".ico";
    std::filesystem::path candidate = inputPath.parent_path() / fileName;

    // Avoid overwriting existing files. Add (2), (3), etc. when necessary.
    std::error_code ec;
    unsigned int duplicateNumber = 2;
    while (std::filesystem::exists(candidate, ec) && !ec)
    {
        fileName = stem + groupSuffix + L" (" + std::to_wstring(duplicateNumber++) + L").ico";
        candidate = inputPath.parent_path() / fileName;
    }

    return candidate;
}

void ShowError(const std::wstring& message)
{
    MessageBoxW(
        nullptr,
        message.c_str(),
        L"Icon Extractor - Error",
        MB_OK | MB_ICONERROR
    );
}

bool WriteIconGroup(HMODULE module,
    LPCWSTR groupName,
    WORD language,
    ExtractContext& context,
    std::size_t groupNumber)
{
    HRSRC groupResource = FindResourceExW(module, RT_GROUP_ICON, groupName, language);
    if (!groupResource)
    {
        ShowError(L"Could not read an icon group (error " + std::to_wstring(GetLastError()) + L").");
        return false;
    }

    const DWORD groupSize = SizeofResource(module, groupResource);
    HGLOBAL loadedGroup = LoadResource(module, groupResource);
    const BYTE* groupBytes = loadedGroup
        ? static_cast<const BYTE*>(LockResource(loadedGroup))
        : nullptr;

    if (!groupBytes || groupSize < sizeof(IconDirHeader))
    {
        ShowError(L"An icon group resource is invalid or empty.");
        return false;
    }

    IconDirHeader groupHeader{};
    std::memcpy(&groupHeader, groupBytes, sizeof(groupHeader));

    if (groupHeader.type != 1 || groupHeader.count == 0)
    {
        ShowError(L"The resource is not a valid icon group.");
        return false;
    }

    const std::uint64_t requiredSize = sizeof(IconDirHeader) +
        static_cast<std::uint64_t>(groupHeader.count) * sizeof(GroupIconEntry);

    if (requiredSize > groupSize)
    {
        ShowError(L"The icon group directory is truncated.");
        return false;
    }

    std::vector<IconFileEntry> outputEntries;
    std::vector<std::vector<BYTE>> imageData;
    outputEntries.reserve(groupHeader.count);
    imageData.reserve(groupHeader.count);

    for (WORD i = 0; i < groupHeader.count; ++i)
    {
        GroupIconEntry sourceEntry{};
        const std::size_t entryOffset = sizeof(IconDirHeader) +
            static_cast<std::size_t>(i) * sizeof(GroupIconEntry);
        std::memcpy(&sourceEntry, groupBytes + entryOffset, sizeof(sourceEntry));

        // The group resource stores an ID for the actual RT_ICON image resource.
        HRSRC iconResource = FindResourceExW(
            module, RT_ICON, MAKEINTRESOURCEW(sourceEntry.resourceId), language);

        // Fall back to the usual language search if this exact language is absent.
        if (!iconResource)
        {
            iconResource = FindResourceW(
                module, RT_ICON, MAKEINTRESOURCEW(sourceEntry.resourceId));
        }

        if (!iconResource)
        {
            ShowError(L"Warning: missing icon image resource ID "
                + std::to_wstring(sourceEntry.resourceId) + L"; skipping it.");
            continue;
        }

        const DWORD imageSize = SizeofResource(module, iconResource);
        HGLOBAL loadedImage = LoadResource(module, iconResource);
        const BYTE* imageBytes = loadedImage
            ? static_cast<const BYTE*>(LockResource(loadedImage))
            : nullptr;

        if (!imageBytes || imageSize == 0)
        {
            ShowError(L"Warning: an icon image could not be loaded; skipping it.");
            continue;
        }

        IconFileEntry fileEntry{};
        fileEntry.width = sourceEntry.width;
        fileEntry.height = sourceEntry.height;
        fileEntry.colorCount = sourceEntry.colorCount;
        fileEntry.reserved = sourceEntry.reserved;
        fileEntry.planes = sourceEntry.planes;
        fileEntry.bitCount = sourceEntry.bitCount;
        fileEntry.bytesInResource = imageSize;

        outputEntries.push_back(fileEntry);
        imageData.emplace_back(imageBytes, imageBytes + imageSize);
    }

    if (outputEntries.empty())
    {
        ShowError(L"No usable icon images were found in this icon group.");
        return false;
    }

    const std::uint64_t directorySize = sizeof(IconDirHeader) +
        static_cast<std::uint64_t>(outputEntries.size()) * sizeof(IconFileEntry);
    std::uint64_t nextImageOffset = directorySize;

    for (std::size_t i = 0; i < outputEntries.size(); ++i)
    {
        if (nextImageOffset > UINT32_MAX || imageData[i].size() > UINT32_MAX)
        {
            ShowError(L"The icon data is too large for the .ico format.");
            return false;
        }

        outputEntries[i].imageOffset = static_cast<DWORD>(nextImageOffset);
        nextImageOffset += imageData[i].size();
    }

    const std::filesystem::path outputPath = MakeOutputPath(context.inputPath, groupNumber);
    std::ofstream output(outputPath, std::ios::binary | std::ios::out);
    if (!output)
    {
        ShowError(L"Could not create: " + outputPath.wstring());
        return false;
    }

    const IconDirHeader outputHeader{ 0, 1, static_cast<WORD>(outputEntries.size()) };
    output.write(reinterpret_cast<const char*>(&outputHeader), sizeof(outputHeader));

    for (const IconFileEntry& entry : outputEntries)
    {
        output.write(reinterpret_cast<const char*>(&entry), sizeof(entry));
    }

    for (const std::vector<BYTE>& image : imageData)
    {
        output.write(reinterpret_cast<const char*>(image.data()),
            static_cast<std::streamsize>(image.size()));
    }

    if (!output)
    {
        ShowError(L"Failed while writing: " + outputPath.wstring());
        return false;
    }
    ++context.filesWritten;
    return true;
}

BOOL CALLBACK EnumGroupLanguageProc(HMODULE module,
    LPCWSTR /*type*/,
    LPCWSTR name,
    WORD language,
    LONG_PTR parameter)
{
    auto* context = reinterpret_cast<ExtractContext*>(parameter);
    const std::size_t groupNumber = context->groupsSeen++;

    try
    {
        if (!WriteIconGroup(module, name, language, *context, groupNumber))
        {
            context->hadError = true;
        }
    }
    catch (...)
    {
        context->hadError = true;
        ShowError(L"An unexpected error occurred while extracting an icon group.");
    }

    return TRUE; // Continue enumerating other icon groups.
}

BOOL CALLBACK EnumGroupNameProc(HMODULE module,
    LPCWSTR /*type*/,
    LPWSTR name,
    LONG_PTR parameter)
{
    auto* context = reinterpret_cast<ExtractContext*>(parameter);

    // Each group icon can have one or more language variants.
    EnumResourceLanguagesW(module, RT_GROUP_ICON, name,
        EnumGroupLanguageProc, parameter);
    return TRUE; // Continue enumerating other group names.
}

bool ExtractFromExecutable(const std::filesystem::path& inputPath)
{
    std::error_code fileError;
    if (!std::filesystem::exists(inputPath, fileError) || fileError)
    {
        ShowError(L"File not found.");
        return false;
    }

    if (_wcsicmp(inputPath.extension().c_str(), L".exe") != 0)
    {
        ShowError(L"  Skipped: this program accepts .exe files only.");
        return false;
    }

    // Load the executable as a data/image resource module. Its code is not run.
    HMODULE module = LoadLibraryExW(
        inputPath.c_str(), nullptr,
        LOAD_LIBRARY_AS_DATAFILE | LOAD_LIBRARY_AS_IMAGE_RESOURCE);

    if (!module)
    {
        ShowError(L"  Could not open the executable's resources (error "
            + std::to_wstring(GetLastError()) + L").");
        return false;
    }

    ExtractContext context;
    context.inputPath = inputPath;

    SetLastError(ERROR_SUCCESS);
    const BOOL enumerated = EnumResourceNamesW(
        module, RT_GROUP_ICON, EnumGroupNameProc,
        reinterpret_cast<LONG_PTR>(&context));

    if (!enumerated && GetLastError() != ERROR_SUCCESS && context.groupsSeen == 0)
    {
        // This commonly means the executable simply has no RT_GROUP_ICON resources.
        ShowError(L"No icon resources found (or resource enumeration failed).");
    }

    FreeLibrary(module);

    if (context.filesWritten == 0)
    {
        ShowError(L"No .ico file was created for this executable.");
        return false;
    }

    return !context.hadError;
}

//int wmain(int argc, wchar_t* argv[])
int WINAPI wWinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,PWSTR pCmdLine,int nCmdShow)
{
    int argc;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);

    if (!argv)
    {
        ShowError(L"Failed to read command-line arguments.");
        return 1;
    }

    bool pauseBeforeExit = true;
    std::vector<std::filesystem::path> inputFiles;

    for (int i = 1; i < argc; ++i)
    {
        std::wstring filePath = argv[i];

        if (std::wstring(argv[i]) == L"--no-pause")
        {
            pauseBeforeExit = false;
        }
        else
        {
            inputFiles.emplace_back(argv[i]);
        }
    }

    int failedFiles = 0;
    for (const auto& inputPath : inputFiles)
    {
        if (!ExtractFromExecutable(inputPath))
        {
            ++failedFiles;
        }
    }

    return failedFiles == 0 ? 0 : 1;
}
