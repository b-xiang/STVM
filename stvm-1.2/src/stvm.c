/*
*  Copyright (c) 2018 Savens Liu
*
*  The original has been patented, Open source is not equal to open rights. 
*  Anyone can clone, download, learn and discuss for free.  Without the permission 
*  of the copyright owner or author, it shall not be merged, published, licensed or sold. 
*  The copyright owner or author has the right to pursue his responsibility.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
*  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
*  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
*  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
*  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
*  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
*  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
*  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
*  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
*  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
*  POSSIBILITY OF SUCH DAMAGE.
*/

#define _GNU_SOURCE
#include    "tvm.h"
#include    "tmain.h"
#include    <readline/readline.h>
#include    <readline/history.h>

/*************************************************************************************************
    macro
 *************************************************************************************************/
#define     DEL_TAIL_CHAR(s,c)         if(c == s[strlen(s) - 1])    s[strlen(s) - 1] = 0x00;

/*************************************************************************************************
   function
 *************************************************************************************************/
Benum  elog  =  0;
extern char     **environ;
extern long     lShutdownTvm();
extern void     vSetNode(char *s);
extern long     lStartupTvm(TBoot *pstBoot);

/*************************************************************************************************
    description：get stvm version 
    parameters:
    return:
        RC_SUCC                    --success
        RC_FAIL                    --failure
  *************************************************************************************************/
static  char*   sGetTVMVers()
{
    struct  utsname sinf;
    static  char    szVersion[512];

    memset(szVersion, 0, sizeof(szVersion));
    sprintf(szVersion, "Compile %s %s on ", __DATE__, __TIME__);
    if(RC_SUCC != uname(&sinf))
        strcat(szVersion, "Linux");
    else
        strcat(szVersion, sinf.sysname);

    sprintf(szVersion + strlen(szVersion), " %s Runtime\nRelease TVM C/C++ Library Version V%s on", 
        sinf.machine, TVM_VKERNEL);
    if(8 == sizeof(long))
        strcat(szVersion, " Linux x64\n");
    else
        strcat(szVersion, " Linux x32\n");
    sprintf(szVersion + strlen(szVersion), "Release STVM %s Production on %s %s\n",
        TVM_VERSION, sinf.sysname, sinf.machine);

    strcat(szVersion, "Author：DeffPuzzL\n");
    strcat(szVersion, "Mail：deffpuzzl@qq.com\n");

    return szVersion;
}

/*************************************************************************************************
    description：Screen output debugging 
    parameters：
    return：
 *************************************************************************************************/
void    vSCRDebug(const char *fmt, ...)
{
    char    szMsg[1024];
    va_list ap;

    if(0 == elog)    return ;

    memset(szMsg, 0, sizeof(szMsg));
    va_start(ap, fmt);
    vsnprintf(szMsg, sizeof(szMsg), fmt, ap);    
    va_end(ap);

    fprintf(stdout, "%s\n", szMsg);
}

/*************************************************************************************************
    description：Get the Domain version
    parameters：
    return：
        char*
 *************************************************************************************************/
char*   sFuncVersion()
{
    struct  utsname sinf;
    static  char    szVersion[512];
    
    memset(szVersion, 0, sizeof(szVersion));
    sprintf(szVersion, "Compile %s %s ", __DATE__, __TIME__);
    uname(&sinf);
    
    sprintf(szVersion + strlen(szVersion), "Release %s Production on %s", TVM_VERSION, 
        sinf.sysname);

    if(8 == sizeof(long)) 
        strcat(szVersion, " x64\n");
    else
        strcat(szVersion, " x32\n");
    
    return szVersion;
}

/*************************************************************************************************
    description：Get the end node of the list
    parameters：
    return：
        SQLFld 
  *************************************************************************************************/
SQLFld* pGetFieldTail(SQLFld *pstRoot)
{
    SQLFld  *pstList = pstRoot;

    if(!pstList)    return NULL;

    while(pstList->pstNext)
        pstList = pstList->pstNext;

    return pstList;
}

/*************************************************************************************************
    description：add node
    parameters：
    return：
        SQLFld 
  *************************************************************************************************/
SQLFld* pInsertFiled(SQLFld *pstRoot, SQLFld *pstNode)
{
    SQLFld  *pstNext = NULL, *pstTail = pGetFieldTail(pstRoot);

    if(NULL == (pstNext = (SQLFld *)malloc(sizeof(SQLFld))))
        return pstRoot;

    memcpy(pstNext, pstNode, sizeof(SQLFld));
    pstNext->pstNext = NULL;

    if(!pstRoot)
        pstRoot = pstNext;
    else
        pstTail->pstNext = pstNext;

    return pstRoot;
}

/*************************************************************************************************
    description：Clean up the list
    parameters：
    return：
  *************************************************************************************************/
void    vDestroyFiled(SQLFld *pstRoot)
{
    SQLFld  *pstNode = pstRoot, *pstList = NULL;

    while(pstNode)
    {
        pstList = pstNode;
        pstNode = pstNode->pstNext;
        IBPFree(pstList);
    }
}

/*************************************************************************************************
    description：Get the number of nodes
    parameters：
    return：
        lCnt                        --number of nodes
  *************************************************************************************************/
long    lGetNodeFiled(SQLFld *pstRoot)
{
    long    lCnt;
    SQLFld  *pstNode = NULL;

    for(pstNode = pstRoot, lCnt = 0; pstNode; pstNode = pstNode->pstNext, lCnt ++);

    return lCnt;
}

/*************************************************************************************************
    description：sort the field
    parameters：
    return：
        SQLFld                        --root
  *************************************************************************************************/
SQLFld*    pSortSQLField(SQLFld *pstRoot)
{
    TblKey   stKey;
    SQLFld   *pstNode = NULL, *pstList = NULL;

    for(pstNode = pstRoot; pstNode; pstNode = pstNode->pstNext)
    {
        for(pstList = pstNode->pstNext; pstList; pstList = pstList->pstNext)
        {
            if(pstNode->m_stKey.m_lFrom < pstList->m_stKey.m_lFrom)
                continue;

            memcpy(&stKey, &pstNode->m_stKey, sizeof(TblKey));
            memcpy(&pstNode->m_stKey, &pstList->m_stKey, sizeof(TblKey));
            memcpy(&pstList->m_stKey, &stKey, sizeof(TblKey));
        }
    }

     return pstRoot;
}

/*************************************************************************************************
    description：Returns the current time in millisecond
    parameters：
    return：
        lTime                      --millisecond
  *************************************************************************************************/
long    lGetTiskTime()
{
    long    lTime;
    struct timeval  t;
    struct tm   *ptm;

    gettimeofday(&t, NULL);
    ptm = localtime(&t.tv_sec);

    lTime = ptm->tm_min * 60 * 1000 + ptm->tm_sec * 1000 + t.tv_usec / 1000;
    return lTime;
}

/*************************************************************************************************
    description：Calculate time consumption (milliseconds)
    parameters：
    return：
        szTime                     --Time string
  *************************************************************************************************/
char*   sGetCostTime(long lTime)
{
    static  char    szTime[30];

    memset(szTime, 0, sizeof(szTime));

    if(lTime < 0)
        return szTime;
    if(lTime < 1000)
        snprintf(szTime, sizeof(szTime), "cost:%ldms", lTime);
    else
        snprintf(szTime, sizeof(szTime), "cost:%lds%ldms", lTime / 1000, lTime % 1000);
    return szTime;
}

/*************************************************************************************************
    description：Printing system index
    parameters：
    return：
  *************************************************************************************************/
void    vPrintIndex()
{
    TIndex  stIndex;
    SATvm   *pstSavm = NULL;
    long    lRet = 0, nRecord = 0, lTime = 0;

    memset(&stIndex, 0, sizeof(TIndex));
    if(NULL == (pstSavm = (SATvm *)pInitSATvm(SYS_TVM_INDEX)))
    {
        fprintf(stderr, "init SYS_TABLE_IDX failure, %s\n", sGetTError(pstSavm->m_lErrno));
        return ;
    }

    pstSavm->bSearch = TYPE_SYSTEM;
    pstSavm->tblName = SYS_TVM_INDEX;
    pstSavm->lSize = sizeof(TIndex);
    lRet = lTableDeclare(pstSavm);
    if(RC_SUCC != lRet)
    {
        fprintf(stderr, "declare SYS_TABLE_IDX failure, (%d)(%s)\n", pstSavm->m_lErrno, 
            sGetTError(pstSavm->m_lErrno));
        return ;
    }

    fprintf(stdout, "------------------------------------------------------------------"
        "------------------------------------------------------------------------------\n");
    fprintf(stdout, "line  table type    table_name              mark          ownner     Key"
        "     shm_id   sem_id   maxrows size flag stat pers     update-time   \n");
    fprintf(stdout, "------------------------------------------------------------------"
        "------------------------------------------------------------------------------\n");
    lTime = lGetTiskTime();
    while(1)
    {
        memset(&stIndex, 0, sizeof(stIndex));
        lRet = lTableFetch(pstSavm, (void *)&stIndex);
        if(RC_FAIL == lRet)
        {
            fprintf(stderr, "Fetch SYS_TABLE_IDX failure, err:(%d)(%s)\n", pstSavm->m_lErrno, 
                sGetTError(pstSavm->m_lErrno));
            vTableClose(pstSavm);
            return ;
        }
        else if(RC_NOTFOUND == lRet)
            break;

        nRecord ++;

        fprintf(stdout, "[%3ld]: %4d|%4ld|%-22s|%-15s|%10s|%8d|%8ld|%8ld|%8ld|%4ld|%4ld|%4d|%4ld|%20s|\n",
            nRecord, stIndex.m_table, stIndex.m_lType, stIndex.m_szTable, stIndex.m_szPart, 
            stIndex.m_szOwner, stIndex.m_yKey, stIndex.m_shmID, stIndex.m_semID, stIndex.m_lMaxRows, 
            stIndex.m_lRowSize, stIndex.m_lLocal, stIndex.m_lState, stIndex.m_lPers, stIndex.m_szTime);
    }
    lTime -= lGetTiskTime();
    vTableClose(pstSavm);
    fprintf(stdout, "----------------------------------------------------------------------%s----"
        "--------------------------------------------------------------\n", sGetCostTime(-1 * lTime));

    return ;
}

/*************************************************************************************************
    description：Print table field information
    parameters：
    return：
  *************************************************************************************************/
void    vPrintField()
{
    TField    stField;
    SATvm   *pstSavm = NULL;
    char    szPrint[256];
    long    lRet = 0, lRow = 0, lTime = 0;

    memset(&stField, 0, sizeof(TField));
    if(NULL == (pstSavm = (SATvm *)pInitSATvm(SYS_TVM_FIELD)))
    {
        fprintf(stderr, "initial SYS_TVM_FIELD failure\n");
        return ;
    }

    pstSavm->lSize = sizeof(TField);
    pstSavm->tblName = SYS_TVM_FIELD;
    lRet = lTableDeclare(pstSavm);
    if(RC_SUCC != lRet)
    {
        fprintf(stderr, "declare SYS_TVM_FIELD failure, err:(%d)(%s)\n", pstSavm->m_lErrno, 
            sGetTError(pstSavm->m_lErrno));
        return ;
    }
    fprintf(stdout, "------------------------------------------------------------------------"
        "------------------------\n");
    fprintf(stdout, "line  table seq      owner                table_name           field_name"
        "       attr   len  PK\n");
    fprintf(stdout, "------------------------------------------------------------------------"
        "------------------------\n");
    lTime = lGetTiskTime();
    while(1)
    {
        memset(&stField, 0, sizeof(TField));
        lRet = lTableFetch(pstSavm, (void *)&stField);
        if(RC_FAIL == lRet)
        {
            fprintf(stderr, "fetch SYS_TVM_FIELD failure, err:(%d)(%s)\n", pstSavm->m_lErrno, 
                sGetTError(pstSavm->m_lErrno));
            vTableClose(pstSavm);
            return ;
        }
        else if(RC_NOTFOUND == lRet)
            break;

        lRow ++;

        memset(szPrint, 0, sizeof(szPrint));
        if(FIELD_CHAR == stField.m_lAttr)
        {
            snprintf(szPrint, sizeof(szPrint), "[%3ld]: %4d|%3ld|%-20s|%-22s|%18s|STRING|"
                "%4ld|%4ld|\n", lRow, stField.m_table, stField.m_lSeq, stField.m_szOwner, 
                stField.m_szTable, stField.m_szField, stField.m_lLen, stField.m_lIsPk);
        }
        else if(FIELD_LONG == stField.m_lAttr)
        {
            snprintf(szPrint, sizeof(szPrint), "[%3ld]: %4d|%3ld|%-20s|%-22s|%18s|NUMBER|"
                "%4ld|%4ld|\n", lRow, stField.m_table, stField.m_lSeq, stField.m_szOwner, 
                stField.m_szTable, stField.m_szField, stField.m_lLen, stField.m_lIsPk);
        }
        else if(FIELD_DOUB == stField.m_lAttr)
        {
            snprintf(szPrint, sizeof(szPrint), "[%3ld]: %4d|%3ld|%-20s|%-22s|%18s|DOUBLE|"
                "%4ld|%4ld|\n", lRow, stField.m_table, stField.m_lSeq, stField.m_szOwner, 
                stField.m_szTable, stField.m_szField, stField.m_lLen, stField.m_lIsPk);
        }
        else
        {
            snprintf(szPrint, sizeof(szPrint), "[%3ld]: %4d|%3ld|%-20s|%-22s|%18s| NAN  |"
                "%4ld|%4ld|\n", lRow, stField.m_table, stField.m_lSeq, stField.m_szOwner, 
                stField.m_szTable, stField.m_szField, stField.m_lLen, stField.m_lIsPk);
        }

        fprintf(stdout, "%s", szPrint);
    }
    lTime -= lGetTiskTime();
    vTableClose(pstSavm);
    fprintf(stdout, "-------------------------------------------%s-----------------------------"
        "----------------\n", sGetCostTime(-1 * lTime));

    return ;
}

/*************************************************************************************************
    description：get permit 
    parameters:
    return:
        char*
  *************************************************************************************************/
char*    sPermitConv(long lPers)
{
    static    char    szPers[5];

    strcpy(szPers, "----");
    szPers[4] = 0x00;

    if(lPers & OPERATE_SELECT)
        szPers[0] = 's';
    if(lPers & OPERATE_UPDATE)
        szPers[1] = 'u';
    if(lPers & OPERATE_DELETE)
        szPers[2] = 'd';
    if(lPers & OPERATE_INSERT)
        szPers[3] = 'i';

    return szPers;
}

/*************************************************************************************************
    description：Print table field information
    parameters：
    return：
  *************************************************************************************************/
