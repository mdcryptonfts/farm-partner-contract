#pragma once

struct farm_struct {
	eosio::name  			farm_name;
	eosio::name 			creator;

	farm_struct(const farms& f)
		: farm_name(f.farm_name),
		  creator(f.creator) {}

	farm_struct() = default;

	// Custom serialization is necessary to pack/unpack the farm_struct when passing
	// to certain actions
	template<typename DataStream>
	friend DataStream& operator << (DataStream& ds, const farm_struct& obj) {
		ds << obj.farm_name
		   << obj.creator;
		return ds;
	}

	template<typename DataStream>
	friend DataStream& operator >> (DataStream& ds, farm_struct& obj) {
		ds >> obj.farm_name
		   >> obj.creator;
		return ds;
	}
};