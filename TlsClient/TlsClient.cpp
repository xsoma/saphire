//#include <WinSock2.h>
//#include "TlsClient.h"
//#include <skCrypter.h>
//#include <lazy_importer.hpp>
//#include <vector>
//
//#define FAIL    -1
//using namespace std;
//
//TlsClient::TlsClient(string hostname, string port)
//{
//	is_send = false;
//	m_host = hostname;
//	m_port = port;
//}
//
//
//int TlsClient::OpenConnection(const char* hostname, int port)
//{
//	int sd;
//	struct hostent* host;
//	struct sockaddr_in addr;
//
//	if ((host = gethostbyname(hostname)) == NULL)
//	{
//		//printf("Eroor: %s\n", hostname);
//		IFH(perror)(hostname);
//		IFH(abort)();
//	}
//	sd = (socket)(PF_INET, SOCK_STREAM, 0);
//	memset(&addr, 0, sizeof(addr));
//	addr.sin_family = AF_INET;
//	addr.sin_port = htons(port);
//	addr.sin_addr.s_addr = *(long*)(host->h_addr);
//	if (::connect(sd, (struct sockaddr*)&addr, sizeof(addr)) != 0)
//	{
//		IFH(closesocket)(sd);
//		IFH(perror)(hostname);
//		//abort();
//	}
//
//	return sd;
//}
//
//SSL_CTX* TlsClient::InitCTX(void)
//{
//	
//	//SSL_METHOD* method;
//	SSL_CTX* ctx;
//	OpenSSL_add_all_algorithms();
//
//	SSL_load_error_strings();   /* Bring in and register error messages */
//
//	auto method = SSLv23_client_method();  /* Create new client-method instance */
//
//
//	ctx = SSL_CTX_new(method);   /* Create new context */
//
//	if (ctx == NULL)
//	{
//		ERR_print_errors_fp(stderr);
//		//printf("Eroor: %s\n", stderr);
//		abort();
//	}
//
//
//	return ctx;
//}
//bool TlsClient::ShowCerts(SSL* ssl)
//{
//	X509* cert;
//	char* line;
//
//	cert = SSL_get_peer_certificate(ssl); /* get the server's certificate */
//	if (cert != NULL)
//	{
//
//		if (std::string(X509_NAME_oneline(X509_get_subject_name(cert), 0, 0)) != skCrypt("/C=RU/ST=Saint Petersburg/L=Saint Petersburg/O=lw/OU=lw/CN=lw/emailAddress=dmitrijminin5@gmail.com").decrypt())
//		{
//			return false;
//		}
//
//
//		if (std::string(X509_NAME_oneline(X509_get_issuer_name(cert), 0, 0)) != skCrypt("/C=RU/ST=Saint Petersburg/L=Saint Petersburg/O=lw/OU=lw/CN=lw/emailAddress=dmitrijminin5@gmail.com").decrypt())
//		{
//			return false;
//		}
//
//		X509_free(cert);
//	}
//
//	return true;
//
//}
//bool TlsClient::connect()
//{
//
//	is_send = true;
//
//#if OPENSSL_VERSION_NUMBER < 0x10100000L
//	SSL_library_init();
//#else
//	if (!OPENSSL_init_ssl(0, NULL)) {
//
//	}
//#endif
//
//
//	ctx = InitCTX();
//
//	server = OpenConnection(m_host.c_str(), atoi(m_port.c_str()));
//
//	ssl = SSL_new(ctx);
//
//	SSL_set_fd(ssl, server);
//
//	if (SSL_connect(ssl) == FAIL)
//	{
//		//printf("Eroor: %s\n", stderr);
//		ERR_print_errors_fp(stderr);
//		return false;
//	}
//
//	return 1;
//}
//
//std::string TlsClient::send(string data)
//{
//	if (!ShowCerts(ssl))
//		return "";
//
//	SSL_write(ssl, data.c_str(), strlen(data.c_str()));
//	SSL_shutdown(ssl);
//
//	int received = 0;
//	int TotalReceived = 0;
//
//	BYTE* buffer = new  BYTE[10250];
//	std::vector<BYTE> out;
//
//	while (1)
//	{
//
//		received = SSL_read(ssl, buffer, 10239);
//
//		if (received > 0)
//		{
//			buffer[received] = '\0';
//			TotalReceived += received;
//			out.insert(out.end(), buffer, &buffer[received]);
//		}
//		else
//			break;
//	}
//
//	is_send = false;
//	delete[] buffer;
//	return std::string(out.begin(), out.end());
//}