void    vPrintDomain()
{
    TDomain stDomain;
    SATvm   *pstSavm = NULL;
    char    szPrint[512];
    long    lRet = 0, lRow = 0, lTime = 0;

    memset(&stDomain, 0, sizeof(TDomain));
    if(NULL == (pstSavm = (SATvm *)pInitSATvm(SYS_TVM_DOMAIN)))
    {
        fprintf(stderr, "initail SYS_TVM_DOMAIN failure\n");
        return ;
    }

    pstSavm->lSize = sizeof(TDomain);
    pstSavm->tblName = SYS_TVM_DOMAIN;
    lRet = lTableDeclare(pstSavm);
    if(RC_SUCC != lRet)
    {
        fprintf(stderr, "declare SYS_TVM_DOMAIN failure, err:(%d)(%s)\n", pstSavm->m_lErrno, 
            sGetTError(pstSavm->m_lErrno));
        return ;
    }
    fprintf(stdout, "------------------------------------------------------------------------"
        "--------------------------\n");
    fprintf(stdout, "line table mtbl     table_name        part          ip       port  keep "
        "tmout mtry ctry stat pers\n");
    fprintf(stdout, "------------------------------------------------------------------------"
        "--------------------------\n");
    lTime = lGetTiskTime();
    while(1)
    {
        memset(&stDomain, 0, sizeof(TDomain));
        lRet = lTableFetch(pstSavm, (void *)&stDomain);
        if(RC_FAIL == lRet)
        {
            fprintf(stderr, "fetch SYS_TVM_DOMAIN failure, %s\n", sGetTError(pstSavm->m_lErrno));
            vTableClose(pstSavm);
            return ;
        }
        else if(RC_NOTFOUND == lRet)
            break;

        lRow ++;

        memset(szPrint, 0, sizeof(szPrint));
        snprintf(szPrint, sizeof(szPrint), "[%3ld]: %3d|%3d|%18s|%10s|%15s|%5ld|%4ld|%5ld|%4ld|%4ld|", 
            lRow, stDomain.m_table, stDomain.m_mtable, stDomain.m_szTable, stDomain.m_szPart, 
            stDomain.m_szIp, stDomain.m_lPort, stDomain.m_lKeepLive, stDomain.m_lTimeOut, 
            stDomain.m_lTryMax, stDomain.m_lTryTimes);

        if(RESOURCE_INIT == stDomain.m_lStatus)
            strcat(szPrint, "INIT|");
        else if(RESOURCE_EXCP == stDomain.m_lStatus)
            strcat(szPrint, "EXCP|");
        else if(RESOURCE_ABLE == stDomain.m_lStatus)
            strcat(szPrint, "ABLE|");
        else if(RESOURCE_STOP == stDomain.m_lStatus)
            strcat(szPrint, "STOP|");
        else if(RESOURCE_AUTH == stDomain.m_lStatus)
            strcat(szPrint, "AUTH|");

        strcat(szPrint, sPermitConv(stDomain.m_lPers));
        fprintf(stdout, "%s|\n", szPrint);
        fflush(stdout);
    }
    lTime -= lGetTiskTime();
    vTableClose(pstSavm);
    fprintf(stdout, "----------------------------------------%s-----------------------------"
        "---------------------\n", sGetCostTime(-1 * lTime));

    return ;
}

/*************************************************************************************************
    description：Gets the value between the string o/d
    parameters：
        flag                --Ignore case
    return：
        char*
  *************************************************************************************************/
char*   sGetTruckValue(const char *s, char *o, char *d, bool flag, char *out, int olen)
{
    int     len = 0;
    char    *p = NULL, *q = NULL;

    if (!o)
    {
        if (!d)
        {
            len = strlen(s);
            return memcpy(out, s, len > olen ? olen : len);
        }

        if (!(q = (flag ? strcasestr(s, d) : strstr(s, d))))
            return NULL;

        return memcpy(out, s, q - s > olen ? olen : q - s);
    }
    else if(!d)
    {
        if (!o)
        {
            len = strlen(s);
            return memcpy(out, s, len > olen ? olen : len);
        }

        if (!(p = (flag ? strcasestr(s, o) : strstr(s, o))))
            return NULL;

        snprintf(out, olen, "%s", p + strlen(o));
        return out;
    }
    else
    {
        if (!(p = (flag ? strcasestr(s, o) : strstr(s, o))))
            return NULL;

        len = strlen(o);

        if (!(q = (flag ? strcasestr(p + len, d) : strstr(s, d))))
            return NULL;

        return memcpy(out, p + len, q - p - len > olen ? olen : q - p - len);
    }
}

/*************************************************************************************************
    description：Ignore case-sensitive substitution of characters in strings
    parameters：
    return：
        s
  *************************************************************************************************/
char*    sUpperWord(char *s, char *w)
{
    long    len;
    char    *p = NULL, *q = s;

    if(!s || !w || !strlen(s))    return NULL;

    if(0 == (len = strlen(w)))    return NULL;
    
    while(NULL != (p = strcasestr(q, w)))
    {
        memcpy(p, w, len);
        q += len;
    }

    return s;
}

/*************************************************************************************************
    description：pitch field
    parameters：
    return：
        void                             --success
        NULL                             --failure
  *************************************************************************************************/
TField *pPitchField(long lCount, TField *pstField, char *pszField)
{
    int      i;

    for(i = 0; i < lCount; i ++)
    {
        if(!strcasecmp(pszField, pstField[i].m_szField))
            return &pstField[i];
    }

    return NULL;
}

/*************************************************************************************************
    description：User-selected field when parsing extreme
    parameters：
    return：
        RC_SUCC                            --success
        RC_FAIL                            --failure
  *************************************************************************************************/
long    _lExtremeField(SATvm *pstSavm, char *p, long lCount, TField *pstField, SQLFld **ppstRoot)
{
    Uenum   uDeco;    
    long    lRec = 0, i;
    TField  *pfld = NULL;
    char    szField[MAX_FIELD_LEN];
    SQLFld  stQField, *pstRoot = NULL;

    for(i = 0, lRec = lfieldnum(p, ","); i < lRec; i ++)
    {
        memset(szField, 0, sizeof(szField));
        memset(&stQField, 0, sizeof(SQLFld));
        strncpy(stQField.m_stKey.m_szField, sfieldvalue(p, ",", i + 1), 
            sizeof(stQField.m_stKey.m_szField));
        if(!strncmp(stQField.m_stKey.m_szField, "max(", 4))
            uDeco =  MATCH_MAX;
        else if(!strncmp(stQField.m_stKey.m_szField, "min(", 4))
            uDeco =  MATCH_MIN;
        else
            return RC_FAIL;

        if(!sGetTruckValue(stQField.m_stKey.m_szField, "(", ")", true, szField, sizeof(szField)))
            return RC_FAIL;

        if(NULL == (pfld = pPitchField(lCount, pstField, szField)))
        {
            fprintf(stderr, "field (%s) undefine\n", szField);
            return RC_FAIL;
        }

        vSetDecorate(&pstSavm->stUpdt, pfld->m_lLen, pfld->m_lFrom, uDeco);
        stQField.m_stKey.m_lLen  = pfld->m_lLen;
        stQField.m_stKey.m_lAttr = pfld->m_lAttr;
        stQField.m_stKey.m_lFrom = pfld->m_lFrom;
        if(NULL == (pstRoot = pInsertFiled(pstRoot, &stQField)))
            return RC_FAIL;
    }

    *ppstRoot = pstRoot;

    return RC_SUCC;
}

/*************************************************************************************************
    description：Parse SQL condition fields
    parameters：
    return：
        RC_SUCC                            --success
        RC_FAIL                            --failure
  *************************************************************************************************/
long    _lSelectField(SATvm *pstSavm, char *p, long lCount, TField *pstField, SQLFld **ppstRoot)
{
    TField  *pfld = NULL;
    long    lRec = 0, i, j;
    char    szField[MAX_FIELD_LEN];
    SQLFld  stQField, *pstRoot = NULL;
    FdCond  *pstExm = &pstSavm->stUpdt;

    if(0 == strcmp(p, "*"))
    {
        if(pstSavm->lFind & GROUP_BY)
            return RC_FAIL;
    
        for(i = 0; i < lCount; i ++)
        {
            memset(&stQField, 0, sizeof(SQLFld));
            stQField.m_stKey.m_lLen  = pstField[i].m_lLen;
            stQField.m_stKey.m_lAttr = pstField[i].m_lAttr;
            stQField.m_stKey.m_lFrom = pstField[i].m_lFrom;
            strcpy(stQField.m_stKey.m_szField, pstField[i].m_szField);
    
            if(NULL == (pstRoot = pInsertFiled(pstRoot, &stQField)))
                return RC_FAIL;
        }
    
        *ppstRoot = pSortSQLField(pstRoot);
        return RC_SUCC;
    }

    for(i = 0, lRec = lfieldnum(p, ","); i < lRec; i ++)
    {
        memset(szField, 0, sizeof(szField));
        strncpy(szField, sfieldvalue(p, ",", i + 1), MAX_FIELD_LEN);
        strimall(szField);

        if((!strcasecmp(szField, "count(*)") ||
            !strcasecmp(szField, "count(1)")))
        {
            fprintf(stderr, "%s, Temporarily not supported\n", szField);
            return RC_FAIL;
        }

        if(NULL == (pfld = pPitchField(lCount, pstField, szField)))
        {
            fprintf(stderr, "field (%s) undefine\n", szField);
            return RC_FAIL;
        }

        if(pstSavm->lFind & GROUP_BY)
        {
            for(j = 0; j < pstExm->uFldcmp; j ++)
            {
                if(pstExm->stFdKey[j].uFldpos == pfld->m_lFrom)
                    break;
            }

            if(j == pstExm->uFldcmp)
            {
                fprintf(stderr, "Fields (%s) are not grouped\n", szField);
                return RC_FAIL;
            }
        }

        memset(&stQField, 0, sizeof(SQLFld));
        stQField.m_stKey.m_lLen  = pfld->m_lLen;
        stQField.m_stKey.m_lAttr = pfld->m_lAttr;
        stQField.m_stKey.m_lFrom = pfld->m_lFrom;
        strcpy(stQField.m_stKey.m_szField, szField);
        if(NULL == (pstRoot = pInsertFiled(pstRoot, &stQField)))
            return RC_FAIL;
    }

    *ppstRoot = pstRoot;
    return RC_SUCC;
}

/*************************************************************************************************
    description：Parse SQL update fields
    parameters：
    return：
        RC_SUCC                            --success
        RC_FAIL                            --failure
  *************************************************************************************************/
long    _lUpdateField(SATvm *pstSavm, char *pszField, long lCount, TField *pstField, void *pvUpdate)
{
    void    *v = NULL;
    TField  *pfld = NULL;
    long    lRec = 0, i, j;
    char    szValue[512], szContent[1024], szField[512], szNumber[16];

    memset(szNumber, 0, sizeof(szNumber));
    for(i = 0, lRec = lfieldnum(pszField, ","), v = (void *)szNumber; i < lRec; i ++)
    {
        memset(szValue, 0, sizeof(szValue));
        memset(szField, 0, sizeof(szField));
        memset(szContent, 0, sizeof(szContent));
        strncpy(szContent, sfieldvalue(pszField, ",", i + 1), sizeof(szContent));
        strncpy(szField, sfieldvalue(szContent, "=", 1), sizeof(szField));
        strimall(szField);

        strncpy(szValue, sfieldvalue(szContent, "=", 2), sizeof(szValue));
        sltrim(szValue);
        srtrim(szValue);
        strimabout(szValue, "\'", "\'");
        if(NULL == (pfld = pPitchField(lCount, pstField, szField)))
        {
            fprintf(stderr, "field (%s) undefine\n", szField);
            return RC_FAIL;
        }

        vSetCodField(&pstSavm->stUpdt, pfld->m_lLen, pfld->m_lFrom);
        switch(pfld->m_lAttr)
        {
        case FIELD_DOUB:
            switch(pfld->m_lLen)
            {
            case    4:
                *((float *)v) = atof(szValue);
                break;
            case    8:
                *((double *)v) = atof(szValue);
                break;
            default:
                return RC_FAIL;
            }
            memcpy(pvUpdate + pfld->m_lFrom, v, pfld->m_lLen);
            break;
        case FIELD_LONG:
            switch(pfld->m_lLen)
            {
            case    2:
                *((sint *)v) = atoi(szValue);
                break;
            case    4:
                *((int *)v) = atoi(szValue);
                break;
            case    8:
                *((llong *)v) = atol(szValue);
                break;
            default:
                return RC_FAIL;
            }
            memcpy(pvUpdate + pfld->m_lFrom, v, pfld->m_lLen);
            break;
        case FIELD_CHAR:
            strncpy(pvUpdate + pfld->m_lFrom, szValue, pfld->m_lLen);
            break;
        default:
            return RC_FAIL;
        }
    }

    return RC_SUCC;
}

/*************************************************************************************************
    description：execute SQL-select
    parameters：
    return：
        RC_SUCC                            --success
        RC_FAIL                            --failure
  *************************************************************************************************/
long    _lExeUpdate(SATvm *pstSavm, TIndex *pstIndex, void *pvNew, char *pvData, BOOL bRmt)
{
    long    lRet;

    pstSavm->pstVoid = (void *)pvData;
    pstSavm->tblName = pstIndex->m_table;
    pstSavm->lSize   = pstIndex->m_lRowSize;
    if(bRmt)
        lRet = lTvmUpdate(pstSavm, (void *)pvNew);
    else
        lRet = lUpdate(pstSavm, (void *)pvNew);
    if(RC_SUCC != lRet)
    {
        fprintf(stderr, "update table (%s) failure, %s\n", pstIndex->m_szTable, 
            sGetTError(pstSavm->m_lErrno));
        return RC_FAIL;
    }

    fprintf(stdout, "---(%ld) records updated -ep(%d)--\n", pstSavm->m_lEffect, pstSavm->m_lEType);
    return RC_SUCC;
}

/*************************************************************************************************
    description：execute SQL-delete
    parameters：
    return：
        RC_SUCC                            --success
        RC_FAIL                            --failure
  *************************************************************************************************/
long    _lExeDelete(SATvm *pstSavm, TIndex *pstIndex, char *pvData, BOOL bRmt)
{
    long    lRet;

    pstSavm->pstVoid = (void *)pvData;
    pstSavm->tblName = pstIndex->m_table;
    pstSavm->lSize = pstIndex->m_lRowSize;
    if(bRmt)
        lRet = lTvmDelete(pstSavm);
    else
        lRet = lDelete(pstSavm);
    if(RC_SUCC != lRet)
        return RC_FAIL;

    fprintf(stdout, "---(%ld) records deleted -ep(%d)--\n", pstSavm->m_lEffect, pstSavm->m_lEType);
    return RC_SUCC;
}

