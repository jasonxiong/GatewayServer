

#include "ConfigFile.hpp"



/*************************************************
  Function:     ConfigFile
  Description:
        ConfigFile的构造函数，用于初始化类的成员变量
*************************************************/

ConfigFile::ConfigFile()
{
    m_pszFilename = NULL;
    m_pszContent = NULL;
    m_pszShadow = NULL;
    m_nSize = 0;

    m_bIsOpened = 0;
}

/*************************************************
  Function:     ~ConfigFile
  Description:
        ConfigFile的析构函数释放申请的资源
*************************************************/
ConfigFile::~ConfigFile()
{
    CloseFile();
}


/*************************************************
  Function:     OpenFile
  Description:
        读取指定的配置文件。
        如果文件读取成功，设置m_bIsOpened为true
  Input:
        pszFilename     - 以NULL结尾的配置文件名
  Output:
  Return:   成功返回0, 失败返回非0
*************************************************/
int ConfigFile::OpenFile(const char* pszFilename)
{
    FILE    *fp;
    size_t  len;
    int     result;

    // 释放资源
    CloseFile();

    if(NULL == pszFilename)
    {
        return -1;
    }

    m_pszFilename = strdup(pszFilename);

    fp = fopen(m_pszFilename, "rb");

    if(NULL == fp)
    {
        return -2;
    }

    result = fseek(fp, 0L, SEEK_END);
    if(0 != result)
    {
        fclose(fp);
        return -3;
    }

    len = (size_t)ftell(fp);
    m_pszContent = (char* )new char [len+1];
    m_pszShadow = (char* )new char [len+1];

    if((NULL == m_pszContent) || (NULL == m_pszShadow))
    {
        fclose(fp);
        return -4;
    }

    result = fseek(fp, 0L, SEEK_SET);
    if(0 != result)
    {
        fclose(fp);
        return -5;
    }

    m_nSize = fread(m_pszContent, 1, len, fp);
    m_pszContent[m_nSize] = '\0';

    // 建立影子内存，里面存放全部是小写的字符串
    memcpy(m_pszShadow, m_pszContent, m_nSize + 1);
    MyToLower(m_pszShadow, m_nSize + 1);

    fclose(fp);

    m_bIsOpened = 1;

    return 0;
}


/*************************************************
  Function:     CloseFile
  Description:
        关闭配置文件。 释放分配资源。
  Input:
  Output:
  Return:
*************************************************/
void ConfigFile::CloseFile()
{
    // 释放资源
    if(m_pszFilename)
    {
        free(m_pszFilename);        //strdup创建
        m_pszFilename = NULL;
    }

    if(m_pszContent)
    {
        delete [] m_pszContent;
        m_pszContent = NULL;
    }

    if(m_pszShadow)
    {
        delete [] m_pszShadow;
        m_pszShadow = NULL;
    }

    m_nSize = 0;
    m_bIsOpened = 0;
}



/*************************************************
  Function:     IsOpened
  Description:
        返回读取配置文件是否成功的标志
  Input:
  Output:
  Return:       如果配置文件读取成功，返回true，否则返回false
*************************************************/
unsigned int ConfigFile::IsOpened()
{
    return m_bIsOpened;
}


/*************************************************
  Function:     GetItemValue数字
  Description:
        从内存中取指定的数字类型的键值，如果不存在，则使用指定的缺省值
  Input:
        pszSectionName  - 以NULL结尾的字符串指针，指示包含Key的片断
        pszKeyName      - 以NULL结尾的字符串指针，指示需要返回值的Key
        lDefaultValue   - 取值失败后使用的缺省值
  Output:
        lReturnedValue  - 指定用于接收结果的缓冲区地址
  Return:       成功返回true, 失败返回false
  Others:       本函数不是UNICODE版本
*************************************************/
unsigned int ConfigFile::GetItemValue( const char* pszSectionName,
                                       const char* pszKeyName,
                                       int &lReturnedValue,
                                       int lDefaultValue )
{
    if(0 == GetItemValue(pszSectionName, pszKeyName, lReturnedValue))
    {
        lReturnedValue = lDefaultValue;
        return 0;
    }

    return 1;
}


