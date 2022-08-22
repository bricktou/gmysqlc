/********************************************************
 * Copyright (c) 2004-2022, Brick Technologies Co., Ltd
 * All rights reserved.
 * Home page:http://www.bricktou.com
********************************************************/
/**
 * Generate mysql c
 */
#include "SCCPGenerate.h"

namespace sccp {
int SCCPGenerate::startGenerate(ITable *pTable) {
    return generateCode(pTable);
}
SCCPGenerate::SCCPGenerate(IAccessSCCT *accessScct) {
    aScct = accessScct;
}
}//sccp
