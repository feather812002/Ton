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
    static constexpr unsigned wrong_bounced_header            = 115;
    static constexpr unsigned put_order_not_support_token     = 116;
    static constexpr unsigned put_order_type_error            = 117;
    static constexpr unsigned put_order_input_error           = 118;
    static constexpr unsigned put_order_no_enough_balance     = 119;
    static constexpr unsigned cancel_order_no_error           = 120;  
    static constexpr unsigned fill_order_input_error          = 121;
    static constexpr unsigned fill_order_status_error         = 122;
    static constexpr unsigned fill_order_no_enough_balance    = 123; 
    static constexpr unsigned fill_order_no_enough_fee        = 124;  
  };

  __always_inline
  void constructor() {
  }
  

  //------------------New Token Support-------------------------
  __always_inline
  void regNewToken(uint256 token_wallet,bytes token_symbol) {
    //until now , I still not found a good way can detect a online smart contract is good or bad.
    //so , this is maybe improve in later ,if we can identification a smart contract is good or bad.
    tvm_accept();
    auto sender=int_sender();
    root_address_hex=std::get<addr_std>(sender()).address;
    support_token supporttoken={token_wallet,token_symbol};
    if(!support_token_list.contains(root_address_hex.get())){
      support_token_list.set_at(root_address_hex.get(), supporttoken);
    }
  }
  // getters
  __always_inline uint256 getRootAddress() {
    return root_address_hex;
  }

  __always_inline support_token getSupportTokenByRoot(uint256 root_addr_hex) {
    support_token token_wallet={uint256(0),{0x0}};
    if(support_token_list.contains(root_addr_hex.get())){
        token_wallet=support_token_list.get_at(root_addr_hex.get());
    }
    return token_wallet;
  }

  __always_inline   dict_array<support_token>  getAllSupportTokens(){
     dict_array<support_token> supportTokenList={};
     for(auto supporttoken:support_token_list){
            support_token new_support_token={supporttoken.first,supporttoken.second.token_symbol};
            supportTokenList.push_back(new_support_token);
        }
    return supportTokenList;
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
       uint256 exchange_token_wallet_hex =support_token_list.get_at(token_root_hex.get()).exchange_wallet_addr;
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
         //Unrecognized token type ,nothing to do.
       }
      
  }

  __always_inline 
  customer_token getFungibleTokenBalance(uint256 customer_wallet_address_hex,uint256 token_root_hex){
      customer_token customerBalance={{0x0},{0x0},uint8(0),TokenAmount(0)};
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
        exchange_wallet_addr_hex=support_token_list.get_at(token_root_hex.get()).exchange_wallet_addr;
      }
      require(exchange_wallet_addr_hex>0, error_code::withdraw_token_no_support);
      tvm_accept();
      auto sender = int_sender();
      auto value_gr = int_value();
      auto workchain_id = std::get<addr_std>(address{tvm_myaddr()}.val()).workchain_id;
      uint256 sender_hex=std::get<addr_std>(sender()).address;

      address exchange_wallet_addr = address::make_std(workchain_id, exchange_wallet_addr_hex);
      
      if(token_type==1){
        //2. check balance from exchange map
        customer_token customerBalance=getFungibleTokenBalance(sender_hex,token_root_hex);
        TokenAmount old_tokenAmount=customerBalance.token_balance;
        require(old_tokenAmount>0, error_code::not_enough_balance);
        require(old_tokenAmount>=tokenAmount, error_code::not_enough_balance);
        //3. check pass, customer have enough balance for withdraw
        //-- send token to customer.
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

      }else if(token_type==2){
        require(nftoken_balance_list.contains(token_root_hex.get()), error_code::withdraw_token_no_support);
        dict_map<uint256,customer_nftoken> customerNFBalanceList=nftoken_balance_list.get_at(token_root_hex.get());
        require(customerNFBalanceList.contains(sender_hex.get()), error_code::not_enough_balance);
        customer_nftoken customerNFBalance=customerNFBalanceList.get_at(sender_hex.get());
        //this is nonfungible token
        //1. check the balance if is enough.
        dict_set<TokenId> tokenIds_=customerNFBalance.tokenid_list;
        require(tokenIds_.size()>0, error_code::not_enough_balance);
        require(tokenIds_.contains(tokenAmount), error_code::not_enough_balance);
        //2. start withdraw to dest wallet
        handle<ITONTokenWallet> dest_exchange_wallet(exchange_wallet_addr);
        dest_exchange_wallet(Grams(value_gr.get())).transfer(sender(),tokenAmount, uint128(value_gr.get()));
        //3.remove  balance from exchange.
        tokenIds_.erase(tokenAmount);
        customerNFBalance={customerNFBalance.token_name,customerNFBalance.token_symbol,tokenIds_};
        customerNFBalanceList.set_at(sender_hex.get(),customerNFBalance);
        nftoken_balance_list.set_at(token_root_hex.get(),customerNFBalanceList);
      }else{
        //Unrecognized token nothing to do 
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

  //----------------execute exchange function--------------------------
  __always_inline 
  void putOrder(uint256 sell_token_addr_hex,TokenAmount sell_amount ,uint256 seller_resive_address,
  uint256 buy_token_addr_hex,TokenAmount buy_amount){
   
    //0.input check .
    require(sell_token_addr_hex>0,error_code::put_order_input_error);
    require(sell_amount>0,error_code::put_order_input_error);
    require(seller_resive_address>0,error_code::put_order_input_error);
    require(buy_token_addr_hex>0,error_code::put_order_input_error);
    require(buy_amount>0,error_code::put_order_input_error);
    tvm_accept();
    uint256 buyer_send_address=uint256(0);
    uint256 buyer_resive_address=uint256(0);
    //1. check if the sell and buy tokens all already support by exchange.
    require(support_token_list.contains(sell_token_addr_hex.get()) && support_token_list.contains(buy_token_addr_hex.get()), error_code::put_order_not_support_token);
    
    
   
    uint256 sender_hex=uint256(0);
    if constexpr (Internal){
       auto sender = int_sender();
       sender_hex=std::get<addr_std>(sender()).address;
    }
      
  
    
    uint8 sell_token_type=getTokenType(sell_token_addr_hex);
    uint8 buy_token_type=getTokenType(buy_token_addr_hex);
    bytes sell_token_symbol=getTokenSymbol(sell_token_addr_hex);
    bytes buy_token_symbol=getTokenSymbol(buy_token_addr_hex);
   
    //2.check if the balance is enough for order maker.
    require(sell_token_type>0, error_code::put_order_type_error);
    require(buy_token_type>0, error_code::put_order_type_error);
    require(putOrderCheckBalance(sell_token_addr_hex,sender_hex,sell_token_type,sell_amount)>0,error_code::put_order_no_enough_balance);

    //3. put order
    order_no_count++;

    //order status:0:expired,1:puted, 2:filled,3:part filled,4:cancle. 
    uint8 order_status=uint8(1);
    order new_order={order_no_count,sell_token_addr_hex,sell_amount,sender_hex,seller_resive_address,sell_token_symbol,
    sell_token_type,buy_token_addr_hex,buy_amount,buyer_send_address,buyer_resive_address,buy_token_symbol,buy_token_type,order_status};
   
    order_array.push_back(new_order);

    //4. update balance of exchange .remove already amount for put order.
    updateBalance(sell_token_addr_hex,sender_hex,sell_token_type,sell_amount,uint8(1),sender_hex);
     
  }
  
  __always_inline 
  void cancelOrder(uint32 order_no){
    require(order_no>0,error_code::cancel_order_no_error);
    tvm_accept();
    uint256 sender_hex=uint256(0);
    if constexpr (Internal){
       auto sender = int_sender();
       sender_hex=std::get<addr_std>(sender()).address;
    }
   
    auto [oder_idx,get_order]=getOrderByNo(order_no);
    require(oder_idx>=0,error_code::cancel_order_no_error);
    //only can cancle waiting or part fill order 
    require(get_order.order_status==1 ||get_order.order_status==3,error_code::cancel_order_no_error);
     //only maker can cancle order for itself.
    require(get_order.seller_send_token_address_hex==sender_hex,error_code::cancel_order_no_error);
    get_order.order_status=uint8(4);

    //update the order array;
    if(oder_idx>=0)
    order_array.set_at(int(oder_idx),get_order);

    //update back to balance of exchange;
    updateBalance(get_order.sell_token_root_address_hex,sender_hex,get_order.sell_token_type,get_order.sell_token_amount,uint8(2),sender_hex);
  }


  __always_inline 
  dict_array<order> getAllOrder() {
    //set default value to 0
    return order_array;
  }

  __always_inline 
  dict_array<order> getMyMakerOrders(uint256 maker_address) {
    //set default value to 0
    dict_array<order> all_orders={{uint32(0),uint256(0),uint128(0),uint256(0),uint256(0),{0x0},uint8(0),uint256(0),uint128(0),uint256(0),uint256(0),{0x0},uint8(0),uint8(0)}};
    for (order order_:order_array){
      if(order_.seller_send_token_address_hex==maker_address){
         all_orders.push_back(order_);
      }
    }
    return all_orders;
  }

  __always_inline 
  dict_array<order> getMyTakerOrders(uint256 taker_address) {
    //set default value to 0
    dict_array<order> all_orders={{uint32(0),uint256(0),uint128(0),uint256(0),uint256(0),{0x0},uint8(0),uint256(0),uint128(0),uint256(0),uint256(0),{0x0},uint8(0),uint8(0)}};
    for (order order_:order_array){
      if(order_.buyer_send_token_address_hex==taker_address){
         all_orders.push_back(order_);
      }
    }
    return all_orders;
  }

  __always_inline 
  dict_array<order> getMyCancelOrders(uint256 maker_address) {
    //set default value to 0
    dict_array<order> all_orders={{uint32(0),uint256(0),uint128(0),uint256(0),uint256(0),{0x0},uint8(0),uint256(0),uint128(0),uint256(0),uint256(0),{0x0},uint8(0),uint8(0)}};
    for (order order_:order_array){
    
      if(order_.buyer_send_token_address_hex==maker_address){
         if(order_.order_status==4){
          all_orders.push_back(order_);
         }
      }
    }
    return all_orders;
  }

  __always_inline 
  dict_array<order> getMyFilledOrders(uint256 maker_address) {
    //set default value to 0
    dict_array<order> all_orders={{uint32(0),uint256(0),uint128(0),uint256(0),uint256(0),{0x0},uint8(0),uint256(0),uint128(0),uint256(0),uint256(0),{0x0},uint8(0),uint8(0)}};
    for (order order_:order_array){
      if(order_.buyer_send_token_address_hex==maker_address){
         if(order_.order_status==2){
          all_orders.push_back(order_);
         }
      }
    }
    return all_orders;
  }

  __always_inline 
  uint8 fillOrder(uint32 order_no,uint256 buyer_resive_token_address_hex){
    uint8 result=uint8(0);
    require(buyer_resive_token_address_hex>0,error_code::fill_order_input_error);
    require(order_no>0,error_code::fill_order_input_error);

    tvm_accept();
    uint256 sender_hex=uint256(0);
    if constexpr (Internal){
       auto sender = int_sender();
       sender_hex=std::get<addr_std>(sender()).address;
    }
    //1.get order by order_no 
    auto [oder_idx,get_order]=getOrderByNo(order_no);
    require(oder_idx>=0,error_code::fill_order_status_error);
    //make sure the order sell amount and buy amount >0
    require(get_order.sell_token_amount>0&&get_order.buy_token_amount>0 ,error_code::fill_order_status_error);
    //make sure the order is put or part filled status.
    require(get_order.order_status==1||get_order.order_status==3 ,error_code::fill_order_status_error);
    //check have enough balance fill the order.
    //TODO if this is fungible tarde to fungible ,allow part filled.
    require(putOrderCheckBalance(get_order.buy_token_root_address_hex,sender_hex,get_order.buy_token_type,get_order.buy_token_amount)>0,error_code::fill_order_no_enough_balance);
    //uint256 sender_hex=uint256(0);
    //start fill order........
    get_order.buyer_send_token_address_hex=sender_hex;
    get_order.buyer_resive_token_address_hex=buyer_resive_token_address_hex;
    get_order.order_status=2;

    //2.1 sell token (only add to buyer) 
    updateBalance(get_order.sell_token_root_address_hex,buyer_resive_token_address_hex,get_order.sell_token_type,get_order.sell_token_amount,uint8(2),get_order.seller_send_token_address_hex);
    //2.2 buyer send token to seller.
    updateBalance(get_order.buy_token_root_address_hex,sender_hex,get_order.buy_token_type,get_order.buy_token_amount,uint8(1),sender_hex);
    updateBalance(get_order.buy_token_root_address_hex,get_order.seller_resive_token_address_hex,get_order.buy_token_type,get_order.buy_token_amount,uint8(2),get_order.buyer_send_token_address_hex);

    //3. update order.
    order_array.set_at(int(oder_idx),get_order);
    //4.send fee to taker and keep rest in the exchange .
    if constexpr (Internal) {
      auto value_gr = int_value();
      require(value_gr>=1000000000,error_code::fill_order_no_enough_fee);
      //transfer 0.5 TON to maker .
      auto workchain_id = std::get<addr_std>(address{tvm_myaddr()}.val()).workchain_id;
      address sender_wallet_addr = address::make_std(workchain_id, get_order.seller_send_token_address_hex);
      tvm_transfer(sender_wallet_addr, 500000000,false);
      

    }
   

    result=uint8(1);
    return result;

  }
  __always_inline 
  void testTransaction(address sender_wallet_addr,uint128 value){
    //tvm_accept();
    if constexpr (Internal) {
      auto value_gr = int_value();
      //require(value_gr>=1000000000,error_code::fill_order_no_enough_fee);
      //transfer 0.5 TON to maker .
     // auto workchain_id = std::get<addr_std>(address{tvm_myaddr()}.val()).workchain_id;
     // address sender_wallet_addr = address::make_std(workchain_id, get_order.seller_send_token_address_hex);
      
      

    }
    tvm_transfer(sender_wallet_addr, value.get(),false);
  }
  __always_inline 
  void putTestDate(uint256 exchangeWallet1,uint256 exchangeWallet2){

    tvm_accept();
    support_token supporttoen1={exchangeWallet1,{0}};
    support_token supporttoen2={exchangeWallet2,{0}};

    support_token_list.set_at(0xcbb30ce4991335ab7f7698d1339c13387471e3626541c66b0428413998339ed7, supporttoen1);
    support_token_list.set_at(0x93caf629948c3c0c73f93f9381e52bcbca5b24ac395d7c70559f9ddd55bc6614, supporttoen2);
    
    customer_token customer_balance={{0},{0},uint8(0),uint128(20)};
    dict_map<uint256,customer_token> fungible_token_list={};
    // fungible_token_list.set_at(0x74b16b79d2172edb4903630ff5a874f7d42e6c2e9a488729198a725a7ce58b03,customer_balance);
    // token_balance_list.set_at(0x93caf629948c3c0c73f93f9381e52bcbca5b24ac395d7c70559f9ddd55bc6614,fungible_token_list);

    customer_balance={{0},{0},uint8(0),uint128(50)};
    fungible_token_list.set_at(0x52fd43a76cb8dceddf5ad15f268a8f0850851354381ca2130261fef326e0d97d,customer_balance);
    token_balance_list.set_at(0xcbb30ce4991335ab7f7698d1339c13387471e3626541c66b0428413998339ed7,fungible_token_list);

    
 
    dict_set<TokenId>  tokenid_list={};
    tokenid_list.insert(uint128(102));
    tokenid_list.insert(uint128(100));
    tokenid_list.insert(uint128(101));
    customer_nftoken customer_balancenf={{0},{0},tokenid_list};
    dict_map<uint256,customer_nftoken> nffungible_token_list={};
    nffungible_token_list.set_at(0x74b16b79d2172edb4903630ff5a874f7d42e6c2e9a488729198a725a7ce58b03,customer_balancenf);
    nftoken_balance_list.set_at(0x93caf629948c3c0c73f93f9381e52bcbca5b24ac395d7c70559f9ddd55bc6614,nffungible_token_list);



    order new_order={uint32(1),uint256(0xcbb30ce4991335ab7f7698d1339c13387471e3626541c66b0428413998339ed7),
    uint128(5),uint256(0x52fd43a76cb8dceddf5ad15f268a8f0850851354381ca2130261fef326e0d97d), 
    uint256(0x745d17bbff9ad8fea8187fe66e2cebb0a1bdd0a1e97cc421e68bb4be591bf73a),{0},uint8(1),
    uint256(0x93caf629948c3c0c73f93f9381e52bcbca5b24ac395d7c70559f9ddd55bc6614),uint128(100),
    uint256(0),uint256(0),{0},uint8(2),uint8(1)};
    order_array.push_back(new_order);
  }
  
  //------------------------System function handle----------------------------------
  // received bounced message back
  __always_inline static int _on_bounced(cell msg, slice msg_body) {
    tvm_accept();
    parser p(msg_body);
    require(p.ldi(32) == -1, error_code::wrong_bounced_header);
    auto [opt_hdr, =p] = parse_continue<abiv1::internal_msg_header>(p);
    require(!!opt_hdr, error_code::wrong_bounced_header);


    //  it should be bounced transfer
    require(opt_hdr->function_id == id_v<&ITONTokenWallet::transfer>,
            error_code::wrong_bounced_header);
    using Args = args_struct_t<&ITONTokenWallet::transfer>;
    static_assert(std::is_same_v<decltype(Args{}.tokens), TokensType>);

    // restore withdraw from bounced message to exchange.
    auto tokenAmount = parse<TokenAmount>(p, error_code::wrong_bounced_args);
    require(tokenAmount > 0, error_code::wrong_bounced_args);
    auto token_type=parse<int8>(p, error_code::wrong_bounced_args);
    require(token_type > 0, error_code::wrong_bounced_args);
    auto token_root_hex=parse<uint256>(p, error_code::wrong_bounced_args);
    require(token_root_hex > 0, error_code::wrong_bounced_args);
    
    auto sender = int_msg().unpack().int_sender(); 

       
    uint256 sender_hex=std::get<addr_std>(sender()).address;

    auto [hdr, persist] = load_persistent_data<ITonExchange, exchange_replay_protection_t, DTonExchange>();
    if(token_type==1){
      dict_map<uint256,customer_token> customer_balance=persist.token_balance_list.get_at(token_root_hex.get());
      customer_token customerBalance=customer_balance.get_at(sender_hex.get());
      TokenAmount old_tokenAmount=customerBalance.token_balance;
      old_tokenAmount +=tokenAmount;
      customerBalance={customerBalance.token_name,customerBalance.token_symbol,customerBalance.decimals,old_tokenAmount};
      customer_balance.set_at(sender_hex.get(),customerBalance);
      persist.token_balance_list.set_at(token_root_hex.get(),customer_balance);
    }else if(token_type==2){
      dict_map<uint256,customer_nftoken> customerNFBalanceList=persist.nftoken_balance_list.get_at(token_root_hex.get());
      customer_nftoken customerNFBalance=customerNFBalanceList.get_at(sender_hex.get());
      dict_set<TokenId> tokenIds_=customerNFBalance.tokenid_list;
      tokenIds_.insert(tokenAmount);
      customerNFBalance={customerNFBalance.token_name,customerNFBalance.token_symbol,tokenIds_};
      customerNFBalanceList.set_at(sender_hex.get(),customerNFBalance);
      persist.nftoken_balance_list.set_at(token_root_hex.get(),customerNFBalanceList);
    }
    save_persistent_data<ITonExchange, exchange_replay_protection_t>(hdr, persist);
    return 0;
  }

 // default processing of unknown messages
  __always_inline static int _fallback(cell msg, slice msg_body) {
    return 0;
  }
  // =============== Support functions ==================
  DEFAULT_SUPPORT_FUNCTIONS(ITonExchange, exchange_replay_protection_t)
private:

  __always_inline uint8 getTokenType(uint256 token_root_addr){
    uint8 token_type=uint8(0); 
    if(token_balance_list.contains(token_root_addr.get())){
      token_type=uint8(1);
    }
    if(nftoken_balance_list.contains(token_root_addr.get())){
      token_type=uint8(2);
    }
    return token_type;
  }

  __always_inline bytes getTokenSymbol(uint256 token_root_addr){
    bytes token_symbol={0};
    if(support_token_list.contains(token_root_addr.get())){
      token_symbol=support_token_list.get_at(token_root_addr.get()).token_symbol;
    }
    return token_symbol;

  }

  __always_inline uint8 putOrderCheckBalance(uint256 token_root_addr_hex,uint256 customer_wallet_addr_hex,uint8 token_type,uint128 token_amount) {
    uint8 result=uint8(0);
    if(token_type==1){
      require(token_balance_list.contains(token_root_addr_hex.get()), error_code::not_enough_balance);
      dict_map<uint256,customer_token> fungible_token_list=token_balance_list.get_at(token_root_addr_hex.get());
      require(fungible_token_list.contains(customer_wallet_addr_hex.get()), error_code::not_enough_balance);
      customer_token customer_balance=fungible_token_list.get_at(customer_wallet_addr_hex.get());
      TokenAmount old_balance=customer_balance.token_balance;
      require(old_balance>=token_amount,error_code::not_enough_balance);
      result=uint8(1);
    }
     if(token_type==2){
      require(nftoken_balance_list.contains(token_root_addr_hex.get()), error_code::not_enough_balance);
      dict_map<uint256,customer_nftoken> nonfungible_token_list=nftoken_balance_list.get_at(token_root_addr_hex.get());
      require(nonfungible_token_list.contains(customer_wallet_addr_hex.get()), error_code::not_enough_balance);
      customer_nftoken customer_nfbalance=nonfungible_token_list.get_at(customer_wallet_addr_hex.get());
      dict_set<TokenId> old_nfbalance=customer_nfbalance.tokenid_list;
      require(old_nfbalance.contains(token_amount),error_code::not_enough_balance);
      result=uint8(2);
    }
    return result;
  }

  //action :1-remove token_amount from blance ,2-add the token_amount from balance.
  __always_inline void updateBalance(uint256 token_root_addr_hex,uint256 customer_wallet_addr_hex,uint8 token_type,uint128 token_amount,uint8 action,uint256 payer_wallet_addr_hex) 
  {
    if(token_type==1){
      require(token_balance_list.contains(token_root_addr_hex.get()), error_code::not_enough_balance);
      dict_map<uint256,customer_token> fungible_token_list=token_balance_list.get_at(token_root_addr_hex.get());
      if(action==1){
        require(fungible_token_list.contains(customer_wallet_addr_hex.get()), error_code::not_enough_balance);
      }
      customer_token customer_balance={};
      TokenAmount old_balance=uint128(0); 
      if(fungible_token_list.contains(customer_wallet_addr_hex.get())){
        customer_balance =fungible_token_list.get_at(customer_wallet_addr_hex.get());
        old_balance=customer_balance.token_balance;
      }else{
        customer_balance =fungible_token_list.get_at(payer_wallet_addr_hex.get());
      }
     
      if(action==1){
        old_balance-=token_amount;
      }
      if(action==2){
        old_balance+=token_amount;
      }
      customer_balance={customer_balance.token_name,customer_balance.token_symbol,customer_balance.decimals,old_balance};
      fungible_token_list.set_at(customer_wallet_addr_hex.get(),customer_balance);
      token_balance_list.set_at(token_root_addr_hex.get(),fungible_token_list);
    }
    if(token_type==2){
      require(nftoken_balance_list.contains(token_root_addr_hex.get()), error_code::not_enough_balance);
      dict_map<uint256,customer_nftoken> nonfungible_token_list=nftoken_balance_list.get_at(token_root_addr_hex.get());
      if(action==1){
        require(nonfungible_token_list.contains(customer_wallet_addr_hex.get()), error_code::not_enough_balance);
      }
      customer_nftoken customer_nfbalance={};
      dict_set<TokenId> old_nfbalance={};
      if(nonfungible_token_list.contains(customer_wallet_addr_hex.get())){
        customer_nfbalance=nonfungible_token_list.get_at(customer_wallet_addr_hex.get());
        old_nfbalance=customer_nfbalance.tokenid_list;
      }else{
        customer_nfbalance=nonfungible_token_list.get_at(payer_wallet_addr_hex.get());
      }
      
      
    
      if(action==1){
        old_nfbalance.erase(token_amount);
      }
      if(action==2){
        old_nfbalance.insert(token_amount);
      }
      customer_nfbalance={customer_nfbalance.token_name,customer_nfbalance.token_symbol,old_nfbalance};
      nonfungible_token_list.set_at(customer_wallet_addr_hex.get(),customer_nfbalance);
      nftoken_balance_list.set_at(token_root_addr_hex.get(),nonfungible_token_list);
      
    }
  }

   __always_inline std::pair<int32, order> getOrderByNo(uint32 order_no)
   {
     //inital a order to zero status, so that we can get a return for check.
      order find_order={uint32(0),uint256(0),uint128(0),uint256(0),uint256(0),
      {0x0},uint8(0),uint256(0),uint128(0),uint256(0),uint256(0),{0x0},uint8(0),uint8(0)};
      int32 idx(-1);
      auto [key, order_, success] = order_array.min();
      if(order_.order_no == order_no){
          find_order=order_;
          idx=key;
      }
     
      while (success) {
        auto [=key, =order_, =success] = order_array.next(key);
        if(order_.order_no == order_no){
          find_order=order_;
          idx=key;
        }
       
      }
     
      return { idx, find_order };
     
   }
};

DEFINE_JSON_ABI(ITonExchange, DTonExchange, ETonExchange);

// ----------------------------- Main entry functions ---------------------- //
DEFAULT_MAIN_ENTRY_FUNCTIONS_TMPL(TonExchange, ITonExchange, DTonExchange, EXCHANGE_TIMESTAMP_DELAY)