#pragma once

// Numeric Limits
static constexpr int64_t MAX_ASSET_AMOUNT = std::numeric_limits<int64_t>::max();

// Contract names
static constexpr eosio::name WAX_CONTRACT = "eosio.token"_n;
static constexpr eosio::name WAXDAO_CONTRACT = "tf.waxdao"_n;

// Symbols
static constexpr eosio::symbol WAX_SYMBOL = eosio::symbol("WAX", 8);

// Error Messages
static const char* ERR_FARM_NOT_FOUND = "this farm does not exist";

//Scaling factors
const uint128_t TWO_POW_64 = uint128_t(1) << 64;
static constexpr uint128_t SCALE_FACTOR_1E6 = 1000000;
static constexpr uint128_t SCALE_FACTOR_1E8 = 100000000;
static constexpr uint128_t SCALE_FACTOR_1E16 = 10000000000000000;