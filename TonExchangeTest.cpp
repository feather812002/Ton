#include "TonExchange.hpp"
#include "TonExchangeTest.hpp"
#include "./tokens-fungible/TONTokenWallet.hpp"

#include <tvm/contract.hpp>
#include <tvm/smart_switcher.hpp>
#include <tvm/contract_handle.hpp>
#include <tvm/default_support_functions.hpp>



using namespace tvm;
using namespace schema;

template<bool Internal>
class TonExchangeTest final : public smart_interface<ITonExchangeTest>, public DTonExchangeTest {
public:
 
  struct error_code : tvm::error_code {
    static constexpr unsigned message_sender_is_not_my_owner  = 100;
    static constexpr unsigned not_enough_balance              = 101;
    static constexpr unsigned add_new_token_no_address        = 102;
    static constexpr unsigned wrong_bounced_args              = 103;
    static constexpr unsigned internal_owner_enabled          = 104;
    static constexpr unsigned internal_owner_disabled         = 105;
    static constexpr unsigned define_pubkey_or_internal_owner = 106;
    static constexpr unsigned wrong_wallet_code_hash          = 107;
    static constexpr unsigned deposit_no_token_type           = 108;
    static constexpr unsigned deposit_sender_wrong            = 109;
    static constexpr unsigned deposit_not_support_token       = 110;
    static constexpr unsigned deposit_no_customer_address     = 111;
    static constexpr unsigned withdraw_no_token_address       = 112;
    static constexpr unsigned withdraw_token_amount_error     = 113;
    static constexpr unsigned withdraw_token_no_support       = 114;
  };

  __always_inline
  void constructor() {
  }
  

  //------------------New Token Support-------------------------
  __always_inline
  void withdrawTest(address exchange_address,address exchange_wallet_address,address to_wallet_address,TokenAmount tokenAmount,WalletGramsType grams) {
     tvm_accept();
      handle<ITonExchange> dest_exchange(exchange_address);
    //dest_exchange_wallet.transfer(sender(),tokenAmount.get(), value_gr.get());
    dest_exchange(Grams(grams.get())).withdrawTest(exchange_wallet_address,to_wallet_address,tokenAmount);
   
  }
 
  // =============== Support functions ==================
  DEFAULT_SUPPORT_FUNCTIONS(ITonExchangeTest, root_replay_protection_t)

  
};

DEFINE_JSON_ABI(ITonExchangeTest, DTonExchangeTest, DTonExchangeTest);

// ----------------------------- Main entry functions ---------------------- //
DEFAULT_MAIN_ENTRY_FUNCTIONS_TMPL(TonExchangeTest, ITonExchangeTest, DTonExchangeTest, EXCHANGE_TIMESTAMP_DELAY)