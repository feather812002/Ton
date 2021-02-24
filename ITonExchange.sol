pragma ton-solidity >= 0.35.0;
pragma AbiHeader expire;

interface ITonExchange {
   
    function regNewToken(uint256 token_wallet_address) 	functionID(0x50) external ;
    function getSupportTokenByRootHexAddres(uint256 root_address_hex) functionID(0x51)  view external returns(uint256);
    function getTvmpublic() functionID(0x52)  view external returns(uint256);
}