#include "partners.hpp"
#include "functions.cpp"
#include "on_notify.cpp"

//contractName: partners

/**
 * Allows `creator` to make a new farm, assuming they have enough points in the `points` table
 * 
 * @param creator - the wax address of the user creating the farm
 * @param farm_name - the name the user wishes to give to their farm
 * @param staking_token - the symbol and contract of the token to stake in the farm. See docs for extended_symbol formatting
 * @param vesting_time - the amount of seconds that the creator wants stakers to lock tokens in the farm for. Resets for user during each new stake.
 */

ACTION partners::createfarm(const name& creator, const name& farm_name, const extended_symbol& staking_token,
    const uint64_t& vesting_time)
{
	require_auth( creator );
	remove_point( creator );
	action( active_perm(), WAXDAO_CONTRACT, "createfarm"_n, std::tuple{ _self, farm_name, staking_token, vesting_time } ).send();
}

ACTION partners::init(){
	require_auth( _self );
	state s{};
	s.accepted_tokens = { {WAX_SYMBOL, WAX_CONTRACT} };
	s.partner_fee_1e6 = 10000000;   // 10%
	state_s.set(s, _self);
}

ACTION partners::rempaymethod(const extended_symbol& payment_method){
	require_auth( _self );

	state s = state_s.get();

	auto itr = std::remove( s.accepted_tokens.begin(), s.accepted_tokens.end(), payment_method );
	check( itr != s.accepted_tokens.end(), "this action would result in no change" );
	
	s.accepted_tokens.erase( itr, s.accepted_tokens.end() );
	state_s.set(s, _self);
}

ACTION partners::setfee(const uint64_t& partner_fee_1e6){
	require_auth( _self );
	check( partner_fee_1e6 >= 0 && partner_fee_1e6 <= SCALE_FACTOR_1E8, "fee must be 0 to 100%" );

	state s = state_s.get();
	check( partner_fee_1e6 != s.partner_fee_1e6, "this action would result in no change" );
	s.partner_fee_1e6 = partner_fee_1e6;
	state_s.set(s, _self);
}

ACTION partners::setpaymethod(const extended_symbol& payment_method){
	require_auth( _self );
	check( token_exists( payment_method ), "this token does not exist" );

	state s = state_s.get();
	check( std::find( s.accepted_tokens.begin(), s.accepted_tokens.end(), payment_method ) == s.accepted_tokens.end(), "this action would result in no change" );
	s.accepted_tokens.push_back( payment_method );
	state_s.set(s, _self);
}