unsigned int ConfigFile::GetItemValue( const char* pszSectionName,
                                       const char* pszKeyName,
                                       int &lReturnedValue )
{
    char szBuf[100];

    if(0 == GetItemValue(pszSectionName, pszKeyName, szBuf, 100))
    {
        return 0;
    }

    lReturnedValue = atol(szBuf);

    return 1;
}


/*************************************************
  Function:     GetItemValue数字
  Description:
        从内存中取指定的数字类型的键值，如果不存在，则使用指定的缺省值
  Input:
        pszSectionName  - 以NULL结尾的字符串指针，指示包含Key的片断
        pszKeyName      - 以NULL结尾的字符串指针，指示需要返回值的Key
        lDefaultValue   - 取值失败后使用的缺省值
  Output:
        lReturnedValue  - 指定用于接收结果的缓冲区地址
  Return:       成功返回true, 失败返回false
  Others:       本函数不是UNICODE版本
*************************************************/
unsigned int ConfigFile::GetItemValue( const char* pszSectionName,
                                       const char* pszKeyName,
                                       int64_t &lReturnedValue,
                                       int64_t lDefaultValue )
{
    if(0 == GetItemValue(pszSectionName, pszKeyName, lReturnedValue))
    {
        lReturnedValue = lDefaultValue;
        return 0;
    }

    return 1;
}

unsigned int ConfigFile::GetItemValue( const char* pszSectionName,
                                       const char* pszKeyName,
                                       int64_t &lReturnedValue )
{
    char szBuf[100];

    if(0 == GetItemValue(pszSectionName, pszKeyName, szBuf, 100))
    {
        return 0;
    }

    lReturnedValue = atoll(szBuf);

    return 1;
}



/*************************************************
  Function:     GetItemValue字符串
  Description:
        从内存中取指定的字符串类型的键值，如果不存在，则使用指定的缺省值
  Input:
        pszSectionName  - 以NULL结尾的字符串指针，指示包含Key的片断
        pszKeyName      - 以NULL结尾的字符串指针，指示需要返回值的Key
        nSize           - 指定接收缓冲区的大小
        pszDefaultValue - 取值失败后使用的缺省值
  Output:
        pszReturnedString - 指定用于接收结果的缓冲区地址
  Return:       返回缓冲区中的有效字符个数。不包括字符串结尾的NULL
  Others:       本函数不是UNICODE版本
*************************************************/
unsigned int ConfigFile::GetItemValue( const char* pszSectionName,
                                       const char* pszKeyName,
                                       char* pszReturnedString,
                                       unsigned int nSize,
                                       const char* pszDefaultValue )
{
    unsigned int len;

    if(nSize <= 0)
    {
        return 0;
    }

    len = GetItemValue(pszSectionName, pszKeyName, pszReturnedString, nSize);
    if(0 != len)
    {
        return len;
    }

    strncpy(pszReturnedString, pszDefaultValue, nSize-1);
    pszReturnedString[nSize-1] = '\0';
    return strlen(pszReturnedString);
}


/*************************************************
  Function:     GetItemValue字符串
  Description:
        从内存缓冲区中找到KeyName，将值拷贝到指定的空间。
        如果返回值大于空间的大小，则对字符串进行截尾处理，
        并在缓冲区的最后一个字节加上NULL。
        当缓冲区的最后两个字符是汉字编码时，将自动加上两个结束符
  Input:
        pszSectionName  - 以NULL结尾的字符串指针，指示包含Key的片断
        pszKeyName      - 以NULL结尾的字符串指针，指示需要返回值的Key
        nSize           - 指定接收缓冲区的大小
  Output:
        pszReturnedString - 指定用于接收结果的缓冲区地址
  Return:       返回缓冲区中的有效字符个数。不包括字符串结尾的NULL
  Others:       本函数不是UNICODE版本
*************************************************/
unsigned int ConfigFile::GetItemValue( const char* pszSectionName,
                                       const char* pszKeyName,
                                       char* pszReturnedString,
                                       unsigned int nSize )
{
    char* pszSectionBegin;
    char* pszSectionEnd;
    char* pszValueBegin;
    char* pszValueEnd;
    unsigned int dwCount;

    // 检查对象是否初始化成功
    if(0 == IsOpened())
    {
        return (unsigned int)0;
    }

    // 检查传入参数合法性
    if((NULL == pszSectionName)
            || (NULL == pszKeyName)
            || (NULL == pszReturnedString)
            || (0 == nSize))
    {
        return (unsigned int)0;
    }

    // 查找SectionName，定位Section的开始和结尾指针
    if(0 == LocateSection(pszSectionName, pszSectionBegin, pszSectionEnd))
    {
        return (unsigned int)0;
    }

    // 在指定范围内定位KeyName的Value
    if(0 == LocateValue( pszKeyName,
                         pszSectionBegin,
                         pszSectionEnd,
                         pszValueBegin,
                         pszValueEnd ))
    {
        return (unsigned int)0;
    }

    // 将需要的值拷贝到缓冲区中，并注意缓冲区长度
    dwCount = 0;

    for(; pszValueBegin < pszValueEnd && dwCount < (nSize-1); pszValueBegin++, dwCount++)
    {
        pszReturnedString[dwCount] = *pszValueBegin;
    }

    pszReturnedString[dwCount] = '\0';


    // 字符串被截断，判断，最后一个字符是否为双字节
    if((dwCount == nSize-1) && ((unsigned char)(pszReturnedString[dwCount-1]) > 0x7f))
    {
        // 将双字节的最后一个字符设置为'\0'
        // 为了防止出现如下情况，导致以后的字符串处理越界
        //     "\xa9"

        pszReturnedString[dwCount-1] = '\0';
        dwCount --;
    }

    return (unsigned int)dwCount;
}