/*************************************************************************************************
    description：execute SQL-insert
    parameters：
    return：
        RC_SUCC                            --success
        RC_FAIL                            --failure
  *************************************************************************************************/
long    _lExeInsert(SATvm *pstSavm, TIndex *pstIndex, void *pvInsert, BOOL bRmt)
{
    long    lRet;
    
    if(bRmt)
        pstSavm->tblName = pstIndex->m_table;
    else
    {
        if(RC_SUCC != lInitSATvm(pstSavm, pstIndex->m_table))
           return RC_FAIL;
    }

    pstSavm->pstVoid = (void *)pvInsert;
    pstSavm->lSize   = pstIndex->m_lRowSize;
    if(bRmt)
        lRet = lTvmInsert(pstSavm);
    else
        lRet = lInsert(pstSavm);
    if(RC_SUCC != lRet)
    {
        fprintf(stderr, "insert table (%s) failure, %s\n", pstIndex->m_szTable, 
            sGetTError(pstSavm->m_lErrno));
        return RC_FAIL;
    }

    fprintf(stdout, "---(%ld) records inserted ---\n", pstSavm->m_lEffect);
    return RC_SUCC;
}

/*************************************************************************************************
    description：Parse SQL condition fields
    parameters：
    return：
        RC_SUCC                            --success
        RC_FAIL                            --failure
  *************************************************************************************************/
long    _lConditField(SATvm *pstSavm, char *pszWhere, long lCount, TField *pstField, void *pvWhere)
{
    long    n, i, j;
    void    *v = NULL;
    TField  *pfld = NULL;
    char    szValue[512], szCondit[1024], szField[512], szNumber[16];

    if(strcasestr(pszWhere, " or "))
    {
        fprintf(stderr, "Unsupported query methods\n");
        return RC_FAIL;
    }

    memset(szNumber, 0, sizeof(szNumber));
    for(i = 0, v = (void *)szNumber, n = lfieldnum(pszWhere, " AND "); i < n; i ++)
    {
        memset(szValue, 0, sizeof(szValue));
        memset(szField, 0, sizeof(szField));
        memset(szCondit, 0, sizeof(szCondit));
        strncpy(szCondit, sfieldvalue(pszWhere, " AND ", i + 1), sizeof(szCondit));

        strncpy(szField, sfieldvalue(szCondit, "=", 1), sizeof(szField));
        strimall(szField);
           strncpy(szValue, sfieldvalue(szCondit, "=", 2), sizeof(szValue));
           sltrim(szValue);
           srtrim(szValue);
        strimabout(szValue, "\'", "\'");
        if(!strcmp(szField, "1") && !strcmp(szValue, "1")) 
            continue;

        if(NULL == (pfld = pPitchField(lCount, pstField, szField)))
        {
            fprintf(stderr, "field (%s) undefine\n", szField);
            return RC_FAIL;
        }

        vSetCodField(&pstSavm->stCond, pfld->m_lLen, pfld->m_lFrom);
        switch(pfld->m_lAttr)
        {
        case FIELD_DOUB:
            switch(pfld->m_lLen)
            {
            case    4:
                *((float *)v) = atof(szValue);
                break;
            case    8:
                *((double *)v) = atof(szValue);
                break;
            default:
                return RC_FAIL;
            }
            memcpy(pvWhere + pfld->m_lFrom, v, pfld->m_lLen);
            break;
        case FIELD_LONG:
            switch(pfld->m_lLen)
            {
            case    2:
                *((sint *)v) = atoi(szValue);
                break;
            case    4:
                *((int *)v) = atoi(szValue);
                break;
            case    8:
                *((llong *)v) = atol(szValue);
                break;
            default:
                return RC_FAIL;
            }
            memcpy(pvWhere + pfld->m_lFrom, v, pfld->m_lLen);
            break;
        case FIELD_CHAR:
            strncpy(pvWhere + pfld->m_lFrom, szValue, pfld->m_lLen);
            break;
         default:
            return RC_FAIL;
        }
    }

    return RC_SUCC;
}

/*************************************************************************************************
    description：execute SQL-count
    parameters：
    return：
        RC_SUCC                            --success
        RC_FAIL                            --failure
  *************************************************************************************************/
long    _lCountSelect(SATvm *pstSavm, TIndex *pstIndex, char *pvData, BOOL bRmt)
{
    size_t    lRet, lSum = 0;

    pstSavm->pstVoid = (void *)pvData;
    pstSavm->tblName = pstIndex->m_table;
    pstSavm->lSize = pstIndex->m_lRowSize;
    if(bRmt)
        lRet = lTvmCount(pstSavm, &lSum);
    else
        lRet = lCount(pstSavm, &lSum);
    if(lRet != RC_SUCC)
    {
        fprintf(stderr, "count table (%s) failure, %s\n", pstIndex->m_szTable, 
            sGetTError(pstSavm->m_lErrno));
        return RC_FAIL;
    }

    fprintf(stdout, "---(%ld) records selected -ep(%d)--\n", pstSavm->m_lEffect, pstSavm->m_lEType);
    fprintf(stdout, "COUNT(*)\n");
    fprintf(stdout, "%zu\n", lSum);
    fflush(stdout);

    return RC_SUCC;
}

/*************************************************************************************************
    description：execute SQL-group
    parameters：
    return：
        RC_SUCC                            --success
        RC_FAIL                            --failure
  *************************************************************************************************/
long    _lExeGroup(SATvm *pstSavm, TIndex *pstIndex, SQLFld *pstRoot, char *pvData, BOOL bRmt)
{
    TblKey  *pstKey = NULL;
    SQLFld  *pstNode = NULL;
    Rowgrp  *pstList = NULL;
    char    *pvResult = NULL;
    size_t  i, lOffset, lRows, lRet;

    pstSavm->pstVoid = (void *)pvData;
    pstSavm->tblName = pstIndex->m_table;
    pstSavm->lSize = pstIndex->m_lRowSize;

    if(bRmt)
        lRet = lTvmGroup(pstSavm, &lRows, (void **)&pvResult);
    else
        lRet = lGroup(pstSavm, &lRows, (void **)&pvResult);
    if(RC_SUCC != lRet)
    {
        if(NO_DATA_FOUND != pstSavm->m_lErrno)
        {
            fprintf(stderr, "Group table (%s) failure，%s\n", pstIndex->m_szTable, 
                sGetTError(pstSavm->m_lErrno));
        }

        return RC_FAIL;
    }

    for(pstNode = pstRoot; pstNode; pstNode = pstNode->pstNext)
        fprintf(stdout, "%s ", pstNode->m_stKey.m_szField);
    fprintf(stdout, "\n");

    for(i = 0; i < lRows && i < pstSavm->m_lEffect; i ++)
    {
        lOffset = pstIndex->m_lRowSize * i;
        for(pstNode = pstRoot; pstNode; pstNode = pstNode->pstNext)
        {
            if(!strcasecmp(pstNode->m_stKey.m_szField, "count(*)") || 
                !strcasecmp(pstNode->m_stKey.m_szField, "count(1)"))
                continue;

            pstKey = &pstNode->m_stKey;
            switch(pstKey->m_lAttr)
            {
            case FIELD_DOUB:
                switch(pstKey->m_lLen)
                {
                case    4:
                    fprintf(stdout, "%f ", *((float *)(pvResult + lOffset + pstKey->m_lFrom)));
                    break;
                case    8:
                    fprintf(stdout, "%f ", *((double *)(pvResult + lOffset + pstKey->m_lFrom)));
                    break;
                default:
                    break;
                }
                break;
            case FIELD_LONG:
                switch(pstKey->m_lLen)
                {
                case    2:
                    fprintf(stdout, "%d ", *((sint *)(pvResult + lOffset + pstKey->m_lFrom)));
                    break;
                case    4:
                    fprintf(stdout, "%d ", *((int *)(pvResult + lOffset + pstKey->m_lFrom)));
                    break;
                case    8:
                    fprintf(stdout, "%lld ", *((llong *)(pvResult + lOffset + pstKey->m_lFrom)));
                    break;
                default:
                    break;
                }
                break;
            case FIELD_CHAR:
                fprintf(stdout, "%.*s ", (int)pstKey->m_lLen, pvResult + lOffset + pstKey->m_lFrom);
                break;
            default:
                break;
            }
        }
        fprintf(stdout, "\n");
        fflush(stdout);
    }
    IBPFree(pvResult);

    fprintf(stdout, "---(%ld) records selected, ep(%d)---\n", pstSavm->m_lEffect, pstSavm->m_lEType);
    return RC_SUCC;
}

/*************************************************************************************************
    description：execute SQL-select
    parameters：
    return：
        RC_SUCC                            --success
        RC_FAIL                            --failure
  *************************************************************************************************/
long    _lExeSelect(SATvm *pstSavm, TIndex *pstIndex, SQLFld *pstRoot, char *pvData, char *pszFile,
            char *pszDem, BOOL bRmt)
{
    FILE    *fp = NULL;
    TblKey  *pstKey = NULL;
    SQLFld  *pstNode = NULL;
    Rowgrp  *pstList = NULL;
    size_t  i, lOffset, lRows, lRet;
    char    szDelmit[64], *pvResult = NULL;

    memset(szDelmit, 0, sizeof(szDelmit));
    pstSavm->pstVoid = (void *)pvData;
    pstSavm->tblName = pstIndex->m_table;
    pstSavm->lSize = pstIndex->m_lRowSize;
    if(bRmt)
        lRet = lTvmQuery(pstSavm, &lRows, (void **)&pvResult);
    else
        lRet = lQuery(pstSavm, &lRows, (void **)&pvResult);
    if(RC_SUCC != lRet)
    {
        if(NO_DATA_FOUND != pstSavm->m_lErrno)
        {
            fprintf(stderr, "Query table (%s) failure，%s\n", pstIndex->m_szTable, 
                sGetTError(pstSavm->m_lErrno));
        }

        return RC_FAIL;
    }

    if(pszFile)
    {
        if(NULL == (fp = fopen(pszFile, "wb")))
        {
            pstSavm->m_lErrno = FILE_NOTFOUND;
            return RC_FAIL;    
        }    

        strncpy(szDelmit, pszDem, sizeof(szDelmit));
    }
    else
    {
        fp = stdout;
        strcpy(szDelmit, " ");
    }

    for(pstNode = pstRoot; pstNode; pstNode = pstNode->pstNext)
        fprintf(fp, "%s ", pstNode->m_stKey.m_szField);
    fprintf(fp, "\n");

    for(i = 0; i < lRows && i < pstSavm->m_lEffect; i ++)
    {
        lOffset = pstIndex->m_lRowSize * i;
        for(pstNode = pstRoot; pstNode; pstNode = pstNode->pstNext)
        {
            if(!strcasecmp(pstNode->m_stKey.m_szField, "count(*)") || 
                !strcasecmp(pstNode->m_stKey.m_szField, "count(1)"))
                continue;

            pstKey = &pstNode->m_stKey;
            switch(pstKey->m_lAttr)
            {
            case FIELD_DOUB:
                switch(pstKey->m_lLen)
                {
                case    4:
                    fprintf(fp, "%f%s", *((float *)(pvResult + lOffset + pstKey->m_lFrom)), szDelmit);
                    break;
                case    8:
                    fprintf(fp, "%f%s", *((double *)(pvResult + lOffset + pstKey->m_lFrom)), szDelmit);
                    break;
                default:
                    break;
                }
                break;
            case FIELD_LONG:
                switch(pstKey->m_lLen)
                {
                case    2:
                    fprintf(fp, "%d%s", *((sint *)(pvResult + lOffset + pstKey->m_lFrom)), szDelmit);
                    break;
                case    4:
                    fprintf(fp, "%d%s", *((int *)(pvResult + lOffset + pstKey->m_lFrom)), szDelmit);
                    break;
                case    8:
                    fprintf(fp, "%lld%s", *((llong *)(pvResult + lOffset + pstKey->m_lFrom)), szDelmit);
                    break;
                default:
                    break;
                }
                break;
            case FIELD_CHAR:
                fprintf(fp, "%.*s%s", (int)pstKey->m_lLen, pvResult + lOffset + pstKey->m_lFrom, szDelmit);
                break;
            default:
                break;
            }
        }
        fprintf(fp, "\n");
        fflush(fp);
    }
    IBPFree(pvResult);

    if(pszFile)
    {
        fprintf(stdout, "---(%ld) records unload ---\n", pstSavm->m_lEffect);
        fclose(fp);
    }
    else
        fprintf(stdout, "---(%ld) records selected, ep(%d)---\n", pstSavm->m_lEffect, pstSavm->m_lEType);

    return RC_SUCC;
}

/*************************************************************************************************
    description：execute SQL-Seque
    parameters：
    return：
        RC_SUCC                            --success
        RC_FAIL                            --failure
  *************************************************************************************************/
long    lParseSequece(SATvm *pstSavm, char *pszSQName, char *pszFiled, BOOL bRmt)
{
    long    lRet;
    ulong   ulSeque;

    if(strcasecmp(pszFiled, "nextval"))
    {
        pstSavm->m_lErrno = SQL_ERR_FIELD;
        return RC_FAIL;
    }

    if(bRmt)
        lRet = lTvmSelectSeque(pstSavm, pszSQName, &ulSeque);
    else 
        lRet = lSelectSeque(pstSavm, pszSQName, &ulSeque);
    if(RC_SUCC != lRet)
        return RC_FAIL;

    fprintf(stdout, "%s\n", pszFiled);
    fprintf(stdout, "%lu\n", ulSeque);

    return RC_SUCC;
}

/*************************************************************************************************
    description：Parse SQL decorative fields
    parameters：
    return：
        RC_SUCC                            --success
        RC_FAIL                            --failure
  *************************************************************************************************/
