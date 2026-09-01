#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <debug/DebugEntry.h>
#include <functional>
#include <array>

class DebugEntryManager
{
public:

    static inline DebugEntryManager* GetInstance()
    {
        static DebugEntryManager instance;
        return &instance;
    }

    DebugEntryManager& operator=(const DebugEntryManager&) = delete;
    DebugEntryManager(const DebugEntryManager&) = delete;
    DebugEntryManager& operator=(DebugEntryManager&&) = delete;


    void Initialize();
    void Finalize();

    void RegisterEntry(const std::string& category, DebugEntry* pDebugEntry);
    void UnregisterEntry(DebugEntry* pDebugEntry);

    void ImGui();

    template <typename T>
    void HandleParameter(
        const std::string& id, 
        const std::string& name, 
        T* ptr, 
        std::function<void()> pFunc = nullptr);

    template <typename T>
    void HandleParameter(
        const std::string& id,
        const std::string& name,
        const T* ptr);

    // カテゴリが変更されたときに呼び出される関数
    //void MoveCategory(const DebugEntry* pEntry, const std::string& oldCategory, const std::string& newCategory);

private:
    DebugEntryManager() = default;
    ~DebugEntryManager() = default;

    std::unordered_map<std::string, std::vector<DebugEntry*>> entries_;
    std::unordered_map<std::string, std::array<float, 4>> categoryColorCache_;
};

template <typename T>
void DebugEntryManager::HandleParameter(const std::string& id, const std::string& name, T* ptr, std::function<void()> pFunc)
{
    entries_.at(id).back()->RegisterParameter(name, ptr, std::move(pFunc));
}

template <typename T>
void DebugEntryManager::HandleParameter(const std::string& id, const std::string& name, const T* ptr)
{
    entries_.at(id).back()->RegisterParameter(name, ptr);
}
