#include "LogAdapter.hpp"

#include "AESCbcEncryptor.hpp"

using namespace ServerLib;

CAESCbcEncryptor::CAESCbcEncryptor()
{
	m_pstEncryptor = NULL;
	m_pstDecryptor = NULL;
}

CAESCbcEncryptor::~CAESCbcEncryptor()
{
	if (m_pstEncryptor)
	{
		delete m_pstEncryptor;
		m_pstEncryptor = NULL;
	}

	if (m_pstDecryptor)
	{
		delete m_pstDecryptor;
		m_pstDecryptor = NULL;
	}
}

int CAESCbcEncryptor::SetAESKey(const char* pszAESKey, int iAESKeyLen)
{
	if (!pszAESKey)
	{
		TRACESVR("Failed to set AES key, invalid key param, key len %u\n", iAESKeyLen);
		return -1;
	}

	HashFunction* hash = get_hash("MD5"); 				//MD5算法可以将任意长度的字节串转换为128位长度的字节串  
	SymmetricKey key = hash->process(std::string(pszAESKey, iAESKeyLen)); 		//产生128位的字节串作为密钥  
	SecureVector<byte> raw_iv = hash->process('0' + std::string(pszAESKey, iAESKeyLen)); //字符串相加,然后对结果做MD5,生成128位的字节串  
	InitializationVector iv(raw_iv, 16); //初始化向量  

										 //AES-128是算法名称,分块大小是128bit; CBC是算法模式.  
										 //AES算法密钥和分块大小可以是128,192,256位.  
										 //AES算法模式还包括: ECB,CFB,OFB,CTR等.  
	m_pstEncryptor = new Pipe(get_cipher("AES-128/CBC", key, iv, ENCRYPTION));

	m_pstDecryptor = new Pipe(get_cipher("AES-128/CBC", key, iv, DECRYPTION));

	return 0;
}

std::string CAESCbcEncryptor::DoAESCbcEncryption(const std::string& strInput)
{
	if (!m_pstEncryptor || strInput.size() == 0)
	{
		return "";
	}

	try
	{
		m_pstEncryptor->process_msg(strInput); //encryption
	}
	catch (Botan::Decoding_Error &e)
	{
		//加密失败，抛出一个Botan::Decoding_Error类型的异常  
		TRACESVR("Failed to do botan encryption, e: %s\n", e.what());

		return "";
	}

	return m_pstEncryptor->read_all_as_string();
}

std::string CAESCbcEncryptor::DoAESCbcDecryption(const std::string& strInput)
{
	if (!m_pstDecryptor || strInput.size() == 0)
	{
		return "";
	}

	try
	{
		m_pstDecryptor->process_msg(strInput); //encryption
	}
	catch (Botan::Decoding_Error &e)
	{
		//解密失败，抛出一个Botan::Decoding_Error类型的异常  
		TRACESVR("Failed to do botan decryption, e: %s\n", e.what());

		return "";
	}

	return m_pstDecryptor->read_all_as_string();
}
