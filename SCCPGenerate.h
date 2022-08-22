/********************************************************
 * Copyright (c) 2004-2022, Brick Technologies Co., Ltd
 * All rights reserved.
 * Home page:http://www.bricktou.com
********************************************************/
/**
 * Generate mysql c
 */
#pragma once

#include "SCCPCode.h"

namespace sccp {
class SCCPGenerate : public IGenerate,
                     private SCCPCode
{
public:
    int startGenerate(ITable *pTable);
    
    SCCPGenerate(IAccessSCCT *accessScct);
    ~SCCPGenerate(void){};
};//SCCPGenerate
}//sccp
