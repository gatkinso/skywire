//
//   Copyright (C) 2017 Geoffrey Atkinson
//
//   Licensed under the Apache License, Version 2.0 (the "License").
//   You may not use this file except in compliance with the License.
//   A copy of the License is located at
//
//   https://www.apache.org/licenses/LICENSE-2.0.html
//
//   or in the "license" file accompanying this file. This file is distributed
//   on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
//   express or implied. See the License for the specific language governing
//   permissions and limitations under the License.
//

#include <fstream>
#include <iterator>
#include "specification.h"
#include "google/protobuf/util/json_util.h"
#include "google/protobuf/reflection.h"
#include "google/protobuf/struct.pb.h"

namespace skyloupe
{

namespace
{

// A spec may name its JSON Schema ("$schema": ..., for editors). Specification
// has no such field and the parser rejects unknown ones, so drop it first.
// Only a top-level "$schema" is dropped; JSON that isn't an object with one
// is left for the real parse to accept or reject.
std::string without_schema_key(const std::string& json_str)
{
    if (json_str.find("\"$schema\"") == std::string::npos)
    {
        return json_str;
    }

    google::protobuf::Struct root;
    if (false == google::protobuf::util::JsonStringToMessage(json_str, &root).ok() ||
        root.mutable_fields()->erase("$schema") == 0)
    {
        return json_str;
    }

    std::string stripped;
    if (false == google::protobuf::util::MessageToJsonString(root, &stripped).ok())
    {
        return json_str;
    }

    return stripped;
}

}

bool Specification::read_file(const std::string& pathname)
{
    if (pathname.length() == 0)
    {
        return false;
    }

    // Read the whole stream rather than trusting tellg(), which fails on pipes.
    std::ifstream file(pathname.c_str(), std::ios::in | std::ios::binary);

    if (file.fail())
    {
        return false;
    }

    std::string str((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    if (file.bad())
    {
        return false;
    }

    if (false == read_string(str))
    {
        spec_proto_.Clear();
        return false;
    }

    return true;
}

bool Specification::write_file(const std::string& pathname) const
{
    if (pathname.length() == 0 || spec_proto_.IsInitialized() == false)
    {
        return false;
    }

    try
    {
        std::string json_str;
        auto ret = write_string(json_str);

        if (ret == false)
        {
            return false;
        }

        std::ofstream file(pathname, std::ios::out | std::ios::trunc | std::ios::binary);

        if (file.fail())
        {
            return false;
        }

        file.write(json_str.c_str(), json_str.size());

        file.close();
    }
    catch (std::ofstream::failure e)
    {
        return false;
    }

    return true;
}

bool Specification::read_string(const std::string& json_str)
{
    if (json_str.length() == 0)
    {
        return false;
    }

    try
    {
        auto ret = google::protobuf::util::JsonStringToMessage(without_schema_key(json_str), &spec_proto_);

        if (ret.ok() == false)
        {
            // If the conversion fails, clear the proto to ensure it is in a clean state.
            spec_proto_.Clear();
            return false;
        }
    }
    catch (std::ifstream::failure e)
    {
        return false;
    }

    return true;
}

bool Specification::write_string(std::string& json_str) const
{
    if (spec_proto_.IsInitialized() == false)
    {
        return false;
    }

    try
    {
        auto ret = google::protobuf::util::MessageToJsonString(spec_proto_, &json_str);

        if (false == ret.ok())
        {
            return false;
        }
    }
    catch (std::ofstream::failure e)
    {
        return false;
    }

    return true;
}

std::vector<std::string> Specification::get_value(const::std::string& key, const std::string nomatch)
{
    std::vector<std::string> values;

    if (key.empty() || spec_proto_.IsInitialized() == false)
    {
        values.push_back(nomatch);
        return values;
    }

    const google::protobuf::Message* pMessage = &spec_proto_;
    const google::protobuf::Descriptor* pDescriptor = pMessage->GetDescriptor();

    if (pDescriptor == nullptr)
    {
        values.push_back(nomatch);
        return values;
    }

    const google::protobuf::FieldDescriptor* pFieldDescriptor = pDescriptor->FindFieldByName(key);

    if (pFieldDescriptor != nullptr)
    {
        const google::protobuf::Reflection* pReflection = pMessage->GetReflection();
        if (pReflection == nullptr)
        {
            values.push_back(nomatch);
            return values;
        }

        if (true == pFieldDescriptor->is_repeated())
        {
            google::protobuf::RepeatedFieldRef<std::string> ref =
                pReflection->GetRepeatedFieldRef<std::string>(*pMessage, pFieldDescriptor);

            for (int i = 0; i < ref.size(); i++)
            {
                values.push_back(pReflection->GetRepeatedString(*pMessage, pFieldDescriptor, i));
            }
        }
        else
        {
            if (false == pReflection->HasField(*pMessage, pFieldDescriptor))
            {
                values.push_back(nomatch);
                return values;
            }

            values.push_back(pReflection->GetString(*pMessage, pFieldDescriptor));
        }
    }
    else
    {
        values.push_back(nomatch);
    }

    if (values.size() == 0)
    {
        values.push_back(nomatch);
    }

    return values;
}

std::string Specification::get_value_string(const::std::string& key, const std::string nomatch)
{
    std::string value(nomatch);

    if (key.empty() || spec_proto_.IsInitialized() == false)
    {
        return nomatch;
    }
    
    for (auto val : spec_proto_.string_values())
    {
        if (val.first == key)
        {
            value = val.second;
            break;
        }
    }

    return value;
}

bool Specification::set_value(const::std::string& key, const std::string& value)
{
    if (key.empty() || spec_proto_.IsInitialized() == false)
    {
        return false;
    }

    google::protobuf::Message* pMessage = &spec_proto_;
    const google::protobuf::Descriptor* pDescriptor = pMessage->GetDescriptor();

    if (pDescriptor == nullptr)
    {
        return false;
    }

    const google::protobuf::FieldDescriptor* pFieldDescriptor = pDescriptor->FindFieldByName(key);

    if (pFieldDescriptor != nullptr)
    {
        const google::protobuf::Reflection* pReflection = pMessage->GetReflection();
        if (pReflection == nullptr)
        {
            return false;
        }

        if (true == pFieldDescriptor->is_repeated())
        {
            pReflection->AddString(pMessage, pFieldDescriptor, value);
        }
        else
        {
            pReflection->SetString(pMessage, pFieldDescriptor, value);
        }
    }

    return true;
}

bool Specification::set_value_string(const::std::string& key, const std::string& value)
{
    if (key.empty() || spec_proto_.IsInitialized() == false)
    {
        return false;
    }

    bool done = false;
    for (auto iter = spec_proto_.mutable_string_values()->begin(); iter != spec_proto_.mutable_string_values()->end(); iter++)
    {
        auto val = iter->first;
        if (val == key)
        {
            iter->second = value;
            done = true;
        }
    }

    if (done == false)
    {
        google::protobuf::MapPair < std::string, std::string > pair(key, value);
        spec_proto_.mutable_string_values()->insert(pair);
    }

    return true;
}

bool Specification::clear()
{
    spec_proto_.Clear();

    return true;
}

bool Specification::clear_key(const::std::string& key)
{
    if (key.empty() || spec_proto_.IsInitialized() == false)
    {
        return false;
    }

    google::protobuf::Message* pMessage = &spec_proto_;
    const google::protobuf::Descriptor* pDescriptor = pMessage->GetDescriptor();

    if (pDescriptor == nullptr)
    {
        return false;
    }

    const google::protobuf::FieldDescriptor* pFieldDescriptor = pDescriptor->FindFieldByName(key);

    if (pFieldDescriptor != nullptr)
    {
        const google::protobuf::Reflection* pReflection = pMessage->GetReflection();
        if (pReflection == nullptr)
        {
            return false;
        }

        pReflection->ClearField(pMessage, pFieldDescriptor);
    }

    return true;
}

bool Specification::clear_key_string(const::std::string& key)
{
    if (key.empty() || spec_proto_.IsInitialized() == false)
    {
        return false;
    }

    spec_proto_.mutable_string_values()->erase(key);

    return true;
}

bool Specification::set_value_member(uint32_t key, const std::string& type, const std::string& name, uint32_t endian)
{
    if (spec_proto_.IsInitialized() == false)
    {
        return false;
    }

    auto iter = spec_proto_.mutable_members()->find(key);
    if (iter == spec_proto_.members().end())
    {
        (*spec_proto_.mutable_members())[key].set_type(type);
        (*spec_proto_.mutable_members())[key].set_name(name);
        (*spec_proto_.mutable_members())[key].set_endian(endian);
    }
    else
    {
        iter->second.set_type(type);
        iter->second.set_name(name);
        iter->second.set_endian(endian);
    }

    return true;
}

bool Specification::get_value_member(uint32_t key, std::string& type_val, std::string& name_val, uint32_t& endian_val)
{
    if (spec_proto_.IsInitialized() == false)
    {
        return false;
    }

    auto iter = spec_proto_.mutable_members()->find(key);
    if (iter == spec_proto_.members().end())
    {
        return false;
    }
    else
    {
        type_val = iter->second.type();
        name_val = iter->second.name();
        endian_val = iter->second.endian();
    }

    return true;
}

bool Specification::clear_key_member(uint32_t key)
{
    size_t ret = spec_proto_.mutable_members()->erase(key);
    if (ret == 0)
        return false;
    return true;
}

}