long    lParseAdorn(SATvm *pstSavm, char *pszAdorn, long lCount, TField *pstField)
{
    TField  *pfld = NULL;
    long    lNumber, i, j;
    char    szWord[64], szField[512], szOrder[512];

    if(0 == strlen(pszAdorn))
        return RC_SUCC;

    memset(szField, 0, sizeof(szField));
    memset(szOrder, 0, sizeof(szOrder));
    if(!strncasecmp(pszAdorn, "group by", 8))
    {
        pstSavm->lFind |= GROUP_BY;
        if(!strcasestr(pszAdorn, "order by"))
        {
            if(!sGetTruckValue(pszAdorn, "group by", NULL, true, szField, sizeof(szField)))
                return RC_FAIL;
            strimall(szField);
        }
        else
        {
            if(!sGetTruckValue(pszAdorn, "group by", "order by", true, szField, sizeof(szField)))
                return RC_FAIL;
            strimall(szField);

            if(!sGetTruckValue(pszAdorn, "order by", NULL, true, szOrder, sizeof(szOrder)))
                return RC_FAIL;
            sltrim(szOrder);
        }

        for(i = 0, lNumber = lgetstrnum(szField, ",") + 1; i < lNumber; i ++)
        {
            memset(szWord, 0, sizeof(szWord));
            strncpy(szWord, sgetvalue(szField, ",", i + 1), sizeof(szWord));
            if(NULL == (pfld = pPitchField(lCount, pstField, szWord)))
            {
                fprintf(stderr, "field (%s) undefine\n", szWord);
                return RC_FAIL;
            }

            vSetDecorate(&pstSavm->stUpdt, pfld->m_lLen, pfld->m_lFrom, GROUP_BY);
        }
        
        if(strlen(szOrder) > 0)    
        {
            for(i = 0, lNumber = lgetstrnum(szOrder, ",") + 1; i < lNumber; i ++)
            {
                memset(szWord, 0, sizeof(szWord));
                strncpy(szWord, sfieldvalue(szOrder, ",", i + 1), sizeof(szWord));
                if(NULL == (pfld = pPitchField(lCount, pstField, sfieldvalue(szWord, " ", 1))))
                {
                    fprintf(stderr, "field (%s) undefine\n", sfieldvalue(szWord, " ", 1));
                    return RC_FAIL;
                }

                for(j = 0; j < pstSavm->stUpdt.uFldcmp; j ++)
                {
                    if(pstSavm->stUpdt.stFdKey[j].uFldpos == pfld->m_lFrom)
                    {
                        if(!strcasecmp(sfieldvalue(szWord, " ", 2), "desc"))
                            pstSavm->stUpdt.stFdKey[j].uDecorate |= ORDER_DESC;
                        else
                            pstSavm->stUpdt.stFdKey[j].uDecorate |= ORDER_ASC;
                        break;
                    }
                }    
                
                if(j == pstSavm->stUpdt.uFldcmp)
                    return RC_FAIL;
            }
        }

        return RC_SUCC;
    }
    else if(!strncasecmp(pszAdorn, "order by", 8))
    {
        pstSavm->lFind |= ORDER_BY;
           if(!sGetTruckValue(pszAdorn, "order by", NULL, true, szOrder, sizeof(szOrder)))
               return RC_FAIL;
        sltrim(szOrder);

        for(i = 0, lNumber = lgetstrnum(szOrder, ",") + 1; i < lNumber; i ++)
        {
            memset(szWord, 0, sizeof(szWord));
            strncpy(szWord, sfieldvalue(szOrder, ",", i + 1), sizeof(szWord));
            if(NULL == (pfld = pPitchField(lCount, pstField, sfieldvalue(szWord, " ", 1))))
            {
                fprintf(stderr, "field (%s) undefine\n", sfieldvalue(szWord, " ", 1));
                return RC_FAIL;
            }

            if(!strcasecmp(sfieldvalue(szWord, " ", 2), "desc"))
                vSetDecorate(&pstSavm->stUpdt, pfld->m_lLen, pfld->m_lFrom, ORDER_DESC);
            else
                vSetDecorate(&pstSavm->stUpdt, pfld->m_lLen, pfld->m_lFrom, ORDER_ASC);
        }

        return RC_SUCC;
    }

    return RC_FAIL;
}

/*************************************************************************************************
    description：execute SQL-extreme
    parameters：
    return：
        RC_SUCC                            --success
        RC_FAIL                            --failure
 *************************************************************************************************/
long    _lExeExtreme(SATvm *pstSavm, TIndex *pstIndex, SQLFld *pstRoot, void *pvData, BOOL bRmt)
{
    long    lRet;
    TblKey  *pstKey = NULL;
    SQLFld  *pstNode = NULL;
    void    *pvResult = NULL;

    if(NULL == (pvResult = malloc(pstIndex->m_lRowSize)))
        return RC_FAIL;

    pstSavm->pstVoid = (void *)pvData;
    pstSavm->tblName = pstIndex->m_table;
    pstSavm->lSize   = pstIndex->m_lRowSize;
    if(bRmt)
        lRet = lTvmExtreme(pstSavm, pvResult);
    else
        lRet = lExtreme(pstSavm, pvResult);
    if(RC_SUCC != lRet)
    {
        IBPFree(pvResult);
        vDestroyFiled(pstRoot);
        fprintf(stderr, "extreme table (%s) failure, %s\n", pstIndex->m_szTable, 
            sGetTError(pstSavm->m_lErrno));
        return RC_FAIL;
    }
  
    for(pstNode = pstRoot; pstNode; pstNode = pstNode->pstNext)
        fprintf(stdout, "%s ", pstNode->m_stKey.m_szField);
    fprintf(stdout, "\n");

    for(pstNode = pstRoot; pstNode; pstNode = pstNode->pstNext)
    {
        pstKey = &pstNode->m_stKey;
        switch(pstKey->m_lAttr)
        {
        case FIELD_DOUB:
            switch(pstKey->m_lLen)
            {
            case    4:
                fprintf(stdout, "%f ", *((float *)(pvResult + pstKey->m_lFrom)));
                break;
            case    8:
                fprintf(stdout, "%f ", *((double *)(pvResult + pstKey->m_lFrom)));
                break;
            default:
                break;
            }
            break;
        case FIELD_LONG:
            switch(pstKey->m_lLen)
            {
            case    2:
                fprintf(stdout, "%d ", *((sint *)(pvResult + pstKey->m_lFrom)));
                break;
            case    4:
                fprintf(stdout, "%d ", *((int *)(pvResult + pstKey->m_lFrom)));
                break;
            case    8:
                fprintf(stdout, "%lld ", *((llong *)(pvResult + pstKey->m_lFrom)));
                break;
            default:
                break;
            }
            break;
        case FIELD_CHAR:
            fprintf(stdout, "%.*s ", (int)pstKey->m_lLen, (char *)(pvResult + pstKey->m_lFrom));
            break;
        default:
            break;
        }
    }

    fprintf(stdout, "\n");
    fflush(stdout);
    IBPFree(pvResult);
    fprintf(stdout, "---(%ld)records selected -ep(%d)--\n", pstSavm->m_lEffect, pstSavm->m_lEType);

    vDestroyFiled(pstRoot);
    return RC_SUCC;
}

/*************************************************************************************************
    description：Parse SQL-select fields
    parameters：
    return：
        RC_SUCC                            --success
        RC_FAIL                            --failure
  *************************************************************************************************/
long    _lParseSelect(SATvm *pstSavm, char *pszTable, char *pszField, char *pszWhere, char *pszFile, 
            char *pszDem, char *pszAdorn, BOOL bRmt)
{
    TIndex  stIndex;
    SQLFld  *pstRoot = NULL;
    void    *pvWhere =  NULL;
    TField  *pstField = NULL;
    size_t  lOut = 0, lRet;

    memset(&stIndex, 0, sizeof(TIndex));
    strncpy(stIndex.m_szPart, sgetvalue(pszTable, "@", 2), sizeof(stIndex.m_szPart));
    strncpy(stIndex.m_szTable, sgetvalue(pszTable, "@", 1), sizeof(stIndex.m_szTable));
    supper(stIndex.m_szTable);    
    if(0x00 == stIndex.m_szPart[0])
        strcpy(stIndex.m_szPart, sGetNode());

    if(!strcmp(stIndex.m_szTable, "SEQUENCE"))
        return lParseSequece(pstSavm, stIndex.m_szPart, pszField, bRmt);

    if(bRmt)
    {
        if(RC_SUCC != lTvmGetTblIndex(pstSavm, stIndex.m_szTable, stIndex.m_szPart, &stIndex))
            return RC_FAIL;

        if(RC_SUCC != lTvmGetTblField(pstSavm, stIndex.m_table, &lOut, &pstField))
            return RC_FAIL;
    }
    else
    {
        if(RC_SUCC != lGetTblIndex(pstSavm, stIndex.m_szTable, stIndex.m_szPart, &stIndex))
            return RC_FAIL;

        if(RC_SUCC != lGetTblField(stIndex.m_table, &lOut, &pstField))
            return RC_FAIL;

        if(RC_SUCC != lInitSATvm(pstSavm, stIndex.m_table))
           return RC_FAIL;
    }

    pstSavm->stCond.uFldcmp = 0;
    pstSavm->stUpdt.uFldcmp = 0;
    if(RC_SUCC != lParseAdorn(pstSavm, pszAdorn, lOut, pstField))
    {
        pstSavm->m_lErrno = SQL_SYNTX_ERR;
        goto ERR_SELECT;
    }

    if(NULL == (pvWhere = (char *)calloc(stIndex.m_lRowSize, sizeof(char))))
    {
        pstSavm->m_lErrno = MALLC_MEM_ERR;
        goto ERR_SELECT;
    }

    if(RC_SUCC != _lConditField(pstSavm, pszWhere, lOut, pstField, pvWhere))
    {
        pstSavm->m_lErrno = SQL_ERR_WHERE;
        goto ERR_SELECT;
    }

    if((!strcasecmp(pszField, "count(*)") || !strcasecmp(pszField, "count(1)")) && 
        !(pstSavm->lFind & GROUP_BY))
    {
        IBPFree(pstField);
        if(RC_SUCC != _lCountSelect(pstSavm, &stIndex, pvWhere, bRmt))
            goto ERR_SELECT;
        
        IBPFree(pvWhere);
        return RC_SUCC;
    }
    else if(!strncasecmp(pszField, "max(", 4) || !strncasecmp(pszField, "min(", 4))
    {
        if(RC_SUCC != _lExtremeField(pstSavm, pszField, lOut, pstField, &pstRoot))
        {
            pstSavm->m_lErrno = SQL_ERR_FIELD;
            goto ERR_SELECT;
        }

        if(RC_SUCC != _lExeExtreme(pstSavm, &stIndex, pstRoot, pvWhere, bRmt))
            goto ERR_SELECT;
        
        IBPFree(pstField);
        IBPFree(pvWhere);
        return RC_SUCC;
    }

    if(RC_SUCC != _lSelectField(pstSavm, pszField, lOut, pstField, &pstRoot))
    {
        pstSavm->m_lErrno = SQL_ERR_FIELD;
        goto ERR_SELECT;
    }

    IBPFree(pstField);
    if(pstSavm->lFind & GROUP_BY)
        lRet = _lExeGroup(pstSavm, &stIndex, pstRoot, pvWhere, bRmt);
    else
        lRet = _lExeSelect(pstSavm, &stIndex, pstRoot, pvWhere, pszFile, pszDem, bRmt);
    if(RC_SUCC != lRet)
        goto ERR_SELECT;

    IBPFree(pvWhere);
    vDestroyFiled(pstRoot);
    return RC_SUCC;

ERR_SELECT:
    IBPFree(pvWhere);
    IBPFree(pstField);
    vDestroyFiled(pstRoot);
    return RC_FAIL;
}

/**************************************************************************************************
    description：Parse SQL-select syntax
    parameters：
    return：
        RC_SUCC                            --success
        RC_FAIL                            --failure
 **************************************************************************************************/
long    _lSelectSyntax(SATvm *pstSavm, char *pszSQL, char *pszFile, char *pszDem, BOOL bRmt)
{
    char    szTable[512], *p = NULL;
    char    szWhere[512], szField[512], szAdorn[1024];

    memset(szWhere, 0, sizeof(szWhere));
    memset(szField, 0, sizeof(szField));
    memset(szTable, 0, sizeof(szTable));
    memset(szAdorn, 0, sizeof(szAdorn));

    if(!sGetTruckValue(pszSQL, "select ", "from ", true, szField, sizeof(szField)))
    {
        pstSavm->m_lErrno = SQL_FIELD_NIL;
        return RC_FAIL;
    }

    strimall(szField);
    if(!strlen(szField))
    {
        pstSavm->m_lErrno = SQL_FIELD_NIL;
        return RC_FAIL;
    }

    if(NULL != strcasestr(pszSQL, "where "))
    {
        if(!sGetTruckValue(pszSQL, "from ", "where", true, szTable, sizeof(szTable)))
        {
            pstSavm->m_lErrno = SQL_TABLE_NIL;
            return RC_FAIL;
        }

        if(!sGetTruckValue(pszSQL, "where ", NULL, true, szWhere, sizeof(szWhere)))
        {
            pstSavm->m_lErrno = SQL_TABLE_NIL;
            return RC_FAIL;
        }

        if((p = strcasestr(szWhere, "group")) || (p = strcasestr(szWhere, "order")))
        {
            strcpy(szAdorn, p);
            memset(p, 0, sizeof(szWhere) - (p - szWhere));
        }

        sltrim(szWhere);
        srtrim(szWhere);
        sUpperWord(szWhere, " AND ");
        DEL_TAIL_CHAR(szWhere, ';');
    }
    else
    {
        if(!sGetTruckValue(pszSQL, "from ", NULL, true, szTable, sizeof(szTable)))
        {
            pstSavm->m_lErrno = SQL_TABLE_NIL;
            return RC_FAIL;
        }
            
        sltrim(szTable);
        if((p = strcasestr(szTable, "group")) || (p = strcasestr(szTable, "order")))
        {
            strcpy(szAdorn, p);
            memset(p, 0, sizeof(szTable) - (p - szTable));
        }

        DEL_TAIL_CHAR(szTable, ';');
    }

    strimall(szTable);
    vSCRDebug("DEBUG:select field:[%s]", szField);
    vSCRDebug("DEBUG:select table:[%s]", szTable);
    vSCRDebug("DEBUG:select where:[%s]", szWhere);
    vSCRDebug("DEBUG:select adorn:[%s]", szAdorn);

    return _lParseSelect(pstSavm, szTable, szField, szWhere, pszFile, pszDem, szAdorn, bRmt);
}

/*************************************************************************************************
    description：Parse SQL-update fields
    parameters：
    return：
        RC_SUCC                            --success
        RC_FAIL                            --failure
  *************************************************************************************************/
