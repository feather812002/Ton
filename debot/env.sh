#!/bin/bash
set -e
export CLI_PATH=./tonos-cli/target/release
export DEPLOY_LOCAL="1"
export LOCAL_GIVER_PATH=../..
export TVM_INCLUDE_PATH=../../TON-Compiler/llvm/projects/ton-compiler/cpp-sdk