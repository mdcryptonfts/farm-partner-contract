#include "partners.hpp"
#include "functions.cpp"
#include "on_notify.cpp"

//contractName: partners

/**
 * Allows farm creator to add a new reward
 * 
 * @param farm_name - the name of the farm to add the reward to
 * @param start_now - `true` if the new period should start instantly. `false` if the creator wants to set a custom `start_time`
 * @param start_time - epoch timestamp to start the new reward period. irrelevant if `true` is passed to `start_now`
 * @param duration - seconds that the reward period will last. minimum 86400 (1 day), maximum 86400*30 (30 days)
 * @param reward_token - symbol and contract for the token that will be distributed as rewards
 */

ACTION partners::addreward(const name& farm_name, const bool& start_now, const uint64_t& start_time, const uint64_t& duration,
	const extended_symbol& reward_token)
{
	farm_struct farm = get_farm( farm_name );
	require_auth( farm.creator );
	action( active_perm(), WAXDAO_CONTRACT, "addreward"_n, std::tuple{ farm_name, start_now, start_time, duration, reward_token } ).send();
}

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

/**
 * Allows farm creator to extend a reward period after the existing period has finished
 * 
 * @param farm_name - the name of the farm that the reward is linked to
 * @param reward_id - the ID of the reward to extend
 * @param start_now - `true` if the new period should start instantly. `false` if the creator wants to set a custom `start_time`
 * @param start_time - epoch timestamp to start the new reward period. irrelevant if `true` is passed to `start_now`
 * @param duration - seconds that the reward period will last. minimum 86400 (1 day), maximum 86400*30 (30 days)
 */

ACTION partners::extendreward(const name& farm_name, const uint64_t& reward_id, const bool& start_now, const uint64_t& start_time, const uint64_t& duration)
{
	farm_struct farm = get_farm( farm_name );
	require_auth( farm.creator );
	action( active_perm(), WAXDAO_CONTRACT, "extendreward"_n, std::tuple{ farm_name, reward_id, start_now, start_time, duration } ).send();
}

ACTION partners::init(){
	require_auth( _self );
	state s{};
	s.accepted_tokens = { {WAX_SYMBOL, WAX_CONTRACT} };
	s.partner_fee_1e6 = 10000000;   // 10%
	s.redirect_fees = false;
	s.fee_wallet = _self;
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

ACTION partners::setfeewallet(const name& fee_wallet){
	require_auth( _self );
	check( is_account( fee_wallet ), "fee_wallet is not an account" );

	state s = state_s.get();
	check( fee_wallet != s.fee_wallet, "this action would result in no change" );
	s.fee_wallet = fee_wallet;
	state_s.set(s, _self);
}

/** 
 * Allows this contract to add a new payment method to accept for farm payments
 * 
 * @param payment_method - the symbol and contract of the token to accept
 */

ACTION partners::setpaymethod(const extended_symbol& payment_method){
	require_auth( _self );
	check( token_exists( payment_method ), "this token does not exist" );

	state s = state_s.get();
	check( std::find( s.accepted_tokens.begin(), s.accepted_tokens.end(), payment_method ) == s.accepted_tokens.end(), "this action would result in no change" );
	s.accepted_tokens.push_back( payment_method );
	state_s.set(s, _self);
}

/** 
 * Allows the farm creator to withdraw unused rewards
 * 
 * The only situation where this is applicable is when 0 people were staking any tokens
 * at the end of the reward period. Otherwise, reward rate gets recalculated when someone stakes,
 * and the full reward pool is distributed by the last block of the reward period.
 * 
 * @param farm_name - the name of the farm to withdraw unused rewards from
 */

ACTION partners::withdraw(const name& farm_name)
{
	farm_struct farm = get_farm( farm_name );
	require_auth( farm.creator );
	action( active_perm(), WAXDAO_CONTRACT, "withdraw"_n, std::tuple{ farm_name } ).send();
}