#include "TONTokenWallet.hpp"

#include <tvm/contract.hpp>
#include <tvm/contract_handle.hpp>
#include <tvm/default_support_functions.hpp>

#include "../TonExchange.hpp"
#include "RootTokenContract.hpp"

using namespace tvm;
using namespace schema;

template<bool Internal>
class TONTokenWallet final : public smart_interface<ITONTokenWallet>, public DTONTokenWallet {
public:
  struct error_code : tvm::error_code {
    static constexpr unsigned message_sender_is_not_my_owner    = 100;
    static constexpr unsigned not_enough_balance                = 101;
    static constexpr unsigned message_sender_is_not_my_root     = 102;
    static constexpr unsigned message_sender_is_not_good_wallet = 103;
    static constexpr unsigned wrong_bounced_header              = 104;
    static constexpr unsigned wrong_bounced_args                = 105;
    static constexpr unsigned non_zero_remaining                = 106;
    static constexpr unsigned no_allowance_set                  = 107;
    static constexpr unsigned wrong_spender                     = 108;
    static constexpr unsigned not_enough_allowance              = 109;
    static constexpr unsigned internal_owner_enabled            = 110;
    static constexpr unsigned internal_owner_disabled           = 111;
    static constexpr unsigned set_balance_no_source_address     = 112;   
    static constexpr unsigned set_balance_no_value              = 113;  
    static constexpr unsigned no_exchange_address_for_deposit   = 114;  
  };

   __always_inline
  void constructor(bytes name, bytes symbol, uint8 decimals,
                   uint256 root_public_key, uint256 wallet_public_key,
                   address root_address, cell code) {
    name_ = name;
    symbol_ = symbol;
    decimals_ = decimals;
    balance_ = TokensType(0);
    root_public_key_ = root_public_key;
    wallet_public_key_ = wallet_public_key;
    root_address_ = root_address;
    code_ = code;
  }

  __always_inline
  void transfer(address dest, TokensType tokens, WalletGramsType grams) {
    check_owner();

    // the function must complete successfully if token balance is less that transfer value.
    require(tokens <= balance_, error_code::not_enough_balance);

    // Transfer to zero address is not allowed.
    require(std::get<addr_std>(dest()).address != 0, error_code::not_enough_balance);

    tvm_accept();

    auto owner_addr = owner_address_ ? std::get<addr_std>((*owner_address_)()).address : uint256(0);

    handle<ITONTokenWallet> dest_wallet(dest);
    dest_wallet(Grams(grams.get())).internalTransfer(tokens, wallet_public_key_, owner_addr);
    //minusBalanceToSender(dest,tokens);
    balance_ -= tokens;
  }

  __always_inline
  TokensType getBalance_InternalOwner() {
    check_internal_owner();
    set_int_return_flag(SEND_REST_GAS_FROM_INCOMING);
    return balance_;
  }

  __always_inline
  void accept(TokensType tokens) {
    // the function must check that message sender is the RTW.
    require(root_address_ == int_sender(),
            error_code::message_sender_is_not_my_root);

    tvm_accept();

    balance_ += tokens;
  }

  __always_inline
  void internalTransfer(TokensType tokens, uint256 pubkey, uint256 my_owner_addr) {
    uint256 expected_address = expected_sender_address(pubkey, my_owner_addr);
    auto sender = int_sender();

    require(std::get<addr_std>(sender()).address == expected_address,
            error_code::message_sender_is_not_good_wallet);

    tvm_accept();
    //store the investment amount from each sender by address.
    if(balance_list.contains(expected_address.get())){
      TokensType old_balance=balance_list.get_at(expected_address.get())+tokens;
      balance_list.set_at(expected_address.get(), old_balance);
    
    }else{
      balance_list.set_at(expected_address.get(), tokens);
     
    }

    balance_ += tokens;
  }

  __always_inline
  void destroy(address dest) {
    check_owner();
    tvm_accept();
    auto empty_cell = builder().endc();
    tvm_transfer(dest, 0, false,
      SEND_ALL_GAS | SENDER_WANTS_TO_PAY_FEES_SEPARATELY | DELETE_ME_IF_I_AM_EMPTY | IGNORE_ACTION_ERRORS,
      empty_cell);
  }

