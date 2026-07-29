#pragma once

#include <FixedString.h>

#include <mmap.h>
#include <string>
#include <unordered_map>
#include <variant>

template<FixedString Path>
class INI: private MMAP<char, MMAPConfig(GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, PAGE_READONLY, FILE_MAP_READ)>
{
public:
    using VariantType = std::variant<bool, int64_t, std::string_view>;

    class INIValue
    {
    public:
        explicit INIValue(const VariantType* a_value): variant(a_value) {}

        operator int64_t() const 
        {
            if(variant) 
            {
                if(auto value = std::get_if<int64_t>(variant)) { return *value; }
                if(auto value = std::get_if<bool>(variant)) { return *value; }
            }

            return 0;
        }

        operator uint64_t() const 
        {
            if(variant) 
            {
                if(auto value = std::get_if<int64_t>(variant)) { return *value; }
                if(auto value = std::get_if<bool>(variant)) { return *value; }
            }

            return 0;
        }

        operator std::string_view() const 
        {
            if(variant) 
            {
                if(auto value = std::get_if<std::string_view>(variant)) { return *value; }
            }

            return "";
        }

        operator bool() const 
        {
            if(variant) 
            {
                if (auto value = std::get_if<bool>(variant)) { return *value; }
                if (auto value = std::get_if<int64_t>(variant)) { return *value != 0; }
            }
            return false;
        }

    private:
        const VariantType* variant = nullptr;
    };

    INIValue operator[](const std::string_view& key) const
    {
        auto it = map.find(key);
        if (it == map.end())
        {
            return INIValue(nullptr);
        }

        return INIValue(&it->second);
    }

    INI();
private:
    std::vector<char> keys;
    std::unordered_map<std::string_view, VariantType> map;
};

#include <ini.tpp>