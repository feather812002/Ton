#pragma once

#include <tvm/contract.hpp>
#include <tvm/smart_switcher.hpp>
#include <tvm/contract_handle.hpp>
#include <tvm/default_support_functions.hpp>
#include <tvm/dict_map.hpp>
#include <tvm/dict_set.hpp>
#include <tvm/dict_array.hpp>
#include <tvm/replay_attack_protection/timestamp.hpp>

namespace tvm { namespace schema {

using TokenAmount = uint128;
using TokenId = uint128;
using WalletGramsType = uint128;

static constexpr unsigned EXCHANGE_TIMESTAMP_DELAY = 1800;
using exchange_replay_protection_t = replay_attack_protection::timestamp<EXCHANGE_TIMESTAMP_DELAY>;

//-----------Token balance Object-----------
struct support_token {
  uint256 exchange_wallet_addr;
  bytes token_symbol;
};

struct show_support_token {
  uint256 token_root_addr;
  bytes token_symbol;
};

struct customer_token {
  //uint256 token_root_aadr_hex;
  bytes token_name;
  bytes token_symbol;
  uint8 decimals;
  TokenAmount token_balance;
};

struct customer_nftoken {
  //uint256 token_root_aadr_hex;
  bytes token_name;
  bytes token_symbol;
  dict_set<TokenId>  tokenid_list;
};

//--------Order------------
struct sellorder {
  uint256 orlder_no;
  uint256 sell_token_root_address_hex;
  uint128 sell_token_amount;
  uint256 seller_send_token_address_hex;
  uint256 seller_resive_token_address_hex;
  bytes   sell_token_symbol;
  //0-inital ,1-fungible token,2-nonfungible
  uint8   sell_token_type;
};
struct order {
  uint32  order_no;
  uint256 sell_token_root_address_hex;
  uint128 sell_token_amount;
  uint256 seller_send_token_address_hex;
  uint256 seller_resive_token_address_hex;
  bytes   sell_token_symbol;
  //0-inital ,1-fungible token,2-nonfungible
  uint8   sell_token_type;
  uint256 buy_token_root_address_hex;
  uint128 buy_token_amount;
  uint256 buyer_send_token_address_hex;
  uint256 buyer_resive_token_address_hex;
  bytes   buy_token_symbol;
  uint8   buy_token_type;
  //0:expired,1:put, 2:full filled,3:part filled,4:cancle. 
  uint8 order_status;
  //perf_get_timestamp();
  //uint64  expired;  
};


// ===== Exchange Contract ===== //
__interface ITonExchange {

  // expected offchain constructor execution
  [[internal, external, dyn_chain_parse]]
  void constructor() ;

  // Should be provided pubkey (for external owned wallet) or std addr (for internal owned wallet).
  // The other value must be zero.
  [[internal, external, noaccept, dyn_chain_parse]]
  void regNewToken(uint256 token_wallet_hex,bytes token_symbol) ;

  [[getter]]
  uint256 getRootAddress() ;

  [[getter]]
  support_token getSupportTokenByRoot(uint256 root_addr_hex) ;


  [[getter]]
  show_support_token getSupportTokenByNo(uint128 token_count) ;
  //---------------Customer Funds Manager---------------
  // [[internal, external, noaccept, dyn_chain_parse]]
  // void deposit() = 15;
  [[internal, external, noaccept, dyn_chain_parse]]
  void deposit(uint256 customer_wallet_address_hex,uint256 token_root_hex,bytes tokenName,bytes tokenSymbol,uint8 decimals, int8 token_type,TokenAmount tokenAmount) ;
  
  [[getter]]
  customer_token getFungibleTokenBalance(uint256 customer_wallet_address_hex,uint256 token_root_hex) ;
  
  [[internal, external, noaccept, dyn_chain_parse]]
  void withdraw(uint256 token_root_hex,int8 token_type,TokenAmount tokenAmount) ;

  [[internal, external, noaccept, dyn_chain_parse]]
  void withdrawTest(address exchange_wallet_address,address to_wallet_address,TokenAmount tokenAmount) ;

  [[getter]]
  dict_array<TokenId> getNFFungibleTokenBalance(uint256 customer_wallet_address_hex,uint256 token_root_hex) ;
  // [[internal, external, noaccept, dyn_chain_parse]]
  // void withdrawal () = 16;    

  //----------------execute exchange function--------------------------
  [[internal, external, dyn_chain_parse]]
  void putOrder(uint256 sell_token_addr_hex,uint128 sell_amount,uint256 seller_resive_address,
  uint256 buy_token_addr_hex,uint128 buy_amount);
  
  [[internal, external, dyn_chain_parse]]
  void cancelOrder(uint32 order_no);

  [[getter]]
  dict_array<order> getAllOrder() ;

  [[getter]]
  dict_array<order> getMyMakerOrders(uint256 maker_address) ;

  [[getter]]
  dict_array<order> getMyTakerOrders(uint256 taker_address) ;

  [[getter]]
  dict_array<order> getMyCancelOrders(uint256 maker_address) ;

   [[getter]]
  dict_array<order> getMyFilledOrders(uint256 maker_address) ;

  [[internal, external,  dyn_chain_parse]]
  uint8 fillOrder(uint32 order_no,uint256 buyer_resive_token_address_hex);

  
  [[internal, external, dyn_chain_parse]]
  void putTestDate(uint256 exchangeWallet1,uint256 exchangeWallet2);

  [[getter]]
  dict_array<show_support_token> getAllSupportTokens();


};

struct DTonExchange {
  uint256 root_address_hex;
  //support token list : token root addr hex--->token wallet addr hex
  dict_map<uint256,support_token> support_token_list;
  //TIP3 Fungible token  balance list: root token addr hex---> [customer wallet addr hex -->token hold details] 
  dict_map<uint256,dict_map<uint256,customer_token>> token_balance_list;
  //TIP3 NFFungible token  balance list:
  dict_map<uint256,dict_map<uint256,customer_nftoken>> nftoken_balance_list;

  uint32 order_no_count;
  //wait filled ordre list: maker  address->order
  dict_map<uint256,order> order_list;
  dict_map<uint256,sellorder> sell_order_list;
  // //already done filled order list: taker address->maker order
  dict_map<uint256,uint256> filled_order_list;

  dict_array<order> order_array;

  


 
};

struct ETonExchange {
};


}} // namespace tvm::schema