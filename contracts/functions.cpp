#pragma once

inline eosio::permission_level partners::active_perm(){
    return eosio::permission_level{ _self, "active"_n };
}

void partners::add_point(const name& user){
  auto itr = points_t.find( user.value );

  if( itr == points_t.end() ){
    points_t.emplace(_self, [&](auto &_p){
      _p.wallet = user;
      _p.points_balance = 1;
    });    
  } else {
    points_t.modify(itr, same_payer, [&](auto &_p){
      _p.points_balance = safecast::add( _p.points_balance, uint8_t(1) );
    });
  }
}

int64_t partners::calculate_asset_share(const int64_t& quantity, const uint64_t& percentage){

  if (quantity <= 0) return 0;

  return mulDiv( uint64_t(quantity), percentage, SCALE_FACTOR_1E8 );
}

int64_t partners::get_expected_quantity(waxdao::global& g, const uint64_t& pool_id, const int64_t& expected_quantity){
  uint32_t oldest_observation = AlcorPriceOracle::getOldestObservationSecondsAgo( pool_id );
  uint128_t twapX64 = AlcorPriceOracle::getPriceTwapX64( pool_id, std::min( g.observation_seconds, oldest_observation ) ); 
  uint128_t price; 

  auto pool_itr = AlcorPriceOracle::pools_.require_find( pool_id, "pool id does not exist" );

  if( is_wax( pool_itr->tokenA.contract, pool_itr->tokenA.quantity ) ){
    price = FullMath::mulDiv( twapX64, uint128_t(expected_quantity), TWO_POW_64 );
  } else {
    uint128_t P_token_A = FullMath::mulDiv( twapX64, SCALE_FACTOR_1E8, TWO_POW_64 );
    price = FullMath::mulDiv( uint128_t(expected_quantity), SCALE_FACTOR_1E8, P_token_A );  
  }  

  return safecast::safe_cast<int64_t>(price);
}

inline farm_struct partners::get_farm(const name& farm_name) {
    auto farm_itr = farms_t.require_find( farm_name.value, ERR_FARM_NOT_FOUND );
    farm_struct f = farm_struct(*farm_itr);
    return f;
}

asset partners::get_farm_price(const name& partner, const extended_symbol& payment_token){
    waxdao::global g = global_s.get(); 

    asset expected_quantity = g.farm_price_in_wax;
    uint64_t discount = 0;

    auto itr = partners_t.find( partner.value );
    if( itr != partners_t.end() ){
        discount += itr->discount_1e6;
    }

    if( !is_wax( payment_token.get_contract(), asset( 0, payment_token.get_symbol() ) ) ){
        auto payments_secondary = payments_t.get_index<"symcontract"_n>();
        uint128_t sym_contract_key = mix64to128( payment_token.get_symbol().code().raw(), payment_token.get_contract().value );
        auto pay_itr = payments_secondary.require_find( sym_contract_key, "unsupported payment method" );
        discount += pay_itr->discount_1e6;
        expected_quantity.amount = get_expected_quantity( g, pay_itr->alcor_pool_id, expected_quantity.amount );
    }

    if( discount > 0 ){
        check( discount <= SCALE_FACTOR_1E8, "discount can not be > 100%" );
        expected_quantity.amount -= calculate_asset_share( expected_quantity.amount, discount );
    }

    return asset( expected_quantity.amount, payment_token.get_symbol() );
}

bool partners::is_wax(const name& contract, const asset& quantity)
{
  if(contract == WAX_CONTRACT && quantity.symbol == WAX_SYMBOL) return true;

  return false;
}

int64_t partners::mulDiv(const uint64_t& a, const uint64_t& b, const uint128_t& denominator){
  uint128_t prod = safecast::mul(uint128_t(a), uint128_t(b));
  uint128_t result = safecast::div(prod, denominator);

  return safecast::safe_cast<int64_t>(result);
}

std::vector<std::string> partners::parse_memo(std::string memo){
  std::string delim = "|";
  std::vector<std::string> words{};
  size_t pos = 0;
  while ((pos = memo.find(delim)) != std::string::npos) {
    words.push_back(memo.substr(0, pos));
    memo.erase(0, pos + delim.length());
  }
  return words;
}

void partners::remove_point(const name& user){
  auto itr = points_t.require_find( user.value, "you don't have a points balance" );
  check( itr->points_balance > 0, "insufficient points balance" );

  points_t.modify(itr, same_payer, [&](auto &_p){
    _p.points_balance -= 1;
  });
}

bool partners::token_exists(const extended_symbol& token){
  stat_table stat_t = stat_table( token.get_contract(), token.get_symbol().code().raw() );
  auto itr = stat_t.find( token.get_symbol().code().raw() );
  if( itr != stat_t.end() && itr->max_supply.symbol.precision() == token.get_symbol().precision() ){
    return true;
  }
  return false;
}

void partners::transfer_tokens(const name& user, const asset& amount_to_send, const name& contract, const std::string& memo){
  action( active_perm(), contract, "transfer"_n, std::tuple{ _self, user, amount_to_send, memo } ).send();
}