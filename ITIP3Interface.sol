pragma ton-solidity >= 0.35.0;
pragma AbiHeader expire;


interface IRootTokenContract {
	function deployWallet(uint32 _answer_id,int8 workchain_id,uint256 pubkey,uint256 internal_owner,uint256 tokens,uint128 grams ) external;
    function deployEmptyWallet(uint32 _answer_id,int8 workchain_id,uint256 pubkey,uint256 internal_owner,uint128 grams ) external returns(address root_address_);
    function grant(address dest,uint128 tokens,uint128 grams ) external;
    function getWalletCode() external;
    function getName() external returns(bytes value0) ;
    function getSymbol() external;
}
interface ITONTokenWallet {
    function transfer(address dest,uint128 tokens,uint128 grams) external;
    function getName(address dest,uint128 tokens,uint128 grams) external;
    function getRootAddress() external;
    function approve(address spender ,uint128 tokens) external;
    function allowance (uint256 spenderAddressHex) external returns (uint128 tokens);
    function getWalletKey () external returns (uint128 tokens);
}

interface IGiver{
    function give(uint value) external;
    function getAddr() external;
}