long    _lParseUpdate(SATvm *pstSavm, char *pszTable, char *pszField, char *pszWhere, BOOL bRmt)
{
    TIndex  stIndex;
    size_t  lOut = 0, lRet;
    TField  *pstField = NULL;
    void    *pvWhere =  NULL, *pvUpdate = NULL;

    memset(&stIndex, 0, sizeof(TIndex));
    strncpy(stIndex.m_szPart, sgetvalue(pszTable, "@", 2), sizeof(stIndex.m_szPart));
    strncpy(stIndex.m_szTable, sgetvalue(pszTable, "@", 1), sizeof(stIndex.m_szTable));
    supper(stIndex.m_szTable);
    if(0x00 == stIndex.m_szPart[0])
        strcpy(stIndex.m_szPart, sGetNode());

    if(bRmt)
    {
        if(RC_SUCC != lTvmGetTblIndex(pstSavm, stIndex.m_szTable, stIndex.m_szPart, &stIndex))
            return RC_FAIL;

        if(RC_SUCC != lTvmGetTblField(pstSavm, stIndex.m_table, &lOut, &pstField))
            return RC_FAIL;
    }
    else
    {
        if(RC_SUCC != lGetTblIndex(pstSavm, stIndex.m_szTable, stIndex.m_szPart, &stIndex))
            return RC_FAIL;

        if(RC_SUCC != lGetTblField(stIndex.m_table, &lOut, &pstField))
            return RC_FAIL;

        if(RC_SUCC != lInitSATvm(pstSavm, stIndex.m_table))
            return RC_FAIL;
    }

    pstSavm->stCond.uFldcmp = 0;
    pstSavm->stUpdt.uFldcmp = 0;
    if(NULL == (pvUpdate = (char *)calloc(stIndex.m_lRowSize, sizeof(char))))
    {
        pstSavm->m_lErrno = MALLC_MEM_ERR;
        goto ERR_UPDATE;
    }

    if(RC_SUCC != _lUpdateField(pstSavm, pszField, lOut, pstField, pvUpdate))
    {
        pstSavm->m_lErrno = SQL_ERR_FIELD;
        goto ERR_UPDATE;
    }

    if(NULL == (pvWhere = (char *)calloc(stIndex.m_lRowSize, sizeof(char))))
    {
        pstSavm->m_lErrno = MALLC_MEM_ERR;
        goto ERR_UPDATE;
    }

    if(RC_SUCC != _lConditField(pstSavm, pszWhere, lOut, pstField, pvWhere))
    {
        pstSavm->m_lErrno = SQL_ERR_WHERE;
        goto ERR_UPDATE;
    }

    IBPFree(pstField);
    if(RC_SUCC != _lExeUpdate(pstSavm, &stIndex, pvUpdate, pvWhere, bRmt))
        goto ERR_UPDATE;

    IBPFree(pvWhere);
    IBPFree(pvUpdate);
    return RC_SUCC;

ERR_UPDATE:
    IBPFree(pvWhere);
    IBPFree(pvUpdate);
    IBPFree(pstField);
    return RC_FAIL;
}


/**************************************************************************************************
    description：Parse SQL-update syntax
    parameters：
    return：
        RC_SUCC                            --success
        RC_FAIL                            --failure
 **************************************************************************************************/
long    _lUpdateSyntax(SATvm *pstSavm, char *pszSQL, BOOL bRmt)
{
    char    szTable[MAX_FIELD_LEN];
    char    szWhere[1024], szField[1024];

    memset(szWhere, 0, sizeof(szWhere));
    memset(szField, 0, sizeof(szField));
    memset(szTable, 0, sizeof(szTable));

    if(!sGetTruckValue(pszSQL, "update ", "set ", true, szTable, sizeof(szTable)))
    {
        pstSavm->m_lErrno = SQL_TABLE_NIL;
        return RC_FAIL;
    }
    strimall(szTable);

    if(!strcasestr(pszSQL, "where"))
    {
        pstSavm->m_lErrno = SQL_WHERE_NIL;
        return RC_FAIL;
    }

    if(!sGetTruckValue(pszSQL, "set ", "where", true, szField, sizeof(szField)))
    {
        pstSavm->m_lErrno = SQL_FIELD_NIL;
        return RC_FAIL;
    }

    sltrim(szField); 
    srtrim(szField);
    if(!strlen(szField))
    {
        pstSavm->m_lErrno = SQL_FIELD_NIL;
        return RC_FAIL;
    }

    if(!sGetTruckValue(pszSQL, "where ", NULL, true, szWhere, sizeof(szWhere)))
    {
        pstSavm->m_lErrno = SQL_WHERE_NIL;
        return RC_FAIL;
    }

    sltrim(szWhere);
    srtrim(szWhere);
    sUpperWord(szWhere, " AND ");
    DEL_TAIL_CHAR(szWhere, ';');

    vSCRDebug("DEBUG:update field:[%s]", szField);
    vSCRDebug("DEBUG:update table:[%s]", szTable);
    vSCRDebug("DEBUG:update where:[%s]", szWhere);

    return _lParseUpdate(pstSavm, szTable, szField, szWhere, bRmt);
}

/*************************************************************************************************
    description：Parse SQL-delete fields
    parameters：
    return：
        RC_SUCC                            --success
        RC_FAIL                            --failure
  *************************************************************************************************/
long    _lParseDelete(SATvm *pstSavm, char *pszTable,  char *pszWhere, BOOL bRmt)
{
    TIndex  stIndex;
    size_t  lOut = 0, lRet;
    TField  *pstField = NULL;
    void    *pvWhere =  NULL;

    memset(&stIndex, 0, sizeof(TIndex));
    strncpy(stIndex.m_szPart, sgetvalue(pszTable, "@", 2), sizeof(stIndex.m_szPart));
    strncpy(stIndex.m_szTable, sgetvalue(pszTable, "@", 1), sizeof(stIndex.m_szTable));
    supper(stIndex.m_szTable);
    if(0x00 == stIndex.m_szPart[0])
        strcpy(stIndex.m_szPart, sGetNode());

    if(bRmt)
    {
        if(RC_SUCC != lTvmGetTblIndex(pstSavm, stIndex.m_szTable, stIndex.m_szPart, &stIndex))
            return RC_FAIL;

        if(RC_SUCC != lTvmGetTblField(pstSavm, stIndex.m_table, &lOut, &pstField))
            return RC_FAIL;
    }
    else
    {
        if(RC_SUCC != lGetTblIndex(pstSavm, stIndex.m_szTable, stIndex.m_szPart, &stIndex))
            return RC_FAIL;

        if(RC_SUCC != lGetTblField(stIndex.m_table, &lOut, &pstField))
            return RC_FAIL;

        if(RC_SUCC != lInitSATvm(pstSavm, stIndex.m_table))
            return RC_FAIL;
    }

    pstSavm->stCond.uFldcmp = 0;
    pstSavm->stUpdt.uFldcmp = 0;
    if(NULL == (pvWhere = (char *)calloc(stIndex.m_lRowSize, sizeof(char))))
    {
        pstSavm->m_lErrno = MALLC_MEM_ERR;
        goto ERR_DELETE;
    }

    if(RC_SUCC != _lConditField(pstSavm, pszWhere, lOut, pstField, pvWhere))
    {
        pstSavm->m_lErrno = SQL_ERR_WHERE;
        goto ERR_DELETE;
    }

    IBPFree(pstField);
    if(RC_SUCC != _lExeDelete(pstSavm, &stIndex, pvWhere, bRmt))
        goto ERR_DELETE;

    IBPFree(pvWhere);
    return RC_SUCC;

ERR_DELETE:
    IBPFree(pvWhere);
    IBPFree(pstField);
    return RC_FAIL;
}

/**************************************************************************************************
    description：Parse SQL-delete syntax
    parameters：
    return：
        RC_SUCC                            --success
        RC_FAIL                            --failure
 **************************************************************************************************/
long    _lDeleteSyntax(SATvm *pstSavm, char *pszSQL, BOOL bRmt)
{
    char    szTable[MAX_FIELD_LEN], szWhere[1024];

    memset(szWhere, 0, sizeof(szWhere));
    memset(szTable, 0, sizeof(szTable));
    if(strcasestr(pszSQL, " where "))
    {
        if(!sGetTruckValue(pszSQL, " from ", " where ", true, szTable, sizeof(szTable)))
        {
            pstSavm->m_lErrno = SQL_TABLE_NIL;
            return RC_FAIL;
        }
        strimall(szTable);

        if(!sGetTruckValue(pszSQL, " where ", NULL, true, szWhere, sizeof(szWhere)))
        {
            pstSavm->m_lErrno = SQL_WHERE_NIL;
            return RC_FAIL;
        }
        sltrim(szWhere);
        srtrim(szWhere);
        sUpperWord(szWhere, " AND ");
        DEL_TAIL_CHAR(szWhere, ';');
    }
    else
    {
        if(!sGetTruckValue(pszSQL, " from ", NULL, true, szTable, sizeof(szTable)))
        {
            pstSavm->m_lErrno = SQL_TABLE_NIL;
            return RC_FAIL;
        }
        DEL_TAIL_CHAR(szTable, ';');
        strimall(szTable);
    }    

    vSCRDebug("DEBUG:delete table:[%s]", szTable);
    vSCRDebug("DEBUG:delete where:[%s]", szWhere);

    return _lParseDelete(pstSavm, szTable, szWhere, bRmt);
}

/*************************************************************************************************
    description：Parse SQL-insert values
    parameters：
    return：
        RC_SUCC                            --success
        RC_FAIL                            --failure
  *************************************************************************************************/
long    _lInsertField(SATvm *pstSavm, char *pszValues, SQLFld *pstRoot, void *pvInsert)
{
    void    *v = NULL;
    TblKey    *pstKey = NULL;
    SQLFld    *pstNode = NULL;
    long    lRec = 0, i, j, lFld;
    char    szValue[512], szNumber[16];

    memset(szNumber, 0, sizeof(szNumber));
    lFld = lGetNodeFiled(pstRoot);
    if(lFld != (lRec = lfieldnum(pszValues, ",")))
    {
        pstSavm->m_lErrno = SQL_FAV_MATCH;
        return RC_FAIL;
    }
    
    for(i = 0, v = (void *)szNumber, pstNode = pstRoot; i < lRec; i ++, pstNode = pstNode->pstNext)
    {
        memset(szValue, 0, sizeof(szValue));
        strncpy(szValue, sfieldvalue(pszValues, ",", i + 1), sizeof(szValue));
        sltrim(szValue);
        srtrim(szValue);
        strimabout(szValue, "\'", "\'");

        if(!pstNode)
        {
            pstSavm->m_lErrno = SQL_FAV_MATCH;
            return RC_FAIL;
        }

        pstKey = &pstNode->m_stKey;
        switch(pstKey->m_lAttr)
        {
        case FIELD_DOUB:
            switch(pstKey->m_lLen)
            {
            case    4:
                *((float *)v) = atof(szValue);
                memcpy(pvInsert + pstKey->m_lFrom, v, pstKey->m_lLen);
                break;
            case    8:
                *((double *)v) = atof(szValue);
                memcpy(pvInsert + pstKey->m_lFrom, v, pstKey->m_lLen);
                break;
            default:
                break;
            }
            break;
        case FIELD_LONG:
            switch(pstKey->m_lLen)
            {
            case    2:
                *((sint *)v) = atoi(szValue);
                memcpy(pvInsert + pstKey->m_lFrom, v, pstKey->m_lLen);
                break;
            case    4:
                *((int *)v) = atoi(szValue);
                memcpy(pvInsert + pstKey->m_lFrom, v, pstKey->m_lLen);
                break;
            case    8:
                *((llong *)v) = atol(szValue);
                memcpy(pvInsert + pstKey->m_lFrom, v, pstKey->m_lLen);
                break;
            default:
                break;
            }
            break;
        case FIELD_CHAR:
            memcpy(pvInsert + pstKey->m_lFrom, szValue,
                MIN(strlen(szValue), pstKey->m_lLen));
            break;
        default:
            break;
        }
    }

    if(pstNode)
    {
        pstSavm->m_lErrno = SQL_FAV_MATCH;
        return RC_FAIL;
    }

    return RC_SUCC;
}

/*************************************************************************************************
    description：Parse SQL-insert fields
    parameters：
    return：
        RC_SUCC                            --success
        RC_FAIL                            --failure
  *************************************************************************************************/
long    _lParseInsert(SATvm *pstSavm, char *pszTable, char *pszField, char *pszValues, BOOL bRmt)
{
    TIndex  stIndex;
    size_t  lOut = 0, lRet;
    SQLFld  *pstRoot = NULL;
    TField  *pstField = NULL;
    void    *pvInsert =  NULL;

    memset(&stIndex, 0, sizeof(TIndex));
    strncpy(stIndex.m_szPart, sgetvalue(pszTable, "@", 2), sizeof(stIndex.m_szPart));
    strncpy(stIndex.m_szTable, sgetvalue(pszTable, "@", 1), sizeof(stIndex.m_szTable));
    supper(stIndex.m_szTable);    
    if(0x00 == stIndex.m_szPart[0])
        strcpy(stIndex.m_szPart, sGetNode());

    if(bRmt)
    {
        if(RC_SUCC != lTvmGetTblIndex(pstSavm, stIndex.m_szTable, stIndex.m_szPart, &stIndex))
            return RC_FAIL;

        if(RC_SUCC != lTvmGetTblField(pstSavm, stIndex.m_table, &lOut, &pstField))
            return RC_FAIL;
    }
    else
    {
        if(RC_SUCC != lGetTblIndex(pstSavm, stIndex.m_szTable, stIndex.m_szPart, &stIndex))
            return RC_FAIL;

        if(RC_SUCC != lGetTblField(stIndex.m_table, &lOut, &pstField))
            return RC_FAIL;

        if(RC_SUCC != lInitSATvm(pstSavm, stIndex.m_table))
            return RC_FAIL;
    }

    pstSavm->stCond.uFldcmp = 0;
    pstSavm->stUpdt.uFldcmp = 0;
    if(NULL == (pvInsert = (char *)calloc(stIndex.m_lRowSize, sizeof(char))))
    {
        pstSavm->m_lErrno = MALLC_MEM_ERR;
        goto ERR_INSERT;
    }

    if(RC_SUCC != _lSelectField(pstSavm, pszField, lOut, pstField, &pstRoot))
    {
        pstSavm->m_lErrno = SQL_ERR_FIELD;
        goto ERR_INSERT;
    }

    if(RC_SUCC != _lInsertField(pstSavm, pszValues, pstRoot, pvInsert))
        goto ERR_INSERT;

//vPrintHex(pvInsert, stIndex.m_lRowSize);    

    IBPFree(pstField);
    if(RC_SUCC != _lExeInsert(pstSavm, &stIndex, pvInsert, bRmt))
        goto ERR_INSERT;

    IBPFree(pvInsert);
    return RC_SUCC;

ERR_INSERT:
    IBPFree(pvInsert);
    IBPFree(pstField);
    return RC_FAIL;
}

/**************************************************************************************************
    description：Parse SQL-insert syntax
    parameters：
    return：
        RC_SUCC                            --success
        RC_FAIL                            --failure
 **************************************************************************************************/
