#pragma once
#include <iostream>
#include <vector>


//using BN_ptr = std::unique_ptr<BIGNUM, decltype(&::BN_free)>;
//using RSA_ptr = std::unique_ptr<RSA, decltype(&::RSA_free)>;
struct key
{
	//std::string priv;
	//std::string pub;
	//std::string pub_s;
};
key get_token();
