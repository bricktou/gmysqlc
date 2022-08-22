/********************************************************
 * Copyright (c) 2004-2022, Brick Technologies Co., Ltd
 * All rights reserved.
 * Home page:http://www.bricktou.com
********************************************************/
/**
 * Interface for generating code
 */
#ifndef _I_PLUGIN_GENERATE_H_
#define _I_PLUGIN_GENERATE_H_

// Output report succeeded
#ifndef PLI_OK
#define PLI_OK      0
#endif

// Stop the program running
#ifndef PLI_EXIT
#define PLI_EXIT    -1
#endif

namespace sccp {
struct IGenerate
{
    virtual int startGenerate(ITable *pTable) = 0;
};
}//sccp
#endif//_I_PLUGIN_GENERATE_H_
