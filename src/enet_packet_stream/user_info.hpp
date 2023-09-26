#pragma once

#include<msgpack.hpp>
#include<cstdint>
#include<string>

/// @brief ユーザID
using ClientID = uint64_t;

/// @brief ユーザ情報
struct UserInfo {
    std::string name;
    ClientID id;
    MSGPACK_DEFINE(name, id);
};