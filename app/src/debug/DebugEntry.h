#pragma once
#include <variant>
#include <string>
#include <type/Vector3.h>
#include <type/Vector4.h>
#include <type/Vector2.h>
#include <math/Color.h>
#include <unordered_map>
#include <transform/WorldTransform.h>
#include <utility/layout/FlexTypes.h>
#include <functional>

class DebugEntry
{
public:
    using AvailableType = std::variant<
        int*,
        float*,
        bool*,
        std::string*,
        Hagine::WorldTransform*,
        Hagine::Vector4*,
        Hagine::Vector3*,
        Hagine::Vector2*,
        RGBA*,
        FlexBox*
    >;

    using ConstAvailableType = std::variant<
        const int*,
        const float*,
        const bool*,
        const std::string*,
        const Hagine::WorldTransform*,
        const Hagine::Vector4*,
        const Hagine::Vector3*,
        const Hagine::Vector2*,
        const RGBA*,
        FlexBox*
    >;

    struct ParameterData
    {
        AvailableType ptr;
        std::function<void()> onChange;
    };

    DebugEntry(const std::string& id, const std::string category);
    ~DebugEntry();

    void ImGui();

    template <typename T>
    void RegisterParameter(const std::string& name, T* ptr, std::function<void()> pFunc);

    template <typename T>
    void RegisterParameter(const std::string& name, const T* ptr);

    inline void RegisterCustomGuiFunction(const std::string& name, std::function<void()> func)
    {
        customGuiFunctions_.insert({ name, std::move(func) });
    }


    const std::string& GetCategory() const { return category_; }
    const std::string& GetName() const { return name_; }
    void SetName(const std::string& name);

private:
    std::unordered_map<std::string, ParameterData> parameters_;
    std::unordered_map<std::string, ConstAvailableType> parametersConstant_;
    std::unordered_map<std::string, std::function<void()>> customGuiFunctions_;
    std::string category_;
    std::string name_;
};

template <typename T>
void DebugEntry::RegisterParameter(const std::string& name, T* ptr, std::function<void()> pFunc)
{
    parameters_.insert({ name, {ptr, pFunc} });
}

template <typename T>
void DebugEntry::RegisterParameter(const std::string& name, const T* ptr)
{
    parametersConstant_.insert({ name, ptr });
}
