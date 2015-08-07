//:FileUtility.cpp
/************************************************************
FileName: FileUtility.cpp
Author: jasonxiong
Description:
Version: 1.0
History:
<author>    <time>    <version >    <desc>
jasonxiong   09/10/29/      1.0         create
***********************************************************/

#include <dirent.h>
#include <sys/file.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include "FileUtility.hpp"

using namespace ServerLib;

int CFileUtility::MakeDir(const char* pszDirName)
{
    DIR* dpLog = opendir((const char *)pszDirName);
    if(!dpLog)
    {
        if(mkdir(pszDirName, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH))
        {
            return -1;
        }
    }
    else
    {
        closedir(dpLog);
    }

    return 0;
}

int CFileUtility::ChangeDir(const char* pszDirName)
{
    if(pszDirName == NULL)
    {
        return -1;
    }

    if(chdir((const char *)pszDirName))
    {
        return -2;
    }

    return 0;
}