long    _lInsertSyntax(SATvm *pstSavm, char *pszSQL, BOOL bRmt)
{
    char    szTable[MAX_FIELD_LEN];
    char    szValues[1024], szField[1024];

    memset(szField, 0, sizeof(szField));
    memset(szTable, 0, sizeof(szTable));
    memset(szValues, 0, sizeof(szValues));
    if(!sGetTruckValue(pszSQL, " into ", "values", true, szValues, sizeof(szValues)))
    {
        pstSavm->m_lErrno = SQL_TABLE_NIL;
        return RC_FAIL;
    }

    if(strstr(szValues, "("))    //    说明有选定字段
    {
        if(!sGetTruckValue(szValues, NULL, "(", true, szTable, sizeof(szTable)))
        {
            pstSavm->m_lErrno = SQL_TABLE_NIL;
            return RC_FAIL;
        }

        if(!sGetTruckValue(szValues, "(", ")", true, szField, sizeof(szField)))
        {
            pstSavm->m_lErrno = SQL_FIELD_NIL;
            return RC_FAIL;
        }

        if(!strlen(strimall(szField)))
        {
            pstSavm->m_lErrno = SQL_FIELD_NIL;
            return RC_FAIL;
        }
    }
    else
    {
        strcpy(szField, "*");
        strncpy(szTable, szValues, sizeof(szTable));
    }
    strimall(szTable);

    memset(szValues, 0, sizeof(szValues));
    if(!sGetTruckValue(pszSQL, "values", NULL, true, szValues, sizeof(szValues)))
    {
        pstSavm->m_lErrno = SQL_WHERE_NIL;
        return RC_FAIL;
    }

    if(!strimabout(szValues, "(", ")"))
    {
        pstSavm->m_lErrno = SQL_WHERE_NIL;
        return RC_FAIL;
    }

    if(!strlen(szValues))
    {
        pstSavm->m_lErrno = SQL_WHERE_NIL;
        return RC_FAIL;
    }

    sltrim(szValues);
    srtrim(szValues);
    sUpperWord(szValues, " AND ");

    vSCRDebug("DEBUG:insert field:[%s]", szField);
    vSCRDebug("DEBUG:insert table:[%s]", szTable);
    vSCRDebug("DEBUG:insert field:[%s]", szValues);

    return _lParseInsert(pstSavm, szTable, szField, szValues, bRmt);
}

/**************************************************************************************************
    description：Parse SQL-truncate syntax
    parameters：
    return：
        RC_SUCC                            --success
        RC_FAIL                            --failure
 **************************************************************************************************/
long    _lTruncateSyntax(SATvm *pstSavm, char *pszSQL, BOOL bRmt)
{
    long    lRet;
    TIndex  stIndex;
    char    szTable[MAX_FIELD_LEN];

    memset(szTable, 0, sizeof(szTable));
    memset(&stIndex, 0, sizeof(TIndex));
    if(!sGetTruckValue(pszSQL, " table ", NULL, true, szTable, sizeof(szTable)))
    {
        pstSavm->m_lErrno = SQL_TABLE_NIL;
        return RC_FAIL;
    }

    strimall(szTable);
    strncpy(stIndex.m_szPart, sgetvalue(szTable, "@", 2), sizeof(stIndex.m_szPart));
    strncpy(stIndex.m_szTable, sgetvalue(szTable, "@", 1), sizeof(stIndex.m_szTable));
    supper(stIndex.m_szTable);
    if(0x00 == stIndex.m_szPart[0])
        strcpy(stIndex.m_szPart, sGetNode());

    if(bRmt)
    {
        if(RC_SUCC != lTvmGetTblIndex(pstSavm, stIndex.m_szTable, stIndex.m_szPart, &stIndex))
            return RC_FAIL;

        pstSavm->tblName = stIndex.m_table;
    }
    else
    {
        if(RC_SUCC != lGetTblIndex(pstSavm, stIndex.m_szTable, stIndex.m_szPart, &stIndex))
            return RC_FAIL;

        if(NULL == (pstSavm = (SATvm *)pInitSATvm(stIndex.m_table)))
            return RC_FAIL;
    }

    pstSavm->lSize = stIndex.m_lRowSize;
    if(bRmt)
        lRet = lTvmTruncate(pstSavm, stIndex.m_table);
    else
        lRet = lTruncate(pstSavm, stIndex.m_table);
    if(RC_SUCC != lRet)
        return RC_FAIL;

    fprintf(stdout, "---(%ld) records deleted ---\n", pstSavm->m_lEffect);
    return RC_SUCC;
}

/**************************************************************************************************
    description：Parse SQL-drop syntax
    parameters：
    return：
        RC_SUCC                            --success
        RC_FAIL                            --failure
 **************************************************************************************************/
long    _lDropSyntax(SATvm *pstSavm, char *pszSQL, BOOL bRmt)
{
    long    lRet;
    TIndex  stIndex;
    char    szTable[MAX_FIELD_LEN];

    memset(szTable, 0, sizeof(szTable));
    memset(&stIndex, 0, sizeof(TIndex));
    if(!sGetTruckValue(pszSQL, " table ", NULL, true, szTable, sizeof(szTable)))
    {
        pstSavm->m_lErrno = SQL_TABLE_NIL;
        return RC_FAIL;
    }

    strimall(szTable);
    strncpy(stIndex.m_szPart, sgetvalue(szTable, "@", 2), sizeof(stIndex.m_szPart));
    strncpy(stIndex.m_szTable, sgetvalue(szTable, "@", 1), sizeof(stIndex.m_szTable));
    supper(stIndex.m_szTable);
    if(0x00 == stIndex.m_szPart[0])
        strcpy(stIndex.m_szPart, sGetNode());

    if(bRmt)
    {
        if(RC_SUCC != lTvmGetTblIndex(pstSavm, stIndex.m_szTable, stIndex.m_szPart, &stIndex))
            return RC_FAIL;

        pstSavm->tblName = stIndex.m_table;
    }
    else
    {
        if(RC_SUCC != lGetTblIndex(pstSavm, stIndex.m_szTable, stIndex.m_szPart, &stIndex))
            return RC_FAIL;

        if(NULL == (pstSavm = (SATvm *)pInitSATvm(stIndex.m_table)))
            return RC_FAIL;
    }

    pstSavm->lSize = stIndex.m_lRowSize;
    if(bRmt)
        lRet = lTvmDropTable(pstSavm, stIndex.m_table);
    else
        lRet = lDropTable(pstSavm, stIndex.m_table);
    if(RC_SUCC != lRet)
        return RC_FAIL;

    fprintf(stdout, "---(%s) was deleted ---\n", szTable);

    return RC_SUCC;
}

/**************************************************************************************************
    description：Parse SQL-load syntax
    parameters：
    return：
        RC_SUCC                            --success
        RC_FAIL                            --failure
 **************************************************************************************************/
long    _lLoadSyntax(SATvm *pstSavm, char *pszSQL)
{
    TIndex  stIndex;
    char    szFile[256], szParam[256], szDelmit[64];

//    load from a.txt DELIMITER ',' insert into tablename@bcs 
    memset(szFile, 0, sizeof(szFile));
    memset(szParam, 0, sizeof(szParam));
    memset(szDelmit, 0, sizeof(szDelmit));
    memset(&stIndex, 0, sizeof(TIndex));

    if(!sGetTruckValue(pszSQL, "insert into ", NULL, true, szParam, sizeof(szParam)))
    {
        pstSavm->m_lErrno = SQL_TABLE_NIL;
        return RC_FAIL;
    }

    sltrim(szParam);
    srtrim(szParam);
    strimall(szParam);
    strncpy(stIndex.m_szPart, sgetvalue(szParam, "@", 2), sizeof(stIndex.m_szPart));
    strncpy(stIndex.m_szTable, sgetvalue(szParam, "@", 1), sizeof(stIndex.m_szTable));
    supper(stIndex.m_szTable);
    if(0x00 == stIndex.m_szPart[0])
        strcpy(stIndex.m_szPart, sGetNode());

    if(RC_SUCC != lGetTblIndex(pstSavm, stIndex.m_szTable, stIndex.m_szPart, &stIndex))
        return RC_FAIL;

    memset(szParam, 0, sizeof(szParam));
    if(!sGetTruckValue(pszSQL, "load from ", "insert", true, szParam, sizeof(szParam)))
    {
        pstSavm->m_lErrno = FILE_NOT_RSET;
        return RC_FAIL;
    }

    strncpy(szFile, sfieldvalue(szParam, " ", 1), sizeof(szFile));
    strimall(szFile);

    strncpy(szDelmit, sfieldvalue(szParam, " ", 2), sizeof(szDelmit));
    strimall(szDelmit);
    if(!strcasecmp(szDelmit, "DELIMITER"))
    {
        strncpy(szDelmit, sfieldvalue(szParam, " ", 3), sizeof(szDelmit));
           strimall(szDelmit);
        strimabout(szDelmit, "\"", "\"");
        strimabout(szDelmit, "\'", "\'");
    }
    else 
        strcpy(szDelmit, ",");

    if(strlen(sfieldvalue(szParam, " ", 4)))
    {
        pstSavm->m_lErrno = SQL_SYNTX_ERR;
        return RC_FAIL;
    }

    if(RC_SUCC != lImportFile(stIndex.m_table, szFile, szDelmit))
        return RC_FAIL;

    fprintf(stdout, "---(%ld) records inserted ---\n", pstSavm->m_lEffect);

    return RC_SUCC;
}

/**************************************************************************************************
    description：Parse SQL-unload syntax
    parameters：
    return：
        RC_SUCC                            --success
        RC_FAIL                            --failure
 **************************************************************************************************/
long    _lUnloadSyntax(SATvm *pstSavm, char *pszSQL, BOOL bRmt)
{
    char    szFile[256], szParam[256], szDelmit[64], *p = NULL;

//    unload to a.txt DELIMITER ',' select * from tablename@bcs 
    memset(szFile, 0, sizeof(szFile));
    memset(szParam, 0, sizeof(szParam));
    memset(szDelmit, 0, sizeof(szDelmit));

    if(NULL == (p = strcasestr(pszSQL, "select")))
    {
        pstSavm->m_lErrno = SQL_SYNTX_ERR;
        return RC_FAIL;
    }

    memset(szParam, 0, sizeof(szParam));
    if(!sGetTruckValue(pszSQL, "unload to ", "select", true, szParam, sizeof(szParam)))
    {
        pstSavm->m_lErrno = FILE_NOT_RSET;
        return RC_FAIL;
    }

    sltrim(szParam);
    srtrim(szParam);
    strncpy(szFile, sfieldvalue(szParam, " ", 1), sizeof(szFile));
    strimall(szFile);

    strncpy(szDelmit, sfieldvalue(szParam, " ", 2), sizeof(szDelmit));
    strimall(szDelmit);
    if(!strcasecmp(szDelmit, "DELIMITER"))
    {
        strncpy(szDelmit, sfieldvalue(szParam, " ", 3), sizeof(szDelmit));
        strimall(szDelmit);
        strimabout(szDelmit, "\"", "\"");
        strimabout(szDelmit, "\'", "\'");
    }
    else
        strcpy(szDelmit, ",");

    if(strlen(sfieldvalue(szParam, " ", 4)))
    {
        pstSavm->m_lErrno = SQL_SYNTX_ERR;
        return RC_FAIL;
    }

    return _lSelectSyntax(pstSavm, p, szFile, szDelmit, bRmt);
}

/**************************************************************************************************
    description：Parse and execute SQL statements
    parameters：
    return：
        RC_SUCC                            --success
        RC_FAIL                            --failure
 **************************************************************************************************/
long    lExecuteSQL(SATvm *pstSavm, char *pszSQL)
{
    if(!pszSQL || !strlen(pszSQL))
    {
        pstSavm->m_lErrno = SQL_SYNTX_ERR;
        return RC_FAIL;
    }

    pstSavm->lFind = 0;
    pstSavm->stCond.uFldcmp = 0;
    pstSavm->stUpdt.uFldcmp = 0;
    if(!strncasecmp(pszSQL, "begin ", 6) && strcmp(sfieldvalue(pszSQL, ";", 2), "work"))
    {
        vBeginWork(pstSavm);    
        fprintf(stdout, "---begin work, %s---\n", sGetTError(pstSavm->m_lErrno));
        return RC_SUCC;
    }

    else if(!strncasecmp(pszSQL, "end ", 4) && strcmp(sfieldvalue(pszSQL, ";", 2), "work"))
    {
        vEndWork(pstSavm);    
        fprintf(stdout, "---end work, %s---\n", sGetTError(pstSavm->m_lErrno));
        return RC_SUCC;
    }
    else if(!strncasecmp(pszSQL, "commit ", 7) && strcmp(sfieldvalue(pszSQL, ";", 2), "work"))
    {
        lCommitWork(pstSavm);    
        fprintf(stdout, "---commit work, %s---\n", sGetTError(pstSavm->m_lErrno));
        return RC_SUCC;
    }
    else if(!strncasecmp(pszSQL, "rollback ", 9) && strcmp(sfieldvalue(pszSQL, ";", 2), "work"))
    {
        lRollbackWork(pstSavm);    
        fprintf(stdout, "---(%ld) records rollback, %s---\n", pstSavm->m_lEffect, 
            sGetTError(pstSavm->m_lErrno));
        return RC_SUCC;
    }
    else if(!strncasecmp(pszSQL, "select ", 7))
        return _lSelectSyntax(pstSavm, pszSQL, NULL, NULL, false);
    else if(!strncasecmp(pszSQL, "update ", 7))
        return _lUpdateSyntax(pstSavm, pszSQL, false);
    else if(!strncasecmp(pszSQL, "delete ", 7))
        return _lDeleteSyntax(pstSavm, pszSQL, false);
    else if(!strncasecmp(pszSQL, "insert ", 7))
        return _lInsertSyntax(pstSavm, pszSQL, false);
    else if(!strncasecmp(pszSQL, "truncate ", 9))
        return _lTruncateSyntax(pstSavm, pszSQL, false);
    else if(!strncasecmp(pszSQL, "drop ", 5))
        return _lDropSyntax(pstSavm, pszSQL, false);
    else if(!strncasecmp(pszSQL, "load ", 5))
        return _lLoadSyntax(pstSavm, pszSQL);
    else if(!strncasecmp(pszSQL, "unload ", 7))
        return _lUnloadSyntax(pstSavm, pszSQL, false);
    else
    {
        pstSavm->m_lErrno = SQL_NOT_SUPPT;
        return RC_FAIL;
    }

    return RC_SUCC;
}

/**************************************************************************************************
    description：Parse and execute SQL statements
    parameters：
    return：
        RC_SUCC                            --success
        RC_FAIL                            --failure
 **************************************************************************************************/
