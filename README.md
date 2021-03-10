# Ton Exchange Install

## 1. TIP3 improve and some limit 
* Improve the (approve) and (allowance) function to support multiple address for fungible and nonfugible tokens. 
* Add some exchange function , let customer can work with exchange from the wallet.
* Add deployEmptyWallet to nonfugible Root contract .
> Note: Don't deploy the TIP3 wallet by tonos-cli , only deploy it by the deployEmptyWallet or deployWallet function from the root contract.because ,According to TIP3's address calculation mechanism , when it try to get the expected address ,it must need the initial parameters input ,for example : token name, token symbol,token decimals etc. but when you work with tonos-cli command ,the command should be(/tonos-cli genaddr -v --genkey ${.key.json} {.tvc} ${.abi.json}) , it must can't pass these initial parameters, so in last you will get a different address, then you can success deploy the wallet contract to online ,but must can't send token to other one .

## 2. Deploy TIP3 token root contract
### 2.1 Deploy TIP3 fungible and nonfugible wotk follow this url:
https://github.com/tonlabs/ton-labs-contracts/tree/master/cpp/tokens-fungible
The fungible and nonfugilbe work .
> Note: It only tested with Ton SE node, it maybe get hang when you want deploy it net.ton.dev, it look like the a large string as input parameter in the deploy will cause hang and always get the 507 error return.

## 3. Call  deployEmptyWallet or deployWallet method from root contract to deploy wallet contract.

## 4. Deploy TonExchange contract by ./deploy.sh
Before you start deploy the TonExchange contract you should setting some environment variable:
```
export CLI_PATH=./tonos-cli/target/release
export DEPLOY_LOCAL="1"
export LOCAL_GIVER_PATH=../
export TVM_INCLUDE_PATH=../../TON-Compiler/llvm/projects/ton-compiler/cpp-sdk
```
Make sure above these path your local path.