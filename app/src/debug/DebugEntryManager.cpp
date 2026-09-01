#include "DebugEntryManager.h"
#include <math/Color.h>
#include <utility/strutl.h>

#ifdef _DEBUG
#include <imgui.h>
#endif // _DEBUG
#include <cassert>


void DebugEntryManager::Initialize()
{
    #ifdef _DEBUG
    ImGui::GetStyle().IndentSpacing = 16.0f;
    #endif // _DEBUG
}

void DebugEntryManager::Finalize()
{
}

void DebugEntryManager::RegisterEntry(const std::string& id, DebugEntry* pDebugEntry)
{
    entries_[id].push_back(pDebugEntry);
}

void DebugEntryManager::UnregisterEntry(DebugEntry* pDebugEntry)
{
    for (auto& [id, entryList] : entries_)
    {
        auto it = std::remove(entryList.begin(), entryList.end(), pDebugEntry);
        if (it != entryList.end())
        {
            entryList.erase(it, entryList.end());
            if (entryList.empty())
            {
                entries_.erase(id);
            }
            break;
        }
    }
}

void DebugEntryManager::ImGui()
{
    #ifdef _DEBUG

    bool isWindowOpen = ImGui::Begin("Debug Entries");

    if (!isWindowOpen)
    {
        ImGui::End();
        return;
    }

    for (auto& [id, entryList] : entries_)
    {
        bool isSoloEntry = entryList.size() == 1;

        /// [ カテゴリカラーの決定 ]
        ImVec4 col = {};
        auto colorIt = categoryColorCache_.find(entryList.front()->GetCategory());

        /// キャッシュがあるなら
        if (colorIt != categoryColorCache_.end())
        {
            // キャッシュから色を取得
            col = { colorIt->second[0], colorIt->second[1], colorIt->second[2], colorIt->second[3] };
        }
        /// キャッシュがないなら
        else
        {
            // カテゴリ名からハッシュ値を生成
            uint32_t hash = utl::string::to_hash(entryList.front()->GetCategory());

            // 色相に変換
            HSV hsv = { static_cast<float>(hash % 360) / 360.0f, 0.65f, 0.85f };
            ImGui::ColorConvertHSVtoRGB(hsv.h(), hsv.s(), hsv.v(), col.x, col.y, col.z);
            col.w = 1.0f;

            // キャッシュに保存
            categoryColorCache_[entryList.front()->GetCategory()] = { col.x, col.y, col.z, col.w };
        };

        // ImGuiに適用
        ImGui::PushStyleColor(ImGuiCol_Text, col);


        /// [ ツリーノードの表示 ]
        bool isOpenParentTree = false;
        if (!isSoloEntry)
        {
            std::string label = entryList.front()->GetCategory() + " (" + std::to_string(entryList.size()) + ")";
            isOpenParentTree = ImGui::TreeNode(label.c_str());
        }

        if (isOpenParentTree || isSoloEntry)
        {
            for (size_t i = 0; i < entryList.size(); ++i)
            {
                auto& entry = entryList[i];
                std::string label;
                if (entry->GetName().empty())
                {
                    label = isSoloEntry ? entry->GetCategory() : entry->GetCategory() + " - " + std::to_string(i + 1);
                }
                else
                {
                    label = entry->GetName();
                }

                bool isOpen = ImGui::TreeNode(label.c_str());
                if (isOpen)
                {
                    ImGui::Indent();
                    entry->ImGui();
                    ImGui::Unindent();

                    ImGui::TreePop();
                }
            }
        }

        if (!isSoloEntry && isOpenParentTree)
        {
            ImGui::TreePop();
        }

        // カテゴリカラーの終了
        ImGui::PopStyleColor();
    }

    ImGui::End();

    #endif // _DEBUG
}

//void DebugEntryManager::MoveCategory(const DebugEntry* pEntry, const std::string& oldCategory, const std::string& newCategory)
//{
//    auto srcEntryListIt = entries_.find(oldCategory);
//    assert(srcEntryListIt != entries_.end() && "oldCategory not found in entries_");
//
//    auto srcEntryIt = std::find(srcEntryListIt->second.begin(), srcEntryListIt->second.end(), pEntry);
//    assert(srcEntryIt != srcEntryListIt->second.end() && "pEntry not found in oldCategory list");
//
//    /// 宛先カテゴリのエントリリストを探す
//    auto destEntryListIt = entries_.find(newCategory);
//    if (destEntryListIt == entries_.end())
//    {
//        // 新しいカテゴリが見つからないとき、新しくカテゴリを作成する
//        // イテレータも更新する
//        destEntryListIt = entries_.emplace(newCategory, std::vector<DebugEntry*>{}).first;
//    }
//
//    /// pEntryを新しいカテゴリに移動する
//    destEntryListIt->second.push_back(const_cast<DebugEntry*>(pEntry));
//    srcEntryListIt->second.erase(srcEntryIt);
//
//
//    /// もし古いカテゴリが空になったら、削除する
//    if (srcEntryListIt->second.empty())
//    {
//        entries_.erase(srcEntryListIt);
//    }
//
//
//    /// カテゴリカラーの古いキャッシュを削除しておく
//    // ImGui()でカテゴリカラーを計算するため、当該関数が実行される前の場合はキャッシュがないことに注意
//    auto colorCacheIt = categoryColorCache_.find(oldCategory);
//    if (colorCacheIt != categoryColorCache_.end())
//    {
//        categoryColorCache_.erase(colorCacheIt);
//    }
//}
