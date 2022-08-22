/********************************************************
 * Copyright (c) 2004-2022, Brick Technologies Co., Ltd
 * All rights reserved.
 * Home page:http://www.bricktou.com
********************************************************/
/**
 * Generate mysql c
 */
#include "SCCPCode.h"

namespace sccp {
int SCCPCode::generateCode(ITable *pTable) {
    /**
     * Preprocessing checks can be ignored!
     */
    //if(checkCode(pTable)) return PLI_EXIT;
    
    /**
     * Auxiliary functions can be omitted.
     */
    initColumnLength(pTable);
    
    if (createFile(pTable)) return PLI_EXIT;
    
    outputFileHead(pTable);
    
    outputInclude();
    
    if (defStruct(pTable)) {
        fclose(hf);
        return PLI_EXIT;
    }
    
    if (genInsert(pTable)) {
        fclose(hf);
        return PLI_EXIT;
    }
    
    if (genUpdateByKey(pTable)) {
        fclose(hf);
        return PLI_EXIT;
    }
    
    if (genSelectByKey(pTable)) {
        fclose(hf);
        return PLI_EXIT;
    }
    
    if (genQuery(pTable)) {
        fclose(hf);
        return PLI_EXIT;
    }
    
    fclose(hf);
    return PLI_OK;
}
int SCCPCode::genInsert(ITable *pTable) {
    fprintf_s(hf, "/* Insert data to %s */\n",
        pTable->getName().empty() ?
        pTable->getCode().c_str() :
        pTable->getName().c_str());
    std::string code = codeToVarName(pTable->getCode());
    fprintf_s(hf, "int insert_%s(MYSQL *hMysql, ST_%s *par)\n{\n",
            code.c_str(), code.c_str());
    
    fprintf_s(hf, "    char *sql = \"Insert Into %s(\"\n",
            pTable->getCode().c_str());
    
    colCount = 0;
    funColumn fcol = std::bind(&SCCPCode::genInsertCol, this,
        std::placeholders::_1);
    if (pTable->for_column(fcol))
        return PLI_EXIT;
    
    if (!colCount) return PLI_EXIT;
    
    fputs("        \") values (\"\n", hf);
    for (int i = 0; i < colCount; i++) {
        if (0 == i % 10) {
            if (i) fputs("\"\n", hf);
            fputs("        \"", hf);
        }
        if (i) fputs(", ?", hf);
        else fputs("?", hf);
    }
    fputs(");\";\n\n", hf);
    
    fputs("    /* initialize stmt */\n", hf);
    fputs("    MYSQL_STMT *stmt = mysql_stmt_init(hMysql);\n", hf);
    fputs("    if (NULL == stmt) {\n", hf);
    fputs("        printf(\"stmt initialize:%s\\n\", mysql_error(hMysql));\n", hf);
    fputs("        return FUNC_FAIL;\n", hf);
    fputs("    }\n\n", hf);
    
    fputs("    /* prepare an INSERT statement */\n", hf);
    fputs("    int ret = mysql_stmt_prepare(stmt, sql, strlen(sql));\n", hf);
    fputs("    if (ret) {\n", hf);
    fputs("        printf(\"stmt prepare[%d]:%s\\n\", ret, mysql_error(hMysql));\n", hf);
    fputs("        mysql_stmt_close(stmt);\n", hf);
    fputs("        return ret;\n", hf);
    fputs("    }\n\n", hf);
    
    fputs("    /* Bind the data for all parameters */\n", hf);
    fprintf_s(hf, "    MYSQL_BIND params[%d];\n", colCount);
    fputs("    memset(&params, 0, sizeof(params));\n", hf);
    
    colCount = 0;
    funColumn fbind = std::bind(&SCCPCode::genInsertBind, this,
        std::placeholders::_1);
    if (pTable->for_column(fbind))
        return PLI_EXIT;
    
    if (!colCount) return PLI_EXIT;
    
    fputs("    ret = mysql_stmt_bind_param(stmt, params);\n", hf);
    fputs("    if (ret) {\n", hf);
    fputs("        printf(\"stmt bind param[%d]:%s\\n\", ret, mysql_error(hMysql));\n", hf);
    fputs("        mysql_stmt_close(stmt);\n", hf);
    fputs("        return ret;\n", hf);
    fputs("    }\n\n", hf);
    
    fputs("    /* Execute the INSERT statement */\n", hf);
    fputs("    ret = mysql_stmt_execute(stmt);\n", hf);
    fputs("    if (ret) {\n", hf);
    fputs("        printf(\"stmt execute[%d]:%s\\n\", ret, mysql_error(hMysql));\n", hf);
    fputs("        mysql_stmt_close(stmt);\n", hf);
    fputs("        return ret;\n", hf);
    fputs("    }\n\n", hf);
    
    fputs("    /* Get the total number of affected rows */\n", hf);
    fputs("    ret = mysql_stmt_affected_rows(stmt);\n", hf);
    fputs("    if (1 != ret) {\n", hf);
    fputs("        printf(\"stmt affected rows[%d]:%s\\n\", ret, mysql_error(hMysql));\n", hf);
    fputs("        mysql_stmt_close(stmt);\n", hf);
    fputs("        return FUNC_FAIL;\n", hf);
    fputs("    }\n\n", hf);
    
    fputs("    /* Close the statement */\n", hf);
    fputs("    mysql_stmt_close(stmt);\n", hf);
    fputs("    return FUNC_SUCCESS;\n", hf);
    fputs("}\n\n", hf);
    return PLI_OK;
}
int SCCPCode::genInsertCol(IColumn *column) {
    /*
     * mysql AUTO_INCREMENT or informix serial
     */
    if (column->isTimeStamp()
    || column->isAuto()) return PLI_OK;
    
    /*
     * Filter out inoperable types
     */
    if (filterColumn(column)) return PLI_OK;
    
    fprintf_s(hf, "                /* %s */\n",
            column->getName().c_str());
    if (colCount)
        fprintf_s(hf, "                \", %s\"\n",
            column->getCode().c_str());
    else fprintf_s(hf, "                \"  %s\"\n",
            column->getCode().c_str());
    colCount++;
    return PLI_OK;
}
int SCCPCode::genInsertBind(IColumn *column) {
    /*
     * mysql AUTO_INCREMENT or informix serial
     */
    if (column->isTimeStamp()
    || column->isAuto()) return PLI_OK;
    
    /*
     * Filter out inoperable types
     */
    if (filterColumn(column)) return PLI_OK;
    
    if (genColumnBind(column, "params", "par->", colCount))
        return PLI_EXIT;
    
    fputs("\n", hf);
    colCount++;
    return PLI_OK;
}
int SCCPCode::genUpdateByKey(ITable *pTable) {
    fprintf_s(hf, "/* Update data to %s */\n",
        pTable->getName().empty() ?
        pTable->getCode().c_str() :
        pTable->getName().c_str());
    std::string code = codeToVarName(pTable->getCode());
    fprintf_s(hf, "int update_%s_ByKey(MYSQL *hMysql, ST_%s *par)\n{\n",
            code.c_str(), code.c_str());
    
    fprintf_s(hf, "    char *sql = \"Update %s Set\"\n",
            pTable->getCode().c_str());
    
    colCount = 0;
    keyCount = 0;
    funColumn fcol = std::bind(&SCCPCode::genUpdateCol, this,
        std::placeholders::_1);
    if (pTable->for_column(fcol))
        return PLI_EXIT;
    
    if (!colCount) return PLI_EXIT;
    
    if (keyCount) {
        fputs("        \" Where ", hf);
        keyCount = 0;
        funColumn fkey = std::bind(&SCCPCode::genWhereByKey, this,
            std::placeholders::_1);
        if (pTable->for_column(fkey))
            return PLI_EXIT;
        
        if (!keyCount) return PLI_EXIT;
    }
    fputs("        \";\";\n\n", hf);
    
    fputs("    /* initialize stmt */\n", hf);
    fputs("    MYSQL_STMT *stmt = mysql_stmt_init(hMysql);\n", hf);
    fputs("    if (NULL == stmt) {\n", hf);
    fputs("        printf(\"stmt initialize:%s\\n\", mysql_error(hMysql));\n", hf);
    fputs("        return FUNC_FAIL;\n", hf);
    fputs("    }\n\n", hf);
    
    fputs("    /* prepare an UPDATE statement */\n", hf);
    fputs("    int ret = mysql_stmt_prepare(stmt, sql, strlen(sql));\n", hf);
    fputs("    if (ret) {\n", hf);
    fputs("        printf(\"stmt prepare[%d]:%s\\n\", ret, mysql_error(hMysql));\n", hf);
    fputs("        mysql_stmt_close(stmt);\n", hf);
    fputs("        return ret;\n", hf);
    fputs("    }\n\n", hf);
    
    fputs("    /* Bind the data for all parameters */\n", hf);
    fprintf_s(hf, "    MYSQL_BIND params[%d];\n", colCount + keyCount);
    fputs("    memset(&params, 0, sizeof(params));\n", hf);
    
    
    colCount = 0;
    funColumn fbind = std::bind(&SCCPCode::genUpdateBind, this,
        std::placeholders::_1);
    if (pTable->for_column(fbind))
        return PLI_EXIT;
    
    if (!colCount) return PLI_EXIT;
    
    if (keyCount) {
        keyCount = 0;
        funColumn fkey = std::bind(&SCCPCode::genWhereBind, this,
            std::placeholders::_1);
        if (pTable->for_column(fkey))
            return PLI_EXIT;
        
        if (!keyCount) return PLI_EXIT;
    }
    
    fputs("    ret = mysql_stmt_bind_param(stmt, params);\n", hf);
    fputs("    if (ret) {\n", hf);
    fputs("        printf(\"stmt bind param[%d]:%s\\n\", ret, mysql_error(hMysql));\n", hf);
    fputs("        mysql_stmt_close(stmt);\n", hf);
    fputs("        return ret;\n", hf);
    fputs("    }\n\n", hf);
    
    fputs("    /* Execute the UPDATE statement */\n", hf);
    fputs("    ret = mysql_stmt_execute(stmt);\n", hf);
    fputs("    if (ret) {\n", hf);
    fputs("        printf(\"stmt execute[%d]:%s\\n\", ret, mysql_error(hMysql));\n", hf);
    fputs("        mysql_stmt_close(stmt);\n", hf);
    fputs("        return ret;\n", hf);
    fputs("    }\n\n", hf);
    
    fputs("    /* Get the total number of affected rows */\n", hf);
    fputs("    ret = mysql_stmt_affected_rows(stmt);\n", hf);
    fputs("    if (1 != ret) {\n", hf);
    fputs("        printf(\"stmt affected rows[%d]:%s\\n\", ret, mysql_error(hMysql));\n", hf);
    fputs("        mysql_stmt_close(stmt);\n", hf);
    fputs("        return FUNC_FAIL;\n", hf);
    fputs("    }\n\n", hf);
    
    fputs("    /* Close the statement */\n", hf);
    fputs("    mysql_stmt_close(stmt);\n", hf);
    fputs("    return FUNC_SUCCESS;\n", hf);
    fputs("}\n\n", hf);
    return PLI_OK;
}
int SCCPCode::genUpdateCol(IColumn *column) {
    /*
     * The primary key should not be modified.
     */
    if (column->isPrimaryKey()
    || column->isTimeStamp()
    || column->isAuto()) {
        keyCount++;
        return PLI_OK;
    }
    /*
     * Filter out inoperable types
     */
    if (filterColumn(column)) return PLI_OK;
    
    fprintf_s(hf, "                /* %s */\n",
            column->getName().c_str());
    std::string code = column->getCode();
    if (colCount)
        fprintf_s(hf, "                \", %s", code.c_str());
    else fprintf_s(hf, "                \"  %s", code.c_str());
    
    if(maxColumnLength) {
        for (size_t i = code.length(); i < maxColumnLength; i++)
            fputs(" ", hf);
    }
    fputs(" = ?\"\n", hf);
    colCount++;
    return PLI_OK;
}
int SCCPCode::genUpdateBind(IColumn *column) {
    /*
     * The primary key should not be modified.
     */
    if (column->isPrimaryKey()
    || column->isTimeStamp()
    || column->isAuto()) {
        keyCount++;
        return PLI_OK;
    }
    /*
     * Filter out inoperable types
     */
    if (filterColumn(column)) return PLI_OK;
    
    if (genColumnBind(column, "params", "par->", colCount))
        return PLI_EXIT;
    fputs("\n", hf);
    colCount++;
    return PLI_OK;
}
int SCCPCode::genSelectByKey(ITable *pTable) {
    fprintf_s(hf, "/* Select data from %s */\n",
        pTable->getName().empty() ?
        pTable->getCode().c_str() :
        pTable->getName().c_str());
    std::string code = codeToVarName(pTable->getCode());
    fprintf_s(hf, "int select_%s_ByKey(MYSQL *hMysql, ST_%s *par)\n{\n",
            code.c_str(), code.c_str());
    
    fputs("    char *sql = \"Select\"\n", hf);
    
    colCount = 0;
    keyCount = 0;
    funColumn fcol = std::bind(&SCCPCode::genSelectCol, this,
        std::placeholders::_1);
    if (pTable->for_column(fcol))
        return PLI_EXIT;
    
    if (!colCount) return PLI_EXIT;
    
    fprintf_s(hf, "        \" From %s \"\n",
            pTable->getCode().c_str());
    
    if (keyCount) {
        fputs("        \" Where ", hf);
        keyCount = 0;
        funColumn fkey = std::bind(&SCCPCode::genWhereByKey, this,
            std::placeholders::_1);
        if (pTable->for_column(fkey))
            return PLI_EXIT;
        
        if (!keyCount) return PLI_EXIT;
    }
    fputs("        \";\";\n\n", hf);
    
    fputs("    /* initialize stmt */\n", hf);
    fputs("    MYSQL_STMT *stmt = mysql_stmt_init(hMysql);\n", hf);
    fputs("    if (NULL == stmt) {\n", hf);
    fputs("        printf(\"stmt initialize:%s\\n\", mysql_error(hMysql));\n", hf);
    fputs("        return FUNC_FAIL;\n", hf);
    fputs("    }\n\n", hf);
    
    fputs("    /* prepare an SELECT statement */\n", hf);
    fputs("    int ret = mysql_stmt_prepare(stmt, sql, strlen(sql));\n", hf);
    fputs("    if (ret) {\n", hf);
    fputs("        printf(\"stmt prepare[%d]:%s\\n\", ret, mysql_error(hMysql));\n", hf);
    fputs("        mysql_stmt_close(stmt);\n", hf);
    fputs("        return ret;\n", hf);
    fputs("    }\n\n", hf);
    
    int resCount = colCount;
    if (keyCount) {
        fputs("    /* Bind the data for all parameters */\n", hf);
        fprintf_s(hf, "    MYSQL_BIND params[%d];\n", keyCount);
        fputs("    memset(&params, 0, sizeof(params));\n", hf);
        
        colCount = 0;
        keyCount = 0;
        funColumn fkey = std::bind(&SCCPCode::genWhereBind, this,
            std::placeholders::_1);
        if (pTable->for_column(fkey))
            return PLI_EXIT;
        
        if (!keyCount) return PLI_EXIT;
    
        fputs("    ret = mysql_stmt_bind_param(stmt, params);\n", hf);
        fputs("    if (ret) {\n", hf);
        fputs("        printf(\"stmt bind param[%d]:%s\\n\", ret, mysql_error(hMysql));\n", hf);
        fputs("        mysql_stmt_close(stmt);\n", hf);
        fputs("        return ret;\n", hf);
        fputs("    }\n\n", hf);
    }
    
    fputs("    /* Fetch result set meta information */\n", hf);
    fputs("    MYSQL_RES *mysqlres = mysql_stmt_result_metadata(stmt);\n", hf);
    fputs("    if (!mysqlres) {\n", hf);
    fputs("        printf(\"stmt result metadata:%s\\n\", mysql_error(hMysql));\n", hf);
    fputs("        mysql_stmt_close(stmt);\n", hf);
    fputs("        return ret;\n", hf);
    fputs("    }\n\n", hf);
    
    fputs("    /* Get total columns in the query */\n", hf);
    fputs("    ret = mysql_num_fields(mysqlres);\n", hf);
    fprintf_s(hf, "    if (%d != ret) {\n", resCount);
    fputs("        printf(\"num fields[%d]:%s\\n\", ret, mysql_error(hMysql));\n", hf);
    fputs("        mysql_free_result(mysqlres);\n", hf);
    fputs("        mysql_stmt_close(stmt);\n", hf);
    fputs("        return ret;\n", hf);
    fputs("    }\n\n", hf);
    
    fputs("    /* Execute the SELECT statement */\n", hf);
    fputs("    ret = mysql_stmt_execute(stmt);\n", hf);
    fputs("    if (ret) {\n", hf);
    fputs("        printf(\"stmt execute[%d]:%s\\n\", ret, mysql_error(hMysql));\n", hf);
    fputs("        mysql_free_result(mysqlres);\n", hf);
    fputs("        mysql_stmt_close(stmt);\n", hf);
    fputs("        return ret;\n", hf);
    fputs("    }\n\n", hf);
    
    fputs("    /* Bind the result buffers for all columns before fetching them */\n", hf);
    fprintf_s(hf, "    unsigned long length[%d];\n", resCount);
    fprintf_s(hf, "    my_bool is_null[%d];\n", resCount);
    fprintf_s(hf, "    MYSQL_BIND result[%d];\n", resCount);
    fputs("    memset(&result, 0, sizeof(result));\n", hf);
    
    colCount = 0;
    funColumn fres = std::bind(&SCCPCode::genResultBind, this,
        std::placeholders::_1);
    if (pTable->for_column(fres))
        return PLI_EXIT;
    
    if (!colCount) return PLI_EXIT;
    
    fputs("    /* Bind the result buffers */\n", hf);
    fputs("    ret = mysql_stmt_bind_result(stmt, result);\n", hf);
    fputs("    if (ret) {\n", hf);
    fputs("        printf(\"stmt bind result[%d]:%s\\n\", ret, mysql_error(hMysql));\n", hf);
    fputs("        mysql_free_result(mysqlres);\n", hf);
    fputs("        mysql_stmt_close(stmt);\n", hf);
    fputs("        return ret;\n", hf);
    fputs("    }\n\n", hf);
    
    fputs("    /* Now buffer all results to client */\n", hf);
    fputs("    ret = mysql_stmt_store_result(stmt);\n", hf);
    fputs("    if (ret) {\n", hf);
    fputs("        printf(\"stmt store result[%d]:%s\\n\", ret, mysql_error(hMysql));\n", hf);
    fputs("        mysql_free_result(mysqlres);\n", hf);
    fputs("        mysql_stmt_close(stmt);\n", hf);
    fputs("        return ret;\n", hf);
    fputs("    }\n\n", hf);
    
    fputs("    /* Fetch rows */\n", hf);
    fputs("    if (mysql_stmt_fetch(stmt)) {\n", hf);
    fputs("        mysql_free_result(mysqlres);\n", hf);
    fputs("        mysql_stmt_close(stmt);\n", hf);
    fputs("        return NOT_FOUND;\n", hf);
    fputs("    }\n\n", hf);
    
    fputs("    /* Free the prepared result metadata */\n", hf);
    fputs("    mysql_free_result(mysqlres);\n", hf);
    fputs("    /* Close the statement */\n", hf);
    fputs("    mysql_stmt_close(stmt);\n", hf);
    fputs("    return FUNC_SUCCESS;\n", hf);
    fputs("}\n\n", hf);
    return PLI_OK;
}
int SCCPCode::genSelectCol(IColumn *column) {
    /*
     * Filter out inoperable types
     */
    if (filterColumn(column)) return PLI_OK;
    
    fprintf_s(hf, "                /* %s */\n",
            column->getName().c_str());
    std::string code = column->getCode();
    if (colCount)
        fprintf_s(hf, "                \", %s\"\n",
            column->getCode().c_str());
    else fprintf_s(hf, "                \"  %s\"\n",
            column->getCode().c_str());
    
    if (column->isPrimaryKey()
    || column->isTimeStamp()
    || column->isAuto()) keyCount++;
    colCount++;
    return PLI_OK;
}
int SCCPCode::genResultBind(IColumn *column) {
    /*
     * Filter out inoperable types
     */
    if (filterColumn(column)) return PLI_OK;
    
    if (genColumnBind(column, "result", "par->", colCount)) return PLI_EXIT;
    fprintf_s(hf, "    result[%d].is_null = &is_null[%d];\n", colCount, colCount);
    fprintf_s(hf, "    result[%d].length = &length[%d];\n", colCount, colCount);
    fputs("\n", hf);
    colCount++;
    return PLI_OK;
}
int SCCPCode::genQuery(ITable *pTable) {
    fprintf_s(hf, "/* Query data from %s */\n",
        pTable->getName().empty() ?
        pTable->getCode().c_str() :
        pTable->getName().c_str());
    std::string code = codeToVarName(pTable->getCode());
    fprintf_s(hf, "int query_%s(MYSQL *hMysql, // Handle of Mysql\n",
            code.c_str());
    fputs("const char   *sqlQuery,  // Sql of query\n", hf);
    fputs("my_ulonglong *offset,    // Row offset\n", hf);
    fprintf_s(hf, "ST_%s *result)   // [PAGE_SIZE]\n{\n", code.c_str());
    
    fputs("    /* Execute query */\n", hf);
    fputs("    if (mysql_real_query(hMysql, sqlQuery, strlen(sqlQuery))) {\n", hf);
    fputs("        printf(\"Execute query:%s\\n\", mysql_error(hMysql));\n", hf);
    fputs("        return FUNC_FAIL;\n", hf);
    fputs("    }\n\n", hf);
    
    fputs("    /* Now buffer all results to client */\n", hf);
    fputs("    MYSQL_RES *res = mysql_store_result(hMysql);\n", hf);
    fputs("    if(!res) return NOT_FOUND;\n\n", hf);
    
    fputs("    /* Get total rows */\n", hf);
    fputs("    my_ulonglong rows = mysql_num_rows(res);\n", hf);
    fputs("    if (*offset >= rows) {\n", hf);
    fputs("        mysql_free_result(res);\n", hf);
    fputs("        return NOT_FOUND;\n", hf);
    fputs("    }\n\n", hf);
    
    fputs("    /* Scrolling row */\n", hf);
    fputs("    mysql_data_seek(res, *offset);\n\n", hf);
    
    fputs("    /* Get total columns in the query */\n", hf);
    fputs("    unsigned int cols = mysql_num_fields(res);\n\n", hf);
    
    fputs("    /* Get information for each column */\n", hf);
    fputs("    MYSQL_FIELD *fields = mysql_fetch_fields(res);\n\n", hf);
    
    fputs("    /* Fetch data */\n", hf);
    fputs("    MYSQL_ROW row;\n\n", hf);
    fputs("    for (int r = 0; (r < PAGE_SIZE)\n", hf);
    fputs("    && (row = mysql_fetch_row(res));\n", hf);
    fputs("    r++, (*offset)++) {\n", hf);
    
    /**
     * Fill the returned results according to the query information.
     * The query information is not known when the code is generated.
     * When the query information and order are known,
     * you can delete loops and judgment statements.
     */
    fputs("        for (int c = 0; c < cols; c++) {\n", hf);
    colCount = 0;
    funColumn fres = std::bind(&SCCPCode::genResultByField, this,
        std::placeholders::_1);
    if (pTable->for_column(fres))
        return PLI_EXIT;
    
    if (!colCount) return PLI_EXIT;
    fputs("        }\n", hf);
    
    fputs("    }\n\n", hf);
    
    fputs("    /* Free result */\n", hf);
    fputs("    mysql_free_result(res);\n", hf);
    fputs("    return FUNC_SUCCESS;\n", hf);
    fputs("}\n\n", hf);
    return PLI_OK;
}
int SCCPCode::genResultByField(IColumn *column) {
    /*
     * Filter out inoperable types
     */
    if (filterColumn(column)) return PLI_OK;
    
    if (colCount)
        fprintf_s(hf, "            else if (!strcmp(\"%s\", fields[c].name)\n",
            column->getCode().c_str());
    else fprintf_s(hf, "            if (!strcmp(\"%s\", fields[c].name)\n",
            column->getCode().c_str());
    fprintf_s(hf, "            || !strcmp(\"%s\", fields[c].org_name)) {\n",
            column->getCode().c_str());
    
    putIndent(16);
    fprintf_s(hf, "/* %s %s */\n", column->getName().empty() ?
            column->getCode().c_str() : column->getName().c_str(),
            column->getType().c_str());
    switch (column->typeKind())
    {
    case ischar:    // mysql char
    case isvarchar:
    case iswchar:
    case isdate:    // mysql date
    case istime:    // mysql time
    case isdatetime:// mysql datetime
    case istimestamp:// mysql timestamp
    case isenum:    // mysql enum
    case isunion:   // mysql set
        putIndent(16);
        fprintf_s(hf, "strcpy(result[r].%s, row[c]);\n",
            codeToVarName(column->getCode()).c_str());
        break;
    case isbyte3:   // mysql mediumint
    case isint:     // mysql int
    case isshort:   // mysql smallint
    case isyear:    // mysql year
    case isbyte:    // mysql tinyint
        putIndent(16);
        fprintf_s(hf, "result[r].%s = row[c] ? atoi(row[c]) : 0;\n",
            codeToVarName(column->getCode()).c_str());
        break;
    case islong:
    case islonglong:// mysql bigint
        putIndent(16);
        fprintf_s(hf, "result[r].%s = row[c] ? atol(row[c]) : 0;\n",
            codeToVarName(column->getCode()).c_str());
        break;
    case isdouble:  // mysql double
    case isdecimal: // mysql decimal
    case isfloat:   // mysql float
        putIndent(16);
        fprintf_s(hf, "result[r].%s = row[c] ? atof(row[c]) : 0;\n",
            codeToVarName(column->getCode()).c_str());
        break;
    default:
        putIndent(16);
        fputs("/* inoperable type */\n", hf);
        break;
    }
    fputs("            }\n", hf);
    colCount++;
    return PLI_OK;
}
int SCCPCode::genWhereByKey(IColumn *column) {
    if (!column->isPrimaryKey()
    && !column->isTimeStamp()
    && !column->isAuto()) return PLI_OK;
    
    std::string code = column->getCode();
    if (keyCount)
        fprintf_s(hf, "        \"   And %s",
            code.c_str());
    else fprintf_s(hf, "%s",
            code.c_str());
    
    if (maxColumnLength) {
        for(size_t i = code.length(); i < maxColumnLength; i++)
            fputs(" ", hf);
    }
    fputs(" = ?\"\n", hf);
    keyCount++;
    return PLI_OK;
}
int SCCPCode::genWhereBind(IColumn *column) {
    if (!column->isPrimaryKey()
    && !column->isTimeStamp()
    && !column->isAuto()) return PLI_OK;
    
    if (genColumnBind(column, "params", "par->", colCount + keyCount))
        return PLI_EXIT;
    fputs("\n", hf);
    keyCount++;
    return PLI_OK;
}
int SCCPCode::genColumnBind(IColumn *column, const char *varName,
const char *objName, const size_t seq, const int indent) {
    putIndent(indent);
    fprintf_s(hf, "/* %s %s */\n", column->getName().empty() ?
            column->getCode().c_str() : column->getName().c_str(),
            column->getType().c_str());
    switch (column->typeKind())
    {
    case ischar:    // mysql char
    case isvarchar:
    case iswchar:
        putIndent(indent);
        fprintf_s(hf, "%s[%d].buffer_type   = MYSQL_TYPE_STRING;\n",
                varName, seq);
        putIndent(indent);
        fprintf_s(hf, "%s[%d].buffer        = (char *)%s%s;\n",
                varName, seq, objName, codeToVarName(column->getCode()).c_str());
        putIndent(indent);
        fprintf_s(hf, "%s[%d].buffer_length = %d;\n",
                varName, seq, column->getPrecision());
        break;
    case isbyte3:   // mysql mediumint
    case isint:     // mysql int
        putIndent(indent);
        fprintf_s(hf, "%s[%d].buffer_type   = MYSQL_TYPE_LONG;\n",
                varName, seq);
        putIndent(indent);
        fprintf_s(hf, "%s[%d].buffer        = &%s%s;\n",
                varName, seq, objName, codeToVarName(column->getCode()).c_str());
        break;
    case islong:
    case islonglong:// mysql bigint
        putIndent(indent);
        fprintf_s(hf, "%s[%d].buffer_type   = MYSQL_TYPE_LONGLONG;\n",
                varName, seq);
        putIndent(indent);
        fprintf_s(hf, "%s[%d].buffer        = &%s%s;\n",
                varName, seq, objName, codeToVarName(column->getCode()).c_str());
        break;
    case isdouble:  // mysql double
    case isdecimal: // mysql decimal
        putIndent(indent);
        fprintf_s(hf, "%s[%d].buffer_type   = MYSQL_TYPE_DOUBLE;\n",
                varName, seq);
        putIndent(indent);
        fprintf_s(hf, "%s[%d].buffer        = &%s%s;\n",
                varName, seq, objName, codeToVarName(column->getCode()).c_str());
        break;
    case isfloat:   // mysql float
        putIndent(indent);
        fprintf_s(hf, "%s[%d].buffer_type   = MYSQL_TYPE_FLOAT;\n",
                varName, seq);
        putIndent(indent);
        fprintf_s(hf, "%s[%d].buffer        = &%s%s;\n",
                varName, seq, objName, codeToVarName(column->getCode()).c_str());
        break;
    case isdate:    // mysql date
    case istime:    // mysql time
    case isdatetime:// mysql datetime
    case istimestamp:// mysql timestamp
    case isenum:    // mysql enum
    case isunion:   // mysql set
        putIndent(indent);
        fprintf_s(hf, "%s[%d].buffer_type   = MYSQL_TYPE_STRING;\n",
                varName, seq);
        putIndent(indent);
        fprintf_s(hf, "%s[%d].buffer        = (char *)%s%s;\n",
                varName, seq, objName, codeToVarName(column->getCode()).c_str());
        putIndent(indent);
        fprintf_s(hf, "%s[%d].buffer_length = %d;\n",
                varName, seq, getLenArrayByCol(column));
        break;
    case isshort:   // mysql smallint
    case isyear:    // mysql year
        putIndent(indent);
        fprintf_s(hf, "%s[%d].buffer_type   = MYSQL_TYPE_SHORT;\n",
                varName, seq);
        putIndent(indent);
        fprintf_s(hf, "%s[%d].buffer        = &%s%s;\n",
                varName, seq, objName, codeToVarName(column->getCode()).c_str());
        break;
    case isbyte:    // mysql tinyint
        putIndent(indent);
        fprintf_s(hf, "%s[%d].buffer_type   = MYSQL_TYPE_TINY;\n",
                varName, seq);
        putIndent(indent);
        fprintf_s(hf, "%s[%d].buffer        = &%s%s;\n",
                varName, seq, objName, codeToVarName(column->getCode()).c_str());
        break;
    default:
        return PLI_EXIT;
        break;
    }
    return PLI_OK;
}
int SCCPCode::defStruct(ITable *pTable) {
    fprintf_s(hf, "/* Record of %s */\n", pTable->getName().c_str());
    fputs("typedef struct {\n", hf);
    
    errCount = 0;
    funColumn fcol = std::bind(&SCCPCode::defMember, this,
        std::placeholders::_1);
    if(pTable->for_column(fcol))
        EXIT_INFO("for_column fail");
    
    if(errCount) return PLI_EXIT;
    
    fprintf_s(hf, "}ST_%s;\n\n", codeToVarName(
            pTable->getCode()).c_str());
    return PLI_OK;
}
int SCCPCode::defMember(IColumn *column) {
    std::string code = column->getCode();
    /**
     * The column name contains wide characters.
     */
    for(size_t i = 0; i < code.length(); i++)
        if(code[i] & 0x80) return PLI_OK;
    
    std::string type = getTypeByCol(column);
    if(type.empty()) {
        SEND_INFO("Column type is null", (char*)column->getCode().c_str());
        return PLI_EXIT;
    }
    
    fprintf_s(hf, "    /* %s */\n", column->getName().c_str());
    fprintf_s(hf, "    %s %s", type.c_str(),
        codeToVarName(column->getCode()).c_str());
    int len = getLenArrayByCol(column);
    if(len > 0)
        fprintf_s(hf, "[%d]", len + 1);
    fputs(";\n", hf);
    return PLI_OK;
}
}//sccp