/*************************************************
  Function:     SetItemValue数字
  Description:
        将指定的数字类型键值，并将结果同时更新内存和配置文件
  Input:
        pszSectionName  - 以NULL结尾的字符串指针，指示包含Key的片断
        pszKeyName      - 以NULL结尾的字符串指针，指示需要设置的Key
        ulKeyValue      - 数字类型，指示需要设置的值
  Output:
  Return:       返回是否成功的标志。成功，返回true；否则返回false
  Others:       本函数不是UNICODE版本
*************************************************/
unsigned int ConfigFile::SetItemValue( const char* pszSectionName,
                                       const char* pszKeyName,
                                       int lKeyValue )
{
    char szBuf[100];

    sprintf(szBuf, "%d", lKeyValue);

    return SetItemValue(pszSectionName, pszKeyName, szBuf);
}


/*************************************************
  Function:     SetItemValue字符串
  Description:
        从内存缓冲区中找到KeyName，将值拷贝到指定的空间，并更新配置文件。
        如果返回值大于空间的大小，则对字符串进行截尾处理，
        并在缓冲区的最后一个字节加上NULL。
        当缓冲区的最后两个字符是汉字编码时，将自动加上两个结束符
  Input:
        pszSectionName  - 以NULL结尾的字符串指针，指示包含Key的片断
        pszKeyName      - 以NULL结尾的字符串指针，指示需要设置的Key
        pszKeyValue     - 以NULL结尾的字符串指针，指示需要设置的值
  Output:
  Return:       返回是否成功的标志。成功，返回true；否则返回false
  Others:       本函数不是UNICODE版本
*************************************************/
unsigned int ConfigFile::SetItemValue( const char* pszSectionName,
                                       const char* pszKeyName,
                                       const char* pszKeyValue )
{
    char* pszSectionBegin;
    char* pszSectionEnd;
    char* pszKeyBegin;
    char* pszKeyEnd;
    char* pszContent;
    char* pszShadow;
    char* pszBuf;
    size_t len;

    // 检查对象是否初始化成功
    if(0 == IsOpened())
    {
        return 0;
    }

    // 检查传入参出合法性
    if((NULL == pszSectionName)
            || (NULL == pszKeyName)
            || (NULL == pszKeyValue))
    {
        return 0;
    }


    // 查找SectionName，定位Section的开始和结尾指针
    if(0 == LocateSection(pszSectionName, pszSectionBegin, pszSectionEnd))
    {
        return 0;
    }

    // 在指定范围内搜索KeyName
    if(0 == LocateKey( pszKeyName,
                       pszSectionBegin,
                       pszSectionEnd,
                       pszKeyBegin,
                       pszKeyEnd ))
    {
        // Key没找到，将pszKeyBegin和pszKeyEnd都定位到Section的开始

        pszKeyBegin = pszSectionBegin;
        pszKeyEnd = pszSectionBegin;
    }

    // 构造新内容的字符串
#ifdef WIN32
    len = strlen(pszKeyName) + strlen(pszKeyValue) + 5;
#else
    len = strlen(pszKeyName) + strlen(pszKeyValue) + 4;
#endif

    // 申请资源
    pszBuf = (char*)new char [len + 1];
    pszContent = (char*)new char [m_nSize - (pszKeyEnd - pszKeyBegin) + len + 1];
    pszShadow = (char*)new char [m_nSize - (pszKeyEnd - pszKeyBegin) + len + 1];

    if((NULL == pszBuf) || (NULL == pszContent) || (NULL == pszShadow))
    {
        if(pszBuf)
        {
            delete [] pszBuf;
        }

        if(pszContent)
        {
            delete [] pszContent;
        }

        if(pszShadow)
        {
            delete [] pszShadow;
        }

        return 0;
    }

    memset(pszBuf, 0, len + 1);
    memset(pszContent, 0, len + 1);
    memset(pszShadow, 0, len + 1);

#ifdef WIN32
    sprintf(pszBuf, "%s = %s\r\n", pszKeyName, pszKeyValue);
#else
    sprintf(pszBuf, "%s = %s\n", pszKeyName, pszKeyValue);
#endif

    // 用新的内容替换原有的内容
    memcpy( (void *)pszContent,
            (void *)m_pszContent,
            (size_t)(pszKeyBegin - m_pszContent) );
    memcpy( (void *)(pszContent + (pszKeyBegin - m_pszContent)),
            (void *)pszBuf,
            len );
    memcpy( (void *)(pszContent + (pszKeyBegin - m_pszContent) + len),
            (void *)pszKeyEnd,
            m_nSize - (pszKeyEnd - m_pszContent) + 1 );

    delete [] pszBuf;
    delete [] m_pszContent;
    delete [] m_pszShadow;

    m_nSize = m_nSize - (pszKeyEnd - pszKeyBegin) + len;

    // 更新文件内存映象
    m_pszContent = pszContent;

    // 更新影子内存，里面存放全部是小写的字符串
    m_pszShadow = pszShadow;
    memcpy(m_pszShadow, m_pszContent, m_nSize + 1);
    MyToLower(m_pszShadow, m_nSize + 1);

    // 更新文件内容
    FILE *fp;

    if(NULL == m_pszFilename)
    {
        return 0;
    }

    fp = fopen(m_pszFilename, "wb");

    if(NULL == fp)
    {
        return 0;
    }

    len = fwrite(m_pszContent, 1, m_nSize, fp);

    // 冗余检查代码，不用测试
    if((size_t)len != m_nSize)
    {
        // 如果实际写入的字节数和真正的长度不相符
        fclose(fp);

        return 0;
    }

    fclose(fp);

    return 1;
}


