#pragma once

#include<msgpack.hpp>
#include<cstdint>

#include"user_info.hpp"

/// @brief システムのリクエストID
enum SysRequestID {
    REGISTER_USER = 0,
};

MSGPACK_ADD_ENUM(SysRequestID);