#pragma once

#include <iostream>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/ossl_typ.h>

class TlsClient
{
private:
	int OpenConnection(const char* hostname, int port);
	SSL_CTX* InitCTX(void);
	bool ShowCerts(SSL* ssl);
private:
	SSL_CTX* ctx;
	int server;
	SSL* ssl;
	std::string m_host, m_port;
public:
	TlsClient(std::string hostname, std::string port);
	bool connect();
	bool is_send;
	std::string send(std::string data);
};