/*************************************************
  Function:     LocateSection
  Description:
        在指定的缓冲区范围中搜索Section，返回与Section的开始地址和结束地址。
  Input:
        pszSectionName  - 以NULL结尾的字符串指针，指示需要返回值的Section
  Output:
        pszSectionBegin - 接收返回值的开始地址
                          指向Section行的下一行第一个字节
        pszSectionEnd   - 接收返回值的结束地址
                          指向最后一个有效字节的下一个地址
  Return:       定位成功，返回true；失败，返回false
  Others:       本函数不是UNICODE版本
*************************************************/
unsigned int ConfigFile::LocateSection( const char* pszSectionName,
                                        char* &pszSectionBegin,
                                        char* &pszSectionEnd )
{
    char*   pszLowerSection;
    char*   pszSectionBeginOnShadow;
    unsigned int   bIsFirstValidCharOnLine;
    char*   pR;

    // 参数合法性检查
    if(NULL == pszSectionName)
    {
        return 0;
    }

    // 冗余检查代码，不用测试
    if((NULL == m_pszContent) || (NULL == m_pszShadow))
    {
        return 0;
    }

    // 将SectionName转换成小写
    pszLowerSection = new char [strlen(pszSectionName) + 2 + 1];

    sprintf(pszLowerSection, "[%s]", pszSectionName);
    MyToLower(pszLowerSection, strlen(pszLowerSection));

    // 在Shadow中定位，然后，计算出正确的指针
    // 得到Key在Shadow中的位置
    pszSectionBeginOnShadow = LocateStr( pszLowerSection,
                                         m_pszShadow,
                                         m_pszShadow + m_nSize );

    if(NULL == pszSectionBeginOnShadow)
    {
        // 释放资源
        delete [] pszLowerSection;
        return 0;
    }

    pszSectionBegin = MapToContent(pszSectionBeginOnShadow)
                      + strlen(pszLowerSection);

    //
    // 将SectionBegin指针指向Section的下一行行首字节
    //

    // 跳到行末
    for(; pszSectionBegin < (m_pszContent + m_nSize); pszSectionBegin++)
    {
        if((*pszSectionBegin == '\r') || (*pszSectionBegin == '\n'))
        {
            break;
        }
    }

    // 跳到非空字符
    for(; pszSectionBegin < (m_pszContent + m_nSize); pszSectionBegin++)
    {
        if((*pszSectionBegin != '\r')
                && (*pszSectionBegin != '\n')
                && (*pszSectionBegin != ' ')
                && (*pszSectionBegin != '\t'))
        {
            break;
        }
    }

    // 释放资源
    delete [] pszLowerSection;


    //
    // 寻找下一行有效字符以'['开头的行
    //
    bIsFirstValidCharOnLine = true;

    pR = pszSectionBegin;
    for(; pR < (m_pszContent + m_nSize + 1); pR++)
    {
        if(bIsFirstValidCharOnLine && *pR == '[')
        {
            break;
        }

        if(*pR == '\0')
        {
            break;
        }

        if(*pR == '\r' || *pR == '\n')
        {
            bIsFirstValidCharOnLine = true;
        }
        else if((*pR != ' ') && (*pR != '\t'))
        {
            bIsFirstValidCharOnLine = false;
        }
    }

    pszSectionEnd = pR;

    return 1;
}


