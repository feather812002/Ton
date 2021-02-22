#pragma once

#include <tvm/schema/message.hpp>
#include <tvm/sequence.hpp>

#include <tvm/replay_attack_protection/timestamp.hpp>
#include <tvm/smart_switcher.hpp>
#include <tvm/dict_map.hpp>

namespace tvm { namespace schema {

using WalletGramsType = uint128;
using TokensType = uint128;

static constexpr unsigned TOKEN_WALLET_TIMESTAMP_DELAY = 1800;
using wallet_replay_protection_t = replay_attack_protection::timestamp<TOKEN_WALLET_TIMESTAMP_DELAY>;


// ===== TON exchange database ===== //
__interface IOrderDatabase {
};


struct DOrderDatabase {
};

struct EOrderDatabase {
};

}};