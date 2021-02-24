#pragma once

#include "./tokens-fungible/TONTokenWallet.hpp"

namespace tvm { namespace schema {

static constexpr unsigned EXCHANGE_TIMESTAMP_DELAY = 1800;
using root_replay_protection_t = replay_attack_protection::timestamp<EXCHANGE_TIMESTAMP_DELAY>;

// ===== Root Token Contract ===== //
__interface ITonExchange {

  // expected offchain constructor execution
  [[internal, external, dyn_chain_parse]]
  void constructor() = 11;

  // Should be provided pubkey (for external owned wallet) or std addr (for internal owned wallet).
  // The other value must be zero.
  [[internal, external, noaccept, dyn_chain_parse]]
  void regNewToken(uint256 token_wallet_hex) = 12;

  
  [[getter]]
  uint256 getRootAddress() = 13;

  [[getter]]
  uint256 getSupportTokenByRoot(uint256 root_addr_hex) = 14;

  

  
};

struct DTonExchange {

  uint256 root_address_hex;
  dict_map<uint256,uint256> support_token_list;
 
};

struct ETonExchange {
};

// Prepare Root StateInit structure and expected contract address (hash from StateInit)
inline
std::pair<StateInit, uint256> prepare_root_state_init_and_addr(cell root_code, DTonExchange root_data) {
  cell root_data_cl =
    prepare_persistent_data<ITonExchange, root_replay_protection_t, DTonExchange>(
      root_replay_protection_t::init(), root_data);
  StateInit root_init {
    /*split_depth*/{}, /*special*/{},
    root_code, root_data_cl, /*library*/{}
  };
  cell root_init_cl = build(root_init).make_cell();
  return { root_init, uint256(tvm_hash(root_init_cl)) };
}

}} // namespace tvm::schema