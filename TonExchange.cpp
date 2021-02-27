#include "TonExchange.hpp"
#include "./tokens-fungible/TONTokenWallet.hpp"

#include <tvm/contract.hpp>
#include <tvm/smart_switcher.hpp>
#include <tvm/contract_handle.hpp>
#include <tvm/default_support_functions.hpp>



using namespace tvm;
using namespace schema;

template<bool Internal>
class TonExchange final : public smart_interface<ITonExchange>, public DTonExchange {
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
  void regNewToken(uint256 token_wallet) {
    //until now , I still not found a good way can detect a online smart contract is good or bad.
    //so , this is maybe improve in later ,if we can identification a smart contract is good or bad.
    tvm_accept();
    auto sender=int_sender();
    root_address_hex=std::get<addr_std>(sender()).address;
    if(!support_token_list.contains(root_address_hex.get())){
      support_token_list.set_at(root_address_hex.get(), token_wallet);
    }
  }
  // getters
  __always_inline uint256 getRootAddress() {
    return root_address_hex;
  }

  __always_inline uint256 getSupportTokenByRoot(uint256 root_addr_hex) {
    uint256 token_wallet=uint256(0);
    if(support_token_list.contains(root_addr_hex.get())){
        token_wallet=support_token_list.get_at(root_addr_hex.get());
    }
    return token_wallet;
  }

  //------------------------Customer Funds Manager---------------------------------
  __always_inline
  void deposit(uint256 customer_wallet_address_hex,uint256 token_root_hex,bytes tokenName,bytes tokenSymbol,uint8 decimals, int8 token_type,TokenAmount tokenAmount) {
       require(token_type!=0, error_code::deposit_no_token_type);
       require(customer_wallet_address_hex!=0, error_code::deposit_no_customer_address);
       //get sender address
       auto sender = int_sender();
       uint256 sender_hex=std::get<addr_std>(sender()).address;
       //check if the token already support by exchange
       require(support_token_list.contains(token_root_hex.get()),error_code::deposit_not_support_token);
       uint256 exchange_token_wallet_hex =support_token_list.get_at(token_root_hex.get());
       //we only allow exchange's wallet update the deposit balance.
       require(exchange_token_wallet_hex==sender_hex, error_code::deposit_sender_wrong);
       tvm_accept();
        
       
       if(token_type == 1){
        
         dict_map<uint256,customer_token>  customer_balance={};
         customer_token customerBalance={};
        // This is fungible token .
        if(token_balance_list.contains(token_root_hex.get())){
          //[customer wallet addr hex -->token hold details]
          //find the token all holder customers list.
          customer_balance=token_balance_list.get_at(token_root_hex.get());
          //find the customer balance from map
          if(customer_balance.contains(customer_wallet_address_hex.get())){
            customerBalance=customer_balance.get_at(customer_wallet_address_hex.get());
            TokenAmount old_tokenAmount=customerBalance.token_balance;
            tokenAmount+=old_tokenAmount;
          }
           
        }

        customerBalance={tokenName,tokenSymbol,decimals,tokenAmount};
        customer_balance.set_at(customer_wallet_address_hex.get(),customerBalance);
        token_balance_list.set_at(token_root_hex.get(),customer_balance);
        
       } if(token_type == 2){
         // This is nffungible token .
         dict_map<uint256,customer_nftoken>  customer_nf_balance={};
         customer_nftoken customerNfBalance={};
         dict_set<TokenId>  tokens_={};

        if(nftoken_balance_list.contains(token_root_hex.get())){
          customer_nf_balance=nftoken_balance_list.get_at(token_root_hex.get());
          if(customer_nf_balance.contains(customer_wallet_address_hex.get())){
            customerNfBalance=customer_nf_balance.get_at(customer_wallet_address_hex.get());
            tokens_=customerNfBalance.tokenid_list;
          }
        }
        if(!tokens_.contains(tokenAmount)){
          tokens_.insert(tokenAmount);
        }
        customerNfBalance={tokenName,tokenSymbol,tokens_};
        customer_nf_balance.set_at(customer_wallet_address_hex.get(),customerNfBalance);
        nftoken_balance_list.set_at(token_root_hex.get(),customer_nf_balance);

       }else{
         //no identification token type ,nothing to do.
       }
      
  }

  __always_inline 
  customer_token getFungibleTokenBalance(uint256 customer_wallet_address_hex,uint256 token_root_hex){
      customer_token customerBalance={{0x30},{0x30},uint8(0),TokenAmount(0)};
      dict_map<uint256,customer_token>  customer_balance={};
      if(token_balance_list.contains(token_root_hex.get())){
        customer_balance=token_balance_list.get_at(token_root_hex.get());
        customerBalance=customer_balance.get_at(customer_wallet_address_hex.get());
      }
      return customerBalance;
  }

