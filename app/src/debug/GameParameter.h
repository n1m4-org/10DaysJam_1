#pragma once
#include <debug/DebugEntryManager.h>
#include <debug/DebugEntry.h>
#include <source_location>
#include <functional>
#include <string>

#ifdef _DEBUG

#define GameParameter(type, name, value)\
        type name = value; \
        GameParameterData<type> debug_##name{std::source_location::current().file_name(), #name, name}

#define GameParameterView(type, name, value) \
        type name = value; \
        GameParameterViewData<type> debug_view_##name{std::source_location::current().file_name(), #name, &name}

#define EnableDebug(category) DebugEntry debugEntry{ std::source_location::current().file_name(), category }

    template <typename ValueType>
    class GameParameterData
    {
    public:
        using OnChangeCallback = std::function<void(const ValueType&)>;

        GameParameterData(
            const std::string& id,
            const std::string& name,
            ValueType& value)
            : v_(value)
        {
            DebugEntryManager::GetInstance()->HandleParameter(id, name, &v_, [this]()
            {
                if (onChange_)
                {
                    onChange_(v_);
                }
            });
        }

        GameParameterData(const GameParameterData&) = delete;
        GameParameterData& operator=(const GameParameterData&) = delete;

        void SetOnChange(OnChangeCallback cb) { onChange_ = std::move(cb); }

    private:
        ValueType& v_ = {};
        OnChangeCallback onChange_;
    };

    template <typename ValueType>
    class GameParameterViewData
    {
    public:
        GameParameterViewData(
            const std::string& id,
            const std::string& name,
            const ValueType* pValue = nullptr)
        {
            DebugEntryManager::GetInstance()->HandleParameter(id, name, pValue);
        }
    };

#else
    #define GameParameter(type, name, value)\
        type name = value;

    #define GameParameterView(type, name, value) \
        type name = value;

    #define EnableDebug(category)

#endif