/*************************************************
  Function:     LocateKey
  Description:
        在指定的缓冲区范围中搜索Key，返回与Key匹配的值的开始地址和结束地址。
        注意：指定范围的结尾地址和返回的取值结束地址，都是指向最后一个有效
        空间后面的地址。
  Input:
        pszKeyName      - 以NULL结尾的字符串指针，指示需要返回值的Key
        pszSectionBegin - 指向需要搜索区域的开始地址
        pszSectionEnd   - 指向需要搜索区域的最后一个字符的下一个地址
  Output:
        pszKeyBegin     - 接收Key项的开始地址
        pszKeyEnd       - 接收Key项的下一行的开始字节地址
                          指向最后一个有效字符的下一个地址
  Return:       定位成功，返回1；失败，返回0
  Others:       本函数不是UNICODE版本
*************************************************/
unsigned int ConfigFile::LocateKey( const char* pszKeyName,
                                    const char* pszSectionBegin,
                                    const char* pszSectionEnd,
                                    char* &pszKeyBegin,
                                    char* &pszKeyEnd )
{
    char* pszLowerKey;

    // 参数合法性检查
    if((NULL == pszKeyName)
            || (NULL == pszSectionBegin)
            || (NULL == pszSectionEnd))
    {
        return 0;
    }

    if(pszSectionBegin >= pszSectionEnd)
    {
        return 0;
    }

    // 冗余检查代码，不用测试
    if((NULL == m_pszContent) || (NULL == m_pszShadow))
    {
        return 0;
    }

    // 将KeyName转换成小写
    pszLowerKey = strdup(pszKeyName);
    MyToLower(pszLowerKey, strlen(pszLowerKey));

    // 在Shadow中定位，然后，计算出正确的指针
    char*   pszKeyBeginOnShadow;

    // 得到Key在Shadow中的位置
    pszKeyBeginOnShadow = LocateStr( pszLowerKey,
                                     MapToShadow(pszSectionBegin),
                                     MapToShadow(pszSectionEnd) );
    if(NULL == pszKeyBeginOnShadow)
    {
        // 释放资源
        free(pszLowerKey);
        return 0;
    }

    // 释放资源
    free(pszLowerKey);

    pszKeyBegin = MapToContent(pszKeyBeginOnShadow);

    pszKeyEnd = pszKeyBegin + strlen(pszKeyName);

    // 在指定范围内寻找关键字后面的'='
    for(; pszKeyEnd < pszSectionEnd; pszKeyEnd++)
    {
        if((*pszKeyEnd != ' ') && (*pszKeyEnd != '\t'))
        {
            break;
        }
    }

    if(*pszKeyEnd != '=')
    {
        // 找到的字符串不是关键字，采用递归方式查找指定范围中的下一个位置
        char* pszSearchBegin;       // 指示搜索区域的开始位置
        // 避免在LocateKey中，
        // 对pszValueBegin修改后，影响搜索区域的开始位置

        pszSearchBegin = pszKeyEnd;

        return LocateKey( pszKeyName,
                          pszSearchBegin,
                          pszSectionEnd,
                          pszKeyBegin,
                          pszKeyEnd );
    }

    // 过滤'='后面的字符
    for(pszKeyEnd++; pszKeyEnd < pszSectionEnd; pszKeyEnd++)
    {
        if((*pszKeyEnd == '\r') || (*pszKeyEnd == '\n'))
        {
            break;
        }
    }

    // 定位后面的取值范围
    for(; pszKeyEnd < pszSectionEnd; pszKeyEnd++)
    {
        if((*pszKeyEnd != '\r') && (*pszKeyEnd != '\n'))
        {
            break;
        }
    }

    if(pszKeyEnd > pszKeyBegin)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}