long    lExecuteTvm(SATvm *pstSavm, char *pszSQL)
{
    if(!pszSQL || !strlen(pszSQL))
    {
        pstSavm->m_lErrno = SQL_SYNTX_ERR;
        return RC_FAIL;
    }

    pstSavm->m_lTimes = 0;
    pstSavm->m_lErrno = 0;
    sfieldreplace(pszSQL, '\t', ' ');
    if(!strncasecmp(pszSQL, "begin ", 6) && strcmp(sfieldvalue(pszSQL, ";", 2), "work"))
    {
        lTvmBeginWork(pstSavm);    
        fprintf(stdout, "---begin work, %s---\n", sGetTError(pstSavm->m_lErrno));
        return RC_SUCC;
    }
    else if(!strncasecmp(pszSQL, "end ", 4) && strcmp(sfieldvalue(pszSQL, ";", 2), "work"))
    {
        lTvmEndWork(pstSavm);    
        fprintf(stdout, "---end work, %s---\n", sGetTError(pstSavm->m_lErrno));
        return RC_SUCC;
    }
    else if(!strncasecmp(pszSQL, "commit ", 7) && strcmp(sfieldvalue(pszSQL, ";", 2), "work"))
    {
        lTvmCommitWork(pstSavm);    
        fprintf(stdout, "---commit work, %s---\n", sGetTError(pstSavm->m_lErrno));
        return RC_SUCC;
    }
    else if(!strncasecmp(pszSQL, "rollback ", 9) && strcmp(sfieldvalue(pszSQL, ";", 2), "work"))
    {
        lTvmRollbackWork(pstSavm);    
        fprintf(stdout, "---(%ld) records rollback, %s---\n", pstSavm->m_lEffect, 
            sGetTError(pstSavm->m_lErrno));
        return RC_SUCC;
    }
    else if(!strncasecmp(pszSQL, "select ", 7))
        return _lSelectSyntax(pstSavm, pszSQL, NULL, NULL, true);
    else if(!strncasecmp(pszSQL, "update ", 7))
        return _lUpdateSyntax(pstSavm, pszSQL, true);
    else if(!strncasecmp(pszSQL, "delete ", 7))
        return _lDeleteSyntax(pstSavm, pszSQL, true);
    else if(!strncasecmp(pszSQL, "insert ", 7))
        return _lInsertSyntax(pstSavm, pszSQL, true);
    else if(!strncasecmp(pszSQL, "truncate ", 9))
        return _lTruncateSyntax(pstSavm, pszSQL, true);
    else if(!strncasecmp(pszSQL, "drop ", 5))
        return _lDropSyntax(pstSavm, pszSQL, true);
/*
    else if(!strncasecmp(pszSQL, "load ", 5))
        return _lLoadSyntax(pstSavm, pszSQL, true);
*/
    else if(!strncasecmp(pszSQL, "unload ", 7))
        return _lUnloadSyntax(pstSavm, pszSQL, true);
    else
    {
        pstSavm->m_lErrno = SQL_NOT_SUPPT;
        return RC_FAIL;
    }

    return RC_SUCC;
    return RC_SUCC;
}

/**************************************************************************************************
    description：Boot STVM 
    parameters：
    return：
        RC_SUCC                            --success
        RC_FAIL                            --failure
 **************************************************************************************************/
long    lStartSystem(TBoot *pstBoot, char *pszMode)
{
    Benum   eMode = 0;
    SATvm   *pstSavm = (SATvm *)pGetSATvm();

    if(0 != access(getenv("TVMCFG"), R_OK))
    {
        fprintf(stdout, "The startup parameter is not set and started by default\n");
        if(RC_SUCC != lDefaultBoot())
            return RC_FAIL;
    }

    if(pszMode && !strcmp(pszMode, "o"))    //    offline
    {
        fprintf(stdout, "Warning:TVM will start offline\n");
        fflush(stdout);
        eMode = 1;
    }

    if(!bIsTvmBoot())
    {
       if(RC_SUCC != lStartupTvm(pstBoot))
       {
           fprintf(stderr, "failed to boot TVM, %s\n", sGetTError(pstSavm->m_lErrno));
           return RC_FAIL;
       }
    }

    if(RC_SUCC != lBootLocal(pstSavm, pstBoot, eMode))
    {
        fprintf(stderr, "failed to boot LIS, %s\n", sGetTError(pstSavm->m_lErrno));
        return RC_SUCC;
    }

    fprintf(stderr, "start TVM : (%s)\n", sGetTError(pstSavm->m_lErrno));

    return RC_SUCC;
}

/**************************************************************************************************
    description：Stop STVM
    parameters：
    return：
        RC_SUCC                            --success
        RC_FAIL                            --failure
 **************************************************************************************************/
long    lStopSystem(TBoot *pstBoot, char *pszApp)
{
    BOOL    bRet;
    long    i, lPid = 0;
    FILE    *fp = NULL;
    static  char    szCmd[128], szPid[20];
    SATvm   *pstSavm = (SATvm *)pGetSATvm();

    memset(szPid, 0, sizeof(szPid));
    memset(szCmd, 0, sizeof(szCmd));
    snprintf(szCmd, sizeof(szCmd), "ps -u %s|grep -E \"%s|%s\"|awk '{print $1}'", 
        getenv("LOGNAME"), TVM_LOCAL_SERV, TVM_REMOTE_DOM);

    if(TVM_BOOT_CLUSTER == pstBoot->m_lBootType)
        lOfflineNotify(pstSavm, pstBoot->m_lBootPort);

    if(NULL == (fp = popen(szCmd, "r")))
    {
        fprintf(stderr, "popen execute comman err:(%s)\n", strerror(errno));
        return RC_FAIL;
    }

    for(fp; fgets(szPid, sizeof(szPid), fp) != NULL; memset(szPid, 0, sizeof(szPid)))
    {
        strimall(szPid);
        lPid = atol(szPid);
        if(lPid <= 0 || lPid == getpid())
            continue;

        for(i = 0; i < 200; i ++, usleep(50000))
        {
            if(FALSE == (bRet = bExistProcess(lPid)))
                break;
        }

        if(!bRet)    continue;
        kill(atol(szPid), SIGKILL);
    }
    pclose(fp);

    if(!bIsTvmBoot()) return RC_SUCC;

    if(RC_SUCC != lShutdownTvm())
    {
        fprintf(stderr, "showdown node failed, %s\n", sGetTError(pstSavm->m_lErrno));
        return RC_FAIL;
    }

    return RC_SUCC;
}

/**************************************************************************************************
    description：Print Stvm parameter information
    parameters：
    return：
        RC_SUCC                            --success
        RC_FAIL                            --failure
 **************************************************************************************************/
void    vPrintParam(char *pszOpt)
{
    long    n = 0, i = 0;

    if(!pszOpt || (0 == (n = strlen(pszOpt))))  return ;

    if(!bIsTvmBoot())
        return ;

    for(i = 0, n = strlen(pszOpt); i < n; i ++)
    {
        switch(pszOpt[i])
        {
        case    'y':
            vPrintIndex();
            break;
        case    't':
            vPrintField();
            break;
        case    'd':
            vPrintDomain();
            break;
        default:
            return ;
        }
    }

    return ;
}

/**************************************************************************************************
    description：Output table space Usage
    parameters：
    return：
 **************************************************************************************************/
void    vPrintAmount(int t, char *pszTable, int nValid, int nMax)
{
    double  dPer;
    int     i, nPer;

    if(nValid < 0 || nMax <= 0)   return ;

    dPer = nValid * 100.0 / nMax;
    nPer = nValid * 50 / nMax > 0 ? nValid * 50 / nMax : 1;

    fprintf(stdout, "TABLE:[%3d][%-20s]: [", t, pszTable);
    if(dPer < 60.00)
        fprintf(stdout, "\033[42;32m");
    else if(dPer < 70.00)
        fprintf(stdout, "\033[45;35m");
    else if(dPer < 80.00)
        fprintf(stdout, "\033[46;36m");
    else  if(dPer < 90.00)
        fprintf(stdout, "\033[43;33m");
    else
        fprintf(stdout, "\033[41;31m");

    fflush(stdout);
    for(i = 0; i < nPer; i ++)
    {
        fprintf(stdout, "|");
        fflush(stdout);
    }

    fprintf(stdout, "\033[0m");
    for(i; i < 50; i ++)
        fprintf(stdout, " ");
    fprintf(stdout, "] %.4f%%, (%d/%d)\n", dPer, nValid, nMax);
    fflush(stdout);
}

/**************************************************************************************************
    description：print system table space usage
    parameters：
    return：
 **************************************************************************************************/
void    vTableAmount()
{
    size_t  i, lOut = 0;
    RunTime *pstRun = NULL;
    TIndex  stIndex, *pstIndex = NULL;
    SATvm   *pstSavm = (SATvm *)pGetSATvm();

    memset(&stIndex, 0, sizeof(TIndex));
    stIndex.m_lLocal = RES_LOCAL_SID;

    pstSavm->pstVoid = &stIndex;
    pstSavm->bSearch = TYPE_SYSTEM;
    pstSavm->tblName = SYS_TVM_INDEX;
    pstSavm->lSize   = sizeof(TIndex);
    if(RC_SUCC != lQuery(pstSavm, &lOut, (void *)&pstIndex))
    {
        if(NO_DATA_FOUND == pstSavm->m_lErrno)
            pstSavm->m_lErrno = TBL_NOT_FOUND;
        return ;
    }

    if(lOut <= 0)    return ;

    fprintf(stdout, "The amount of table is using as follows:\n\n");
    for(i = 0; i < lOut; i ++)
    {
        pstRun = (RunTime *)pGetRunTime(pstSavm, pstIndex[i].m_table);
        pstRun->m_shmID  = pstIndex[i].m_shmID;
        if(NULL == (pstRun = pInitHitTest(pstSavm, pstIndex[i].m_table)))
            continue;

        vPrintAmount(pstIndex[i].m_table, pstIndex[i].m_szTable, lGetTblValid(pstIndex[i].m_table),
             lGetTblRow(pstIndex[i].m_table));
        vTblDisconnect(pstSavm, pstIndex[i].m_table);
    }
    IBPFree(pstIndex);
    fprintf(stdout, "\n");

    return ;
}

/**************************************************************************************************
    description：STVM operation function description
    parameters：
    return：
 **************************************************************************************************/
void    vPrintFunc(char *s)
{
    fprintf(stdout, "\nUsage:\t%s -wspfco[oytd]\n", s);
    fprintf(stdout, "\t-w[o]\t\t--Startup STVM\n");
    fprintf(stdout, "\t-s[o]\t\t--Shutdown STVM\n");
    fprintf(stdout, "\t-p(ytd)\t\t--Output STVM Running information\n");
    fprintf(stdout, "\t-f\t\t--Display the amount of table\n");
    fprintf(stdout, "\t-c[file]\t--Compile the startup config file\n");
    fprintf(stdout, "\t-o[file]\t--Decompile the startup config\n");
    fprintf(stdout, "\n");
    fprintf(stdout, "\n");
    fprintf(stdout, "\033[4m\033[45;33mDESCRIPTION\033[0m\n");
    fprintf(stdout, "\t\033[0m\033[33;40mSQL\t\t--SQL control\033[0m\n");
    fprintf(stdout, "\t\033[0m\033[33;40mDOM\t\t--DOM control\033[0m\n");
    fprintf(stdout, "\n");
}

/*************************************************************************************************
    description：add history command
    parameters:
    return:
  *************************************************************************************************/
void   vAddHistory(char *s)
{
    FILE   *fp = NULL;
    char   szPath[512];

    memset(szPath, 0, sizeof(szPath));
    snprintf(szPath, sizeof(szPath), "%s/%s", getenv("TVMDBD"), STVM_SQL_LINE);
    if(NULL == (fp = fopen(szPath, "a+")))
        return ;

    fprintf(fp, "%s\n", s); 
    fclose(fp);
}

/*************************************************************************************************
    description：set user history command
    parameters:
    return:
  *************************************************************************************************/
void   vSetHistory()
{
    long   lRow = 0, i;
    FILE   *fp = NULL, *fb = NULL;
    char   szPath[512], szLine[512], szBak[512];

    memset(szBak, 0, sizeof(szBak));
    memset(szPath, 0, sizeof(szPath));
    memset(szLine, 0, sizeof(szLine));
    snprintf(szBak, sizeof(szBak), "%s/%s@", getenv("TVMDBD"), STVM_SQL_LINE);
    snprintf(szPath, sizeof(szPath), "%s/%s", getenv("TVMDBD"), STVM_SQL_LINE);
    if(NULL == (fp = fopen(szPath, "r")))
        return ;

    if(NULL == (fb = fopen(szBak, "w")))
    {
        fclose(fp);
        return ;
    }

    while(fgets(szLine, sizeof(szLine), fp) != NULL)
        lRow ++;

    fseek(fp, 0L, SEEK_SET);
    for(i = 0; i < lRow && fgets(szLine, sizeof(szLine), fp); i ++)
    {
        if(100 < lRow - i)
           continue; 
        fprintf(fb, "%s", szLine);
        strimcrlf(szLine);
        add_history(szLine);
        memset(szLine, 0, sizeof(szLine));
    }

    fclose(fp);
    fclose(fb);

    rename(szBak, szPath);
}

/**************************************************************************************************
    description：Execute SQL functions
    parameters：
    return：
 **************************************************************************************************/
