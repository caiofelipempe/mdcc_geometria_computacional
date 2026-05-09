#include "imgui.h"
#include <string>
#include <fstream>
#include <vector>
#include <cstring>
#include <algorithm>

#ifdef _WIN32
    #include <direct.h>
    #define GETCWD _getcwd
    #include <io.h>
    #include <windows.h>
#else
    #include <unistd.h>
    #define GETCWD getcwd
    #include <dirent.h>
    #include <sys/stat.h>
#endif

class FileSaver {
private:
    struct FileItem {
        std::string name;
        bool isDirectory;
    };
    
    std::vector<FileItem> items;
    std::string selectedItem;
    char fileNameBuffer[256];
    bool isOpen;
    std::string currentDirectory;
    
    // NOVO: Armazena o resultado final
    std::string selectedPath;
    bool hasSelected; 

    void UpdateFileList() {
        items.clear();
        // ... (Mesmo código de listagem anterior) ...
#ifdef _WIN32
        std::string searchPath = currentDirectory + "\\*";
        WIN32_FIND_DATAA findData;
        HANDLE hFind = FindFirstFileA(searchPath.c_str(), &findData);
        if (hFind != INVALID_HANDLE_VALUE) {
            do {
                std::string name = findData.cFileName;
                if (name != "." && name != "..") {
                    FileItem item;
                    item.name = name;
                    item.isDirectory = (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
                    items.push_back(item);
                }
            } while (FindNextFileA(hFind, &findData) != 0);
            FindClose(hFind);
        }
#else
        DIR* dir = opendir(currentDirectory.c_str());
        if (dir != nullptr) {
            struct dirent* entry;
            while ((entry = readdir(dir)) != nullptr) {
                std::string name = entry->d_name;
                if (name != "." && name != "..") {
                    FileItem item;
                    item.name = name;
                    struct stat statbuf;
                    std::string fullPath = currentDirectory + "/" + name;
                    if (stat(fullPath.c_str(), &statbuf) == 0) {
                        item.isDirectory = S_ISDIR(statbuf.st_mode);
                        items.push_back(item);
                    }
                }
            }
            closedir(dir);
        }
#endif
    }

public:
    FileSaver() {
        char buffer[1024];
        currentDirectory = (GETCWD(buffer, sizeof(buffer)) != nullptr) ? buffer : ".";
        memset(fileNameBuffer, 0, sizeof(fileNameBuffer));
        isOpen = false;
        hasSelected = false;
    }

    void Open() {
        isOpen = true;
        hasSelected = false;
        selectedPath.clear();
        UpdateFileList();
    }

    // Métodos para o usuário recuperar o path depois
    bool HasSelected() const { return hasSelected; }
    std::string GetSelectedPath() { 
        hasSelected = false; // Reset após leitura
        return selectedPath; 
    }

    void Draw() {
        if (!isOpen) return;

        ImGui::Begin("Selecionar Destino", &isOpen);

        ImGui::Text("Pasta atual: %s", currentDirectory.c_str());

        if (ImGui::Button(".. (Subir)")) {
            size_t pos = currentDirectory.find_last_of("\\/");
            if (pos != std::string::npos) {
                currentDirectory = currentDirectory.substr(0, pos);
                UpdateFileList();
            }
        }

        ImGui::BeginChild("Files", ImVec2(0, -ImGui::GetFrameHeightWithSpacing() * 3), true);
        for (const auto& item : items) {
            if (item.isDirectory) {
                if (ImGui::Selectable(item.name.c_str())) {
                    currentDirectory += (currentDirectory.back() == '/' || currentDirectory.back() == '\\' ? "" : "/") + item.name;
                    UpdateFileList();
                    break;
                }
            } else {
                if (ImGui::Selectable(item.name.c_str(), selectedItem == item.name)) {
                    selectedItem = item.name;
                    strncpy(fileNameBuffer, item.name.c_str(), sizeof(fileNameBuffer)-1);
                }
            }
        }
        ImGui::EndChild();

        ImGui::InputText("Nome do ficheiro", fileNameBuffer, sizeof(fileNameBuffer));

        if (ImGui::Button("Confirmar Local")) {
            std::string finalPath = currentDirectory;
            finalPath += (finalPath.back() == '/' || finalPath.back() == '\\' ? "" : "/");
            finalPath += fileNameBuffer;

            // Garante a extensão
            if (finalPath.find(".obj") == std::string::npos) finalPath += ".obj";

            selectedPath = finalPath;
            hasSelected = true;
            isOpen = false;
        }

        ImGui::SameLine();
        if (ImGui::Button("Cancelar")) isOpen = false;

        ImGui::End();
    }
};