  // getters
  __always_inline bytes getName() {
    return name_;
  }
  __always_inline bytes getSymbol() {
    return symbol_;
  }
  __always_inline uint8 getDecimals() {
    return decimals_;
  }
  __always_inline TokensType getBalance() {
    return balance_;
  }
  __always_inline uint256 getWalletKey() {
    return wallet_public_key_;
  }
  __always_inline address getRootAddress() {
    return root_address_;
  }
  __always_inline address getOwnerAddress() {
    return owner_address_ ? *owner_address_ : address::make_std(int8(0), uint256(0));
  }
  __always_inline TokensType allowance(uint256 spenderAddressHex) {

    TokensType spender_balance_ = TokensType(0);
    if(allowance_.contains(spenderAddressHex.get())){
        spender_balance_=  allowance_.get_at(spenderAddressHex.get());
    }
    return spender_balance_;
  }

  __always_inline TokensType getSenderBalance(uint256 senderAddressHex) {
    TokensType sender_balance_ = TokensType(0);
    if(balance_list.contains(senderAddressHex.get())){
        sender_balance_=  balance_list.get_at(senderAddressHex.get());
    }
    return sender_balance_;
  }

  // __always_inline uint8 getTokenType() {
  //   return uint8(1);
  // }


  
  __always_inline
  void approve(address spender, TokensType tokens) {
    //check_owner();
    //aready setting allowance
    TokensType sum=TokensType(0);
    for (auto pair : allowance_) {
      sum += pair.second;
    };
    //only allow approve work have remain tokens
    //require(sum < balance_, error_code::not_enough_balance);
    tvm_accept();
    tvm_commit();
    //max allow approve value
    TokensType remainingToken=balance_-sum;
    if(tokens>remainingToken){
      tokens=remainingToken;
    }
    uint256 spender_address_hex_=std::get<addr_std>(spender()).address;
    if(allowance_.contains(spender_address_hex_.get())){
      TokensType old_allowance=allowance_.get_at(spender_address_hex_.get())+tokens;
      allowance_.set_at(spender_address_hex_.get(), old_allowance);
    
    }else{
      allowance_.set_at(spender_address_hex_.get(), tokens);
    }
  }
  __always_inline
  void approveTest(address spender, TokensType tokens){
      //approve_total_ += tokens;
  }

  __always_inline
  void transferFrom(address dest, address to, TokensType tokens,
                    WalletGramsType grams) {
    check_owner();
    tvm_accept();

    handle<ITONTokenWallet> dest_wallet(dest);
    dest_wallet(Grams(grams.get())).
      internalTransferFrom(to, tokens);
  }

  __always_inline
  void internalTransferFrom(address to, TokensType tokens) {
  
    uint256 spender_address_hex_=std::get<addr_std>(to()).address;
    TokensType old_allowance=TokensType(0);
    
    uint256 findSpender_address_hex_=uint256(0);
    if(allowance_.contains(spender_address_hex_.get())){
      old_allowance=allowance_.get_at(spender_address_hex_.get());
      findSpender_address_hex_=spender_address_hex_;
    }
    auto sender = int_sender();
    uint256 sender_address_hex=std::get<addr_std>(sender()).address; 
    require(old_allowance!=0, error_code::no_allowance_set);
    require(sender_address_hex == findSpender_address_hex_, error_code::wrong_spender);
    require(tokens <= old_allowance, error_code::not_enough_allowance);
    require(tokens <= balance_, error_code::not_enough_balance);

    auto owner_addr = owner_address_ ? std::get<addr_std>((*owner_address_)()).address : uint256(0);
    handle<ITONTokenWallet> dest_wallet(to);
    dest_wallet(Grams(0), SEND_REST_GAS_FROM_INCOMING).
      internalTransfer(tokens, wallet_public_key_, owner_addr);

    //allowance_->remainingTokens -= tokens;
    old_allowance -= tokens;
    if(old_allowance==0){
      allowance_.erase(spender_address_hex_.get());
    }else{
      allowance_.set_at(spender_address_hex_.get(), old_allowance);
    }
    //update balance_list
     TokensType old_blance=TokensType(0);
    if(balance_list.contains(spender_address_hex_.get())){
      old_blance=balance_list.get_at(spender_address_hex_.get());
      if(old_blance>tokens){
         old_blance -= tokens;
         balance_list.set_at(spender_address_hex_.get(),old_blance);
      }else{
        balance_list.erase(spender_address_hex_.get());
      }
    }
    balance_ -= tokens;
  }

  __always_inline
  void disapprove() {
    check_owner();
    tvm_accept();
    allowance_.clear();
  }


