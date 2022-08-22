/********************************************************
 * Copyright (c) 2004-2022, Brick Technologies Co., Ltd
 * All rights reserved.
 * Home page:http://www.bricktou.com
********************************************************/
/**
 * Generate mysql c
 */
#include "SCCPBase.h"

namespace sccp {
bool SCCPBase::filterColumn(IColumn *column) {
    switch (column->typeKind())
    {
    case isbyte:    // mysql tinyint
    case isshort:   // mysql smallint
    case isbyte3:   // mysql mediumint
    case isint:     // mysql int
    case islong:
    case islonglong:// mysql bigint
    case isfloat:   // mysql float
    case isdouble:  // mysql double
    case isdecimal: // mysql decimal
    case isdate:    // mysql date
    case istime:    // mysql time
    case isdatetime:// mysql datetime
    case istimestamp:// mysql timestamp
    case isyear:    // mysql year
    case ischar:    // mysql char
    case isvarchar:
    case iswchar:
    case isenum:    // mysql enum
    case isunion:   // mysql set
        return false;
        break;
    default:
        break;
    }
    return true;
}
std::string SCCPBase::getTypeByCol(IColumn *column) {
    switch (column->typeKind())
    {
    case isbyte:    // mysql tinyint
        if (column->isUnsigned()) return "unsigned char";
        return "char";
        break;
    case isshort:   // mysql smallint
        if (column->isUnsigned()) return "unsigned short";
        return "short";
        break;
    case isbyte3:   // mysql mediumint
    case isint:     // mysql int
    case islong:
        if (column->isUnsigned()) return "unsigned int";
        return "int";
        break;
    case islonglong:// mysql bigint
        if (column->isUnsigned()) return "unsigned long long";
        return "long long";
        break;
    case isfloat:   // mysql float
        return "float";
        break;
    case isdouble:  // mysql double
    case isdecimal: // mysql decimal
        return "double";
        break;
    case isdate:    // mysql date
    case istime:    // mysql time
    case isdatetime:// mysql datetime
    case istimestamp:// mysql timestamp
        return "char";
        break;
    case isyear:    // mysql year
        return "short";
        break;
    case ischar:    // mysql char
    case isvarchar:
    case iswchar:
    case isenum:    // mysql enum
    case isunion:   // mysql set
        return "char";
        break;
    default:
        break;
    }
    /**
     * The type can be converted according to the getType() value.
     */
    return "";
}
int SCCPBase::getLenArrayByCol(IColumn *column) {
    switch (column->typeKind())
    {
    case isdate:    // yyyy-mm-dd
        return 10;
        break;
    case istime:    // hh:mm:ss
        return 8;
        break;
    case isdatetime:// yyyy-mm-dd hh:mm:ss
        return 19;
        break;
    case istimestamp:// mysql timestamp
        return 30;
        break;
    case ischar:    // mysql char
    case isvarchar:
    case iswchar:
        return column->getPrecision();
        break;
    case isenum:    // mysql enum
        return 20;  // Maximum descriptor length +.
        break;
    case isunion:   // mysql set
        // Sum of all descriptor lengths plus number of descriptors.
        return 400;
        break;
    default:
        break;
    }
    return -1;
}
/**
 * Convert to variable name
 */
std::string SCCPBase::codeToVarName(std::string &code) {
    std::vector<std::string> arr = split(code, "_");
    if (arr.empty()) return upFirst(code);
    
    std::string res;
    for (size_t i = 0; i < arr.size(); i++)
        res += upFirst(arr[i]);
    return res;
}
int SCCPBase::checkCode(ITable *pTable) {
    /**
     * You can add inspection items according to the actual situation.
     */
    std::string chk = pTable->getCode();
    for (size_t i = 0; i < chk.length(); i++) {
        if (chk[i] & 0x80) {
            if (aScct->getRegLanguage().compare("Chinese"))
                MessageBox(NULL, TEXT("The table name contains wide characters, which is not suitable for C language variable naming!"), TEXT("Warning"), MB_OK | MB_ICONWARNING);
            else MessageBox(NULL, TEXT("表名称包含宽字符，不适合c语言变量命名!"), TEXT("警告"), MB_OK | MB_ICONWARNING);
            return PLI_EXIT;
        }
    }
    errCount = 0;
    funColumn fchk = std::bind(&SCCPBase::checkColCode, this,
        std::placeholders::_1);
    pTable->for_column(fchk);
    return errCount;
}
int SCCPBase::checkColCode(IColumn *column) {
    std::string chk = column->getCode();
    for (size_t i = 0; i < chk.length(); i++) {
        if (chk[i] & 0x80) {
            if (aScct->getRegLanguage().compare("Chinese"))
                MessageBox(NULL, TEXT("The column name contains wide characters, which is not suitable for C language variable naming!"), TEXT("Warning"), MB_OK | MB_ICONWARNING);
            else MessageBox(NULL, TEXT("列名称包含宽字符，不适合c语言变量命名!"), TEXT("警告"), MB_OK | MB_ICONWARNING);
            errCount++;
        }
    }
    return PLI_OK;
}
void SCCPBase::outputFileHead(ITable *pTable) {
    fputs("/********************************************************************\n", hf);
    time_t t = time(0);
    struct tm tt;
    localtime_s(&tt, &t);
    fprintf_s(hf, " * Copyright (c) <2004-%4d>, <%s>\n", tt.tm_year + 1900, Brick);
    fputs(" * All rights reserved.\n", hf);
    fputs(" * Home page:http://www.bricktou.com\n", hf);
    fputs(" * Generated by Brick - SCCT.\n", hf);
    char buf[MAX_PATH] = { 0 };
    strftime(buf, MAX_PATH, "%Y-%m-%d %H:%M:%S", &tt);
    fprintf_s(hf, " * Creation Date:%s\n", buf);
    fputs("********************************************************************/\n\n", hf);
    
    if (aScct->getRegLanguage().compare("Chinese")) {
        std::string name = pTable->getName();
        if (name.empty())
            fprintf_s(hf, "/**\n * The following code is generated from the\n * %s.\n",
                pTable->getCode().c_str());
        else fprintf_s(hf, "/**\n * The following code is generated from the\n * %s - %s.\n",
                pTable->getCode().c_str(), name.c_str());
        fputs(" * The code applies to mysql c projects.\n */\n\n", hf);
        
        fputs("/*********************usage method***********************************\n", hf);
        fputs(" * Clip the following code to embed in your project.\n", hf);
        fputs(" * Note: the related code of MySQL database has been verified.\n", hf);
        fputs(" * If you have any questions or suggestions,\n", hf);
        fputs(" * please email brick_com@126.cn inform.\n", hf);
    }
    else {
        std::string name = pTable->getName();
        if (name.empty())
            fprintf_s(hf, "/**\n * 以下代码依据%s生成\n",
                pTable->getCode().c_str());
        else fprintf_s(hf, "/**\n * 以下代码依据%s-%s生成\n",
                pTable->getCode().c_str(), name.c_str());
        fputs(" * 代码适用于mysql c项目.\n */\n\n", hf);
        
        fputs("/***********************使用方法*************************************\n", hf);
        fputs(" * 裁剪以下代码嵌入到你的项目中.\n", hf);
        fputs(" * 注：Mysql数据库相关代码已验证.\n", hf);
        fputs(" * 如有问题或建议请邮件brick_com@126.cn告知.\n", hf);
    }
    fputs("********************************************************************/\n\n", hf);
}
void SCCPBase::outputInclude() {
    fputs("#include \"stdio.h\"\n", hf);
    fputs("#include \"string.h\"\n", hf);
    fputs("#include \"mysql.h\"\n", hf);
    fputs("#include \"time.h\"\n\n", hf);
    
    if (aScct->getRegLanguage().compare("Chinese")) {
        fputs("/* Function executed successfully */\n", hf);
        fputs("#define FUNC_SUCCESS 0\n\n", hf);
        fputs("/* Function execution failed */\n", hf);
        fputs("#define FUNC_FAIL -1\n\n", hf);
        fputs("/* Not found data */\n", hf);
        fputs("#define NOT_FOUND 100\n\n", hf);
        fputs("/* Maximum number of records returned by query */\n", hf);
        fputs("#define PAGE_SIZE 30\n\n", hf);
    }
    else {
        fputs("/* 函数执行成功 */\n", hf);
        fputs("#define FUNC_SUCCESS 0\n\n", hf);
        fputs("/* 函数执行失败 */\n", hf);
        fputs("#define FUNC_FAIL -1\n\n", hf);
        fputs("/* 没有查询结果 */\n", hf);
        fputs("#define NOT_FOUND 100\n\n", hf);
        fputs("/* 查询返回最大记录数 */\n", hf);
        fputs("#define PAGE_SIZE 30\n\n", hf);
    }
}
int SCCPBase::createFile(ITable *pTable) {
    std::string fname = aScct->getOutRoot();
    std::string db = pTable->getDBCode();
    if (!db.empty())
        fname += db + "\\";
    
    if (createPath(fname)) return PLI_EXIT;
    
    fname += pTable->getCode() + ".c";
    
    if (NULL != hf) fclose(hf);
    
    fopen_s(&hf, fname.c_str(), "wt+");
    if (NULL == hf) {
        char buf[300] = { 0 };
        sprintf_s(buf, "Create %s fail!", fname.c_str());
        MessageBoxA(NULL, buf, "Message", MB_OK | MB_ICONWARNING);
        return PLI_EXIT;
    }
    
    return PLI_OK;
}
int SCCPBase::createPath(const std::string &path) {
    for (size_t i = 0; i < path.length(); i++) {
        if (('\\' != path[i]) && ('/' != path[i])) continue;
        if (!_access(path.substr(0, i).c_str(), 0)) continue;
        if (_mkdir(path.substr(0, i).c_str())) return PLI_EXIT;
    }
    return PLI_OK;
}
SCCPBase::SCCPBase(void) {
    hf = NULL;
    maxColumnLength = 0;
}
void SCCPBase::putIndent(const int indent) {
    for (int i = 0; i < indent; i++)
        fputs(" ", hf);
}
/**
 * Auxiliary function to make the output code beautiful.
 */
int SCCPBase::initColumnLength(ITable *pTable) {
    maxColumnLength = 0;
    funColumn flen = std::bind(&SCCPBase::setColumnLength, this,
        std::placeholders::_1);
    if (pTable->for_column(flen))
        return PLI_EXIT;
    return PLI_OK;
}
int SCCPBase::setColumnLength(IColumn *column) {
    size_t width = column->getCode().length();
    if (width > maxColumnLength)
        maxColumnLength = width;
    return PLI_OK;
}
std::vector<std::string> SCCPBase::split(const std::string &str,
 const std::string &sep)
{
    std::vector<std::string> res;
    if (sep.empty()) return res;
    size_t b = 0;
    for (size_t i = 0; i < str.length(); i++) {
        if (str[i] != sep[0]) continue;
        size_t j = 1;
        for (; (j < sep.length()) && (j + i < str.length()); j++)
            if (str[i + j] != sep[j]) break;
        if (j < sep.length()) continue;
        if (i > b)
            res.push_back(str.substr(b, i - b));
        b = i + sep.length();
        i = b - 1;
    }
    if (b && (str.length() > b))
        res.push_back(str.substr(b, str.length() - b));
    return res;
}
std::string SCCPBase::upFirst(const std::string &str) {
    std::string res = str;
    std::transform(res.begin(), res.end(), res.begin(), ::tolower);
    unsigned char c = res[0];
    if ((c >= 'a') && (c <= 'z'))
        res[0] = c - 32;
    return res;
}
}//sccp
