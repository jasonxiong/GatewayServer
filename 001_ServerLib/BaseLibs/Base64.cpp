/**********
This library is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the
Free Software Foundation; either version 2.1 of the License, or (at your
option) any later version. (See <http://www.gnu.org/copyleft/lesser.html>.)

This library is distributed pszSrcBuffer the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
more details.

You should have received a copy of the GNU Lesser General Public License
along with this library; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
**********/
// "liveMedia"
// Copyright (c) 1996-2010 Live Networks, Inc.  All rights reserved.
// Base64 encoding and decoding
// implementation

#include "Base64.hpp"
#include <string.h>

static char base64DecodeTable[256];

static void initBase64DecodeTable()
{
    int i;
    for (i = 0; i < 256; ++i) base64DecodeTable[i] = (char)0x80;
    // default value: invalid

    for (i = 'A'; i <= 'Z'; ++i) base64DecodeTable[i] = 0 + (i - 'A');
    for (i = 'a'; i <= 'z'; ++i) base64DecodeTable[i] = 26 + (i - 'a');
    for (i = '0'; i <= '9'; ++i) base64DecodeTable[i] = 52 + (i - '0');
    base64DecodeTable[(unsigned char)'+'] = 62;
    base64DecodeTable[(unsigned char)'/'] = 63;
    base64DecodeTable[(unsigned char)'='] = 0;
}

int base64Decode(const char* pszSrcBuffer, int iSrcLen, char* pszDstBuffer, int& riDstLen, bool trimTrailingZeros)
{
    static bool haveInitedBase64DecodeTable = false;
    if (!haveInitedBase64DecodeTable)
    {
        initBase64DecodeTable();
        haveInitedBase64DecodeTable = true;
    }
    if (NULL == pszSrcBuffer || NULL == pszDstBuffer)
    {
        return -1;
    }

    // 原码长度必须是4的倍数
    if ((iSrcLen % 4) != 0)
    {
        return -2;
    }

    int k = 0;
    for (int j = 0; j < iSrcLen; j += 4)
    {
        char inTmp[4], outTmp[4];
        for (int i = 0; i < 4; ++i)
        {
            inTmp[i] = pszSrcBuffer[i+j];
            outTmp[i] = base64DecodeTable[(unsigned char)inTmp[i]];
            if ((outTmp[i]&0x80) != 0) outTmp[i] = 0; // pretend the input was 'A'
        }

        pszDstBuffer[k++] = (outTmp[0]<<2) | (outTmp[1]>>4);
        pszDstBuffer[k++] = (outTmp[1]<<4) | (outTmp[2]>>2);
        pszDstBuffer[k++] = (outTmp[2]<<6) | outTmp[3];
    }

    if (trimTrailingZeros)
    {
        while (k > 0 && pszDstBuffer[k-1] == '\0')
        {
            --k;
        }
    }
    riDstLen = k;

    return 0;
}

static const char base64Char[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

int base64Encode(char const* pszSrcBuffer, int iSrcStrLen, char* pszDstBuffer, int& riDstStrLen)
{
    unsigned char const* orig = (unsigned char const*)pszSrcBuffer; // pszSrcBuffer case any input bytes have the MSB set
    if (orig == NULL || pszDstBuffer == NULL)
    {
        return -1;
    }

    int numOrig24BitValues = iSrcStrLen/3;
    bool havePadding = iSrcStrLen > numOrig24BitValues*3;
    bool havePadding2 = iSrcStrLen == numOrig24BitValues*3 + 2;
    int numResultBytes = 4*(numOrig24BitValues + havePadding);
    if (riDstStrLen < numResultBytes+1)
    {
        return -2;
    }

    // Map each full group of 3 input bytes into 4 output base-64 characters:
    int i;
    for (i = 0; i < numOrig24BitValues; ++i)
    {
        pszDstBuffer[4*i+0] = base64Char[(orig[3*i]>>2)&0x3F];
        pszDstBuffer[4*i+1] = base64Char[(((orig[3*i]&0x3)<<4) | (orig[3*i+1]>>4))&0x3F];
        pszDstBuffer[4*i+2] = base64Char[((orig[3*i+1]<<2) | (orig[3*i+2]>>6))&0x3F];
        pszDstBuffer[4*i+3] = base64Char[orig[3*i+2]&0x3F];
    }

    // Now, take padding into account.  (Note: i == numOrig24BitValues)
    if (havePadding)
    {
        pszDstBuffer[4*i+0] = base64Char[(orig[3*i]>>2)&0x3F];
        if (havePadding2)
        {
            pszDstBuffer[4*i+1] = base64Char[(((orig[3*i]&0x3)<<4) | (orig[3*i+1]>>4))&0x3F];
            pszDstBuffer[4*i+2] = base64Char[(orig[3*i+1]<<2)&0x3F];
        }
        else
        {
            pszDstBuffer[4*i+1] = base64Char[((orig[3*i]&0x3)<<4)&0x3F];
            pszDstBuffer[4*i+2] = '=';
        }
        pszDstBuffer[4*i+3] = '=';
    }

    pszDstBuffer[numResultBytes] = '\0';

    riDstStrLen = numResultBytes;

    return 0;
}
