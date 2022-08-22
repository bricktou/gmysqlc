/********************************************************
 * Copyright (c) 2004-2022, Brick Technologies Co., Ltd
 * All rights reserved.
 * Home page:http://www.bricktou.com
********************************************************/
/**
 * Generate mysql c
 */
#pragma once

#include "SCCPBase.h"

namespace sccp {
class SCCPCode : public SCCPBase {
public:
    int generateCode(ITable *pTable);
private:
    int colCount, keyCount;
    
    int genInsert(ITable *pTable);
    int genInsertCol(IColumn *column);
    int genInsertBind(IColumn *column);
    
    int genUpdateByKey(ITable *pTable);
    int genUpdateCol(IColumn *column);
    int genUpdateBind(IColumn *column);
    
    int genSelectByKey(ITable *pTable);
    int genSelectCol(IColumn *column);
    int genResultBind(IColumn *column);
    
    int genQuery(ITable *pTable);
    int genResultByField(IColumn *column);
    
    int genWhereByKey(IColumn *column);
    int genWhereBind(IColumn *column);
    
    int genColumnBind(IColumn *column,
        const char *varName, const char *objName,
        const size_t seq, const int indent = 4);
    
    int defStruct(ITable *pTable);
    int defMember(IColumn *column);
};
}//sccp