/*************************************************
  Function:     LocateValue
  Description:
        在指定的缓冲区范围中搜索Key，返回与Key匹配的值的开始地址和结束地址。
        注意：指定范围的结尾地址和返回的取值结束地址，都是指向最后一个有效
        空间后面的地址。
  Input:
        pszKeyName      - 以NULL结尾的字符串指针，指示需要返回值的Key
        pszSectionBegin - 指向需要搜索区域的开始地址
        pszSectionEnd   - 指向需要搜索区域的最后一个字符的下一个地址
  Output:
        pszValueBegin   - 接收返回值的开始地址
        pszValueEnd     - 接收返回值的结束地址
                          指向最后一个有效字符的下一个地址
  Return:       定位成功，返回1；失败，返回0
  Others:       本函数不是UNICODE版本
*************************************************/
unsigned int ConfigFile::LocateValue( const char* pszKeyName,
                                      const char* pszSectionBegin,
                                      const char* pszSectionEnd,
                                      char* &pszValueBegin,
                                      char* &pszValueEnd )
{
    char* pszLowerKey;

    // 参数合法性检查
    if((NULL == pszKeyName)
            || (NULL == pszSectionBegin)
            || (NULL == pszSectionEnd))
    {
        return 0;
    }

    if(pszSectionBegin >= pszSectionEnd)
    {
        return 0;
    }

    // 冗余检查代码，不用测试
    if((NULL == m_pszContent) || (NULL == m_pszShadow))
    {
        return 0;
    }

    // 将KeyName转换成小写
    pszLowerKey = strdup(pszKeyName);
    MyToLower(pszLowerKey, strlen(pszLowerKey));

    // 在Shadow中定位，然后，计算出正确的指针
    char    *pszKeyBeginOnShadow;

    // 得到Key在Shadow中的位置
    pszKeyBeginOnShadow = LocateStr( pszLowerKey,
                                     MapToShadow(pszSectionBegin),
                                     MapToShadow(pszSectionEnd) );

    if (NULL == pszKeyBeginOnShadow)
    {
        // 释放资源
        free(pszLowerKey);
        return 0;
    }

    // 释放资源
    free(pszLowerKey);

    pszValueBegin = MapToContent(pszKeyBeginOnShadow) + strlen(pszKeyName);

    // 在指定范围内寻找关键字后面的'='
    for (; pszValueBegin < pszSectionEnd; pszValueBegin++)
    {
        if((*pszValueBegin != ' ') && (*pszValueBegin != '\t'))
        {
            break;
        }
    }

    if(*pszValueBegin != '=')
    {
        // 找到的字符串不是关键字，采用递归方式查找指定范围中的下一个位置
        char* pszSearchBegin;       // 指示搜索区域的开始位置
        // 避免在LocateValue中，
        // 对pszValueBegin修改后，影响搜索区域的开始位置

        pszSearchBegin = pszValueBegin;

        return LocateValue( pszKeyName,
                            pszSearchBegin,
                            pszSectionEnd,
                            pszValueBegin,
                            pszValueEnd );
    }

    // 过滤'='后面的空格
    for(pszValueBegin++; pszValueBegin < pszSectionEnd; pszValueBegin++)
    {
        if((*pszValueBegin != '\t') && (*pszValueBegin != ' '))
        {
            break;
        }
    }

    pszValueEnd = pszValueBegin;

    // 过滤空格后，找到行尾
    for(; pszValueEnd < pszSectionEnd; pszValueEnd++)
    {
        if((*pszValueEnd == '\r') || (*pszValueEnd == '\n'))
        {
            break;
        }
    }

    // 继续过滤行尾的空格
    for(; pszValueEnd > pszValueBegin; pszValueEnd--)
    {
        if((*(pszValueEnd-1) != '\t') && (*(pszValueEnd-1) != ' '))
        {
            break;
        }
    }


    if(pszValueEnd > pszValueBegin)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}


