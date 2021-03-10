pragma ton-solidity >= 0.35.0;
pragma AbiHeader expire;
pragma AbiHeader pubkey;
pragma AbiHeader time;

import "ITIP3Interface.sol";
import "ITonExchange.sol";
contract TonExchange is ITonExchange{

    // Exception codes:
	uint constant NO_PUBKEY                             = 101;	
    uint constant MESSAGE_SENDER_IS_NOT_THE_OWNER       = 102;
    uint constant REG_NEW_TOKEN_ADDRESS_MISS            = 103;
    uint constant ADD_NEW_TOKEN_ADDRESS_ERROR           = 104;
    uint constant MISS_QUERY_ROOT_ADDRESS_ERROR         = 105;
    //punliv var
    uint public counter; 
	bytes public name;
	bytes public symbol;
	bytes public error;

struct TokenBalance {
	bytes token_name;
	bytes token_symbol;
	uint128 token_balance;
	uint256 root_token_address_hex;
}

struct support_token {
  uint256 exchange_wallet_addr;
  bytes token_symbol;
}

struct show_support_token {
  uint256 token_root_addr;
  bytes token_symbol;
}


    //------------------Token Manager------------------------------
    //support token list.   root address hex-->exchange wallet address hex.
    mapping(uint256 => uint256) exchangeTokenWalletList;
    mapping(uint256 =>support_token) support_token_list;

	address public roor_return_address;

    constructor() public {
		// check that contract's public key is set
		require(tvm.pubkey() != 0, NO_PUBKEY);
		// Check that message has signature (msg.pubkey() is not zero) and message is signed with the owner's private key
		require(msg.pubkey() == tvm.pubkey(), MESSAGE_SENDER_IS_NOT_THE_OWNER);
		tvm.accept();
	}

	modifier checkOwnerAndAccept {
		// Check that message was signed with contracts key.
		require(msg.pubkey() == tvm.pubkey(), MESSAGE_SENDER_IS_NOT_THE_OWNER);
		tvm.accept();
		_;
	}
	modifier AlwaysAccept() {
   		tvm.accept(); 
		_;
  	}
    //-------------------------------Token manager------------------------------------------------------------------------------------------
	 /**
     * This method will reg new token into exchange
     */   

    function regNewToken(uint256 tooken_wallet_address_hex)
        public
        override
        functionID(0x50) 
    {
        require(tooken_wallet_address_hex!=0, 103);
        tvm.accept(); 
        tvm.commit();
        //1.get the  sender address from message
        address root_address = msg.sender;
        int8 wid=root_address.wid;
        uint256 root_address_hex=root_address.value;
        //2.get token wallet stander format from parameter
       roor_return_address = address.makeAddrStd(wid, tooken_wallet_address_hex);
       
        if(root_address_hex!=0){
            if(!exchangeTokenWalletList.exists(root_address_hex)){
                exchangeTokenWalletList.add(root_address_hex,tooken_wallet_address_hex);
            }
        }

    } 

    function getSupportTokenByRootHexAddres(uint256 root_address_hex)
        public
        view
        override
        functionID(0x51) 
        returns(uint256) 
    {
        uint256 token_address= uint256(0);
        require(root_address_hex!=0, MISS_QUERY_ROOT_ADDRESS_ERROR);
        if(exchangeTokenWalletList.exists(root_address_hex)){
            optional(uint256) info = exchangeTokenWalletList.fetch(root_address_hex);
            if (info.hasValue()) {
               // token_address=info;
            }
        }


        return token_address;
    }

    //-------------------------------Customer manager------------------------------------------------------------------------------------------
	
       
    
    // function updateNewToken(uint256 root_address,uint256 token_wallet_address)
    //     public
    //     AlwaysAccept
    //     functionID(0x30)
    // {
    //     if(exchangeTokenWalletList.exists(root_address)){

    //     }
    // }
    //Customer call the method first, exchange will send a request to target wallet.
    function sendUpdateRquestToWallet(address token_wallet)
        public
        functionID(0x31)
    {

    }

    function queryWalletKey()
        public
        functionID(0x32)
    {

    }
   
    
    //Exchange inital 
	function deployTipWalletByWalletAddress(address wallet_address_,uint128 grams_)
		public
		functionID(0x10)
	{
		 //ITONTokenWallet(wallet_address_).getRootAddress{value: grams_, callback: deployTipWalletByRootWalletAddress}();
	}
	function deployTipWalletByRootWalletAddress()
		public
		functionID(0x11)
	{
		tvm.accept(); 
		 // Calculate expiration timestamp.
		//uint32 expiretime = now + 60000;
		optional(uint) pubkey;
        pubkey.set(tvm.pubkey());
		
		//tvm.sendrawmsg(m_cell,1);
		 //address addr = address.makeAddrStd(0, 0x0123456789012345678901234567890123456789012345678901234567890123);
        //TvmCell m_cell = tvm.buildExtMsg({abiVer: 1, callbackId: 0, onErrorId: 0, dest: addr, time: 0x123, expire: 0x12345, call: {IRootTokenContract.deployEmptyWallet, uint32(0),int8(0),publickey,internal_owner,grams_value_}});
		//IRootTokenContract(root_address).deployEmptyWallet{value:200000000,callback:handelDeployEmptyWallteByRootaddress}(uint32(0),int8(0),publickey,internal_owner,grams_value_);
		counter++;
	}
	function deployTipWalletByRootWalletAddress2(address root_address,uint256 publickey,uint256 internal_owner,uint128 grams_value_)
		public
		view
		functionID(0x18)
	{
		tvm.accept();
		optional(uint) pubkey;
        pubkey.set(tvm.pubkey());
		uint32 expiretime = now + 60000;
		TvmCell m_cell= tvm.buildExtMsg({abiVer: 2, callbackId:0x12, onErrorId: 0x13, dest: root_address, time: now, expire: expiretime, call: {IRootTokenContract.deployEmptyWallet, uint32(0),int8(0),publickey,internal_owner,grams_value_}, pubkey: pubkey, sign: true});
		root_address.transfer({value:200000000,body:m_cell});

	}	
	//handel the deploy
	function handelDeployEmptyWallteByRootaddress(address walletress)
		public
		functionID(0x12)
	{
		tvm.accept();
		roor_return_address=walletress;
		
	}
	function deployWalletError()
		internal
		functionID(0x13)
	{

	}
	//approve for traget wallet
	function approveWallet(address dest_wallet_,address spender_address_,uint128 tokens,uint128 grams_value_)
		public
		pure
		AlwaysAccept
		functionID(0x14)
	{
		ITONTokenWallet(dest_wallet_).approve{value:grams_value_}(spender_address_,tokens);
	}
	function setGiver(address giver_address,uint256 value,uint128 grams_value_)
		public 
		pure 
		AlwaysAccept
		functionID(0x20)
	{
		IGiver(giver_address).give{value:grams_value_}(value);
	}	

	function approveWalletExt(address dest_wallet_,address spender_address_,uint128 tokens)
		public
		view
		AlwaysAccept
		functionID(0x15)
	{
		uint32 expiretime = now + 60000;
		optional(uint) pubkey;
        pubkey.set(tvm.pubkey());
		ITONTokenWallet(dest_wallet_).approve{extMsg: true,abiVer:2,callbackId:0,onErrorId:0, expire: expiretime, time: now, pubkey: pubkey, sign: true}(spender_address_,tokens);
		//ITONTokenWallet(dest_wallet_).approve{value:grams_value_}(spender_address_,tokens);
	}
	function approveWalletExtError()
		public
		pure
		AlwaysAccept
		functionID(0x19)
	{
		//ITONTokenWallet(dest_wallet_).approve{value:grams_value_}(spender_address_,tokens);
	}

	function getTvmpublic()
		public
		view
        override
        functionID(0x52) 
		returns(uint256) 
	{
		return tvm.pubkey();
	}

	function getName(address root_address)
		public
		AlwaysAccept
		functionID(0x17)
	{
		uint32 expiretime = now + 60000;
		optional(uint) pubkey;
        pubkey.set(tvm.pubkey());
		IRootTokenContract(root_address).getName{
			extMsg: true,abiVer: 2, expire: expiretime, time: now, pubkey: pubkey, sign: true,callbackId:0x17,onErrorId:0}();
		//IRootTokenContract(root_address).getName{callback:setName}();
		counter++;
	}
	function setName(bytes value0)
		public
		AlwaysAccept
		functionID(0x18) 
	{
		name=value0;
		tvm.commit();
	}

	function showName()
		public
		view
		returns(bytes)
	{
		return name;
	}

	function getReturnAddress()
		public
		view
		returns(address)
	{
		return roor_return_address;
	}

	function addNewTokenToDatabase()
		public
	{

	}



	//Exchange function

	function putOrder(address sellTokenWalletAddress, uint128 sellTokenNumber,address buyTokenWalletAddress,uint128 buyTokenNumber) 
        public pure  
    {
	
	}

    function cancleOrder(uint128 orderId)
         public pure 
    {

    }

    function executeOrder(uint128 orderId)
		public
    {
        
    }

	function putTestDate(uint256 exchangeWallet1,uint256 exchangeWallet2)
	 public
	 AlwaysAccept
	{
	support_token supporttoen1=support_token(exchangeWallet1,"0");
    support_token supporttoen2=support_token(exchangeWallet2,"0");

    support_token_list.add(0xcbb30ce4991335ab7f7698d1339c13387471e3626541c66b0428413998339ed7,supporttoen1);
    support_token_list.add(0x93caf629948c3c0c73f93f9381e52bcbca5b24ac395d7c70559f9ddd55bc6614,supporttoen2);
    
	} 


	function getAllSupportTokens() 
		public 
		view
	returns (support_token[] value0)
	{
	
		for((,support_token value):support_token_list){
			value0.push(value);
		
		}
		
	}	

    //-----------------------------------------------
     fallback() external {
         
     }
}