  __always_inline
  uint256 getWalletCodeHash() {
    return uint256{__builtin_tvm_hashcu(code_)};
  }

 
  //---------------------exchange functions -----------------------------------

  __always_inline
  void depositToExchange(address exchange_address) {
      //check_owner();
      //get source_address from sender , only can handle and send self funds
      auto sender = int_sender();
      uint256 sender_address_hex=std::get<addr_std>(sender()).address;
      uint256 root_address_hex= std::get<addr_std>(root_address_()).address;
      //require(exchange_address != 0,error_code::no_exchange_address_for_deposit);
      require(sender_address_hex != 0,error_code::set_balance_no_source_address);
      tvm_accept();
      //sender_addr_hex=sender_address_hex;
      if(balance_list.contains(sender_address_hex.get())){
        //1.get the balance from deposit
        TokensAmount old_balance=balance_list.get_at(sender_address_hex.get());
        

        //auto token_wallet_hex = std::get<addr_std>(address{tvm_myaddr()}.val()).address;
        //2. send balance to exchange 
        handle<ITonExchange> dest_exchange(exchange_address);  
        dest_exchange(Grams(0), SEND_REST_GAS_FROM_INCOMING).deposit(sender_address_hex,root_address_hex,name_,symbol_,decimals_, int8(1),old_balance);
        
        //3.remove the balance from wallet.
        balance_list.erase(sender_address_hex.get());

      }

    }

  __always_inline
  void sendDepositToExchangeRequst(address exchange_wallet_address,address exchange_address,WalletGramsType grams_exchange) {
        check_owner();
        tvm_accept();
        //transfer(exchange_wallet_address, tokens, grams_transfer);
        handle<ITONTokenWallet> dest_exchange(exchange_wallet_address);  
        dest_exchange(Grams(grams_exchange.get())).depositToExchange(exchange_address);

  } 
  //This method will reg this token into exchange
  __always_inline
  void regTokenToExchangeFromRoot(address exchange_address,WalletGramsType grams){
    check_owner();
    tvm_accept();

   // uint256 exchange_address_hex=std::get<addr_std>(exchange_address()).address;

    handle<IRootTokenContract> dest_root(root_address_);
    dest_root(Grams(grams.get())).regTokenToExchange(exchange_address);
  }
  __always_inline
  void sendTransaction(address dest,WalletGramsType grams,cell msgBody)
  {
    check_owner();
    //tvm_transfer(slice dest, unsigned nanograms, unsigned bounce, unsigned flags, cell payload) 
    require(grams.get() > 0 ,error_code::not_enough_balance);
    //simple set the bounce to false, it can be improve in next stage.
    tvm_transfer(dest, grams.get(), false, SEND_ALL_GAS ,msgBody);

  }

  __always_inline
  void withdrawFromExchange(address exchange_address,TokensAmount tokenAmount,WalletGramsType grams)
  {
    check_owner();
    tvm_accept();
    uint256 root_address_hex= std::get<addr_std>(root_address_()).address;
    //transfer(exchange_wallet_address, tokens, grams_transfer);
    handle<ITonExchange> dest_exchange(exchange_address);  
    dest_exchange(Grams(grams.get())).withdraw(root_address_hex,int8(1),tokenAmount);
  }

  __always_inline
  void putOrder(uint128 sell_amount,uint256 seller_resive_address,uint256 buy_token_addr_hex,uint128 buy_amount,address exchange_address,WalletGramsType grams)
  {
    check_owner();
    tvm_accept();
    uint256 sell_token_addr_hex=std::get<addr_std>(root_address_()).address;
    handle<ITonExchange> dest_exchange(exchange_address); 
    dest_exchange(Grams(grams.get())).putOrder(sell_token_addr_hex,sell_amount,seller_resive_address,buy_token_addr_hex,buy_amount);

  }

  __always_inline
  void cancelOrder(uint32 order_no,address exchange_address,WalletGramsType grams)
  {
    check_owner();
    tvm_accept();
    handle<ITonExchange> dest_exchange(exchange_address);  
    dest_exchange(Grams(grams.get())).cancelOrder(order_no);
  }


    __always_inline
  void fillOrder(uint32 order_no,uint256 buyer_resive_token_address_hex,address exchange_address,WalletGramsType grams)
  {
    check_owner();
    tvm_accept();
    handle<ITonExchange> dest_exchange(exchange_address);  
    dest_exchange(Grams(grams.get())).fillOrder(order_no,buyer_resive_token_address_hex);
  }