void    vSQLStatement(int argc, char *argv[])
{
    char    *pszUser, *p, szSQL[2048];
    long    i = 0, lRec, lRemote = 0, lRet;
    SATvm   *pstSavm = (SATvm *)pGetSATvm();

    system("stty erase ^?");
    system("stty erase ^H");
    fprintf(stdout, "\n%s\n", sFuncVersion());
    for(i = 2; i < argc; i ++)
    {
        if(!strcasecmp(argv[i], "--debug=on"))
            elog = 1;
        else if(!strncasecmp(argv[i], "--connect=", 10))
        {
            if(NULL == (pszUser = strstr(argv[i], "--connect=")))
            {
                fprintf(stderr, "Invalid address:%s\n", argv[i]);
                exit(-1);
            }

            pszUser += 10;
            if(NULL == (p = strstr(pszUser, "@")))
            {
                fprintf(stderr, "Invalid domain:%s\n", pszUser);
                exit(-1);
            }

            vSetNode(sfieldvalue(pszUser, "@", 1));
            pszUser = pszUser + (p - pszUser) + 1;
            lRemote = atol(sfieldvalue(pszUser, ":", 2));
            if(RC_SUCC != lTvmConnect(pstSavm, sfieldvalue(pszUser, ":", 1), lRemote, 5))
            {
                fprintf(stderr, "connect to the server %s:%ld failure, %s\n", 
                    sfieldvalue(pszUser, ":", 1), lRemote, sGetTError(pstSavm->m_lErrno));
                exit(-1);
            }

            fprintf(stdout, "Connect domain:%s@%s:%ld server successfully!!\n\n", sGetNode(), 
                sfieldvalue(pszUser, ":", 1), lRemote);
            fflush(stdout);
        }
    }            

    vSetHistory();
    while(1)
    {
        if(NULL == (pszUser = readline("M-SQL>")))
            continue;

        if(!strcmp(pszUser, "q") || !strcmp(pszUser, "Q") || !strcmp(pszUser, "exit"))
            break;

        add_history(pszUser);
        vAddHistory(pszUser);
        if(!strcasecmp(pszUser, "clear"))
        {
            system("clear");
            IBPFree(pszUser);
            continue;
        }

        strimcrlf(pszUser);
        lRec = lfieldnum(pszUser, ";");
        for(i = 0; i < lRec; i ++)
        {
            memset(szSQL, 0, sizeof(szSQL));
            strncpy(szSQL, sfieldvalue(pszUser, ";", i + 1), sizeof(szSQL));
            sltrim(szSQL);
            srtrim(szSQL);
            if(!strlen(szSQL))    continue;

            sfieldreplace(szSQL, '\t', ' ');
            fprintf(stdout, "\n------------------------------------------------------SQL Result"
                "-----------------------------------------------------\n");
            if(lRemote > 0)
                lRet = lExecuteTvm(pstSavm, szSQL);
            else
                lRet = lExecuteSQL(pstSavm, szSQL);
            if(RC_SUCC != lRet)
            {
                fprintf(stderr, "execute M-SQL error, (%d)(%s)\n", pstSavm->m_lErrno, 
                    sGetTError(pstSavm->m_lErrno));
                continue;
            }
        }
        IBPFree(pszUser);
        fprintf(stdout, "\n\n\n");
    }

    if(lRemote)    vTvmDisconnect(pstSavm);
    exit(0);
}

/**************************************************************************************************
    description：Output domain table information
    parameters：
    return：
 **************************************************************************************************/
void    vPrintDomTable()
{
    size_t  i, lOut = 0;
    char    szPrint[512];
    TDomain *pstDomain = NULL;
    SATvm   *pstSavm = (SATvm *)pGetSATvm();

    if(RC_SUCC != lExportTable(SYS_TVM_DOMAIN, &lOut, (void **)&pstDomain))
    {
        fprintf(stderr, "get domain node info failed, %s\n", sGetTError(pstSavm->m_lErrno));
        return ;
    }

    fprintf(stdout, "\tROW    DOMAINID   TABLE_NAME         PART       STAT  PERS\n");
    fprintf(stdout, "\t----------------------------------------------------------\n");
    for(i = 0; i < lOut; i ++)
    {
        memset(szPrint, 0, sizeof(szPrint));
        snprintf(szPrint, sizeof(szPrint), "\t[%-3ld]: %-10s %-18s %-10s ", i, 
            pstDomain[i].m_szOwner, pstDomain[i].m_szTable, pstDomain[i].m_szPart);

        if(RESOURCE_INIT == pstDomain[i].m_lStatus)
            strcat(szPrint, "INIT ");
        else if(RESOURCE_EXCP == pstDomain[i].m_lStatus)
            strcat(szPrint, "EXCP ");
        else if(RESOURCE_ABLE == pstDomain[i].m_lStatus)
            strcat(szPrint, "ABLE ");
        else if(RESOURCE_STOP == pstDomain[i].m_lStatus)
            strcat(szPrint, "STOP ");
        else if(RESOURCE_AUTH == pstDomain[i].m_lStatus)
            strcat(szPrint, "AUTH ");

        fprintf(stdout, "%s %s\n", szPrint, sPermitConv(pstDomain[i].m_lPers));
        fflush(stdout);
    }
    IBPFree(pstDomain);
    fprintf(stdout, "\t----------------------------------------------------------\n");

    return ;
}

/**************************************************************************************************
    description：Manually connect to the remote domain
    parameters：
    return：
 **************************************************************************************************/
void    vConnectDomain(char *pszDomain, TBoot *pstBoot)
{
    TDomain stDomain;
    SATvm   *pstSavm = (SATvm *)pGetSATvm();

    if(!strlen(strimall(pszDomain)))
    {
        fprintf(stderr, "*illegal domain name\n");
        return ;
    }

    if(RC_SUCC != lInitSATvm(pstSavm, SYS_TVM_DOMAIN))
    {
        fprintf(stderr, "*initail SYS_TVM_DOMAIN failure, %s\n", sGetTError(pstSavm->m_lErrno));
        return ;
    }

    conditinit(pstSavm, stDomain, SYS_TVM_DOMAIN);
    stringsetv(pstSavm, stDomain, m_szOwner, pszDomain);
    decorate(pstSavm, TDomain, m_szOwner, FIRST_ROW);
    if(RC_SUCC != lSelect(pstSavm, (void *)&stDomain))
    {
        fprintf(stderr, "*get domain %s error, %s\n", pszDomain, sGetTError(pstSavm->m_lErrno));
        return ;
    }

    if(RC_SUCC != lConnectNotify(pstSavm, &stDomain, pstBoot->m_lBootPort))
    {
        fprintf(stderr, "*reconnect remote domain(%s) failure, %s\n", pszDomain, 
            sGetTError(pstSavm->m_lErrno));
        return ;
    }

    vPrintDomTable();
    return ;
}

/**************************************************************************************************
    description：Clone remote domain table data
    parameters：
    return：
 **************************************************************************************************/
void    vPullTableDomain(char *pszParam)
{
    TABLE   table = 0;
    TDomain stDomain;
    char    szCmd[512], szTable[128];
    SATvm   *pstSavm = (SATvm *)pGetSATvm();

//    BCS@TBL_ACCT_INFO/PART --table=1 
    memset(szCmd, 0, sizeof(szCmd));
    memset(szTable, 0, sizeof(szTable));
    if(NULL == strstr(pszParam, "/"))
    {
        fprintf(stderr, "*pull Invalid parameter:\"%s\"\n", szCmd);
        return ;
    }

    if(RC_SUCC != lInitSATvm(pstSavm, SYS_TVM_DOMAIN))
    {
        fprintf(stderr, "*initail SYS_TVM_DOMAIN failure, %s\n", sGetTError(pstSavm->m_lErrno));
        return ;
    }

    conditinit(pstSavm, stDomain, SYS_TVM_DOMAIN);
    stringsetv(pstSavm, stDomain, m_szOwner, sgetvalue(pszParam, "/", 1));
    if(!strlen(strimall(stDomain.m_szOwner)))
    {
        fprintf(stderr, "*illegal domain name\n");
        return ;
    }

    //    TBL_ACCT_INFO/PART --table=1
    strncpy(szCmd, sgetvalue(pszParam, "/", 2), sizeof(szCmd));
    strncpy(szTable, sgetvalue(szCmd, " ", 1), sizeof(szTable));
    strimall(szTable);

    stringsetv(pstSavm, stDomain, m_szTable, sgetvalue(szTable, "@", 1));
    supper(stDomain.m_szTable);
    if(!strlen(stDomain.m_szTable))
    {
        fprintf(stderr, "*Please input table name\n");
        return ;
    }

    stringsetv(pstSavm, stDomain, m_szPart, sgetvalue(szTable, "@", 2));
    if(!strlen(stDomain.m_szPart))
        strcpy(stDomain.m_szPart, stDomain.m_szOwner);

    memset(szTable, 0, sizeof(szTable));
    strncpy(szTable, sgetvalue(szCmd, " ", 2), sizeof(szTable));
    strimall(szTable);
    if(strlen(szTable) > 0)
    {
        if(!strncmp(szTable, "--table=", 8))
        {
            table = atol(szTable + 8);
            if(table <= 0 || table > TVM_MAX_TABLE)
            {
                fprintf(stderr, "*Local table set error \"%s\"\n", szTable + 8);
                return ;
            }
        }
        else
        {
            fprintf(stderr, "*command not supported \"%s\"\n", szTable);
            return ;
        }
    }

    if(RC_SUCC != lSelect(pstSavm, (void *)&stDomain))
    {
        fprintf(stderr, "*Select domain (%s)(%s) failure, %s\n", stDomain.m_szOwner, 
            stDomain.m_szTable, sGetTError(pstSavm->m_lErrno));
        return ;
    }

    if(table != 0)
        stDomain.m_table = table;
    else
        stDomain.m_table = stDomain.m_mtable;

    if(bTableIsExist(stDomain.m_table) || bPartIsExist(stDomain.m_szTable, pstSavm->m_szNode))
    {
        fprintf(stderr, "*Table number (%d) already exists\n", stDomain.m_table);
        return ;
    }

    if((RESOURCE_ABLE != stDomain.m_lStatus) || !(stDomain.m_lPers & OPERATE_SELECT))
    {   
        fprintf(stderr, "*(%s)(%s) Resources are not available\n", stDomain.m_szTable, stDomain.m_szPart);
        return ;
    }

    if(RC_SUCC != lPullNotify(pstSavm, &stDomain, 1))
    {
        fprintf(stderr, "*Cloning domain (%s) data failure, %s\n", stDomain.m_szOwner,
            sGetTError(pstSavm->m_lErrno));
        return ;
    }

    return ;
}

/**************************************************************************************************
    description：domain function
    parameters：
    return：
 **************************************************************************************************/
void    vPrintDomFunc()
{
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "    table\t\t--show remote tables\n");
    fprintf(stderr, "    connect $DOM\t--connect remote domain\n");
    fprintf(stderr, "    pull\t\t--pull remote table\n");
    fprintf(stderr, "    exp:pull domain/table_name@part --table=table\n");
    return ;
}

/**************************************************************************************************
    description：Remote domain control
    parameters：
    return：
 **************************************************************************************************/
void    vDomainCrontrl(TBoot *pstBoot)
{
    char    *p = NULL, *pszUser = NULL;
    SATvm   *pstSavm = (SATvm *)pGetSATvm();

    if(!bIsTvmBoot())
    {
        fprintf(stderr, "stvm has not been started yet!\n");
        exit(0);
    }

    system("stty erase ^?");
    system("stty erase ^H");
    fprintf(stdout, "\n%s\n", sFuncVersion());
    while(1)
    {
        if(NULL == (pszUser = readline("DOMAIN>")))
            continue;

        strimcrlf(pszUser);
        sltrim(pszUser);
        srtrim(pszUser);
        if(!strlen(pszUser))    continue;

        add_history(pszUser);
        
        if(!strcasecmp(pszUser, "help") || !strcasecmp(pszUser, "--help") || 
            !strcasecmp(pszUser, "-?"))
            vPrintDomFunc();
        else if(!strcasecmp(pszUser, "table"))
            vPrintDomTable();
        else if(!strncasecmp(pszUser, "connect", 7))
        {
            if(strncasecmp(pszUser, "connect ", 8))
            {
                fprintf(stderr, "Usage:\n\t--connect domain\n\n");
                IBPFree(pszUser);    
                continue;    
            }

            vConnectDomain(sgetvalue(pszUser, " ", 2), pstBoot);
        }
        else if(!strncasecmp(pszUser, "pull", 4))
        {
            if(strncasecmp(pszUser, "pull ", 5))
            {
                fprintf(stderr, "Usage:\n\tpull domain/table_name@part\n\n");
                IBPFree(pszUser);    
                continue;    
            }

            p = pszUser + 5;
            vPullTableDomain(p);
        }
        else if(!strcasecmp(pszUser, "q") || !strcasecmp(pszUser, "exit") || 
            !strcasecmp(pszUser, "quit"))
            exit(0);
        else
        {
            fprintf(stderr, "invalid option -- \"%s\"\n", pszUser);
            vPrintDomFunc();
        }

        fprintf(stdout, "\n");
        IBPFree(pszUser);    
    }

    exit(0);
}

/**************************************************************************************************
    description：Check the stvm operating environment variables
    parameters：
    return：
 **************************************************************************************************/
void    vCheckTvmEnv()
{
    int   nRet;

    if(!getenv("TVMCFG"))
    {
        fprintf(stderr, "Environment variable \"TVMCFG\" is not set\n");
        exit(-1);
    }

    if(!getenv("TVMDBD"))
    {
        fprintf(stderr, "Environment variable \"TVMDBD\" is not set\n");
        exit(-1);
    }

/*
    if(0 != access(getenv("TVMDBD"), F_OK | X_OK ))
    {
        fprintf(stderr, "\"TVMDBD\" directory isnot set yet!\n");
        exit(-1);
    }
*/

    mkdir(getenv("TVMDBD"), S_IRWXU | S_IRWXG | S_IROTH | S_IEXEC );
}

/**************************************************************************************************
    description：main(int argc, char *argv[])
    parameters：
    return：
        RC_SUCC                            --success
        RC_FAIL                            --failure
 **************************************************************************************************/
int     main(int argc, char *argv[])
{
    TABLE   table;
    char    szCom[256];
    SATvm   *pstSavm = (SATvm *)pGetSATvm();
    int     iChoose = 0, lAction = 0, lRet = 0;
    TBoot   *pstBoot = (TBoot *)pBootInitial();

    if(3 == argc && !strcmp(argv[1], "-c"))
        return lMakeConfig(argv[2]);
    else if(3 == argc && !strcmp(argv[1], "-o"))
        return lUnmakeConfig(argv[2]);

    memset(szCom, 0, sizeof(szCom));
    if(2 <= argc && (!strcasecmp(argv[1], "sql")))
        vSQLStatement(argc, argv);
    if(2 <= argc && (!strcasecmp(argv[1], "dom")))
        vDomainCrontrl(pstBoot);

    vCheckTvmEnv();
    memset(szCom, 0, sizeof(szCom));
    while(-1 != (iChoose = getopt(argc, argv, "w::s::p::f::i:c:o:v?::")))
    {
        switch(iChoose)
        {
        case    'w':
            vInitTitle(argc, argv, environ);
            return lStartSystem(pstBoot, optarg);
        case    's': 
            return lStopSystem(pstBoot, argv[0]);
        case    'p':
            vPrintParam(optarg);
            return RC_SUCC;
        case    'f':
            vTableAmount();
            return RC_SUCC;
        case    'i':
            table = atol(optarg);
            if(RC_SUCC != lRebuildIndex(pstSavm, table))
                fprintf(stderr, "rebuild index failure, %s\n", sGetTError(pstSavm->m_lErrno));
            return RC_SUCC;
        case    'v':
            fprintf(stdout, "%s\n", sGetTVMVers());
            fflush(stdout);
            return RC_SUCC;
        case    '?':
        default:
            break;
        }
    }
    
    vPrintFunc(basename(argv[0]));
    return RC_SUCC;
}

/**************************************************************************************************
 * code end
 **************************************************************************************************/