  __always_inline 
  dict_array<TokenId> getNFFungibleTokenBalance(uint256 customer_wallet_address_hex,uint256 token_root_hex){
      dict_array<TokenId> tokenIds_={};
     
      customer_nftoken customerBalance={{0x30},{0x30},{}};
      dict_map<uint256,customer_nftoken>  customer_balance={};
      if(nftoken_balance_list.contains(token_root_hex.get())){
        customer_balance=nftoken_balance_list.get_at(token_root_hex.get());
        if(customer_balance.contains(customer_wallet_address_hex.get())){
          customerBalance=customer_balance.get_at(customer_wallet_address_hex.get());
          dict_set<TokenId>  tokens_=customerBalance.tokenid_list;
          if(tokens_.size()>0){
            for(auto tokenid:tokens_){
              tokenIds_.push_back(tokenid);
            }
          }

        }
        
      }

       return dict_array<TokenId>(tokenIds_.begin(), tokenIds_.end());
     
  }

  __always_inline 
  void withdraw(uint256 token_root_hex,int8 token_type,TokenAmount tokenAmount){
      require(token_root_hex!=0, error_code::withdraw_no_token_address);
      require(tokenAmount>0, error_code::withdraw_token_amount_error);
      uint256 exchange_wallet_addr_hex=uint256(0);
      //1.get exchange wallet address from token_root_hex
      if(support_token_list.contains(token_root_hex.get())){
        exchange_wallet_addr_hex=support_token_list.get_at(token_root_hex.get());
      }
      require(exchange_wallet_addr_hex>0, error_code::withdraw_token_no_support);
      tvm_accept();
      auto sender = int_sender();
      auto value_gr = int_value();
      auto workchain_id = std::get<addr_std>(address{tvm_myaddr()}.val()).workchain_id;
     
      if(token_type==1){
        //2. check balance from exchange map
        uint256 sender_hex=std::get<addr_std>(sender()).address;
        customer_token customerBalance=getFungibleTokenBalance(sender_hex,token_root_hex);
        TokenAmount old_tokenAmount=customerBalance.token_balance;
        require(old_tokenAmount>0, error_code::not_enough_balance);
        require(old_tokenAmount>=tokenAmount, error_code::not_enough_balance);
        //3. check pass, customer have enough balance for withdraw
        //--3.1 get stander exchange address from exchange hex
        address exchange_wallet_addr = address::make_std(workchain_id, exchange_wallet_addr_hex);
        //--3.2 send token to customer.
        handle<ITONTokenWallet> dest_exchange_wallet(exchange_wallet_addr);
        //dest_exchange_wallet.transfer(sender(),tokenAmount.get(), value_gr.get());
        dest_exchange_wallet(Grams(value_gr.get())).transfer(sender(),tokenAmount, uint128(value_gr.get()));
        //4. clear already withdraw balance
        old_tokenAmount -=tokenAmount;
        customerBalance={customerBalance.token_name,customerBalance.token_symbol,customerBalance.decimals,old_tokenAmount};
        dict_map<uint256,customer_token> customer_balance=token_balance_list.get_at(token_root_hex.get());
        //customerBalance=customer_balance.get_at(sender_hex.get());
        
        customer_balance.set_at(sender_hex.get(),customerBalance);
        token_balance_list.set_at(token_root_hex.get(),customer_balance);

      }
      
  }

   __always_inline 
  void withdrawTest(address exchange_wallet_address,address to_wallet_address,TokenAmount tokenAmount){
    tvm_accept();
    auto value_gr = int_value();
    handle<ITONTokenWallet> dest_exchange_wallet(exchange_wallet_address);
    //dest_exchange_wallet.transfer(sender(),tokenAmount.get(), value_gr.get());
    dest_exchange_wallet(Grams(value_gr.get())).transfer(to_wallet_address,tokenAmount, uint128(value_gr.get()));
  }




  //------------------------System function handle----------------------------------
  // received bounced message back
  __always_inline static int _on_bounced(cell msg, slice msg_body) {
    tvm_accept();
    return 0;
  }

 // default processing of unknown messages
  __always_inline static int _fallback(cell msg, slice msg_body) {
    return 0;
  }
  // =============== Support functions ==================
  DEFAULT_SUPPORT_FUNCTIONS(ITonExchange, root_replay_protection_t)

  
};

DEFINE_JSON_ABI(ITonExchange, DTonExchange, ETonExchange);

// ----------------------------- Main entry functions ---------------------- //
DEFAULT_MAIN_ENTRY_FUNCTIONS_TMPL(TonExchange, ITonExchange, DTonExchange, EXCHANGE_TIMESTAMP_DELAY)