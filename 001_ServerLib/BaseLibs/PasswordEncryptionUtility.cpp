
#include "lib_md5.hpp"
#include "LogAdapter.hpp"
#include "StringUtility.hpp"

#include "PasswordEncryptionUtility.hpp"

using namespace ServerLib;

#define MD5_SIZE 16

//对密码进行加密，生成密码的密文
int CPasswordEncryptionUtility::DoPasswordEncryption(const char* pszPasswd, int iPasswdLen, char* pszOutBuff, int& iOutBuffLen)
{
    if(!pszPasswd || !pszOutBuff || iPasswdLen<=0 || iOutBuffLen<=(MD5_SIZE*2+1))
    {
        TRACESVR("Failed to generate session key, invalid param!");
        return -1;
    }

    unsigned char uMD5Value[MD5_SIZE] = {0};

    MD5Context stContext;
    MD5Init(&stContext);
    MD5Update(&stContext, (const unsigned char*)pszPasswd, iPasswdLen);
    MD5Final(uMD5Value, &stContext);

    for(int i=0; i<MD5_SIZE; ++i)
    {
        SAFE_SPRINTF(pszOutBuff+2*i, iOutBuffLen, "%02x", uMD5Value[i]);
    }

    pszOutBuff[MD5_SIZE*2] = '\0';

    return 0;
}

