/********************************************************
 * Copyright (c) 2004-2022, Brick Technologies Co., Ltd
 * All rights reserved.
 * Home page:http://www.bricktou.com
********************************************************/
/**
 * Generate mysql c
 */
#pragma once

#define Brick "Brick Technologies Co., Ltd"

// Output report succeeded
#define PLI_OK      0

// Stop the program running
#define PLI_EXIT    -1

#define SEND_INFO(major, minor) \
    aScct->sendInfo(major, minor, __FILE__, __LINE__)

#define EXIT_INFO(major) do { \
    aScct->sendInfo(major, "", __FILE__, __LINE__); \
    return PLI_EXIT; \
}while (0)

#include "stdafx.h"

#include "IAccessSCCT.h"
#include "ISCCGenerate.h"

namespace sccp {
class SCCPBase {
public:
    /**
     * Handle of access SCCT
     */
    IAccessSCCT *aScct;
    
    bool filterColumn(IColumn *column);
    std::string getTypeByCol(IColumn *column);
    int getLenArrayByCol(IColumn *column);
    
    std::string codeToVarName(std::string &code);
    
    int errCount;
    int checkCode(ITable *pTable);
    int checkColCode(IColumn *column);
    
    void outputFileHead(ITable *pTable);
    void outputInclude();
    /**
     * Handle of report file
     */
    FILE *hf;
    int createFile(ITable *pTable);
    int createPath(const std::string &path);
    
    SCCPBase(void);
    
    void putIndent(const int indent = 4);
    /**
     * Auxiliary function to make the output code beautiful
     */
    size_t maxColumnLength;
    int initColumnLength(ITable *pTable);
private:
    int setColumnLength(IColumn *column);
    
    std::vector<std::string> split(const std::string &str, const std::string &sep);
    std::string upFirst(const std::string &str);
};
}//sccp
