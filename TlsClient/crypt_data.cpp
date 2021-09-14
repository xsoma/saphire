#include "crypt_data.h"
#include <cassert>


#define ASSERT assert
//using std::unique_ptr;
//using BN_ptr = std::unique_ptr<BIGNUM, decltype(&::BN_free)>;
//using RSA_ptr = std::unique_ptr<RSA, decltype(&::RSA_free)>;
//using EVP_KEY_ptr = std::unique_ptr<EVP_PKEY, decltype(&::EVP_PKEY_free)>;
//using BIO_FILE_ptr = std::unique_ptr<BIO, decltype(&::BIO_free)>;

key get_token()
{


	//int rc;
	//RSA_ptr rsa(RSA_new(), ::RSA_free);

	//BN_ptr bn(BN_new(), ::BN_free);
	//rc = BN_set_word(bn.get(), RSA_F4);
	//ASSERT(rc == 1);


	//rc = RSA_generate_key_ex(rsa.get(), 1024, bn.get(), NULL);


	//ASSERT(rc == 1);

	//EVP_KEY_ptr pkey(EVP_PKEY_new(), ::EVP_PKEY_free);
	//rc = EVP_PKEY_set1_RSA(pkey.get(), rsa.get());
	//ASSERT(rc == 1);

	//size_t pri_len;
	//size_t pub_len;
	//char* pri_key;
	//char* pub_key;

	//BIO* pri = BIO_new(BIO_s_mem());
	//BIO* pub = BIO_new(BIO_s_mem());
	//PEM_write_bio_PUBKEY(pub, pkey.get());
	//PEM_write_bio_PKCS8PrivateKey(pri, pkey.get(), NULL, NULL, 0, NULL, NULL);

	//pri_len = BIO_pending(pri);
	//pub_len = BIO_pending(pub);
	//pri_key = (char*)malloc(pri_len + 1);
	//pub_key = (char*)malloc(pub_len + 1);

	//BIO_read(pri, pri_key, pri_len);
	//BIO_read(pub, pub_key, pub_len);
	//pri_key[pri_len] = '\0';
	//pub_key[pub_len] = '\0';

	//key m_key;
	//m_key.priv = pri_key;
	//m_key.pub = pub_key;

	//BIO_free_all(pub);
	//BIO_free_all(pri);
	//free(pri_key);
	//free(pub_key);
	//return m_key;
}