  __always_inline
  void sendTransaction(address dest,uint128 value){
    check_owner();
    tvm_accept();
    int balance = tvm::smart_contract_info::balance_remaining();
    require(value.get() > 0 && value.get() < balance,error_code::not_enough_balance);

    tvm_transfer(dest, value.get(),false);
  }


  //-----------------------------System function--------------------------------
  // received bounced message back
  __always_inline static int _on_bounced(cell msg, slice msg_body) {
    tvm_accept();

    parser p(msg_body);
    require(p.ldi(32) == -1, error_code::wrong_bounced_header);
    auto [opt_hdr, =p] = parse_continue<abiv1::internal_msg_header>(p);
    require(!!opt_hdr, error_code::wrong_bounced_header);
    // If it is bounced internalTransferFrom, do nothing
    if (opt_hdr->function_id == id_v<&ITONTokenWallet::internalTransferFrom>)
      return 0;

    auto [hdr, persist] = load_persistent_data<ITONTokenWallet, wallet_replay_protection_t, DTONTokenWallet>();
     

    //if it is bounced from ITonExchange.deposit
     if (opt_hdr->function_id == id_v<&ITonExchange::deposit>){
        using Args = args_struct_t<&ITonExchange::deposit>;
        static_assert(std::is_same_v<decltype(Args{}.tokenAmount), TokenAmount>);
        auto bounced_deposit_val = parse<TokenAmount>(p, error_code::wrong_bounced_args);
        auto sender = int_msg().unpack().int_sender(); 
        uint256 sender_hex=std::get<addr_std>(sender()).address;
        persist.balance_list.set_at(sender_hex.get(),bounced_deposit_val);
        save_persistent_data<ITONTokenWallet, wallet_replay_protection_t>(hdr, persist);
        return 0;
     }  

    // Otherwise, it should be bounced internalTransfer
    require(opt_hdr->function_id == id_v<&ITONTokenWallet::internalTransfer>,
            error_code::wrong_bounced_header);
    using Args = args_struct_t<&ITONTokenWallet::internalTransfer>;
    static_assert(std::is_same_v<decltype(Args{}.tokens), TokensType>);

    // Parsing only first tokens variable internalTransfer pubkey argument won't fit into bounced response
    auto bounced_val = parse<TokensType>(p, error_code::wrong_bounced_args);

   persist.balance_ += bounced_val;
    save_persistent_data<ITONTokenWallet, wallet_replay_protection_t>(hdr, persist);
    return 0;
  }
  // default processing of unknown messages
  __always_inline static int _fallback(cell msg, slice msg_body) {
    return 0;
  }

  // =============== Support functions ==================
  DEFAULT_SUPPORT_FUNCTIONS(ITONTokenWallet, wallet_replay_protection_t)
private:
  __always_inline uint256 expected_sender_address(uint256 sender_public_key, uint256 sender_owner_addr) {
    std::optional<address> owner_addr;
    if (sender_owner_addr)
      owner_addr = address::make_std(workchain_id_, sender_owner_addr);
    DTONTokenWallet wallet_data {
      name_, symbol_, decimals_,
      TokensType(0), root_public_key_, sender_public_key,
      root_address_, owner_addr, code_, {}, workchain_id_,{}
    };
    return prepare_wallet_state_init_and_addr(wallet_data).second;
  }

  __always_inline bool is_internal_owner() const { return owner_address_.has_value(); }

  __always_inline void check_internal_owner() {
    require(is_internal_owner(), error_code::internal_owner_disabled);
    require(*owner_address_ == int_sender(),
            error_code::message_sender_is_not_my_owner);
  }

  __always_inline void check_external_owner() {
    require(!is_internal_owner(), error_code::internal_owner_enabled);
    require(tvm_pubkey() == wallet_public_key_, error_code::message_sender_is_not_my_owner);
  }

  __always_inline void check_owner() {
    if constexpr (Internal)
      check_internal_owner();
    else
      check_external_owner();
  }

 
 
};

DEFINE_JSON_ABI(ITONTokenWallet, DTONTokenWallet, ETONTokenWallet);

// ----------------------------- Main entry functions ---------------------- //
DEFAULT_MAIN_ENTRY_FUNCTIONS_TMPL(TONTokenWallet, ITONTokenWallet, DTONTokenWallet, TOKEN_WALLET_TIMESTAMP_DELAY)