/*************************************************
  Function:     LocateStr
  Description:
        在缓冲区中指定的范围内搜索CharSet，返回CharSet在缓冲区中的开始地址。
        本函数核列字母的大小写
        注意：整个缓冲区应该是NULL结尾的字符串。
  Input:
        pszStr      - 以NULL结尾的字符串指针，指示需要搜索的字符串
        pszBegin        - 指向需要搜索区域的开始地址
        pszEnd          - 指向需要搜索区域的最后一个字符的下一个地址
  Output:
  Return:       搜索成功，返回一个有效的指针；失败，返回NULL
*************************************************/
char* ConfigFile::LocateStr( const char* pszStr,
                             const char* pszBegin,
                             const char* pszEnd )
{
    char* pFind;

    // 参数合法性检查
    if((NULL == pszStr)
            || (NULL == pszBegin)
            || (NULL == pszEnd))
    {
        return NULL;
    }

    if(pszBegin >= pszEnd)
    {
        return NULL;
    }

    // 搜索字符串在缓冲区中的位置
    pFind = FindStr(pszBegin, pszStr);

    if((NULL == pFind) || ((pFind + strlen(pszStr)) > pszEnd))
    {
        return NULL;
    }
    else
    {
        return pFind;
    }
}

char* ConfigFile::FindStr(const char* pszBegin, const char* pszStr)
{
    char* pFind;
    char* pEnd;
    const char* pTempBegin = pszBegin;

    while(1)
    {
        pFind = strstr(const_cast<char*>(pTempBegin), const_cast<char*>(pszStr));
        if(NULL == pFind)
        {
            return NULL;
        }

        pEnd = pFind + strlen(pszStr);
        if( '=' == *pEnd || ' ' == *pEnd || '\t' == *pEnd || '\r' == *pEnd|| '\n' == *pEnd )
        {
            return pFind;
        }

        pTempBegin = pFind + strlen(pszStr);
    }

    return NULL;
}


/*************************************************
  Function:     MapToContent
  Description:
        从Shadow的地址映射到Content。
  Input:
        p       指向Shadow中的地址
  Output:
  Return:       如果p是Shadow中的有效地址，返回指向Content中的对应地址
*************************************************/
char* ConfigFile::MapToContent(const char* p)
{
    return (m_pszContent + (p - m_pszShadow));
}


/*************************************************
  Function:     MapToShadow
  Description:
        从Content的地址映射到Shadow。
  Input:
        p       指向Content中的地址
  Output:
  Return:       如果p是Content中的有效地址，返回指向Shadow中的对应地址
*************************************************/
char* ConfigFile::MapToShadow(const char* p)
{
    return (m_pszShadow + (p - m_pszContent));
}


/*************************************************
  Function:     MyToLower
  Description:
        将字符串中的大写字母变成小写字母。
        strlwr在处理某些汉字编码时，会出错，导致汉字编码被改变
  Input:
        pszSrc  需要处理的字符串地址
        len     需要处理的长度
  Output:
        pszSrc  存放处理完成返回的内容
*************************************************/
void ConfigFile::MyToLower(char* pszSrc, size_t len)
{
    unsigned char cb;
    size_t  i;

    if(NULL == pszSrc)
    {
        return;
    }

    for(i=0; i<len; i++)
    {
        cb = *(unsigned char* )(pszSrc + i);
        if(cb >='A' && cb<='Z')
        {
            *(unsigned char* )(pszSrc + i) = (unsigned char)(cb + 32);
        }
    }
}


