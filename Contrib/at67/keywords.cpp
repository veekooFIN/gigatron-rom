#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <cmath>
#include <ctime>
#include <fstream>
#include <algorithm>
#include <random>

#include "memory.h"
#include "cpu.h"
#include "loader.h"
#include "image.h"
#include "assembler.h"
#include "keywords.h"
#include "operators.h"
#include "linker.h"


#define FONT_WIDTH   6
#define FONT_HEIGHT  8
#define MAPPING_SIZE 96


namespace Keywords
{
    enum EmitStringResult {SyntaxError, InvalidStringVar, ValidStringVar};

    std::mt19937_64 _randGenerator;

    std::vector<std::string> _operators;
    std::map<std::string, Pragma> _pragmas;
    std::map<std::string, Keyword> _keywords;
    std::map<std::string, std::string> _functions;
    std::map<std::string, std::string> _stringKeywords;
    std::map<std::string, std::string> _equalsKeywords;


    std::vector<std::string>& getOperators(void)                {return _operators;     }
    std::map<std::string, Pragma>& getPragmas(void)             {return _pragmas;       }
    std::map<std::string, Keyword>& getKeywords(void)           {return _keywords;      }
    std::map<std::string, std::string>& getFunctions(void)      {return _functions;     }
    std::map<std::string, std::string>& getStringKeywords(void) {return _stringKeywords;}
    std::map<std::string, std::string>& getEqualsKeywords(void) {return _equalsKeywords;}


    bool initialise(void)
    {
        // Operators
        _operators.push_back(" AND ");
        _operators.push_back(" XOR ");
        _operators.push_back(" OR " );
        _operators.push_back(" NOT ");
        _operators.push_back(" MOD ");
        _operators.push_back(" LSL ");
        _operators.push_back(" LSR ");
        _operators.push_back(" ASR ");
        _operators.push_back("<<"   );
        _operators.push_back(">>"   );

        // Functions
        _functions["PEEK"]   = "PEEK";
        _functions["DEEK"]   = "DEEK";
        _functions["USR" ]   = "USR";
        _functions["RND" ]   = "RND";
        _functions["LEN" ]   = "LEN";
        _functions["GET" ]   = "GET";
        _functions["POS" ]   = "POS";
        _functions["ABS" ]   = "ABS";
        _functions["ASC" ]   = "ASC";
        _functions["SIN" ]   = "SIN";
        _functions["COS" ]   = "COS";
        _functions["TAN" ]   = "TAN";
        _functions["ASN" ]   = "ASN";
        _functions["ACS" ]   = "ACS";
        _functions["ATN" ]   = "ATN";
        _functions["EXP" ]   = "EXP";
        _functions["LOG" ]   = "LOG";
        _functions["FRE" ]   = "FRE";
        _functions["TIME"]   = "TIME";
        _functions["POINT"]  = "POINT";
        _functions["STRCMP"] = "POINT";

        // Pragmas
        _pragmas["_useOpcodeCALLI_"]     = {"_useOpcodeCALLI_",     pragmaUSEOPCODECALLI    };
        _pragmas["_runtimePath_"]        = {"_runtimePath_",        pragmaRUNTIMEPATH       };
        _pragmas["_runtimeStart_"]       = {"_runtimeStart_",       pragmaRUNTIMESTART      };
        _pragmas["_stringWorkArea_"]     = {"_stringWorkArea_",     pragmaSTRINGWORKAREA    };
        _pragmas["_codeOptimiseType_"]   = {"_codeOptimiseType_",   pragmaCODEOPTIMISETYPE  };
        _pragmas["_arrayIndiciesOne_"]   = {"_arrayIndiciesOne_",   pragmaARRAYINDICIESONE  };
        _pragmas["_spriteStripeChunks_"] = {"_spriteStripeChunks_", pragmaSPRITESTRIPECHUNKS};

        // Keywords
        _keywords["END"     ] = {"END",      keywordEND,      Compiler::SingleStatementParsed};
        _keywords["INC"     ] = {"INC",      keywordINC,      Compiler::SingleStatementParsed};
        _keywords["DEC"     ] = {"DEC",      keywordDEC,      Compiler::SingleStatementParsed};
        _keywords["ON"      ] = {"ON",       keywordON,       Compiler::SingleStatementParsed};
        _keywords["GOTO"    ] = {"GOTO",     keywordGOTO,     Compiler::SingleStatementParsed};
        _keywords["GOSUB"   ] = {"GOSUB",    keywordGOSUB,    Compiler::SingleStatementParsed};
        _keywords["RETURN"  ] = {"RETURN",   keywordRETURN,   Compiler::SingleStatementParsed};
        _keywords["CLS"     ] = {"CLS",      keywordCLS,      Compiler::SingleStatementParsed};
        _keywords["?"       ] = {"?",        keywordPRINT,    Compiler::SingleStatementParsed};
        _keywords["PRINT"   ] = {"PRINT",    keywordPRINT,    Compiler::SingleStatementParsed};
        _keywords["INPUT"   ] = {"INPUT",    keywordINPUT,    Compiler::SingleStatementParsed};
        _keywords["FOR"     ] = {"FOR",      keywordFOR,      Compiler::SingleStatementParsed};
        _keywords["NEXT"    ] = {"NEXT",     keywordNEXT,     Compiler::SingleStatementParsed};
        _keywords["IF"      ] = {"IF",       keywordIF,       Compiler::MultiStatementParsed };
        _keywords["ELSEIF"  ] = {"ELSEIF",   keywordELSEIF,   Compiler::SingleStatementParsed};
        _keywords["ELSE"    ] = {"ELSE",     keywordELSE,     Compiler::SingleStatementParsed};
        _keywords["ENDIF"   ] = {"ENDIF",    keywordENDIF,    Compiler::SingleStatementParsed};
        _keywords["WHILE"   ] = {"WHILE",    keywordWHILE,    Compiler::SingleStatementParsed};
        _keywords["WEND"    ] = {"WEND",     keywordWEND,     Compiler::SingleStatementParsed};
        _keywords["REPEAT"  ] = {"REPEAT",   keywordREPEAT,   Compiler::SingleStatementParsed};
        _keywords["UNTIL"   ] = {"UNTIL",    keywordUNTIL,    Compiler::SingleStatementParsed};
        _keywords["CONST"   ] = {"CONST",    keywordCONST,    Compiler::SingleStatementParsed};
        _keywords["DIM"     ] = {"DIM",      keywordDIM,      Compiler::SingleStatementParsed};
        _keywords["DEF"     ] = {"DEF",      keywordDEF,      Compiler::SingleStatementParsed};
        _keywords["ALLOC"   ] = {"ALLOC",    keywordALLOC,    Compiler::SingleStatementParsed};
        _keywords["FREE"    ] = {"FREE",     keywordFREE,     Compiler::SingleStatementParsed};
        _keywords["AT"      ] = {"AT",       keywordAT,       Compiler::SingleStatementParsed};
        _keywords["PUT"     ] = {"PUT",      keywordPUT,      Compiler::SingleStatementParsed};
        _keywords["MODE"    ] = {"MODE",     keywordMODE,     Compiler::SingleStatementParsed};
        _keywords["WAIT"    ] = {"WAIT",     keywordWAIT,     Compiler::SingleStatementParsed};
        _keywords["PSET"    ] = {"PSET",     keywordPSET,     Compiler::SingleStatementParsed};
        _keywords["LINE"    ] = {"LINE",     keywordLINE,     Compiler::SingleStatementParsed};
        _keywords["HLINE"   ] = {"HLINE",    keywordHLINE,    Compiler::SingleStatementParsed};
        _keywords["VLINE"   ] = {"VLINE",    keywordVLINE,    Compiler::SingleStatementParsed};
        _keywords["CIRCLE"  ] = {"CIRCLE",   keywordCIRCLE,   Compiler::SingleStatementParsed};
        _keywords["CIRCLEF" ] = {"CIRCLEF",  keywordCIRCLEF,  Compiler::SingleStatementParsed};
        _keywords["RECT"    ] = {"RECT",     keywordRECT,     Compiler::SingleStatementParsed};
        _keywords["RECTF"   ] = {"RECTF",    keywordRECTF,    Compiler::SingleStatementParsed};
        _keywords["POLY"    ] = {"POLY",     keywordPOLY,     Compiler::SingleStatementParsed};
        _keywords["SCROLL"  ] = {"SCROLL",   keywordSCROLL,   Compiler::SingleStatementParsed};
        _keywords["POKE"    ] = {"POKE",     keywordPOKE,     Compiler::SingleStatementParsed};
        _keywords["DOKE"    ] = {"DOKE",     keywordDOKE,     Compiler::SingleStatementParsed};
        _keywords["PLAY"    ] = {"PLAY",     keywordPLAY,     Compiler::SingleStatementParsed};
        _keywords["LOAD"    ] = {"LOAD",     keywordLOAD,     Compiler::SingleStatementParsed};
        _keywords["SPRITE"  ] = {"SPRITE",   keywordSPRITE,   Compiler::SingleStatementParsed};
        _keywords["SOUND"   ] = {"SOUND",    keywordSOUND,    Compiler::SingleStatementParsed};
        _keywords["SET"     ] = {"SET",      keywordSET,      Compiler::SingleStatementParsed};

        // String keywords
        _stringKeywords["CHR$"  ] = "CHR$";
        _stringKeywords["HEX$"  ] = "HEX$";
        _stringKeywords["HEXW$" ] = "HEXW$";
        _stringKeywords["MID$"  ] = "MID$";
        _stringKeywords["LEFT$" ] = "LEFT$";
        _stringKeywords["RIGHT$"] = "RIGHT$";
        _stringKeywords["SPC$"  ] = "SPC$";
        _stringKeywords["STR$"  ] = "STR$";
        _stringKeywords["TIME$" ] = "TIME$";

        // Equals keywords
        _equalsKeywords["CONST" ] = "CONST";
        _equalsKeywords["DIM"   ] = "DIM";
        _equalsKeywords["DEF"   ] = "DEF";
        _equalsKeywords["FOR"   ] = "FOR";
        _equalsKeywords["IF"    ] = "IF";
        _equalsKeywords["ELSEIF"] = "ELSEIF";
        _equalsKeywords["WHILE" ] = "WHILE";
        _equalsKeywords["UNTIL" ] = "UNTIL";

        uint64_t timeSeed = time(NULL);
        std::seed_seq seedSequence{uint32_t(timeSeed & 0xffffffff), uint32_t(timeSeed>>32)};
        _randGenerator.seed(seedSequence);

        return true;
    }


    bool findPragma(std::string code, const std::string& pragma, size_t& foundPos)
    {
        foundPos = code.find(pragma);
        if(foundPos != std::string::npos)
        {
            foundPos += pragma.size();
            return true;
        }
        return false;
    }

    KeywordResult handlePragmas(std::string& input, int codeLineIndex)
    {
        std::vector<std::string> tokens = Expression::tokenise(input, ' ', false);
        if(tokens.size() >= 1)
        {
            std::string token = tokens[0];
            Expression::stripNonStringWhitespace(token);

            if(_pragmas.find(token) == _pragmas.end()) return KeywordNotFound;

            // Handle pragma in input
            size_t foundPos;
            if(findPragma(token, _pragmas[token]._name, foundPos)  &&  _pragmas[token]._func)
            {
                bool success = _pragmas[token]._func(input, codeLineIndex, foundPos);
                if(success) return KeywordFound;
                
                return KeywordError;
            }
        }

        return KeywordNotFound;
    }

    bool findKeyword(std::string code, const std::string& keyword, size_t& foundPos)
    {
        Expression::strToUpper(code);
        foundPos = code.find(keyword);
        if(foundPos != std::string::npos)
        {
            foundPos += keyword.size();
            return true;
        }
        return false;
    }

    KeywordResult handleKeywords(Compiler::CodeLine& codeLine, const std::string& keyword, int codeLineIndex, int tokenIndex, KeywordFuncResult& result)
    {
        size_t foundPos;

        std::string key = keyword;
        Expression::strToUpper(key);
        if(_keywords.find(key) == _keywords.end()) return KeywordNotFound;

        // Handle keyword in code line
        if(findKeyword(key, _keywords[key]._name, foundPos)  &&  _keywords[key]._func)
        {
            bool success = _keywords[key]._func(codeLine, codeLineIndex, tokenIndex, foundPos, result);
            return (!success) ? KeywordError : KeywordFound;
        }

        return KeywordFound;
    }

    EmitStringResult emitStringAddr(const std::string& token, const std::string& operand)
    {
        std::string strToken = token;
        Expression::stripNonStringWhitespace(strToken);
        if(strToken.back() == '$'  &&  Expression::isVarNameValid(strToken))
        {
            uint16_t srcAddr;
            int strIndexSrc = Compiler::findStr(strToken);
            if(strIndexSrc >= 0)
            {
                srcAddr = Compiler::getStringVars()[strIndexSrc]._address;
            }
            else
            {
                strIndexSrc = Compiler::findConst(strToken);
                if(strIndexSrc == -1) return SyntaxError;
                
                srcAddr = Compiler::getConstants()[strIndexSrc]._address;
            }

            Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(srcAddr), false);
            Compiler::emitVcpuAsm("STW", operand, false);

            return ValidStringVar;
        }

        return InvalidStringVar;
    }

    void getOrCreateString(const Expression::Numeric& numeric, std::string& name, uint16_t& addr, int& index)
    {
        switch(numeric._varType)
        {
            case Expression::String:
            {
                Compiler::getOrCreateConstString(numeric._text, index);
                name = Compiler::getStringVars()[index]._name;
                addr = Compiler::getStringVars()[index]._address;
            }
            break;

            case Expression::StrVar:
            {
                name = Compiler::getStringVars()[index]._name;
                addr = Compiler::getStringVars()[index]._address;
            }
            break;

            case Expression::Constant:
            {
                name = Compiler::getConstants()[index]._name;
                addr = Compiler::getConstants()[index]._address;
            }
            break;

            default: break;
        }
    }

    void handleConstantString(const Expression::Numeric& numeric, Compiler::ConstStrType constStrType, std::string& name, int& index)
    {
        switch(constStrType)
        {
            case Compiler::StrLeft:

            case Compiler::StrRight:
            {
                uint8_t length = uint8_t(std::lround(numeric._parameters[0]._value));
                Compiler::getOrCreateConstString(constStrType, numeric._text, length, 0, index);
            }
            break;

            case Compiler::StrMid:
            {
                uint8_t offset = uint8_t(std::lround(numeric._parameters[0]._value));
                uint8_t length = uint8_t(std::lround(numeric._parameters[1]._value));
                Compiler::getOrCreateConstString(constStrType, numeric._text, length, offset, index);
            }
            break;

            default: break;
        }

        name = Compiler::getStringVars()[index]._name;
        uint16_t srcAddr = Compiler::getStringVars()[index]._address;

        if(Expression::getEnableOptimisedPrint())
        {
            Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(srcAddr), false);
            Compiler::emitVcpuAsm("%PrintAcString", "", false);
        }
        else
        {
            uint16_t dstAddr = Compiler::getStringVars()[Expression::getOutputNumeric()._index]._address;
            Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(srcAddr), false);
            Compiler::emitVcpuAsm("STW", "strSrcAddr", false);
            Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(dstAddr), false);
            Compiler::emitVcpuAsm("STW", "strDstAddr", false);
            Compiler::emitVcpuAsm("%StringCopy", "", false);
        }
    }

    void handleStringParameter(Expression::Numeric& param)
    {
        // Literals
        if(param._varType == Expression::Number)
        {
            // 8bit
            if(param._value >=0  &&  param._value <= 255)
            {
                Compiler::emitVcpuAsm("LDI", std::to_string(int16_t(std::lround(param._value))), false);
            }
            // 16bit
            else
            {
                Compiler::emitVcpuAsm("LDWI", std::to_string(int16_t(std::lround(param._value))), false);
            }

            return;
        }

        Operators::handleSingleOp("LDW", param);
    }

    void eraseKeywordFromCode(Compiler::CodeLine& codeLine, const std::string& keyword, size_t foundPos)
    {
        // Remove from code
        codeLine._code.erase(foundPos, keyword.size());

        // Remove from expression
        size_t keywordPos;
        std::string expr = codeLine._expression;
        Expression::strToUpper(expr);
        if((keywordPos = expr.find(keyword)) != std::string::npos)
        {
            codeLine._expression.erase(keywordPos, keyword.size());
        }

        // Remove from tokens
        for(int i=0; i<int(codeLine._tokens.size()); i++)
        {
            std::string str = codeLine._tokens[i];
            Expression::strToUpper(str);
            if((keywordPos = expr.find(keyword)) != std::string::npos)
            {
                codeLine._tokens[i].erase(keywordPos, keyword.size());
                break;
            }
        }
    }


    // ********************************************************************************************
    // Functions
    // ********************************************************************************************
    Expression::Numeric functionARR(Expression::Numeric& numeric)
    {
        if(Expression::getOutputNumeric()._staticInit)
        {
            fprintf(stderr, "Keywords::functionARR() : ARRAY() cannot be used in static initialisation!\n");
            return numeric;
        }

        Compiler::getNextTempVar();

        int intSize = Compiler::getIntegerVars()[numeric._index]._intSize;
        uint16_t arrayPtr = Compiler::getIntegerVars()[numeric._index]._array;

        // Literal array index
        if(numeric._parameters[0]._varType == Expression::Number)
        {
            std::string operand = Expression::wordToHexString(arrayPtr + uint16_t(numeric._parameters[0]._value*intSize));
            switch(numeric._int16Byte)
            {
                case Expression::Int16Low:  Compiler::emitVcpuAsm("LDWI", operand,          false); Compiler::emitVcpuAsm("PEEK", "", false); break;
                case Expression::Int16High: Compiler::emitVcpuAsm("LDWI", operand + " + 1", false); Compiler::emitVcpuAsm("PEEK", "", false); break;
                case Expression::Int16Both: Compiler::emitVcpuAsm("LDWI", operand,          false); Compiler::emitVcpuAsm("DEEK", "", false); break;

                default: break;
            }

            Operators::createTmpVar(numeric);
        }
        // Variable array index
        else
        {
            // Can't call Operators::handleSingleOp() here, so special case it
            switch(numeric._parameters[0]._varType)
            {
                // Temporary variable address
                case Expression::TmpVar:
                {
                    Compiler::emitVcpuAsm("LDW", Expression::byteToHexString(uint8_t(std::lround(numeric._parameters[0]._value))), false);
                }
                break;

                // User variable name
                case Expression::IntVar:
                {
                    Compiler::emitVcpuAsmUserVar("LDW", numeric._parameters[0], false);
                }
                break;

                default: break;
            }

            Operators::createTmpVar(numeric);

            // TODO: currently makes code bigger AND slower because of optimiser
            if(0) //Assembler::getUseOpcodeCALLI()  &&  Compiler::getCodeOptimiseType() == Compiler::CodeSize)
            {
                // Saves 2 bytes per array access but costs an extra 2 instructions in performance
                Compiler::emitVcpuAsm("STW", "memIndex", false);
                Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(arrayPtr), false);
                switch(numeric._int16Byte)
                {
                    case Expression::Int16Low:  Compiler::emitVcpuAsm("CALLI", "getArrayInt16Low",  false); break;
                    case Expression::Int16High: Compiler::emitVcpuAsm("CALLI", "getArrayInt16High", false); break;
                    case Expression::Int16Both: Compiler::emitVcpuAsm("CALLI", "getArrayInt16",     false); break;

                    default: break;
                }
            }
            //else if(Compiler::getCodeOptimiseType() == Compiler::CodeSpeed)
            {
                Compiler::emitVcpuAsm("STW", "register2", false);
                Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(arrayPtr), false);
                Compiler::emitVcpuAsm("ADDW", "register2", false);
                Compiler::emitVcpuAsm("ADDW", "register2", false);
                switch(numeric._int16Byte)
                {
                    case Expression::Int16Low:  Compiler::emitVcpuAsm("PEEK", "",  false);                                           break;
                    case Expression::Int16High: Compiler::emitVcpuAsm("ADDI", "1", false); Compiler::emitVcpuAsm("PEEK", "", false); break;
                    case Expression::Int16Both: Compiler::emitVcpuAsm("DEEK", "",  false);                                           break;

                    default: break;
                }
            }
        }

        Compiler::emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(Compiler::getTempVarStart())), false);

        return numeric;
    }

    Expression::Numeric functionPEEK(Expression::Numeric& numeric)
    {
        if(Expression::getOutputNumeric()._staticInit)
        {
            fprintf(stderr, "Keywords::functionPEEK() : PEEK() cannot be used in static initialisation!\n");
            return numeric;
        }

        if(numeric._varType == Expression::Number)
        {
            // Optimise for page 0
            if(numeric._value >= 0  && numeric._value <= 255)
            {
                Compiler::emitVcpuAsm("LD",  Expression::byteToHexString(uint8_t(std::lround(numeric._value))), false);
                Compiler::emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(Compiler::getTempVarStart())), false);
                Operators::createTmpVar(numeric);
                return numeric;
            }
            else
            {
                Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(int16_t(std::lround(numeric._value))), false);
            }
        }

        Compiler::getNextTempVar();
        Operators::handleSingleOp("LDW", numeric);
        Compiler::emitVcpuAsm("PEEK", "", false);
        Compiler::emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(Compiler::getTempVarStart())), false);

        return numeric;
    }

    Expression::Numeric functionDEEK(Expression::Numeric& numeric)
    {
        if(numeric._varType == Expression::Number)
        {
            // Optimise for page 0
            if(numeric._value >= 0  && numeric._value <= 255)
            {
                Compiler::emitVcpuAsm("LDW", Expression::byteToHexString(uint8_t(std::lround(numeric._value))), false);
                Compiler::emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(Compiler::getTempVarStart())), false);
                Operators::createTmpVar(numeric);
                return numeric;
            }
            else
            {
                Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(int16_t(std::lround(numeric._value))), false);
            }
        }

        Compiler::getNextTempVar();
        Operators::handleSingleOp("LDW", numeric);
        Compiler::emitVcpuAsm("DEEK", "", false);
        Compiler::emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(Compiler::getTempVarStart())), false);

        return numeric;
    }

    Expression::Numeric functionUSR(Expression::Numeric& numeric)
    {
        if(Expression::getOutputNumeric()._staticInit)
        {
            fprintf(stderr, "Keywords::functionUSR() : USR() cannot be used in static initialisation!\n");
            return numeric;
        }

        if(numeric._varType == Expression::Number)
        {
            if(Assembler::getUseOpcodeCALLI())
            {
                (numeric._value >= 0  && numeric._value <= 255) ? Compiler::emitVcpuAsm("CALLI", Expression::byteToHexString(uint8_t(std::lround(numeric._value))), false) : 
                                                                  Compiler::emitVcpuAsm("CALLI", Expression::wordToHexString(int16_t(std::lround(numeric._value))), false);
            }
            else
            {
                (numeric._value >= 0  && numeric._value <= 255) ? Compiler::emitVcpuAsm("LDI", Expression::byteToHexString(uint8_t(std::lround(numeric._value))), false) : 
                                                                  Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(int16_t(std::lround(numeric._value))), false);
            }
        }

        Compiler::getNextTempVar();

        if(Assembler::getUseOpcodeCALLI())
        {
            Operators::handleSingleOp("CALLI", numeric);
        }
        else
        {
            Operators::handleSingleOp("LDW", numeric);
            Compiler::emitVcpuAsm("CALL", "giga_vAC", false);
        }
        Compiler::emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(Compiler::getTempVarStart())), false);

        return numeric;
    }

    Expression::Numeric functionRND(Expression::Numeric& numeric)
    {
        bool useMod = true;
        if(numeric._varType == Expression::Number)
        {
            // No code needed for static initialisation
            if(Expression::getOutputNumeric()._staticInit)
            {
                if(numeric._value == 0)
                {
                    std::uniform_int_distribution<uint16_t> distribution(0, 0xFFFF);
                    numeric._value = distribution(_randGenerator);
                }
                else
                {
                    std::uniform_int_distribution<uint16_t> distribution(0, uint16_t(numeric._value));
                    numeric._value = distribution(_randGenerator);
                }

                return numeric;
            }

            // RND(0) skips the MOD call and allows you to filter the output manually
            if(numeric._value == 0)
            {
                useMod = false;
            }
            else
            {
                (numeric._value > 0  && numeric._value <= 255) ? Compiler::emitVcpuAsm("LDI", Expression::byteToHexString(uint8_t(std::lround(numeric._value))), false) : 
                                                                 Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(int16_t(std::lround(numeric._value))), false);
            }
        }

        Compiler::getNextTempVar();
        if(useMod)
        {
            Operators::handleSingleOp("LDW", numeric);
            Compiler::emitVcpuAsm("%RandMod", "", false);
        }
        else
        {
            Operators::createTmpVar(numeric);
            Compiler::emitVcpuAsm("%Rand", "", false);
        }
        Compiler::emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(Compiler::getTempVarStart())), false);

        return numeric;
    }

    Expression::Numeric functionLEN(Expression::Numeric& numeric)
    {
        if(numeric._varType != Expression::Number)
        {
            Compiler::getNextTempVar();

            // Handle non variables
            if(numeric._index == -1)
            {
                switch(numeric._varType)
                {
                    // Get or create constant string
                    case Expression::String:
                    {
                        int index;
                        Compiler::getOrCreateConstString(numeric._text, index);
                        numeric._index = int16_t(index);
                        numeric._varType = Expression::StrVar;
                    }
                    break;

                    case Expression::TmpStrVar:
                    {
                    }
                    break;

                    default:
                    {
                        fprintf(stderr, "Keywords::functionLEN() : couldn't find variable name '%s'\n", numeric._name.c_str());
                        return numeric;
                    }
                }
            }

            int length = 0;
            switch(numeric._varType)
            {
                case Expression::IntVar:   length = Compiler::getIntegerVars()[numeric._index]._intSize; break;
                case Expression::ArrVar:   length = Compiler::getIntegerVars()[numeric._index]._arrSize; break;
                case Expression::StrVar:   length = Compiler::getStringVars()[numeric._index]._size;     break;
                case Expression::Constant: length = Compiler::getConstants()[numeric._index]._size;      break;

                default: break;
            }

            // No code needed for static initialisation
            if(Expression::getOutputNumeric()._staticInit)
            {
                numeric._value = length;
                return numeric;
            }

            // Variables
            if(numeric._varType == Expression::StrVar  &&  !Compiler::getStringVars()[numeric._index]._constant)
            {
                Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(Compiler::getStringVars()[numeric._index]._address), false);
                Compiler::emitVcpuAsm("PEEK", "", false);
            }
            else if(numeric._varType == Expression::TmpStrVar)
            {
                Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(Compiler::getStrWorkArea()), false);
                Compiler::emitVcpuAsm("PEEK", "", false);
            }
            // Constants
            else
            {
                // Generate code to save result into a tmp var
                (length <= 255) ? Compiler::emitVcpuAsm("LDI", std::to_string(length), false) : Compiler::emitVcpuAsm("LDWI", std::to_string(length), false);
            }

            Operators::createTmpVar(numeric);
            Compiler::emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(Compiler::getTempVarStart())), false);
            
        }

        return numeric;
    }

    Expression::Numeric functionGET(Expression::Numeric& numeric)
    {
        if(Expression::getOutputNumeric()._staticInit)
        {
            fprintf(stderr, "Keywords::functionGET() : GET() cannot be used in static initialisation!\n");
            return numeric;
        }

        if(numeric._varType == Expression::String)
        {
            std::string sysVarName = numeric._text;
            Expression::strToUpper(sysVarName);
            if(sysVarName == "SPRITELUT"  &&  numeric._parameters.size() == 1)
            {
                // Literal constant
                if(numeric._parameters[0]._varType == Expression::Number)
                {
                    Compiler::emitVcpuAsm("LDI", Expression::byteToHexString(uint8_t(std::lround(numeric._parameters[0]._value))), false);
                }

                // Look up sprite lut from sprites lut using a sprite index, (handleSingleOp LDW is skipped if above was a constant literal)
                Compiler::getNextTempVar();
                Operators::handleSingleOp("LDW", numeric._parameters[0]);
                Compiler::emitVcpuAsm("STW", "spriteId", false);
                Compiler::emitVcpuAsm("%GetSpriteLUT", "", false);
                Compiler::emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(Compiler::getTempVarStart())), false);
                return numeric._parameters[0];
            }
            else if(numeric._parameters.size() == 0)
            {
                Compiler::getNextTempVar();
                Operators::createTmpVar(numeric);

                if(sysVarName == "CURSORX")
                {
                    Compiler::emitVcpuAsm("LD", "cursorXY", false);
                }
                else if(sysVarName == "CURSORY")
                {
                    Compiler::emitVcpuAsm("LD", "cursorXY + 1", false);
                }
                else if(sysVarName == "CURSORXY")
                {
                    Compiler::emitVcpuAsm("LDW", "cursorXY", false);
                }
                else if(sysVarName == "FGCOLOUR")
                {
                    Compiler::emitVcpuAsm("LD", "fgbgColour + 1", false);
                }
                else if(sysVarName == "BGCOLOUR")
                {
                    Compiler::emitVcpuAsm("LD", "fgbgColour", false);
                }
                else if(sysVarName == "FGBGCOLOUR")
                {
                    Compiler::emitVcpuAsm("LDW", "fgbgColour", false);
                }
                else if(sysVarName == "MIDISTREAM")
                {
                    Compiler::emitVcpuAsm("LDW", "midiStream", false);
                }
                else if(sysVarName == "LEDTEMPO")
                {
                    Compiler::emitVcpuAsm("LD", "giga_ledTempo", false);
                }
                else if(sysVarName == "LEDSTATE")
                {
                    Compiler::emitVcpuAsm("LD", "giga_ledState", false);
                }
                else if(sysVarName == "SOUNDTIMER")
                {
                    Compiler::emitVcpuAsm("LD", "giga_soundTimer", false);
                }
                else if(sysVarName == "CHANNELMASK")
                {
                    Compiler::emitVcpuAsm("LD", "giga_channelMask", false);
                    Compiler::emitVcpuAsm("ANDI", "0x03", false);
                }
                else if(sysVarName == "ROMTYPE")
                {
                    Compiler::emitVcpuAsm("LD", "giga_romType", false);
                    Compiler::emitVcpuAsm("ANDI", "0xF8", false);
                }
                else if(sysVarName == "VSP")
                {
                    Compiler::emitVcpuAsm("LD", "giga_vSP", false);
                }
                else if(sysVarName == "VLR")
                {
                    Compiler::emitVcpuAsm("LD", "giga_vLR", false);
                }
                else if(sysVarName == "VAC")
                {
                    Compiler::emitVcpuAsm("LD", "giga_vAC", false);
                }
                else if(sysVarName == "VPC")
                {
                    Compiler::emitVcpuAsm("LD", "giga_vPC", false);
                }
                else if(sysVarName == "XOUTMASK")
                {
                    Compiler::emitVcpuAsm("LD", "giga_xoutMask", false);
                }
                else if(sysVarName == "BUTTONSTATE")
                {
                    Compiler::emitVcpuAsm("LD", "giga_buttonState", false);
                }
                else if(sysVarName == "SERIALRAW")
                {
                    Compiler::emitVcpuAsm("LD", "giga_serialRaw", false);
                }
                else if(sysVarName == "FRAMECOUNT")
                {
                    Compiler::emitVcpuAsm("LD", "giga_frameCount", false);
                }
                else if(sysVarName == "VIDEOY")
                {
                    Compiler::emitVcpuAsm("LD", "giga_videoY", false);
                }
                else if(sysVarName == "RAND2")
                {
                    Compiler::emitVcpuAsm("LD", "giga_rand2", false);
                }
                else if(sysVarName == "RAND1")
                {
                    Compiler::emitVcpuAsm("LD", "giga_rand1", false);
                }
                else if(sysVarName == "RAND0")
                {
                    Compiler::emitVcpuAsm("LD", "giga_rand0", false);
                }
                else if(sysVarName == "MEMSIZE")
                {
                    Compiler::emitVcpuAsm("LD", "giga_memSize", false);
                }
                else if(sysVarName == "YRES")
                {
                    Compiler::emitVcpuAsm("LDI", "giga_yres", false);
                }
                else if(sysVarName == "XRES")
                {
                    Compiler::emitVcpuAsm("LDI", "giga_xres", false);
                }
                else if(sysVarName == "SNDCHN4")
                {
                    Compiler::emitVcpuAsm("LDWI", "giga_soundChan4", false);
                }
                else if(sysVarName == "SNDCHN3")
                {
                    Compiler::emitVcpuAsm("LDWI", "giga_soundChan3", false);
                }
                else if(sysVarName == "SNDCHN2")
                {
                    Compiler::emitVcpuAsm("LDWI", "giga_soundChan2", false);
                }
                else if(sysVarName == "SNDCHN1")
                {
                    Compiler::emitVcpuAsm("LDWI", "giga_soundChan1", false);
                }
                else if(sysVarName == "VTOP")
                {
                    Compiler::emitVcpuAsm("LDWI", "giga_videoTop", false);
                }
                else if(sysVarName == "VTABLE")
                {
                    Compiler::emitVcpuAsm("LDWI", "giga_videoTable", false);
                }
                else if(sysVarName == "VRAM")
                {
                    Compiler::emitVcpuAsm("LDWI", "giga_vram", false);
                }
                else if(sysVarName == "ROMNOTES")
                {
                    Compiler::emitVcpuAsm("LDWI", "giga_notesTable", false);
                }
                else if(sysVarName == "ROMTEXT82")
                {
                    Compiler::emitVcpuAsm("LDWI", "giga_text82", false);
                }
                else if(sysVarName == "ROMTEXT32")
                {
                    Compiler::emitVcpuAsm("LDWI", "giga_text32", false);
                }
                else
                {
                    fprintf(stderr, "*** Warning *** Keywords::functionGET() : system variable name '%s' does not exist\n", numeric._text.c_str());
                    return numeric;
                }

                Compiler::emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(Compiler::getTempVarStart())), false);
            }
        }

        return numeric;
    }

    Expression::Numeric functionABS(Expression::Numeric& numeric)
    {
        if(Expression::getOutputNumeric()._staticInit)
        {
            fprintf(stderr, "Keywords::functionABS() : ABS() cannot be used in static initialisation!\n");
            return numeric;
        }

        if(numeric._varType != Expression::String  &&  numeric._varType != Expression::StrVar &&  numeric._varType != Expression::TmpStrVar)
        {
            Compiler::getNextTempVar();

            if(numeric._varType == Expression::Number)
            {
                numeric._value = abs(numeric._value);
                (numeric._value > 0  && numeric._value <= 255) ? Compiler::emitVcpuAsm("LDI", Expression::byteToHexString(uint8_t(std::lround(numeric._value))), false) : 
                                                                 Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(int16_t(std::lround(numeric._value))), false);
                Operators::createTmpVar(numeric);
                Compiler::emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(Compiler::getTempVarStart())), false);
            }
            else
            {
                Operators::handleSingleOp("LDW", numeric);
                Compiler::emitVcpuAsm("%Absolute", "", false);
                Compiler::emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(Compiler::getTempVarStart())), false);
            }
        }

        return numeric;
    }

    Expression::Numeric functionASC(Expression::Numeric& numeric)
    {
        if(numeric._varType != Expression::Number)
        {
            Compiler::getNextTempVar();

            // Handle non variables
            if(numeric._index == -1)
            {
                switch(numeric._varType)
                {
                    // Get or create constant string
                    case Expression::String:
                    {
                        int index;
                        Compiler::getOrCreateConstString(numeric._text, index);
                        numeric._index = int16_t(index);
                        numeric._varType = Expression::StrVar;
                    }
                    break;

                    case Expression::TmpStrVar:
                    {
                    }
                    break;

                    default:
                    {
                        fprintf(stderr, "Keywords::functionASC() : couldn't find variable name '%s'\n", numeric._name.c_str());
                        return numeric;
                    }
                }
            }

            uint8_t ascii = 0;
            switch(numeric._varType)
            {
                case Expression::StrVar:   ascii = Compiler::getStringVars()[numeric._index]._text[0]; break;
                case Expression::Constant: ascii = Compiler::getConstants()[numeric._index]._text[0];  break;

                default: break;
            }

            // No code needed for static initialisation
            if(Expression::getOutputNumeric()._staticInit)
            {
                numeric._value = ascii;
                return numeric;
            }

            // Variables
            if(numeric._varType == Expression::StrVar  &&  !Compiler::getStringVars()[numeric._index]._constant)
            {
                Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(Compiler::getStringVars()[numeric._index]._address) + " + 1", false);
                Compiler::emitVcpuAsm("PEEK", "", false);
            }
            else if(numeric._varType == Expression::TmpStrVar)
            {
                Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(Compiler::getStrWorkArea()) + " + 1", false);
                Compiler::emitVcpuAsm("PEEK", "", false);
            }
            // Constants
            else
            {
                // Generate code to save result into a tmp var
                Compiler::emitVcpuAsm("LDI", std::to_string(ascii), false);
            }

            Operators::createTmpVar(numeric);
            Compiler::emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(Compiler::getTempVarStart())), false);
        }

        return numeric;
    }

    Expression::Numeric functionCHR$(Expression::Numeric& numeric)
    {
        if(Expression::getOutputNumeric()._staticInit)
        {
            fprintf(stderr, "Keywords::functionCHR$() : CHR$() cannot be used in static initialisation!\n");
            return numeric;
        }

        int index;
        uint16_t dstAddr;
        Expression::VarType varType;
        if(Expression::getOutputNumeric()._varType == Expression::StrVar)
        {
            index = Expression::getOutputNumeric()._index;
            dstAddr = Compiler::getStringVars()[index]._address;
            varType = Expression::StrVar;
        }
        else
        {
            index = -1;
            dstAddr = Compiler::getStrWorkArea();
            varType = Expression::TmpStrVar;
        }

        if(numeric._varType == Expression::Number)
        {
            // Print CHR string, (without wasting memory)
            if(Expression::getEnableOptimisedPrint())
            {
                Compiler::emitVcpuAsm("LDI", std::to_string(int16_t(std::lround(numeric._value))), false);
                Compiler::emitVcpuAsm("%PrintAcChar", "", false);
                return numeric;
            }

            // Create CHR string
            Compiler::emitVcpuAsm("LDI", std::to_string(int16_t(std::lround(numeric._value))), false);
            Compiler::emitVcpuAsm("STW", "strChr", false);
            Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(dstAddr), false);
            Compiler::emitVcpuAsm("STW", "strDstAddr", false);
            Compiler::emitVcpuAsm("%StringChr", "", false);

            return Expression::Numeric(0, uint16_t(index), true, false, varType, Expression::BooleanCC, Expression::Int16Both, std::string(""), std::string(""));
        }

        Compiler::getNextTempVar();
        Operators::handleSingleOp("LDW", numeric);
        if(Expression::getEnableOptimisedPrint())
        {
            Compiler::emitVcpuAsm("%PrintAcChar", "", false);
            return numeric;
        }

        // Create CHR string
        Compiler::emitVcpuAsm("STW", "strChr", false);
        Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(dstAddr), false);
        Compiler::emitVcpuAsm("STW", "strDstAddr", false);
        Compiler::emitVcpuAsm("%StringChr", "", false);

        return Expression::Numeric(0, uint16_t(index), true, false, varType, Expression::BooleanCC, Expression::Int16Both, std::string(""), std::string(""));
    }

    Expression::Numeric functionHEX$(Expression::Numeric& numeric)
    {
        if(Expression::getOutputNumeric()._staticInit)
        {
            fprintf(stderr, "Keywords::functionHEX$() : HEX$() cannot be used in static initialisation!\n");
            return numeric;
        }

        int index;
        uint16_t dstAddr;
        Expression::VarType varType;
        if(Expression::getOutputNumeric()._varType == Expression::StrVar)
        {
            index = Expression::getOutputNumeric()._index;
            dstAddr = Compiler::getStringVars()[index]._address;
            varType = Expression::StrVar;
        }
        else
        {
            index = -1;
            dstAddr = Compiler::getStrWorkArea();
            varType = Expression::TmpStrVar;
        }

        if(numeric._varType == Expression::Number)
        {
            // Print HEX string, (without wasting memory)
            if(Expression::getEnableOptimisedPrint())
            {
                Compiler::emitVcpuAsm("LDI", Expression::byteToHexString(uint8_t(std::lround(numeric._value))), false);
                Compiler::emitVcpuAsm("%PrintAcHexByte", "", false);
                return numeric;
            }

            // Create HEX string
            Compiler::emitVcpuAsm("LDI", Expression::byteToHexString(uint8_t(std::lround(numeric._value))), false);
            Compiler::emitVcpuAsm("STW", "strChr", false);
            Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(dstAddr), false);
            Compiler::emitVcpuAsm("STW", "strDstAddr", false);
            Compiler::emitVcpuAsm("%StringHex", "", false);

            return Expression::Numeric(0, uint16_t(index), true, false, varType, Expression::BooleanCC, Expression::Int16Both, std::string(""), std::string(""));
        }

        Compiler::getNextTempVar();
        Operators::handleSingleOp("LDW", numeric);
        if(Expression::getEnableOptimisedPrint())
        {
            Compiler::emitVcpuAsm("%PrintAcHexByte", "", false);
            return numeric;
        }

        // Create HEX string
        Compiler::emitVcpuAsm("STW", "strChr", false);
        Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(dstAddr), false);
        Compiler::emitVcpuAsm("STW", "strDstAddr", false);
        Compiler::emitVcpuAsm("%StringHex", "", false);

        return Expression::Numeric(0, uint16_t(index), true, false, varType, Expression::BooleanCC, Expression::Int16Both, std::string(""), std::string(""));
    }

    Expression::Numeric functionHEXW$(Expression::Numeric& numeric)
    {
        if(Expression::getOutputNumeric()._staticInit)
        {
            fprintf(stderr, "Keywords::functionHEXW$() : HEXW$() cannot be used in static initialisation!\n");
            return numeric;
        }

        int index;
        uint16_t dstAddr;
        Expression::VarType varType;
        if(Expression::getOutputNumeric()._varType == Expression::StrVar)
        {
            index = Expression::getOutputNumeric()._index;
            dstAddr = Compiler::getStringVars()[index]._address;
            varType = Expression::StrVar;
        }
        else
        {
            index = -1;
            dstAddr = Compiler::getStrWorkArea();
            varType = Expression::TmpStrVar;
        }

        if(numeric._varType == Expression::Number)
        {
            // Print HEXW string, (without wasting memory)
            if(Expression::getEnableOptimisedPrint())
            {
                Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(int16_t(std::lround(numeric._value))), false);
                Compiler::emitVcpuAsm("%PrintAcHexWord", "", false);
                return numeric;
            }

            // Create HEXW string
            Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(int16_t(std::lround(numeric._value))), false);
            Compiler::emitVcpuAsm("STW", "strHex", false);
            Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(dstAddr), false);
            Compiler::emitVcpuAsm("STW", "strDstAddr", false);
            Compiler::emitVcpuAsm("%StringHexw", "", false);

            return Expression::Numeric(0, uint16_t(index), true, false, varType, Expression::BooleanCC, Expression::Int16Both, std::string(""), std::string(""));
        }

        Compiler::getNextTempVar();
        Operators::handleSingleOp("LDW", numeric);
        if(Expression::getEnableOptimisedPrint())
        {
            Compiler::emitVcpuAsm("%PrintAcHexWord", "", false);
            return numeric;
        }

        // Create HEXW string
        Compiler::emitVcpuAsm("STW", "strHex", false);
        Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(dstAddr), false);
        Compiler::emitVcpuAsm("STW", "strDstAddr", false);
        Compiler::emitVcpuAsm("%StringHexw", "", false);

        return Expression::Numeric(0, uint16_t(index), true, false, varType, Expression::BooleanCC, Expression::Int16Both, std::string(""), std::string(""));
    }

    Expression::Numeric functionLEFT$(Expression::Numeric& numeric)
    {
        if(Expression::getOutputNumeric()._staticInit)
        {
            fprintf(stderr, "Keywords::functionLEFT$() : LEFT$() cannot be used in static initialisation!\n");
            return numeric;
        }

        // Literal string and parameter, (optimised case)
        if(numeric._varType == Expression::String  &&  numeric._parameters.size() == 1  &&  numeric._parameters[0]._varType == Expression::Number)
        {
            int index;
            std::string name;
            handleConstantString(numeric, Compiler::StrLeft, name, index);

            return Expression::Numeric(0, uint16_t(index), true, false, Expression::StrVar, Expression::BooleanCC, Expression::Int16Both, name, std::string(""));
        }

        // Non optimised case
        if(numeric._parameters.size() == 1)
        {
            std::string name;
            uint16_t srcAddr;
            Expression::VarType varType = Expression::StrVar;

            int index = int(numeric._index);
            getOrCreateString(numeric, name, srcAddr, index);

            if(Expression::getEnableOptimisedPrint())
            {
                handleStringParameter(numeric._parameters[0]);
                Compiler::emitVcpuAsm("STW", "textLen", false);
                Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(srcAddr), false);
                Compiler::emitVcpuAsm("%PrintAcLeft", "", false);
            }
            else
            {
                uint16_t dstAddr;
                if(Expression::getOutputNumeric()._varType == Expression::StrVar)
                {
                    dstAddr = Compiler::getStringVars()[Expression::getOutputNumeric()._index]._address;
                }
                else
                {
                    index = -1;
                    dstAddr = Compiler::getStrWorkArea();
                    varType = Expression::TmpStrVar;
                }

                handleStringParameter(numeric._parameters[0]);
                Compiler::emitVcpuAsm("STW", "strLength", false);
                Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(srcAddr), false);
                Compiler::emitVcpuAsm("STW", "strSrcAddr", false);
                Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(dstAddr), false);
                Compiler::emitVcpuAsm("STW", "strDstAddr", false);
                Compiler::emitVcpuAsm("%StringLeft", "", false);
            }

            return Expression::Numeric(0, uint16_t(index), true, false, varType, Expression::BooleanCC, Expression::Int16Both, name, std::string(""));
        }

        return numeric;
    }

    Expression::Numeric functionRIGHT$(Expression::Numeric& numeric)
    {
        if(Expression::getOutputNumeric()._staticInit)
        {
            fprintf(stderr, "Keywords::functionRIGHT$() : RIGHT$() cannot be used in static initialisation!\n");
            return numeric;
        }

        // Literal string and parameter, (optimised case)
        if(numeric._varType == Expression::String  &&  numeric._parameters.size() == 1  &&  numeric._parameters[0]._varType == Expression::Number)
        {
            int index;
            std::string name;
            handleConstantString(numeric, Compiler::StrRight, name, index);

            return Expression::Numeric(0, uint16_t(index), true, false, Expression::StrVar, Expression::BooleanCC, Expression::Int16Both, name, std::string(""));
        }

        // Non optimised case
        if(numeric._parameters.size() == 1)
        {
            std::string name;
            uint16_t srcAddr;
            Expression::VarType varType = Expression::StrVar;

            int index = int(numeric._index);
            getOrCreateString(numeric, name, srcAddr, index);

            if(Expression::getEnableOptimisedPrint())
            {
                handleStringParameter(numeric._parameters[0]);
                Compiler::emitVcpuAsm("STW", "textLen", false);
                Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(srcAddr), false);
                Compiler::emitVcpuAsm("%PrintAcRight", "", false);
            }
            else
            {
                uint16_t dstAddr;
                if(Expression::getOutputNumeric()._varType == Expression::StrVar)
                {
                    dstAddr = Compiler::getStringVars()[Expression::getOutputNumeric()._index]._address;
                }
                else
                {
                    index = -1;
                    dstAddr = Compiler::getStrWorkArea();
                    varType = Expression::TmpStrVar;
                }

                handleStringParameter(numeric._parameters[0]);
                Compiler::emitVcpuAsm("STW", "strLength", false);
                Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(srcAddr), false);
                Compiler::emitVcpuAsm("STW", "strSrcAddr", false);
                Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(dstAddr), false);
                Compiler::emitVcpuAsm("STW", "strDstAddr", false);
                Compiler::emitVcpuAsm("%StringRight", "", false);
            }

            return Expression::Numeric(0, uint16_t(index), true, false, varType, Expression::BooleanCC, Expression::Int16Both, name, std::string(""));
        }

        return numeric;
    }

    Expression::Numeric functionMID$(Expression::Numeric& numeric)
    {
        if(Expression::getOutputNumeric()._staticInit)
        {
            fprintf(stderr, "Keywords::functionMID$() : MID$() cannot be used in static initialisation!\n");
            return numeric;
        }

        // Literal string and parameters, (optimised case)
        if(numeric._varType == Expression::String  &&  numeric._parameters.size() == 2  &&  numeric._parameters[0]._varType == Expression::Number  &&  
           numeric._parameters[1]._varType == Expression::Number)
        {
            int index;
            std::string name;
            handleConstantString(numeric, Compiler::StrMid, name, index);

            return Expression::Numeric(0, uint16_t(index), true, false, Expression::StrVar, Expression::BooleanCC, Expression::Int16Both, name, std::string(""));
        }

        // Non optimised case
        if(numeric._parameters.size() == 2)
        {
            std::string name;
            uint16_t srcAddr;
            Expression::VarType varType = Expression::StrVar;

            int index = int(numeric._index);
            getOrCreateString(numeric, name, srcAddr, index);

            if(Expression::getEnableOptimisedPrint())
            {
                handleStringParameter(numeric._parameters[0]);
                Compiler::emitVcpuAsm("STW", "textOfs", false);
                handleStringParameter(numeric._parameters[1]);
                Compiler::emitVcpuAsm("STW", "textLen", false);
                Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(srcAddr), false);
                Compiler::emitVcpuAsm("%PrintAcMid", "", false);
            }
            else
            {
                uint16_t dstAddr;
                if(Expression::getOutputNumeric()._varType == Expression::StrVar)
                {
                    dstAddr = Compiler::getStringVars()[Expression::getOutputNumeric()._index]._address;
                }
                else
                {
                    index = -1;
                    dstAddr = Compiler::getStrWorkArea();
                    varType = Expression::TmpStrVar;
                }

                handleStringParameter(numeric._parameters[0]);
                Compiler::emitVcpuAsm("STW", "strOffset", false);
                handleStringParameter(numeric._parameters[1]);
                Compiler::emitVcpuAsm("STW", "strLength", false);
                Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(srcAddr), false);
                Compiler::emitVcpuAsm("STW", "strSrcAddr", false);
                Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(dstAddr), false);
                Compiler::emitVcpuAsm("STW", "strDstAddr", false);
                Compiler::emitVcpuAsm("%StringMid", "", false);
            }

            return Expression::Numeric(0, uint16_t(index), true, false, varType, Expression::BooleanCC, Expression::Int16Both, name, std::string(""));
        }

        return numeric;
    }

    Expression::Numeric functionSTRCMP(Expression::Numeric& numeric)
    {
        // Literal strings, (optimised case)
        if(numeric._varType == Expression::String  &&  numeric._parameters.size() == 1  &&  numeric._parameters[0]._varType == Expression::String)
        {
            // No code needed for static initialisation
            if(Expression::getOutputNumeric()._staticInit)
            {
                numeric._varType = Expression::Number;
                numeric._value = uint8_t(numeric._text == numeric._parameters[0]._text);
                return numeric;
            }
            // Generate code to save result into a tmp var
            else
            {
                Compiler::emitVcpuAsm("LDI", std::to_string(uint8_t(numeric._text == numeric._parameters[0]._text)), false);
            }
        }
        else
        {
            // Get addresses of strings to be compared
            std::string name0, name1;
            uint16_t srcAddr0, srcAddr1;
            int index0 = int(numeric._index);
            int index1 = int(numeric._parameters[0]._index);
            getOrCreateString(numeric, name0, srcAddr0, index0);
            getOrCreateString(numeric._parameters[0], name1, srcAddr1, index1);

            // Bby definition this must be a match
            if(srcAddr0 == srcAddr1)
            {
                Compiler::emitVcpuAsm("LDI", "1", false);
            }
            // Compare strings
            else
            {
                Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(srcAddr0), false);
                Compiler::emitVcpuAsm("STW",  "strSrcAddr", false);
                Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(srcAddr1), false);
                Compiler::emitVcpuAsm("STW",  "strSrcAddr2", false);
                Compiler::emitVcpuAsm("%StringCmp", "", false);
            }
        }

        Compiler::getNextTempVar();
        Operators::createTmpVar(numeric);
        Compiler::emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(Compiler::getTempVarStart())), false);

        return numeric;
    }

    Expression::Numeric functionPOINT(Expression::Numeric& numeric)
    {
        if(Expression::getOutputNumeric()._staticInit)
        {
            fprintf(stderr, "Keywords::functionPOINT() : POINT() cannot be used in static initialisation!\n");
            return numeric;
        }

        if(numeric._parameters.size() == 1)
        {
            if(numeric._varType == Expression::Number)
            {
                Compiler::emitVcpuAsm("LDI", Expression::byteToHexString(uint8_t(std::lround(numeric._value))), false);
                Compiler::emitVcpuAsm("ST", "readPixel_xy", false);
            }
            else
            {
                Operators::createSingleOp("LDW", numeric);
                Compiler::emitVcpuAsm("ST", "readPixel_xy", false);
            }

            if(numeric._parameters[0]._varType == Expression::Number)
            {
                Compiler::emitVcpuAsm("LDI", Expression::byteToHexString(uint8_t(std::lround(numeric._parameters[0]._value))), false);
                Compiler::emitVcpuAsm("ST", "readPixel_xy + 1", false);
            }
            else
            {
                Operators::createSingleOp("LDW", numeric._parameters[0]);
                Compiler::emitVcpuAsm("ST", "readPixel_xy + 1", false);
            }

            Compiler::getNextTempVar();
            Operators::createTmpVar(numeric);
            Compiler::emitVcpuAsm("%ReadPixel", "", false);
            Compiler::emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(Compiler::getTempVarStart())), false);
        }

        return numeric;
    }


    // ********************************************************************************************
    // Pragmas
    // ********************************************************************************************
    bool pragmaUSEOPCODECALLI(const std::string& input, int codeLineIndex, size_t foundPos)
    {
        UNREFERENCED_PARAM(input);
        UNREFERENCED_PARAM(foundPos);
        UNREFERENCED_PARAM(codeLineIndex);

        Assembler::setUseOpcodeCALLI(true);

        return true;
    }

    bool pragmaRUNTIMEPATH(const std::string& input, int codeLineIndex, size_t foundPos)
    {
        std::string pragma = input;
        Expression::stripNonStringWhitespace(pragma);
        if(foundPos > pragma.size()-3  ||  !Expression::isStringValid(pragma.substr(foundPos)))
        {
            fprintf(stderr, "Keywords::pragmaRUNTIMEPATH() : Syntax error, _runtimePath_ <\"Path to runtime\">, in '%s' on line %d\n", input.c_str(), codeLineIndex);
            return false;
        }

        // Strip quotes
        std::string runtimePath = pragma.substr(foundPos);
        runtimePath.erase(0, 1);
        runtimePath.erase(runtimePath.size()-1, 1);

        // Prepend current file path to relative paths
        if(runtimePath.find(":") == std::string::npos  &&  runtimePath[0] != '/')
        {
            std::string filepath = Loader::getFilePath();
            size_t slash = filepath.find_last_of("\\/");
            filepath = (slash != std::string::npos) ? filepath.substr(0, slash) : ".";
            std::string includePath = filepath + "/" + runtimePath;
            Assembler::setIncludePath(includePath);
        }
        else
        {
            Assembler::setIncludePath(runtimePath);
        }

        Compiler::setRuntimePath(runtimePath);

        return true;
    }

    bool pragmaRUNTIMESTART(const std::string& input, int codeLineIndex, size_t foundPos)
    {
        std::string pragma = input;
        Expression::stripWhitespace(pragma);
        std::vector<std::string> tokens = Expression::tokenise(pragma.substr(foundPos), ',', false);
        if(tokens.size() != 1)
        {
            fprintf(stderr, "Keywords::pragmaRUNTIMESTART() : Syntax error, _runtimeStart_ <address>, in '%s' on line %d\n", input.c_str(), codeLineIndex);
            return false;
        }

        Expression::Numeric addrNumeric;
        std::string addrOperand;
        Compiler::parseExpression(codeLineIndex, tokens[0], addrOperand, addrNumeric);

        uint16_t address = uint16_t(std::lround(addrNumeric._value));
        if(address < DEFAULT_START_ADDRESS)
        {
            fprintf(stderr, "Keywords::pragmaRUNTIMESTART() : Address field must be above %04x, found %s in '%s' on line %d\n", DEFAULT_START_ADDRESS, tokens[0].c_str(), input.c_str(), codeLineIndex);
            return false;
        }

        Compiler::setRuntimeStart(address);

        // String work area needs to be updated, (return old work area and get a new one)
        uint16_t strWorkArea;
        Memory::giveFreeRAM(Compiler::getStrWorkArea(), USER_STR_SIZE + 2);
        if(!Memory::getFreeRAM(Memory::FitDescending, USER_STR_SIZE + 2, USER_CODE_START, address, strWorkArea))
        {
            fprintf(stderr, "Keywords::pragmaRUNTIMESTART() : Setting new String Work Area failed, in '%s' on line %d\n", input.c_str(), codeLineIndex);
            return false;
        }
        Compiler::setStrWorkArea(strWorkArea);

        return true;
    }

    bool pragmaSTRINGWORKAREA(const std::string& input, int codeLineIndex, size_t foundPos)
    {
        std::string pragma = input;
        Expression::stripWhitespace(pragma);
        std::vector<std::string> tokens = Expression::tokenise(pragma.substr(foundPos), ',', false);
        if(tokens.size() != 1)
        {
            fprintf(stderr, "Keywords::pragmaSTRINGWORKAREA() : Syntax error, _stringWorkArea_ <address>, in '%s' on line %d\n", input.c_str(), codeLineIndex);
            return false;
        }

        Expression::Numeric addrNumeric;
        std::string addrOperand;
        Compiler::parseExpression(codeLineIndex, tokens[0], addrOperand, addrNumeric);
        uint16_t strWorkArea = uint16_t(std::lround(addrNumeric._value));
        if(strWorkArea < DEFAULT_START_ADDRESS)
        {
            fprintf(stderr, "Keywords::pragmaSTRINGWORKAREA() : Address field must be above %04x, found %s in '%s' on line %d\n", DEFAULT_START_ADDRESS, tokens[0].c_str(), input.c_str(), codeLineIndex);
            return false;
        }

        // String work area needs to be updated, (return old work area and get a new one)
        Memory::giveFreeRAM(Compiler::getStrWorkArea(), USER_STR_SIZE + 2);
        if(!Memory::takeFreeRAM(strWorkArea, USER_STR_SIZE + 2))
        {
            fprintf(stderr, "Keywords::pragmaSTRINGWORKAREA() : Setting new String Work Area failed, in '%s' on line %d\n", input.c_str(), codeLineIndex);
            return false;
        }
        Compiler::setStrWorkArea(strWorkArea);

        return true;
    }

    bool pragmaCODEOPTIMISETYPE(const std::string& input, int codeLineIndex, size_t foundPos)
    {
        std::string pragma = input;
        Expression::stripWhitespace(pragma);
        std::vector<std::string> tokens = Expression::tokenise(pragma.substr(foundPos), ',', false);
        if(tokens.size() != 1)
        {
            fprintf(stderr, "Keywords::pragmaCODEOPTIMISETYPE() : Syntax error, _codeOptimiseType_ <size/speed>, in '%s' on line %d\n", input.c_str(), codeLineIndex);
            return false;
        }

        if(tokens[0] == "SIZE")
        {
            Compiler::setCodeOptimiseType(Compiler::CodeSize);
            return true;
        }
        else if(tokens[0] == "SPEED")
        {
            Compiler::setCodeOptimiseType(Compiler::CodeSpeed);
            return true;
        }

        fprintf(stderr, "Keywords::pragmaCODEOPTIMISETYPE() : Syntax error, _codeOptimiseType_ <'size'/'speed'>, in '%s' on line %d\n", input.c_str(), codeLineIndex);

        return false;
    }

    bool pragmaARRAYINDICIESONE(const std::string& input, int codeLineIndex, size_t foundPos)
    {
        UNREFERENCED_PARAM(input);
        UNREFERENCED_PARAM(foundPos);
        UNREFERENCED_PARAM(codeLineIndex);

        Compiler::setArrayIndiciesOne(true);

        return true;
    }

    bool pragmaSPRITESTRIPECHUNKS(const std::string& input, int codeLineIndex, size_t foundPos)
    {
        std::string pragma = input;
        Expression::stripWhitespace(pragma);
        std::vector<std::string> tokens = Expression::tokenise(pragma.substr(foundPos), ',', false, true);
        if(tokens.size() < 1  ||  tokens.size() > 3)
        {
            fprintf(stderr, "Keywords::pragmaSPRITESTRIPECHUNKS() : Syntax error, _spriteStripeChunks_ <num chunks>, <optional minimum address>, <optional ascending/descending> in '%s' on line %d\n",
                    input.c_str(), codeLineIndex);
            return false;
        }

        // Number of chunks
        Expression::Numeric chunksNumeric;
        std::string chunksOperand;
        Compiler::parseExpression(codeLineIndex, tokens[0], chunksOperand, chunksNumeric);
        uint16_t spriteStripeChunks = uint16_t(std::lround(chunksNumeric._value));
        if(spriteStripeChunks > SPRITE_STRIPE_CHUNKS_HI)
        {
            fprintf(stderr, "Keywords::pragmaSPRITESTRIPECHUNKS() : Num chunks field can not be larger than %d, found %s in '%s' on line %d\n", SPRITE_STRIPE_CHUNKS_HI, tokens[0].c_str(),
                                                                                                                                                                         input.c_str(), codeLineIndex);
            return false;
        }

        Compiler::setSpriteStripeChunks(spriteStripeChunks);

        // RAM minimum address
        if(tokens.size() >= 2)
        {
            Expression::Numeric addrNumeric;
            std::string addrOperand;
            Compiler::parseExpression(codeLineIndex, tokens[1], addrOperand, addrNumeric);
            uint16_t minAddress = uint16_t(std::lround(addrNumeric._value));
            if(minAddress < DEFAULT_START_ADDRESS)
            {
                fprintf(stderr, "Keywords::pragmaSPRITESTRIPECHUNKS() : Address field must be above %04x, found %s in '%s' on line %d\n", DEFAULT_START_ADDRESS, tokens[1].c_str(),
                                                                                                                                                                 input.c_str(), codeLineIndex);
                return false;
            }

            Compiler::setSpriteStripeMinAddress(minAddress);
        }

        // RAM fit type
        if(tokens.size() == 3)
        {
            if(tokens[2] == "ASCENDING")
            {
                Compiler::setSpriteStripeFitType(Memory::FitAscending);
                return true;
            }
            else if(tokens[2] == "DESCENDING")
            {
                Compiler::setSpriteStripeFitType(Memory::FitDescending);
                return true;
            }
            else
            {
                fprintf(stderr, "Keywords::pragmaSPRITESTRIPECHUNKS() : Search direction field must be 'ascending or descending', found '%s' in '%s' on line %d\n", tokens[2].c_str(),
                                                                                                                                                                    input.c_str(), codeLineIndex);
                return false;
            }
        }

        return true;
    }



    // ********************************************************************************************
    // Keywords
    // ********************************************************************************************
    bool keywordEND(Compiler::CodeLine& codeLine, int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(foundPos);
        UNREFERENCED_PARAM(tokenIndex);
        UNREFERENCED_PARAM(codeLineIndex);
        UNREFERENCED_PARAM(codeLine);

        std::string labelName = "_end_" + Expression::wordToHexString(Compiler::getVasmPC());
        Compiler::emitVcpuAsm("BRA", labelName, false, codeLineIndex, labelName);

        return true;
    }

    bool keywordINC(Compiler::CodeLine& codeLine, int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(tokenIndex);

        // Operand must be an integer var
        std::string varToken = codeLine._code.substr(foundPos);
        Expression::stripWhitespace(varToken);
        int varIndex = Compiler::findVar(varToken, false);
        if(varIndex < 0)
        {
            fprintf(stderr, "Keywords::keywordINC() : Syntax error, integer variable '%s' not found, in '%s' on line %d\n", varToken.c_str(), codeLine._text.c_str(), codeLineIndex);
            return false;
        }

        Compiler::emitVcpuAsm("INC", "_" + Compiler::getIntegerVars()[varIndex]._name, false, codeLineIndex);

        return true;
    }

    bool keywordDEC(Compiler::CodeLine& codeLine, int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(tokenIndex);

        // Operand must be an integer var
        std::string varToken = codeLine._code.substr(foundPos);
        Expression::stripWhitespace(varToken);
        int varIndex = Compiler::findVar(varToken, false);
        if(varIndex < 0)
        {
            fprintf(stderr, "Keywords::keywordDEC() : Syntax error, integer variable '%s' not found, in '%s' on line %d\n", varToken.c_str(), codeLine._text.c_str(), codeLineIndex);
            return false;
        }

        Compiler::emitVcpuAsm("LDW",  "_" + Compiler::getIntegerVars()[varIndex]._name, false, codeLineIndex);
        Compiler::emitVcpuAsm("SUBI", "1", false, codeLineIndex);
        Compiler::emitVcpuAsm("STW",  "_" + Compiler::getIntegerVars()[varIndex]._name, false, codeLineIndex);

        return true;
    }

    bool keywordON(Compiler::CodeLine& codeLine, int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(tokenIndex);

        std::string code = codeLine._code;
        Expression::strToUpper(code);
        size_t gotoOffset = code.find("GOTO");
        size_t gosubOffset = code.find("GOSUB");
        if(gotoOffset == std::string::npos  &&  gosubOffset == std::string::npos)
        {
            fprintf(stderr, "Keywords::keywordON() : Syntax error in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex);
            return false;
        }

        size_t gSize = (gotoOffset != std::string::npos) ? 4 : 5;
        size_t gOffset = (gotoOffset != std::string::npos) ? gotoOffset : gosubOffset;

        // Parse ON field
        Expression::Numeric onValue;
        std::string onToken = codeLine._code.substr(foundPos, gOffset - (foundPos + 1));
        Expression::stripWhitespace(onToken);
        Compiler::parseExpression(codeLineIndex, onToken, onValue);
        Compiler::emitVcpuAsm("STW", "register0", false, codeLineIndex);

        // Parse labels
        std::vector<size_t> gOffsets;
        std::vector<std::string> gTokens = Expression::tokenise(codeLine._code.substr(gOffset + gSize), ',', gOffsets, false);
        if(gTokens.size() < 1)
        {
            fprintf(stderr, "Keywords::keywordON() : Syntax error, must have at least one label after GOTO/GOSUB, in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex);
            return false;
        }

        // Create on goto/gosub label LUT
        Compiler::getCodeLines()[codeLineIndex]._onGotoGosubLut._lut.clear();
        for(int i=0; i<int(gTokens.size()); i++)
        {
            std::string gotoLabel = gTokens[i];
            Expression::stripWhitespace(gotoLabel);
            int labelIndex = Compiler::findLabel(gotoLabel);
            if(labelIndex == -1)
            {
                fprintf(stderr, "Keywords::keywordON() : invalid label %s in slot %d in '%s' on line %d\n", gotoLabel.c_str(), i, codeLine._text.c_str(), codeLineIndex);
                Compiler::getCodeLines()[codeLineIndex]._onGotoGosubLut._lut.clear();
                return false;
            }
                
            // Only ON GOSUB needs a PUSH, (emitted in createVasmCode())
            if(gosubOffset != std::string::npos) Compiler::getLabels()[labelIndex]._gosub = true;

            // Create lookup table out of label addresses
            Compiler::getCodeLines()[codeLineIndex]._onGotoGosubLut._lut.push_back(labelIndex);
        }

        // Allocate giga memory for LUT
        int size = int(gTokens.size()) * 2;
        uint16_t address;
        if(!Memory::getFreeRAM(Memory::FitDescending, size, USER_CODE_START, Compiler::getRuntimeStart(), address))
        {
            fprintf(stderr, "Keywords::keywordON() : Not enough RAM for onGotoGosub LUT of size %d\n", size);
            return false;
        }
        Compiler::getCodeLines()[codeLineIndex]._onGotoGosubLut._address = address;
        Compiler::getCodeLines()[codeLineIndex]._onGotoGosubLut._name = "lut_" + Expression::wordToHexString(address);

        Compiler::emitVcpuAsm("ADDW", "register0", false, codeLineIndex);
        Compiler::emitVcpuAsm("STW",  "register0", false, codeLineIndex);
        Compiler::emitVcpuAsm("LDWI", Compiler::getCodeLines()[codeLineIndex]._onGotoGosubLut._name, false, codeLineIndex);
        Compiler::emitVcpuAsm("ADDW", "register0", false, codeLineIndex);
        if(Compiler::getArrayIndiciesOne())
        {
            Compiler::emitVcpuAsm("SUBI", "2",         false, codeLineIndex);  // enable this to start at 1 instead of 0
        }
        Compiler::emitVcpuAsm("DEEK", "",          false, codeLineIndex);
        Compiler::emitVcpuAsm("CALL", "giga_vAC",  false, codeLineIndex);

        return true;
    }

    bool keywordGOTO(Compiler::CodeLine& codeLine, int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(tokenIndex);

        // Parse labels
        std::vector<size_t> gotoOffsets;
        std::vector<std::string> gotoTokens = Expression::tokenise(codeLine._code.substr(foundPos), ',', gotoOffsets, false);
        if(gotoTokens.size() < 1  ||  gotoTokens.size() > 2)
        {
            fprintf(stderr, "Keywords::keywordGOTO() : Syntax error, must have one or two parameters, e.g. 'GOTO 200' or 'GOTO k+1,default' : in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex);
            return false;
        }

        // Parse GOTO field
        Expression::Numeric gotoValue;
        std::string gotoToken = gotoTokens[0];
        Expression::stripWhitespace(gotoToken);

        bool useBRA = false;
        if(gotoToken[0] == '&')
        {
            useBRA = true;
            gotoToken.erase(0, 1);
        }

        int labelIndex = Compiler::findLabel(gotoToken);
        if(labelIndex == -1)
        {
            Compiler::setCreateNumericLabelLut(true);

            Compiler::parseExpression(codeLineIndex, gotoToken, gotoValue);
            Compiler::emitVcpuAsm("STW", "numericLabel", false, codeLineIndex);

            // Default label exists
            if(gotoTokens.size() == 2)
            {
                std::string defaultToken = gotoTokens[1];
                Expression::stripWhitespace(defaultToken);
                labelIndex = Compiler::findLabel(defaultToken);
                if(labelIndex == -1)
                {
                    fprintf(stderr, "Keywords::keywordGOTO() : Default label does not exist : in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex);
                    return false;
                }

                Compiler::emitVcpuAsm("LDWI", "_" + Compiler::getLabels()[labelIndex]._name, false, codeLineIndex);
            }
            // No default label
            else
            {
                Compiler::emitVcpuAsm("LDI", "0", false, codeLineIndex);
            }
            Compiler::emitVcpuAsm("STW", "defaultLabel", false, codeLineIndex);

            // Call gotoNumericLabel
            Compiler::emitVcpuAsm("%GotoNumeric", "", false, codeLineIndex);

            return true;
        }

        // Within same page, (validation check on same page branch may fail after outputCode(), user will be warned)
        if(useBRA)
        {
            Compiler::emitVcpuAsm("BRA", "_" + gotoToken, false, codeLineIndex);
        }
        // Long jump
        else
        {
            if(Assembler::getUseOpcodeCALLI())
            {
                Compiler::emitVcpuAsm("CALLI", "_" + gotoToken, false, codeLineIndex);
            }
            else
            {
                Compiler::emitVcpuAsm("LDWI", "_" + gotoToken, false, codeLineIndex);
                Compiler::emitVcpuAsm("CALL", "giga_vAC",      false, codeLineIndex);
            }
        }

        return true;
    }

    bool keywordGOSUB(Compiler::CodeLine& codeLine, int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(tokenIndex);

        // Parse labels
        std::vector<size_t> gosubOffsets;
        std::vector<std::string> gosubTokens = Expression::tokenise(codeLine._code.substr(foundPos), ',', gosubOffsets, false);
        if(gosubTokens.size() < 1  ||  gosubTokens.size() > 2)
        {
            fprintf(stderr, "Keywords::keywordGOSUB() : Syntax error, must have one or two parameters, e.g. 'GOSUB 200' or 'GOSUB k+1,default' : in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex);
            return false;
        }

        // Parse GOSUB field
        Expression::Numeric gosubValue;
        std::string gosubToken = gosubTokens[0];
        Expression::stripWhitespace(gosubToken);
        int labelIndex = Compiler::findLabel(gosubToken);
        if(labelIndex == -1)
        {
            Compiler::setCreateNumericLabelLut(true);

            Compiler::parseExpression(codeLineIndex, gosubToken, gosubValue);
            Compiler::emitVcpuAsm("STW", "numericLabel", false, codeLineIndex);

            // Default label exists
            if(gosubTokens.size() == 2)
            {
                std::string defaultToken = gosubTokens[1];
                Expression::stripWhitespace(defaultToken);
                labelIndex = Compiler::findLabel(defaultToken);
                if(labelIndex == -1)
                {
                    fprintf(stderr, "Keywords::keywordGOSUB() : Default label does not exist : in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex);
                    return false;
                }

                Compiler::getLabels()[labelIndex]._gosub = true;
                Compiler::emitVcpuAsm("LDWI", "_" + Compiler::getLabels()[labelIndex]._name, false, codeLineIndex);
            }
            // No default label
            else
            {
                Compiler::emitVcpuAsm("LDI", "0", false, codeLineIndex);
            }
            Compiler::emitVcpuAsm("STW", "defaultLabel", false, codeLineIndex);

            // Call gosubNumericLabel
            Compiler::emitVcpuAsm("%GosubNumeric", "", false, codeLineIndex);

            return true;
        }

        // CALL label
        Compiler::getLabels()[labelIndex]._gosub = true;

        if(Assembler::getUseOpcodeCALLI())
        {
            Compiler::emitVcpuAsm("CALLI", "_" + gosubToken, false, codeLineIndex);
        }
        else
        {
            Compiler::emitVcpuAsm("LDWI", "_" + gosubToken, false, codeLineIndex);
            Compiler::emitVcpuAsm("CALL", "giga_vAC", false, codeLineIndex);
        }

        return true;
    }

    bool keywordRETURN(Compiler::CodeLine& codeLine, int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(foundPos);
        UNREFERENCED_PARAM(tokenIndex);
        UNREFERENCED_PARAM(codeLine);

        // Use a macro instead of separate "POP" and "RET", otherwise page jumps could be inserted in between the "POP" and "RET" causing havoc and mayhem
        Compiler::emitVcpuAsm("%Return", "", false, codeLineIndex);

        return true;
    }

    bool keywordCLS(Compiler::CodeLine& codeLine, int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(foundPos);
        UNREFERENCED_PARAM(tokenIndex);

        if(codeLine._tokens.size() > 2)
        {
            fprintf(stderr, "Keywords::keywordCLS() : Syntax error in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex);
            return false;
        }

        if(codeLine._tokens.size() == 2)
        {
            std::string token = codeLine._tokens[1];
            Expression::strToUpper(token);
            if(token == "INIT")
            {
                Compiler::emitVcpuAsm("%ResetVideoTable", "", false, codeLineIndex);
            }
            else
            {
                Expression::Numeric param;
                Compiler::parseExpression(codeLineIndex, codeLine._tokens[1], param);
                Compiler::emitVcpuAsm("STW", "clsAddress", false, codeLineIndex);
                Compiler::emitVcpuAsm("%ClearScreen", "", false, codeLineIndex);
            }
        }
        else
        {
            Compiler::emitVcpuAsm("%ClearVertBlinds", "", false, codeLineIndex);
        }

        return true;
    }

    bool keywordPRINT(Compiler::CodeLine& codeLine, int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(tokenIndex);

        // Parse print tokens
        //std::vector<std::string> tokens = Expression::tokeniseLine(codeLine._code.substr(foundPos), ";");
        std::vector<std::string> tokens = Expression::tokenise(codeLine._code.substr(foundPos), ';', false, false);

        for(int i=0; i<int(tokens.size()); i++)
        {
            Expression::Numeric numeric;
            int varIndex = -1, constIndex = -1, strIndex = -1;
            uint32_t expressionType = Compiler::isExpression(tokens[i], varIndex, constIndex, strIndex);

            if((expressionType & Expression::HasStringKeywords)  &&  (expressionType & Expression::HasOptimisedPrint))
            {
                // Prints text on the fly without creating strings
                Expression::setEnableOptimisedPrint(true);
                Expression::parse(tokens[i], codeLineIndex, numeric);
                Expression::setEnableOptimisedPrint(false);
            }
            else if(expressionType & Expression::HasFunctions)
            {
                Expression::parse(tokens[i], codeLineIndex, numeric);
                if(numeric._varType == Expression::Number)
                {
                    Compiler::emitVcpuAsm("%PrintInt16", Expression::wordToHexString(int16_t(std::lround(numeric._value))), false, codeLineIndex);
                }
                else
                {
                    Compiler::emitVcpuAsm("LDW", Expression::byteToHexString(uint8_t(Compiler::getTempVarStart())), false, codeLineIndex);
                    Compiler::emitVcpuAsm("%PrintAcInt16", "", false, codeLineIndex);
                }
            }
            else if((expressionType & Expression::HasIntVars)  &&  (expressionType & Expression::HasOperators))
            {
                Expression::parse(tokens[i], codeLineIndex, numeric);
                if(numeric._varType == Expression::Number)
                {
                    Compiler::emitVcpuAsm("%PrintInt16", Expression::wordToHexString(int16_t(std::lround(numeric._value))), false, codeLineIndex);
                }
                else
                {
                    Compiler::emitVcpuAsm("LDW", Expression::byteToHexString(uint8_t(Compiler::getTempVarStart())), false, codeLineIndex);
                    Compiler::emitVcpuAsm("%PrintAcInt16", "", false, codeLineIndex);
                }
            }
            else if(expressionType & Expression::HasIntVars)
            {
                Expression::parse(tokens[i], codeLineIndex, numeric);
                if(varIndex >= 0)
                {
                    if(Compiler::getIntegerVars()[varIndex]._varType == Compiler::VarArray)
                    {
                        Compiler::emitVcpuAsm("LDW", Expression::byteToHexString(uint8_t(Compiler::getTempVarStart())), false, codeLineIndex);
                        Compiler::emitVcpuAsm("%PrintAcInt16", "", false, codeLineIndex);
                    }
                    else
                    {
                        switch(numeric._int16Byte)
                        {
                            case Expression::Int16Low:  Compiler::emitVcpuAsm("LD",  "_" + Compiler::getIntegerVars()[varIndex]._name,          false, codeLineIndex); break;
                            case Expression::Int16High: Compiler::emitVcpuAsm("LD",  "_" + Compiler::getIntegerVars()[varIndex]._name + " + 1", false, codeLineIndex); break;
                            case Expression::Int16Both: Compiler::emitVcpuAsm("LDW", "_" + Compiler::getIntegerVars()[varIndex]._name,          false, codeLineIndex); break;

                            default: break;
                        }

                        Compiler::emitVcpuAsm("%PrintAcInt16", "", false, codeLineIndex);
                    }
                }
                else
                {
                    Compiler::emitVcpuAsm("%PrintAcInt16", "", false, codeLineIndex);
                }
            }
            else if(expressionType & Expression::HasStrVars)
            {
                if(strIndex >= 0)
                {
                    std::string strName = Compiler::getStringVars()[strIndex]._name;
                    Compiler::emitVcpuAsm("%PrintString", "_" + strName, false, codeLineIndex);
                }
            }
            else if(expressionType & Expression::HasKeywords)
            {
                Expression::parse(tokens[i], codeLineIndex, numeric);
                Compiler::emitVcpuAsm("LDW", Expression::byteToHexString(uint8_t(Compiler::getTempVarStart())), false, codeLineIndex);
                Compiler::emitVcpuAsm("%PrintAcInt16", "", false, codeLineIndex);
            }
            else if(expressionType & Expression::HasOperators)
            {
                Expression::parse(tokens[i], codeLineIndex, numeric);
                Compiler::emitVcpuAsm("%PrintInt16", Expression::wordToHexString(int16_t(std::lround(numeric._value))), false, codeLineIndex);
            }
            else if(expressionType & Expression::HasStrings)
            {
                size_t lquote = tokens[i].find_first_of("\"");
                size_t rquote = tokens[i].find_first_of("\"", lquote + 1);
                if(lquote != std::string::npos  &&  rquote != std::string::npos)
                {
                    if(rquote == lquote + 1) continue; // skip empty strings
                    std::string str = tokens[i].substr(lquote + 1, rquote - (lquote + 1));

                    // Create string
                    std::string name;
                    uint16_t address;
                    if(Compiler::getOrCreateString(codeLine, codeLineIndex, str, name, address) == -1) return false;

                    // Print string
                    Compiler::emitVcpuAsm("%PrintString", "_" + name, false, codeLineIndex);
                }
            }
            else if(expressionType == Expression::HasStrConsts)
            {
                // Print constant string
                std::string internalName = Compiler::getConstants()[constIndex]._internalName;
                Compiler::emitVcpuAsm("%PrintString", "_" + internalName, false, codeLineIndex);
            }
            else if(expressionType == Expression::HasIntConsts)
            {
                // Print constant int
                int16_t data = Compiler::getConstants()[constIndex]._data;
                Compiler::emitVcpuAsm("%PrintInt16", Expression::wordToHexString(data), false, codeLineIndex);
            }
            else if(expressionType == Expression::HasNumbers)
            {
                // If valid expression
                if(Expression::parse(tokens[i], codeLineIndex, numeric))
                {
                    Compiler::emitVcpuAsm("%PrintInt16", Expression::wordToHexString(int16_t(std::lround(numeric._value))), false, codeLineIndex);
                }
            }
        }

        // New line
        if(codeLine._code[codeLine._code.size() - 1] != ';')
        {
            Compiler::emitVcpuAsm("%NewLine", "", false, codeLineIndex);
        }

        return true;
    }

    bool keywordINPUT(Compiler::CodeLine& codeLine, int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(tokenIndex);

        // Tokenise string and vars
        std::vector<std::string> strings;
        std::string text = codeLine._code.substr(foundPos);
        Expression::stripNonStringWhitespace(text);
        std::vector<std::string> tokens = Expression::tokenise(text, ',', false, false);
        std::string code = Expression::stripStrings(text, strings, true);
        std::vector<std::string> varTokens = Expression::tokenise(code, ',', false, false);

        if(varTokens.size() < 1  ||  (strings.size() > varTokens.size() + 1))
        {
            fprintf(stderr, "Keywords::keywordINPUT() : Syntax error in INPUT statement, must be 'INPUT <heading string>, <int/str var0>, <prompt string0>, ... <int/str varN>, <prompt stringN>', in '%s' on line %d\n",
                            codeLine._code.c_str(), codeLineIndex);
            return false;
        }

        // Print heading string
        bool foundHeadingString = false;
        if(tokens.size()  &&  Expression::isStringValid(tokens[0]))
        {
            size_t lquote = tokens[0].find_first_of("\"");
            size_t rquote = tokens[0].find_first_of("\"", lquote + 1);
            if(lquote != std::string::npos  &&  rquote != std::string::npos)
            {
                // Skip empty strings
                if(rquote > lquote + 1)
                {
                    std::string str = tokens[0].substr(lquote + 1, rquote - (lquote + 1));

                    // Create string
                    std::string name;
                    uint16_t address;
                    if(Compiler::getOrCreateString(codeLine, codeLineIndex, str, name, address) == -1) return false;

                    // Print string
                    Compiler::emitVcpuAsm("%PrintString", "_" + name, false, codeLineIndex);
                    foundHeadingString = true;
                }
            }
        }

        // INPUT vars/strs/types LUTs, (extra 0x0000 delimiter used by VASM runtime)
        std::vector<uint16_t> varsLut(varTokens.size());
        std::vector<uint16_t> strsLut(varTokens.size());
        std::vector<uint16_t> typesLut(varTokens.size() + 1, 0x0000);

        // Loop through vars
        for(int i=0; i<int(varTokens.size()); i++)
        {
            // Int var exists
            bool isStrVar = false;
            int intVar = Compiler::findVar(varTokens[i]);
            if(intVar >= 0)
            {
                varsLut[i] = Compiler::getIntegerVars()[intVar]._address;
                typesLut[i] = Compiler::VarInt16;
                continue;
            }
            // Str var exists
            else
            {
                if(varTokens[i].back() == '$'  &&  Expression::isVarNameValid(varTokens[i]))
                {
                    isStrVar = true;
                    int strIndex = Compiler::findStr(varTokens[i]);
                    if(strIndex >= 0)
                    {
                        varsLut[i] = Compiler::getStringVars()[strIndex]._address;
                        typesLut[i] = Compiler::VarStr;
                        continue;
                    }
                }
            }

            // Create int var
            int varIndex = 0;
            if(!isStrVar)
            {
                Compiler::createIntVar(varTokens[i], 0, 0, codeLine, codeLineIndex, false, varIndex);
                if(varIndex == -1) return false;
                varsLut[i] = Compiler::getIntegerVars()[varIndex]._address;
                typesLut[i] = Compiler::VarInt16;
            }
            // Create str var
            else
            {
                uint16_t address;
                varIndex = getOrCreateString(codeLine, codeLineIndex, "", varTokens[i], address, USER_STR_SIZE, false);
                if(varIndex == -1) return false;
                varsLut[i] = Compiler::getStringVars()[varIndex]._address;
                typesLut[i] = Compiler::VarStr;
            }
        }

        // Loop through strs
        for(int i=0; i<int(varTokens.size()); i++)
        {
            // Create string
            std::string name;
            uint16_t address;
            int index = (foundHeadingString) ? i + 1 : i;
            std::string str = (index < int(strings.size())) ? strings[index] : "\"?\";;";
            size_t fquote = str.find_first_of('"');
            size_t lquote = str.find_last_of('"');

            // Semicolons
            if(str.size() > lquote + 1  &&  str[lquote + 1] != ';') typesLut[i] |= 0x40;
            if(str.size() > lquote + 1  &&  str[lquote + 1] == ';') str.erase(lquote + 1, 1);
            if(str.back() != ';') typesLut[i] |= 0x80;
            if(str.back() == ';') str.erase(str.size() - 1, 1);

            // Text length field
            uint8_t length = USER_STR_SIZE;
            if(str.size() > lquote + 1  &&  isdigit((unsigned char)str[lquote + 1]))
            {
                std::string field = str.substr(lquote + 1);
                if(!Expression::stringToU8(field, length))
                {
                    fprintf(stderr, "Keywords::keywordINPUT() : Syntax error in text size field of string '%s' of INPUT statement, in '%s' on line %d\n", str.c_str(), codeLine._code.c_str(), codeLineIndex);
                    return false;
                }
                if(length > USER_STR_SIZE)
                {
                    fprintf(stderr, "Keywords::keywordINPUT() : Text size field > %d of string '%s' of INPUT statement, in '%s' on line %d\n", USER_STR_SIZE, str.c_str(), codeLine._code.c_str(), codeLineIndex);
                    return false;
                }

                str.erase(lquote + 1, field.size());
            }
            typesLut[i] |= (length + 1) << 8; // increment length as INPUT VASM code counts cursor
        
            // Remove quotes, (remove last quote first)
            str.erase(lquote, 1);
            str.erase(fquote, 1);

            if(Compiler::getOrCreateString(codeLine, codeLineIndex, str, name, address) == -1) return false;
            strsLut[i] = address;
        }

        // INPUT LUTs
        const int lutSize = 3;
        uint16_t lutAddr, varsAddr, strsAddr, typesAddr;
        if(!Memory::getFreeRAM(Memory::FitDescending, lutSize*2, USER_CODE_START, Compiler::getRuntimeStart(), lutAddr))
        {
            fprintf(stderr, "Keywords::keywordINPUT() : Not enough RAM for INPUT LUT of size %d, in '%s' on line %d\n", lutSize*2, codeLine._code.c_str(), codeLineIndex);
            return false;
        }
        if(!Memory::getFreeRAM(Memory::FitDescending, int(varsLut.size()*2), USER_CODE_START, Compiler::getRuntimeStart(), varsAddr))
        {
            fprintf(stderr, "Keywords::keywordINPUT() : Not enough RAM for INPUT Vars LUT of size %d, in '%s' on line %d\n", int(varsLut.size()*2), codeLine._code.c_str(), codeLineIndex);
            return false;
        }
        if(!Memory::getFreeRAM(Memory::FitDescending, int(strsLut.size()*2), USER_CODE_START, Compiler::getRuntimeStart(), strsAddr))
        {
            fprintf(stderr, "Keywords::keywordINPUT() : Not enough RAM for INPUT Strings LUT of size %d, in '%s' on line %d\n", int(strsLut.size()*2), codeLine._code.c_str(), codeLineIndex);
            return false;
        }
        if(!Memory::getFreeRAM(Memory::FitDescending, int(typesLut.size()*2), USER_CODE_START, Compiler::getRuntimeStart(), typesAddr))
        {
            fprintf(stderr, "Keywords::keywordINPUT() : Not enough RAM for INPUT Var Types LUT of size %d, in '%s' on line %d\n", int(typesLut.size()*2), codeLine._code.c_str(), codeLineIndex);
            return false;
        }
        Compiler::getCodeLines()[codeLineIndex]._inputLut = {lutAddr, varsAddr, strsAddr, typesAddr, varsLut, strsLut, typesLut}; // save LUT in global codeLine not local copy
        Compiler::emitVcpuAsm("LDWI", Expression::wordToHexString(lutAddr), false, codeLineIndex);
        Compiler::emitVcpuAsm("%Input", "", false, codeLineIndex);

        return true;
    }

    bool keywordFOR(Compiler::CodeLine& codeLine, int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(tokenIndex);

        bool optimise = true;
        int varIndex, constIndex, strIndex;
        uint32_t expressionType;

        // Parse first line of FOR loop
        std::string code = codeLine._code;
        Expression::strToUpper(code);
        size_t equals, to, step;
        if((equals = code.find("=")) == std::string::npos)
        {
            fprintf(stderr, "Keywords::keywordFOR() : Syntax error, (missing '='), in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex);
            return false;
        }
        
        bool farJump = (code.find("&TO") == std::string::npos);
        if((to = code.find("TO")) == std::string::npos)
        {
            fprintf(stderr, "Keywords::keywordFOR() : Syntax error, (missing 'TO'), in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex);
            return false;
        }
        step = code.find("STEP");

        // Maximum of 4 nested loops
        if(Compiler::getForNextDataStack().size() == 4)
        {
            fprintf(stderr, "Keywords::keywordFOR() : Syntax error, (maximum nested loops is 4), in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex);
            return false;
        }

        // Nested loops temporary variables
        uint16_t offset = uint16_t(Compiler::getForNextDataStack().size()) * 4;
        uint16_t varEnd = LOOP_VAR_START + offset;
        uint16_t varStep = LOOP_VAR_START + offset + 2;

        // Loop start
        int16_t loopStart = 0;
        int toOffset = (farJump) ? 0 : -1;
        std::string startToken = codeLine._code.substr(equals + 1, to - (equals + 1) + toOffset);
        Expression::stripWhitespace(startToken);
        expressionType = Compiler::isExpression(startToken, varIndex, constIndex, strIndex);
        if((expressionType & Expression::HasIntVars)  ||  (expressionType & Expression::HasKeywords)  ||  (expressionType & Expression::HasFunctions)) optimise = false;

        // Var counter, (create or update if being reused)
        std::string var = codeLine._code.substr(foundPos, equals - foundPos);
        Expression::stripWhitespace(var);
        int varCounter = Compiler::findVar(var);
        (varCounter < 0) ? Compiler::createIntVar(var, loopStart, 0, codeLine, codeLineIndex, false, varCounter) : Compiler::updateVar(loopStart, codeLine, varCounter, false);

        // Loop end
        int16_t loopEnd = 0;
        size_t end = (step == std::string::npos) ? codeLine._code.size() : step;
        std::string endToken = codeLine._code.substr(to + 2, end - (to + 2));
        Expression::stripWhitespace(endToken);
        expressionType = Compiler::isExpression(endToken, varIndex, constIndex, strIndex);
        if((expressionType & Expression::HasIntVars)  ||  (expressionType & Expression::HasKeywords)  ||  (expressionType & Expression::HasFunctions)) optimise = false;

        // Loop step
        int16_t loopStep = 1;
        std::string stepToken;
        if(step != std::string::npos)
        {
            end = codeLine._code.size();
            stepToken = codeLine._code.substr(step + 4, end - (step + 4));
            Expression::stripWhitespace(stepToken);
            expressionType = Compiler::isExpression(stepToken, varIndex, constIndex, strIndex);
            if((expressionType & Expression::HasIntVars)  ||  (expressionType & Expression::HasKeywords)  ||  (expressionType & Expression::HasFunctions)) optimise = false;
        }

        Expression::Numeric startNumeric, endNumeric, stepNumeric;
        if(optimise)
        {
            // Parse start
            Expression::parse(startToken, codeLineIndex, startNumeric);
            loopStart = int16_t(std::lround(startNumeric._value));

            // Parse end
            Expression::parse(endToken, codeLineIndex, endNumeric);
            loopEnd = int16_t(std::lround(endNumeric._value));

            // Parse step
            if(stepToken.size())
            {
                Expression::parse(stepToken, codeLineIndex, stepNumeric);
                loopStep = int16_t(std::lround(stepNumeric._value));
                if(abs(loopStep) > 255) optimise = false;
            }
            else
            {
                // Auto step based on start and end
                loopStep = (loopEnd >= loopStart) ? 1 : -1;
            }

            // 8bit constants
            if(optimise  &&  startNumeric._isValid  &&  loopStart >= 0  &&  loopStart <= 255  &&  endNumeric._isValid  &&  loopEnd >= 0  &&  loopEnd <= 255)
            {
                Compiler::emitVcpuAsm("LDI", std::to_string(loopStart), false, codeLineIndex);
                Compiler::emitVcpuAsm("STW", "_" + Compiler::getIntegerVars()[varCounter]._name, false, codeLineIndex);
            }
            // 16bit constants require variables
            else
            {
                optimise = false;

                Compiler::emitVcpuAsm("LDWI", std::to_string(loopStart), false, codeLineIndex);
                Compiler::emitVcpuAsm("STW", "_" + Compiler::getIntegerVars()[varCounter]._name, false, codeLineIndex);
                Compiler::emitVcpuAsm("LDWI", std::to_string(loopEnd), false, codeLineIndex);
                Compiler::emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(varEnd)), false, codeLineIndex);
                Compiler::emitVcpuAsm("LDWI", std::to_string(loopStep), false, codeLineIndex);
                Compiler::emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(varStep)), false, codeLineIndex);
            }
        }
        else
        {
            // Parse start
            Compiler::parseExpression(codeLineIndex, startToken, startNumeric);
            loopStart = int16_t(std::lround(startNumeric._value));
            Compiler::emitVcpuAsm("STW", "_" + Compiler::getIntegerVars()[varCounter]._name, false, codeLineIndex);

            // Parse end
            Compiler::parseExpression(codeLineIndex, endToken, endNumeric);
            loopEnd = int16_t(std::lround(endNumeric._value));
            Compiler::emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(varEnd)), false, codeLineIndex);

            // Parse step
            if(stepToken.size())
            {
                Compiler::parseExpression(codeLineIndex, stepToken, stepNumeric);
                loopStep = int16_t(std::lround(stepNumeric._value));
            }
            else
            {
                loopStep = 1;
                Compiler::emitVcpuAsm("LDI", std::to_string(loopStep), false, codeLineIndex);
            }
            Compiler::emitVcpuAsm("STW", Expression::byteToHexString(uint8_t(varStep)), false, codeLineIndex);
        }

        // Label and stack
        Compiler::setNextInternalLabel("_next_" + Expression::wordToHexString(Compiler::getVasmPC()));
        Compiler::getForNextDataStack().push({varCounter, Compiler::getNextInternalLabel(), loopEnd, loopStep, varEnd, varStep, farJump, optimise, codeLineIndex});

        return true;
    }

    bool keywordNEXT(Compiler::CodeLine& codeLine, int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(tokenIndex);

        if(codeLine._tokens.size() != 2)
        {
            fprintf(stderr, "Keywords::keywordNEXT() : Syntax error, (wrong number of tokens), in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex);
            return false;
        }

        std::string var = codeLine._code.substr(foundPos);
        int varIndex = Compiler::findVar(codeLine._tokens[1]);
        if(varIndex < 0)
        {
            fprintf(stderr, "Keywords::keywordNEXT() : Syntax error, (bad var), in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex);
            return false;
        }

        // Pop stack for this nested loop
        if(Compiler::getForNextDataStack().empty())
        {
            fprintf(stderr, "Keywords::keywordNEXT() : Syntax error, missing FOR statement, for '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex);
            return false;
        }
        Compiler::ForNextData forNextData = Compiler::getForNextDataStack().top();
        Compiler::getForNextDataStack().pop();

        if(varIndex != forNextData._varIndex)
        {
            fprintf(stderr, "Keywords::keywordNEXT() : Syntax error, (wrong var), in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex);
            return false;
        }

        std::string varName = Compiler::getIntegerVars()[varIndex]._name;
        std::string labName = forNextData._labelName;
        int16_t loopEnd = forNextData._loopEnd;
        int16_t loopStep = forNextData._loopStep;
        uint16_t varEnd = forNextData._varEnd;
        uint16_t varStep = forNextData._varStep;
        bool farJump = forNextData._farJump;
        bool optimise = forNextData._optimise;

        std::string forNextCmd;
        if(optimise)
        {
            // INC + BLE in ForNextFarInc will fail when loopEnd = 255
            if(abs(loopStep) == 1  &&  !(loopStep == 1  &&  loopEnd >= 255))
            {
                // Increment/decrement step
                forNextCmd = (loopStep > 0) ? (farJump) ? "%ForNextFarInc" : "%ForNextInc" : (farJump) ? "%ForNextFarDec" : "%ForNextDec";
                Compiler::emitVcpuAsm(forNextCmd, "_" + varName + " " + labName + " " + std::to_string(loopEnd), false, codeLineIndex);
            }
            else
            {
                // Additive/subtractive step
                forNextCmd = (loopStep > 0) ? (farJump) ? "%ForNextFarAdd" : "%ForNextAdd" : (farJump) ? "%ForNextFarSub" : "%ForNextSub";
                Compiler::emitVcpuAsm(forNextCmd, "_" + varName + " " + labName + " " + std::to_string(loopEnd) + " " + std::to_string(abs(loopStep)), false, codeLineIndex);
            }
        }
        else
        {
            // TODO: this can fail for corner cases
            // Positive/negative variable step
            forNextCmd = (loopStep > 0) ? (farJump) ? "%ForNextFarVarPos" : "%ForNextVarPos" : (farJump) ? "%ForNextFarVarNeg" : "%ForNextVarNeg";
            Compiler::emitVcpuAsm(forNextCmd, "_" + varName + " " + labName + " " + Expression::byteToHexString(uint8_t(varEnd)) + " " + Expression::byteToHexString(uint8_t(varStep)), false, codeLineIndex);
        }

        return true;
    }

    bool keywordIF(Compiler::CodeLine& codeLine, int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(tokenIndex);

        bool ifElseEndif = false;

        // IF
        std::string code = Compiler::getCodeLines()[codeLineIndex]._code;
        Expression::strToUpper(code);
        size_t offsetIF = code.find("IF");

        // THEN
        code = codeLine._code;
        Expression::strToUpper(code);
        size_t offsetTHEN = code.find("THEN");
        if(offsetTHEN == std::string::npos) ifElseEndif = true;

        // Condition
        Expression::Numeric condition;
        std::string conditionToken = codeLine._code.substr(foundPos, offsetTHEN - foundPos);
        Compiler::parseExpression(codeLineIndex, conditionToken, condition);
        if(condition._ccType == Expression::BooleanCC) Compiler::emitVcpuAsm("%JumpFalse", "", false, codeLineIndex); // Boolean condition requires this extra check
        int jmpIndex = int(Compiler::getCodeLines()[codeLineIndex]._vasm.size()) - 1;

        // Bail early as we assume this is an IF ELSE ENDIF block
        if(ifElseEndif)
        {
            Compiler::getElseIfDataStack().push({jmpIndex, "", codeLineIndex, Compiler::IfBlock, condition._ccType});
            return true;
        }

        // Action
        std::string actionToken = Compiler::getCodeLines()[codeLineIndex]._code.substr(offsetIF + offsetTHEN + 4);
        if(actionToken.size() == 0)
        {
            fprintf(stderr, "Keywords::keywordIF() : Syntax error, IF THEN <action>, (missing action), in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex);
            return false;
        }

        Expression::trimWhitespace(actionToken);
        std::string actionText = Expression::collapseWhitespaceNotStrings(actionToken);

        // Multi-statements
        int varIndex, strIndex;
        if(Compiler::parseMultiStatements(actionText, codeLine, codeLineIndex, varIndex, strIndex) == Compiler::StatementError) return false;

        // Create label on next line of vasm code
        Compiler::setNextInternalLabel("_else_" + Expression::wordToHexString(Compiler::getVasmPC()));
        std::string nextInternalLabel = Compiler::getNextInternalLabel() + " " + std::to_string(Compiler::getNextJumpFalseUniqueId());

        // Update if's jump to this new label
        Compiler::VasmLine* vasm = &Compiler::getCodeLines()[codeLineIndex]._vasm[jmpIndex];
        switch(condition._ccType)
        {
            case Expression::BooleanCC: vasm->_code = "JumpFalse" + std::string(OPCODE_TRUNC_SIZE - (sizeof("JumpFalse")-1), ' ') + nextInternalLabel; break;
            case Expression::NormalCC:  addLabelToJumpCC(Compiler::getCodeLines()[codeLineIndex]._vasm, nextInternalLabel);                            break;
            case Expression::FastCC:    addLabelToJumpCC(Compiler::getCodeLines()[codeLineIndex]._vasm, Compiler::getNextInternalLabel());             break;

            default: break;
        }

        return true;
    }

    bool keywordELSEIF(Compiler::CodeLine& codeLine, int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(tokenIndex);

        // Check stack for this IF ELSE ENDIF block
        if(Compiler::getElseIfDataStack().empty())
        {
            fprintf(stderr, "Keywords::keywordELSEIF() : Syntax error, missing IF statement, for '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex);
            return false;
        }

        Compiler::ElseIfData elseIfData = Compiler::getElseIfDataStack().top();
        int jmpIndex = elseIfData._jmpIndex;
        int codeIndex = elseIfData._codeLineIndex;
        Expression::CCType ccType = elseIfData._ccType;
        Compiler::getElseIfDataStack().pop();

        if(elseIfData._ifElseEndType != Compiler::IfBlock  &&  elseIfData._ifElseEndType != Compiler::ElseIfBlock)
        {
            fprintf(stderr, "Keywords::keywordELSEIF() : Syntax error, ELSEIF follows IF or ELSEIF, for '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex);
            return false;
        }

        // Jump to endif for previous BASIC line
        if(Assembler::getUseOpcodeCALLI())
        {
            Compiler::emitVcpuAsm("CALLI", "CALLI_JUMP", false, codeLineIndex - 1);
            Compiler::getEndIfDataStack().push({int(Compiler::getCodeLines()[codeLineIndex - 1]._vasm.size()) - 1, codeLineIndex - 1, ccType});
        }
        else
        {
            // There are no checks to see if this BRA's destination is in the same page, programmer discretion required when using this feature
            if(ccType == Expression::FastCC)
            {
                Compiler::emitVcpuAsm("BRA", "BRA_JUMP", false, codeLineIndex - 1);
                Compiler::getEndIfDataStack().push({int(Compiler::getCodeLines()[codeLineIndex - 1]._vasm.size()) - 1, codeLineIndex - 1, ccType});
            }
            else
            {
                Compiler::emitVcpuAsm("LDWI", "LDWI_JUMP", false, codeLineIndex - 1);
                Compiler::emitVcpuAsm("CALL", "giga_vAC", false, codeLineIndex - 1);
                Compiler::getEndIfDataStack().push({int(Compiler::getCodeLines()[codeLineIndex - 1]._vasm.size()) - 2, codeLineIndex - 1, ccType});
            }
        }

        // Create label on next line of vasm code
        Compiler::setNextInternalLabel("_elseif_" + Expression::wordToHexString(Compiler::getVasmPC()));
        std::string nextInternalLabel = Compiler::getNextInternalLabel() + " " + std::to_string(Compiler::getNextJumpFalseUniqueId());

        // Update if's jump to this new label
        Compiler::VasmLine* vasm = &Compiler::getCodeLines()[codeIndex]._vasm[jmpIndex];
        switch(ccType)
        {
            case Expression::BooleanCC: vasm->_code = "JumpFalse" + std::string(OPCODE_TRUNC_SIZE - (sizeof("JumpFalse")-1), ' ') + nextInternalLabel; break;
            case Expression::NormalCC:  addLabelToJumpCC(Compiler::getCodeLines()[codeIndex]._vasm, nextInternalLabel);                                break;
            case Expression::FastCC:    addLabelToJumpCC(Compiler::getCodeLines()[codeIndex]._vasm, Compiler::getNextInternalLabel());                 break;

            default: break;
        }

        // Condition
        Expression::Numeric condition;
        std::string conditionToken = codeLine._code.substr(foundPos);
        Compiler::parseExpression(codeLineIndex, conditionToken, condition);
        if(condition._ccType == Expression::BooleanCC) Compiler::emitVcpuAsm("%JumpFalse", "", false, codeLineIndex); // Boolean condition requires this extra check
        jmpIndex = int(Compiler::getCodeLines()[codeLineIndex]._vasm.size()) - 1;

        Compiler::getElseIfDataStack().push({jmpIndex, "", codeLineIndex, Compiler::ElseIfBlock, condition._ccType});

        return true;
    }

    bool keywordELSE(Compiler::CodeLine& codeLine, int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(foundPos);
        UNREFERENCED_PARAM(tokenIndex);

        if(codeLine._tokens.size() != 1)
        {
            fprintf(stderr, "Keywords::keywordELSE() : Syntax error, (wrong number of tokens), in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex);
            return false;
        }

        // Check stack for this IF ELSE ENDIF block
        if(Compiler::getElseIfDataStack().empty())
        {
            fprintf(stderr, "Keywords::keywordELSE() : Syntax error, missing IF statement, for '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex);
            return false;
        }

        Compiler::ElseIfData elseIfData = Compiler::getElseIfDataStack().top();
        int jmpIndex = elseIfData._jmpIndex;
        int codeIndex = elseIfData._codeLineIndex;
        Expression::CCType ccType = elseIfData._ccType;
        Compiler::getElseIfDataStack().pop();

        // Jump to endif for previous BASIC line
        if(Assembler::getUseOpcodeCALLI())
        {
            Compiler::emitVcpuAsm("CALLI", "CALLI_JUMP", false, codeLineIndex - 1);
            Compiler::getEndIfDataStack().push({int(Compiler::getCodeLines()[codeLineIndex - 1]._vasm.size()) - 1, codeLineIndex - 1, ccType});
        }
        else
        {
            // There are no checks to see if this BRA's destination is in the same page, programmer discretion required when using this feature
            if(ccType == Expression::FastCC)
            {
                Compiler::emitVcpuAsm("BRA", "BRA_JUMP", false, codeLineIndex - 1);
                Compiler::getEndIfDataStack().push({int(Compiler::getCodeLines()[codeLineIndex - 1]._vasm.size()) - 1, codeLineIndex - 1, ccType});
            }
            else
            {
                Compiler::emitVcpuAsm("LDWI", "LDWI_JUMP", false, codeLineIndex - 1);
                Compiler::emitVcpuAsm("CALL", "giga_vAC", false, codeLineIndex - 1);
                Compiler::getEndIfDataStack().push({int(Compiler::getCodeLines()[codeLineIndex - 1]._vasm.size()) - 2, codeLineIndex - 1, ccType});
            }
        }

        // Create label on next line of vasm code
        Compiler::setNextInternalLabel("_else_" + Expression::wordToHexString(Compiler::getVasmPC()));
        std::string nextInternalLabel = Compiler::getNextInternalLabel() + " " + std::to_string(Compiler::getNextJumpFalseUniqueId());

        // Update if's or elseif's jump to this new label
        Compiler::VasmLine* vasm = &Compiler::getCodeLines()[codeIndex]._vasm[jmpIndex];
        switch(ccType)
        {
            case Expression::BooleanCC: vasm->_code = "JumpFalse" + std::string(OPCODE_TRUNC_SIZE - (sizeof("JumpFalse")-1), ' ') + nextInternalLabel; break;
            case Expression::NormalCC:  addLabelToJumpCC(Compiler::getCodeLines()[codeIndex]._vasm, nextInternalLabel);                                break;
            case Expression::FastCC:    addLabelToJumpCC(Compiler::getCodeLines()[codeIndex]._vasm, Compiler::getNextInternalLabel());                 break;

            default: break;
        }

        Compiler::getElseIfDataStack().push({jmpIndex, nextInternalLabel, codeIndex, Compiler::ElseBlock, ccType});

        return true;
    }

    bool keywordENDIF(Compiler::CodeLine& codeLine, int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(foundPos);
        UNREFERENCED_PARAM(tokenIndex);

        if(codeLine._tokens.size() != 1)
        {
            fprintf(stderr, "Keywords::keywordENDIF() : Syntax error, (wrong number of tokens), in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex);
            return false;
        }

        // Check stack for this IF ELSE ENDIF block
        if(Compiler::getElseIfDataStack().empty())
        {
            fprintf(stderr, "Keywords::keywordENDIF() : Syntax error, missing IF statement, for '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex);
            return false;
        }

        Compiler::ElseIfData elseIfData = Compiler::getElseIfDataStack().top();
        int jmpIndex = elseIfData._jmpIndex;
        int codeIndex = elseIfData._codeLineIndex;
        Compiler::IfElseEndType ifElseEndType = elseIfData._ifElseEndType;
        Expression::CCType ccType = elseIfData._ccType;
        Compiler::getElseIfDataStack().pop();

        // Create label on next line of vasm code
        Compiler::setNextInternalLabel("_endif_" + Expression::wordToHexString(Compiler::getVasmPC()));
        std::string nextInternalLabel = Compiler::getNextInternalLabel() + " " + std::to_string(Compiler::getNextJumpFalseUniqueId());

        // Update if's/elseif's jump to this new label
        if(ifElseEndType == Compiler::IfBlock  ||  ifElseEndType == Compiler::ElseIfBlock)
        {
            Compiler::VasmLine* vasm = &Compiler::getCodeLines()[codeIndex]._vasm[jmpIndex];
            switch(ccType)
            {
                case Expression::BooleanCC: vasm->_code = "JumpFalse" + std::string(OPCODE_TRUNC_SIZE - (sizeof("JumpFalse")-1), ' ') + nextInternalLabel; break;
                case Expression::NormalCC:  addLabelToJumpCC(Compiler::getCodeLines()[codeIndex]._vasm, nextInternalLabel);                                break;
                case Expression::FastCC:    addLabelToJumpCC(Compiler::getCodeLines()[codeIndex]._vasm, Compiler::getNextInternalLabel());                 break;

                default: break;
            }
        }

        // Update elseif's and/or else's jump to endif label
        while(!Compiler::getEndIfDataStack().empty())
        {
            codeIndex = Compiler::getEndIfDataStack().top()._codeLineIndex;
            jmpIndex = Compiler::getEndIfDataStack().top()._jmpIndex;
            ccType = Compiler::getEndIfDataStack().top()._ccType;
            Compiler::VasmLine* vasm = &Compiler::getCodeLines()[codeIndex]._vasm[jmpIndex];
            switch(ccType)
            {
                case Expression::BooleanCC: 
                {
                    if(Assembler::getUseOpcodeCALLI())
                    {
                        vasm->_code = "CALLI" + std::string(OPCODE_TRUNC_SIZE - (sizeof("CALLI")-1), ' ') + Compiler::getNextInternalLabel();
                    }
                    else
                    {
                        vasm->_code = "LDWI" + std::string(OPCODE_TRUNC_SIZE - (sizeof("LDWI")-1), ' ') + Compiler::getNextInternalLabel();
                    }
                }
                break;

                case Expression::NormalCC:  addLabelToJump(Compiler::getCodeLines()[codeIndex]._vasm, Compiler::getNextInternalLabel()); break;
                case Expression::FastCC:    addLabelToJump(Compiler::getCodeLines()[codeIndex]._vasm, Compiler::getNextInternalLabel()); break;
        
                default: break;
            }

            Compiler::getEndIfDataStack().pop();
        }

        return true;
    }

    bool keywordWHILE(Compiler::CodeLine& codeLine, int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(tokenIndex);

        Compiler::setNextInternalLabel("_while_" + Expression::wordToHexString(Compiler::getVasmPC()));
        Compiler::getWhileWendDataStack().push({0, Compiler::getNextInternalLabel(), codeLineIndex, Expression::BooleanCC});

        // Condition
        Expression::Numeric condition;
        std::string conditionToken = codeLine._code.substr(foundPos);
        Compiler::parseExpression(codeLineIndex, conditionToken, condition);
        if(condition._ccType == Expression::BooleanCC) Compiler::emitVcpuAsm("%JumpFalse", "", false, codeLineIndex); // Boolean condition requires this extra check
        Compiler::getWhileWendDataStack().top()._jmpIndex = int(Compiler::getCodeLines()[codeLineIndex]._vasm.size()) - 1;
        Compiler::getWhileWendDataStack().top()._ccType = condition._ccType;

        return true;
    }

    bool keywordWEND(Compiler::CodeLine& codeLine, int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(foundPos);
        UNREFERENCED_PARAM(tokenIndex);

        // Pop stack for this WHILE loop
        if(Compiler::getWhileWendDataStack().empty())
        {
            fprintf(stderr, "Keywords::keywordWEND() : Syntax error, missing WHILE statement, for '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex);
            return false;
        }
        Compiler::WhileWendData whileWendData = Compiler::getWhileWendDataStack().top();
        Compiler::getWhileWendDataStack().pop();

        // Branch to WHILE and check condition again
        if(Assembler::getUseOpcodeCALLI())
        {
            Compiler::emitVcpuAsm("CALLI", whileWendData._labelName, false, codeLineIndex);
        }
        else
        {
            // There are no checks to see if this BRA's destination is in the same page, programmer discretion required when using this feature
            if(whileWendData._ccType == Expression::FastCC)
            {
                Compiler::emitVcpuAsm("BRA", whileWendData._labelName, false, codeLineIndex);
            }
            else
            {
                Compiler::emitVcpuAsm("LDWI", whileWendData._labelName, false, codeLineIndex);
                Compiler::emitVcpuAsm("CALL", "giga_vAC",      false, codeLineIndex);
            }
        }

        // Branch if condition false to instruction after WEND
        Compiler::setNextInternalLabel("_wend_" + Expression::wordToHexString(Compiler::getVasmPC()));
        Compiler::VasmLine* vasm = &Compiler::getCodeLines()[whileWendData._codeLineIndex]._vasm[whileWendData._jmpIndex];
        switch(whileWendData._ccType)
        {
            case Expression::BooleanCC:
            {
                vasm->_code = "JumpFalse" + std::string(OPCODE_TRUNC_SIZE - (sizeof("JumpFalse")-1), ' ') + Compiler::getNextInternalLabel() + " " + std::to_string(Compiler::getNextJumpFalseUniqueId());
            }
            break;

            case Expression::NormalCC:
            {
                addLabelToJumpCC(Compiler::getCodeLines()[whileWendData._codeLineIndex]._vasm, Compiler::getNextInternalLabel() + " " + std::to_string(Compiler::getNextJumpFalseUniqueId()));
            }
            break;

            case Expression::FastCC:
            {
                addLabelToJumpCC(Compiler::getCodeLines()[whileWendData._codeLineIndex]._vasm, Compiler::getNextInternalLabel());
            }
            break;

            default: break;
        }

        return true;
    }

    bool keywordREPEAT(Compiler::CodeLine& codeLine, int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(foundPos);
        UNREFERENCED_PARAM(tokenIndex);
        UNREFERENCED_PARAM(codeLine);

        Compiler::setNextInternalLabel("_repeat_" + Expression::wordToHexString(Compiler::getVasmPC()));
        Compiler::getRepeatUntilDataStack().push({Compiler::getNextInternalLabel(), codeLineIndex});

        return true;
    }

    bool keywordUNTIL(Compiler::CodeLine& codeLine, int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(tokenIndex);

        // Pop stack for this REPEAT loop
        if(Compiler::getRepeatUntilDataStack().empty())
        {
            fprintf(stderr, "Keywords::keywordUNTIL() : Syntax error, missing REPEAT statement, for '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex);
            return false;
        }
        Compiler::RepeatUntilData repeatUntilData = Compiler::getRepeatUntilDataStack().top();
        Compiler::getRepeatUntilDataStack().pop();

        // Condition
        Expression::Numeric condition;
        std::string conditionToken = codeLine._code.substr(foundPos);
        Compiler::parseExpression(codeLineIndex, conditionToken, condition);

        // Branch if condition false to instruction after REPEAT
        switch(condition._ccType)
        {
            case Expression::BooleanCC: Compiler::emitVcpuAsm("%JumpFalse", repeatUntilData._labelName + " " + std::to_string(Compiler::getNextJumpFalseUniqueId()), false, codeLineIndex);       break;
            case Expression::NormalCC:  addLabelToJumpCC(Compiler::getCodeLines()[codeLineIndex]._vasm, repeatUntilData._labelName + " " + std::to_string(Compiler::getNextJumpFalseUniqueId())); break;
            case Expression::FastCC:    addLabelToJumpCC(Compiler::getCodeLines()[codeLineIndex]._vasm, repeatUntilData._labelName);                                                              break;

            default: break;
        }

        return true;
    }

    bool keywordCONST(Compiler::CodeLine& codeLine, int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(tokenIndex);

        std::vector<std::string> tokens = Expression::tokenise(codeLine._code.substr(foundPos), '=', true);
        if(tokens.size() != 2)
        {
            fprintf(stderr, "Keywords::keywordCONST() : Syntax error, require a variable and an int or str constant, e.g. CONST a=50 or CONST a$=\"doggy\", in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex);
            return false;
        }

        Expression::stripWhitespace(tokens[0]);
        if(!Expression::isVarNameValid(tokens[0]))
        {
            fprintf(stderr, "Keywords::keywordCONST() : Syntax error, name must contain only alphanumerics and '$', in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex);
            return false;
        }

        // String
        if(tokens[0].back() == '$')
        {
            // Strip whitespace
            Expression::stripNonStringWhitespace(tokens[1]);
            if(Expression::isStringValid(tokens[1]))
            {
                uint16_t address;
                std::string internalName;

                // Strip quotes
                tokens[1].erase(0, 1);
                tokens[1].erase(tokens[1].size()-1, 1);

                // Don't count escape char '\'
                int escCount = 0;
                int strLength = int(tokens[1].size());
                for(int i=0; i<strLength; i++)
                {
                    if(tokens[1][i] == '\\') escCount++;
                }
                strLength -= escCount;

                Compiler::getOrCreateString(codeLine, codeLineIndex, tokens[1], internalName, address);
                Compiler::getConstants().push_back({uint8_t(strLength), 0, address, tokens[1], tokens[0], internalName, Compiler::ConstStr});
            }
            // String keyword
            else
            {
                size_t lbra, rbra;
                if(!Expression::findMatchingBrackets(tokens[1], 0, lbra, rbra))
                {
                    fprintf(stderr, "Keywords::keywordCONST() : Syntax error, invalid string or keyword, in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex);
                    return false;
                }

                std::string keywordToken = tokens[1].substr(0, lbra);
                std::string paramToken = tokens[1].substr(lbra + 1, rbra - (lbra + 1));
                Expression::strToUpper(keywordToken);
                if(_stringKeywords.find(keywordToken) == _stringKeywords.end())
                {
                    fprintf(stderr, "Keywords::keywordCONST() : Syntax error, invalid string or keyword, in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex);
                    return false;
                }

                int16_t param;
                if(!Expression::stringToI16(paramToken, param))
                {
                    fprintf(stderr, "Keywords::keywordCONST() : Syntax error, keyword param must be a constant number, in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex);
                    return false;
                }

                // Create constant string
                int index;
                uint8_t length = 0;
                uint16_t address = 0x0000;
                if(keywordToken == "CHR$")       {length = 1; address = Compiler::getOrCreateConstString(Compiler::StrChar, param, index);}
                else if(keywordToken == "HEX$")  {length = 2; address = Compiler::getOrCreateConstString(Compiler::StrHex,  param, index);}
                else if(keywordToken == "HEXW$") {length = 4; address = Compiler::getOrCreateConstString(Compiler::StrHexw, param, index);}

                // Create constant
                if(address)
                {
                    std::string internalName = Compiler::getStringVars().back()._name;
                    Compiler::getConstants().push_back({length, 0, address, Compiler::getStringVars().back()._text, tokens[0], internalName, Compiler::ConstStr});
                }
            }
        }
        // Integer
        else
        {
            Expression::stripWhitespace(tokens[1]);

            Expression::Numeric numeric(true); // true = static init
            Expression::parse(tokens[1], codeLineIndex, numeric);
            if(tokens[1].size() == 0  ||  !numeric._isValid  ||  numeric._varType == Expression::TmpVar  ||  numeric._varType == Expression::IntVar)
            {
                fprintf(stderr, "Keywords::keywordCONST() : Syntax error, invalid constant expression, in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex);
                return false;
            }

            Compiler::getConstants().push_back({2, int16_t(std::lround(numeric._value)), 0x0000, "", tokens[0], "_" + tokens[0], Compiler::ConstInt16});
        }

        return true;
    }

    bool keywordDIM(Compiler::CodeLine& codeLine, int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(tokenIndex);

        std::string dimText = codeLine._code.substr(foundPos);

        size_t lbra, rbra;
        if(!Expression::findMatchingBrackets(codeLine._code, foundPos, lbra, rbra))
        {
            fprintf(stderr, "Keywords::keywordDIM() : Syntax error in DIM statement, must be DIM var(x), in '%s' on line %d\n", codeLine._code.c_str(), codeLineIndex);
            return false;
        }

        // Positive constant size
        uint16_t arraySize;
        std::string sizeText = codeLine._code.substr(lbra + 1, rbra - (lbra + 1));
        int varIndex = -1, constIndex = -1, strIndex = -1;
        uint32_t expressionType = Compiler::isExpression(sizeText, varIndex, constIndex, strIndex);
        if(expressionType == Expression::HasIntConsts)
        {
            // Print constant int
            arraySize = Compiler::getConstants()[constIndex]._data;
        }
        else if(!Expression::stringToU16(sizeText, arraySize)  ||  arraySize <= 0)
        {
            fprintf(stderr, "Keywords::keywordDIM() : Array size must be a positive constant, found %s in '%s' on line %d\n", sizeText.c_str(), codeLine._code.c_str(), codeLineIndex);
            return false;
        }

        // Most BASIC's declared 0 to n elements, hence size = n + 1
        if(!Compiler::getArrayIndiciesOne())
        {
            arraySize++;
        }

        std::string varName = codeLine._code.substr(foundPos, lbra - foundPos);
        Expression::stripWhitespace(varName);

        // Var already exists?
        varIndex = Compiler::findVar(varName);
        if(varIndex >= 0)
        {
            fprintf(stderr, "Keywords::keywordDIM() : Var %s already exists in '%s' on line %d\n", varName.c_str(), codeLine._code.c_str(), codeLineIndex);
            return false;
        }

        // Optional array var init values
        uint16_t varInit = 0;
        std::vector<int16_t> arrInits;
        size_t varPos = codeLine._code.find("=");
        if(varPos != std::string::npos)
        {
            std::string varText = codeLine._code.substr(varPos + 1);
            Expression::stripWhitespace(varText);
            std::vector<std::string> varTokens = Expression::tokenise(varText, ',', true);
            if(varTokens.size() == 0)
            {
                fprintf(stderr, "Keywords::keywordDIM() : Initial value must be a constant, found %s in '%s' on line %d\n", varText.c_str(), codeLine._code.c_str(), codeLineIndex);
                return false;
            }
            else if(varTokens.size() == 1)
            {
                std::string operand;
                Expression::Numeric numeric(true); // true = static init
                Compiler::parseExpression(codeLineIndex, varTokens[0], operand, numeric);
                varInit = int16_t(std::lround(numeric._value));
            }
            else if(int(varTokens.size()) > arraySize)
            {
                fprintf(stderr, "Keywords::keywordDIM() : Too many initialisation values for size of array, found %d for a size of %d, in '%s' on line %d\n", int(varTokens.size()), arraySize,
                                                                                                                                                              codeLine._code.c_str(), codeLineIndex);
                return false;
            }
            // Multiple initialisation values
            else
            {
                std::string operand;
                arrInits.resize(varTokens.size());
                std::vector<Expression::Numeric> funcParams(varTokens.size(), Expression::Numeric(true)); // true = static init
                for(int i=0; i<int(varTokens.size()); i++)
                {
                    Compiler::parseExpression(codeLineIndex, varTokens[i], operand, funcParams[i]);
                    arrInits[i] = int16_t(std::lround(funcParams[i]._value));
                }
            }
        }

        arraySize *= 2;
        uint16_t arrayStart = 0x0000;
        if(!Memory::getFreeRAM(Memory::FitDescending, arraySize, USER_CODE_START, Compiler::getRuntimeStart(), arrayStart, false)) // arrays do not need to be contained within pages
        {
            fprintf(stderr, "Keywords::keywordDIM() : Not enough RAM for int array of size %d in '%s' on line %d\n", arraySize, codeLine._code.c_str(), codeLineIndex);
            return false;
        }

        createIntVar(varName, 0, varInit, codeLine, codeLineIndex, false, varIndex, arrInits, Compiler::VarArray, arrayStart, Compiler::Int16, arraySize);

        return true;
    }

    bool keywordDEF(Compiler::CodeLine& codeLine, int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(tokenIndex);

        std::string defText = codeLine._code.substr(foundPos);

        // Equals
        size_t equalsPos = codeLine._code.find("=");
        if(equalsPos == std::string::npos)
        {
            fprintf(stderr, "Keywords::keywordDEF() : Syntax error, missing equals sign, in '%s' on line %d\n", codeLine._code.c_str(), codeLineIndex);
            return false;
        }

        // Address field
        size_t typePos, lbra, rbra;
        uint16_t address = 0;
        bool foundAddress = false;
        bool foundFunction = false;
        std::string addrText, operand;
        std::vector<std::string> addrTokens;
        Expression::Numeric addrNumeric(true);  // true = static init
        if(Expression::findMatchingBrackets(codeLine._code, foundPos, lbra, rbra))
        {
            // Check for function generator
            addrText = codeLine._code.substr(lbra + 1, rbra - (lbra + 1));
            addrTokens = Expression::tokenise(addrText, ',', true);
            if(addrTokens.size() > 1)
            {
                foundFunction = true;
            }

            // Parse address field
            if(addrTokens.size() == 0)
            {
                fprintf(stderr, "Keywords::keywordDEF() : Address field does not exist, found %s in '%s' on line %d\n", addrText.c_str(), codeLine._code.c_str(), codeLineIndex);
                return false;
            }
            Compiler::parseExpression(codeLineIndex, addrTokens[0], operand, addrNumeric);
            address = uint16_t(std::lround(addrNumeric._value));
            typePos = lbra;
            foundAddress = true;
        }
        else
        {
            typePos = equalsPos;
        }

        // Type field
        std::string typeText = codeLine._code.substr(foundPos, typePos - foundPos);
        Expression::stripWhitespace(typeText);
        Expression::strToUpper(typeText);
        if(typeText != "BYTE"  &&  typeText != "WORD")
        {
            fprintf(stderr, "Keywords::keywordDEF() : Type field must be either BYTE or WORD, found %s in '%s' on line %d\n", typeText.c_str(), codeLine._code.c_str(), codeLineIndex);
            return false;
        }

        // ************************************************************************************************************
        // Function generator
        if(foundFunction)
        {
            if(addrTokens.size() < 4  ||  addrTokens.size() > 5)
            {
                fprintf(stderr, "Keywords::keywordDEF() : function generator must have 4 or 5 parameters, '(ADDR, <VAR>, START, STOP, SIZE)', (<VAR> is optional), found %d in '%s' on line %d\n",
                        int(addrTokens.size()), codeLine._code.c_str(), codeLineIndex);
                return false;
            }

            for(int i=0; i<int(addrTokens.size()); i++) Expression::stripWhitespace(addrTokens[i]);

            std::string function = codeLine._code.substr(equalsPos + 1);
            Expression::stripWhitespace(function);
            if(function.size() == 0)
            {
                fprintf(stderr, "Keywords::keywordDEF() : function '%s' is invalid in '%s' on line %d\n", function.c_str(), codeLine._code.c_str(), codeLineIndex);
                return false;
            }

            // Parse function variable
            bool foundVar = false;
            std::string funcVar;
            size_t varPos = 0;
            std::vector<size_t> varPositions;
            if(addrTokens.size() == 5)
            {
                foundVar = true;
                funcVar = addrTokens[1];
                bool foundVarFirstTime = false;
                for(;;)
                {
                    varPos = function.find(funcVar, varPos);
                    if(varPos == std::string::npos)
                    {
                        if(!foundVarFirstTime)
                        {
                            fprintf(stderr, "Keywords::keywordDEF() : function variable '%s' invalid in '%s' on line %d\n", funcVar.c_str(), codeLine._code.c_str(), codeLineIndex);
                            return false;
                        }

                        // Found all occurenced of function variable
                        break;
                    }
                    function.erase(varPos, funcVar.size());
                    varPositions.push_back(varPos);
                    foundVarFirstTime = true;
                    varPos++;
                }
            }

            // Parse function parameters
            int paramsOffset = (foundVar) ? 2 : 1;
            std::vector<Expression::Numeric> funcParams = {Expression::Numeric(true), Expression::Numeric(true), Expression::Numeric(true)}; // true = static init
            for(int i=0; i<int(funcParams.size()); i++)
            {
                Compiler::parseExpression(codeLineIndex, addrTokens[i + paramsOffset], operand, funcParams[i]);
            }
            if(funcParams[2]._value == 0.0)
            {
                fprintf(stderr, "Keywords::keywordDEF() : Divide by zero detected in '%s' : '%s' : on line %d\n", function.c_str(), codeLine._code.c_str(), codeLineIndex);
                return false;
            }

            // Evaluate function
            double start = funcParams[0]._value;
            double end = funcParams[1]._value;
            double count = fabs(funcParams[2]._value);
            double step = (end - start) / count;
            std::vector<int16_t> funcData;
            for(double d=start; d<end; d+=step)
            {
                std::string var;
                if(foundVar)
                {
                    // Insert substitute values into var's positions
                    var = std::to_string(d);
                    for(int i=0; i<int(varPositions.size()); i++)
                    {
                        function.insert(varPositions[i] + i*var.size(), var);
                    }
                }

                Expression::Numeric funcResult;
                Compiler::parseExpression(codeLineIndex, function, operand, funcResult);
                funcData.push_back(int16_t(std::lround(funcResult._value)));

                if(foundVar)
                {
                    // Erase every occurence of substitute values
                    for(int i=0; i<int(varPositions.size()); i++)
                    {
                        function.erase(varPositions[i], var.size());
                    }
                }
            }

            if(typeText == "BYTE")
            {
                std::vector<uint8_t> dataBytes(int(count), 0);
                for(int i=0; i<int(dataBytes.size()); i++) dataBytes[i] = uint8_t(funcData[i]);
                if(!Memory::takeFreeRAM(address, int(dataBytes.size()))) return false;
                Compiler::getDefDataBytes().push_back({address, dataBytes});
            }
            else if(typeText == "WORD")
            {
                std::vector<int16_t> dataWords(int(count), 0);
                for(int i=0; i<int(dataWords.size()); i++) dataWords[i] = int16_t(funcData[i]);
                if(!Memory::takeFreeRAM(address, int(dataWords.size()) * 2)) return false;
                Compiler::getDefDataWords().push_back({address, dataWords});
            }

            return true;
        }

        // ************************************************************************************************************
        // Data fields
        std::vector<std::string> dataTokens = Expression::tokenise(codeLine._code.substr(equalsPos + 1), ',', true);
        if(dataTokens.size() == 0)
        {
            fprintf(stderr, "Keywords::keywordDEF() : Syntax error, require at least one data field in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex);
            return false;
        }

        // BYTE data
        if(typeText == "BYTE")
        {
            std::vector<uint8_t> dataBytes;
            for(int i=0; i<int(dataTokens.size()); i++)
            {
                Expression::Numeric numeric(true); // true = static init
                Compiler::parseExpression(codeLineIndex, dataTokens[i], operand, numeric);
                dataBytes.push_back(uint8_t(std::lround(numeric._value)));
            }

            // New address entry
            if(foundAddress)
            {
                if(address >= DEFAULT_START_ADDRESS  &&  !Memory::takeFreeRAM(address, int(dataBytes.size()))) return false;
                Compiler::getDefDataBytes().push_back({address, dataBytes});
            }
            // Update current address data
            else
            {
                // Address
                address = Compiler::getDefDataBytes().back()._address + uint16_t(Compiler::getDefDataBytes().back()._data.size());

                // Mark new RAM chunk as used
                if(address >= DEFAULT_START_ADDRESS  &&  !Memory::takeFreeRAM(address, int(dataBytes.size()))) return false;

                // Append data
                for(int i=0; i<int(dataBytes.size()); i++) Compiler::getDefDataBytes().back()._data.push_back(dataBytes[i]);
            }
        }
        // WORD data
        else if(typeText == "WORD")
        {
            std::vector<int16_t> dataWords;
            for(int i=0; i<int(dataTokens.size()); i++)
            {
                Expression::Numeric numeric(true); // true = static init
                Compiler::parseExpression(codeLineIndex, dataTokens[i], operand, numeric);
                dataWords.push_back(int16_t(std::lround(numeric._value)));
            }

            // New address entry
            if(foundAddress)
            {
                if(address >= DEFAULT_START_ADDRESS  &&  !Memory::takeFreeRAM(address, int(dataWords.size()) * 2)) return false;
                Compiler::getDefDataWords().push_back({address, dataWords});
            }
            // Update current address data
            else
            {
                // Address
                address = Compiler::getDefDataWords().back()._address + uint16_t(Compiler::getDefDataWords().back()._data.size()) * 2;

                // Mark new RAM chunk as used
                if(address >= DEFAULT_START_ADDRESS  &&  !Memory::takeFreeRAM(address, int(dataWords.size()) * 2)) return false;

                // Append data
                for(int i=0; i<int(dataWords.size()); i++) Compiler::getDefDataWords().back()._data.push_back(dataWords[i]);
            }
        }

        return true;
    }

    bool keywordALLOC(Compiler::CodeLine& codeLine, int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(tokenIndex);

        std::vector<std::string> tokens = Expression::tokenise(codeLine._code.substr(foundPos), ',', false);
        if(tokens.size() < 1  &&  tokens.size() > 4)
        {
            fprintf(stderr, "Keywords::keywordALLOC() : Syntax error, 'ALLOC <address>, <optional size>, <optional count>, <optional offset=0x0100>', in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex);
            return false;
        }

        int count = 1;
        uint16_t address, end, size = 0x0000, offset = 0x0100;
        std::string addrOperand, sizeOperand, countOperand, offsetOperand;
        Expression::Numeric addrNumeric(true), sizeNumeric(true), countNumeric(true), offsetNumeric(true);
        Compiler::parseExpression(codeLineIndex, tokens[0], addrOperand, addrNumeric);
        if(tokens.size() >= 2)
        {
            Compiler::parseExpression(codeLineIndex, tokens[1], sizeOperand, sizeNumeric);
            size = uint16_t(std::lround(sizeNumeric._value));
        }
        if(tokens.size() >= 3)
        {
            Compiler::parseExpression(codeLineIndex, tokens[2], countOperand, countNumeric);
            count = std::lround(countNumeric._value);
        }
        if(tokens.size() >= 4)
        {
            Compiler::parseExpression(codeLineIndex, tokens[3], offsetOperand, offsetNumeric);
            offset = uint16_t(std::lround(offsetNumeric._value));
            if(count == 0  ||  offset == 0)
            {
                fprintf(stderr, "Keywords::keywordALLOC() : Count and offset must both be non zero, found %d and 0x%04x in '%s' on line %d\n", count, offset, codeLine._code.c_str(), codeLineIndex);
                return false;
            }
        }

        address = uint16_t(std::lround(addrNumeric._value));
        if(address < DEFAULT_START_ADDRESS)
        {
            fprintf(stderr, "Keywords::keywordALLOC() : Address field must be above %04x, found %s in '%s' on line %d\n", DEFAULT_START_ADDRESS, tokens[0].c_str(), codeLine._code.c_str(), codeLineIndex);
            return false;
        }

        end = (size == 0) ? 0xFFFF : address + size;
        for(int i=0; i<count; i++)
        {
            //fprintf(stderr, "0x%04x 0x%04x %d\n", address, end, end - address);
            for(uint16_t j=address; j<end; j++)
            {
                if(!Memory::takeFreeRAM(j, 1, false))
                {
                    fprintf(stderr, "Keywords::keywordALLOC() : Trying to allocate already allocated memory at 0x%04x in '%s' on line %d\n", j, codeLine._code.c_str(), codeLineIndex);
                    return false;
                }
            }
            address += offset;
            end += offset;
        }
    
        //Memory::printFreeRamList(Memory::NoSort);

        return true;
    }

    bool keywordFREE(Compiler::CodeLine& codeLine, int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(tokenIndex);

        std::vector<std::string> tokens = Expression::tokenise(codeLine._code.substr(foundPos), ',', false);
        if(tokens.size() != 2)
        {
            fprintf(stderr, "Keywords::keywordFREE() : Syntax error, 'FREE <address>, <size>', in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex);
            return false;
        }

        Expression::Numeric addrNumeric(true), sizeNumeric(true);  // true = static init
        std::string addrOperand, sizeOperand;
        Compiler::parseExpression(codeLineIndex, tokens[0], addrOperand, addrNumeric);
        Compiler::parseExpression(codeLineIndex, tokens[1], sizeOperand, sizeNumeric);
        uint16_t address = uint16_t(std::lround(addrNumeric._value));
        uint16_t size = uint16_t(std::lround(sizeNumeric._value));

        //Memory::printFreeRamList(Memory::NoSort);
        Memory::giveFreeRAM(address, size);
        //Memory::printFreeRamList(Memory::NoSort);

        return true;
    }

    bool keywordAT(Compiler::CodeLine& codeLine, int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(tokenIndex);

        std::vector<std::string> tokens = Expression::tokenise(codeLine._code.substr(foundPos), ',', false);
        if(tokens.size() < 1  ||  tokens.size() > 2)
        {
            fprintf(stderr, "Keywords::keywordAT() : Syntax error, 'AT <x>' or 'AT <x>, <y>', in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex);
            return false;
        }

        Expression::Numeric numeric;
        Compiler::parseExpression(codeLineIndex, tokens[0], numeric);
        Compiler::emitVcpuAsm("ST", "cursorXY", false, codeLineIndex);

        if(tokens.size() == 2)
        {
            Compiler::parseExpression(codeLineIndex, tokens[1], numeric);
            Compiler::emitVcpuAsm("ST", "cursorXY + 1", false, codeLineIndex);
        }

        Compiler::emitVcpuAsm("%AtTextCursor", "", false, codeLineIndex);
        return true;
    }

    bool keywordPUT(Compiler::CodeLine& codeLine, int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(tokenIndex);

        std::vector<std::string> tokens = Expression::tokenise(codeLine._code.substr(foundPos), ',', false);
        if(tokens.size() != 1)
        {
            fprintf(stderr, "Keywords::keywordPUT() : Syntax error, 'PUT <ascii>', in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex);
            return false;
        }

        Expression::Numeric numeric;
        Compiler::parseExpression(codeLineIndex, tokens[0], numeric);
        Compiler::emitVcpuAsm("%PrintAcChar", "", false, codeLineIndex);

        return true;
    }

    bool keywordMODE(Compiler::CodeLine& codeLine, int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(tokenIndex);

        std::vector<std::string> tokens = Expression::tokenise(codeLine._code.substr(foundPos), ',', false);
        if(tokens.size() != 1)
        {
            fprintf(stderr, "Keywords::keywordMODE() : Syntax error, 'MODE <0 - 3>', in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex);
            return false;
        }

        Expression::Numeric numeric;
        Compiler::parseExpression(codeLineIndex, tokens[0], numeric);
        Compiler::emitVcpuAsm("STW", "graphicsMode", false, codeLineIndex);
        Compiler::emitVcpuAsm("%ScanlineMode", "",   false, codeLineIndex);

        return true;
    }

    bool keywordWAIT(Compiler::CodeLine& codeLine, int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(tokenIndex);

        std::vector<std::string> tokens = Expression::tokenise(codeLine._code.substr(foundPos), ',', false);
        if(tokens.size() > 1)
        {
            fprintf(stderr, "Keywords::keywordWAIT() : Syntax error, 'WAIT <optional vblank count>', in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex);
            return false;
        }

        if(tokens.size() == 0)
        {
            Compiler::emitVcpuAsm("%WaitVBlank", "", false, codeLineIndex);
            return true;
        }

        Expression::Numeric numeric;
        Compiler::parseExpression(codeLineIndex, tokens[0], numeric);
        Compiler::emitVcpuAsm("STW", "waitVBlankNum", false, codeLineIndex);
        Compiler::emitVcpuAsm("%WaitVBlanks", "",     false, codeLineIndex);

        return true;
    }

    bool keywordPSET(Compiler::CodeLine& codeLine, int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(tokenIndex);

        std::vector<std::string> tokens = Expression::tokenise(codeLine._code.substr(foundPos), ',', false);
        if(tokens.size() < 2  ||  tokens.size() > 3)
        {
            fprintf(stderr, "Keywords::keywordPSET() : Syntax error, 'PSET <x>, <y>' or 'PSET <x>, <y>, <colour>', in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex);
            return false;
        }

        Expression::Numeric numeric;
        Compiler::parseExpression(codeLineIndex, tokens[0], numeric);
        Compiler::emitVcpuAsm("ST", "drawPixel_xy", false, codeLineIndex);
        Compiler::parseExpression(codeLineIndex, tokens[1], numeric);
        Compiler::emitVcpuAsm("ST", "drawPixel_xy + 1", false, codeLineIndex);

        if(tokens.size() == 3)
        {
            Compiler::parseExpression(codeLineIndex, tokens[2], numeric);
            Compiler::emitVcpuAsm("ST", "fgbgColour + 1", false, codeLineIndex);
        }

        Compiler::emitVcpuAsm("%DrawPixel", "", false, codeLineIndex);
        return true;
    }

    bool keywordLINE(Compiler::CodeLine& codeLine, int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(tokenIndex);

        std::vector<std::string> tokens = Expression::tokenise(codeLine._code.substr(foundPos), ',', false);
        if(tokens.size() != 2  &&  tokens.size() != 4)
        {
            fprintf(stderr, "Keywords::keywordLINE() : Syntax error, 'LINE <x>, <y>' or 'LINE <x1>, <y1>, <x2>, <y2>', in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex);
            return false;
        }

        if(tokens.size() == 2)
        {
            std::vector<Expression::Numeric> params = {Expression::Numeric(), Expression::Numeric()};
            for(int i=0; i<int(tokens.size()); i++)
            {
                Compiler::parseExpression(codeLineIndex, tokens[i], params[i]);
                switch(i)
                {
                    case 0: Compiler::emitVcpuAsm("STW", "drawLine_x2", false, codeLineIndex); break;
                    case 1: Compiler::emitVcpuAsm("STW", "drawLine_y2", false, codeLineIndex); break;

                    default: break;
                }
            }

            Compiler::emitVcpuAsm("%AtLineCursor", "", false, codeLineIndex);
            Compiler::emitVcpuAsm("%DrawVTLine", "", false, codeLineIndex);
        }
        else
        {
            std::vector<Expression::Numeric> params = {Expression::Numeric(), Expression::Numeric(), Expression::Numeric(), Expression::Numeric()};
            for(int i=0; i<int(tokens.size()); i++)
            {
                Compiler::parseExpression(codeLineIndex, tokens[i], params[i]);
                switch(i)
                {
                    case 0: Compiler::emitVcpuAsm("STW", "drawLine_x1", false, codeLineIndex); break;
                    case 1: Compiler::emitVcpuAsm("STW", "drawLine_y1", false, codeLineIndex); break;
                    case 2: Compiler::emitVcpuAsm("STW", "drawLine_x2", false, codeLineIndex); break;
                    case 3: Compiler::emitVcpuAsm("STW", "drawLine_y2", false, codeLineIndex); break;

                    default: break;
                }
            }

            Compiler::emitVcpuAsm("%DrawLine", "", false, codeLineIndex);
        }

        return true;
    }

    bool keywordHLINE(Compiler::CodeLine& codeLine, int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(tokenIndex);

        std::vector<std::string> tokens = Expression::tokenise(codeLine._code.substr(foundPos), ',', false);
        if(tokens.size() != 3)
        {
            fprintf(stderr, "Keywords::keywordHLINE() : Syntax error, 'HLINE <x1>, <y>, <x2>', in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex);
            return false;
        }

        std::vector<Expression::Numeric> params = {Expression::Numeric(), Expression::Numeric(), Expression::Numeric()};
        for(int i=0; i<int(tokens.size()); i++)
        {
            Compiler::parseExpression(codeLineIndex, tokens[i], params[i]);
            switch(i)
            {
                case 0: Compiler::emitVcpuAsm("STW", "drawHLine_x1", false, codeLineIndex); break;
                case 1: Compiler::emitVcpuAsm("STW", "drawHLine_y1", false, codeLineIndex); break;
                case 2: Compiler::emitVcpuAsm("STW", "drawHLine_x2", false, codeLineIndex); break;

                default: break;
            }
        }

        Compiler::emitVcpuAsm("%DrawHLine", "", false, codeLineIndex);

        return true;
    }

    bool keywordVLINE(Compiler::CodeLine& codeLine, int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(tokenIndex);

        std::vector<std::string> tokens = Expression::tokenise(codeLine._code.substr(foundPos), ',', false);
        if(tokens.size() != 3)
        {
            fprintf(stderr, "Keywords::keywordVLINE() : Syntax error, 'VLINE <x>, <y1>, <y2>', in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex);
            return false;
        }

        std::vector<Expression::Numeric> params = {Expression::Numeric(), Expression::Numeric(), Expression::Numeric()};
        for(int i=0; i<int(tokens.size()); i++)
        {
            Compiler::parseExpression(codeLineIndex, tokens[i], params[i]);
            switch(i)
            {
                case 0: Compiler::emitVcpuAsm("STW", "drawVLine_x1", false, codeLineIndex); break;
                case 1: Compiler::emitVcpuAsm("STW", "drawVLine_y1", false, codeLineIndex); break;
                case 2: Compiler::emitVcpuAsm("STW", "drawVLine_y2", false, codeLineIndex); break;

                default: break;
            }
        }

        Compiler::emitVcpuAsm("%DrawVLine", "", false, codeLineIndex);

        return true;
    }

    bool keywordCIRCLE(Compiler::CodeLine& codeLine, int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(tokenIndex);

        std::vector<std::string> tokens = Expression::tokenise(codeLine._code.substr(foundPos), ',', false);
        if(tokens.size() != 3)
        {
            fprintf(stderr, "Keywords::keywordCIRCLE() : Syntax error, 'CIRCLE <x>, <y>, <radius>', in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex);
            return false;
        }

        std::vector<Expression::Numeric> params = {Expression::Numeric(), Expression::Numeric(), Expression::Numeric()};
        for(int i=0; i<int(tokens.size()); i++)
        {
            Compiler::parseExpression(codeLineIndex, tokens[i], params[i]);
            switch(i)
            {
                case 0: Compiler::emitVcpuAsm("STW",  "drawCircle_cx", false, codeLineIndex);                                                          break;
                case 1: Compiler::emitVcpuAsm("ADDI", "8", false, codeLineIndex); Compiler::emitVcpuAsm("STW", "drawCircle_cy", false, codeLineIndex); break;
                case 2: Compiler::emitVcpuAsm("STW",  "drawCircle_r",  false, codeLineIndex);                                                          break;

                default: break;
            }
        }

        Compiler::emitVcpuAsm("%DrawCircle", "", false, codeLineIndex);

        return true;
    }

    bool keywordCIRCLEF(Compiler::CodeLine& codeLine, int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(tokenIndex);

        std::vector<std::string> tokens = Expression::tokenise(codeLine._code.substr(foundPos), ',', false);
        if(tokens.size() != 3)
        {
            fprintf(stderr, "Keywords::keywordCIRCLEF() : Syntax error, 'CIRCLEF <x>, <y>, <radius>', in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex);
            return false;
        }

        std::vector<Expression::Numeric> params = {Expression::Numeric(), Expression::Numeric(), Expression::Numeric()};
        for(int i=0; i<int(tokens.size()); i++)
        {
            Compiler::parseExpression(codeLineIndex, tokens[i], params[i]);
            switch(i)
            {
                case 0: Compiler::emitVcpuAsm("STW", "drawCircleF_cx", false, codeLineIndex); break;
                case 1: Compiler::emitVcpuAsm("STW", "drawCircleF_cy", false, codeLineIndex); break;
                case 2: Compiler::emitVcpuAsm("STW", "drawCircleF_r",  false, codeLineIndex); break;

                default: break;
            }
        }

        Compiler::emitVcpuAsm("%DrawCircleF", "", false, codeLineIndex);

        return true;
    }

    bool keywordRECT(Compiler::CodeLine& codeLine, int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(tokenIndex);

        std::vector<std::string> tokens = Expression::tokenise(codeLine._code.substr(foundPos), ',', false);
        if(tokens.size() != 4)
        {
            fprintf(stderr, "Keywords::keywordRECT() : Syntax error, 'RECT <x1>, <y1>, <x2>, <y2>', in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex);
            return false;
        }

        std::vector<Expression::Numeric> params = {Expression::Numeric(), Expression::Numeric(), Expression::Numeric(), Expression::Numeric()};
        for(int i=0; i<int(tokens.size()); i++)
        {
            Compiler::parseExpression(codeLineIndex, tokens[i], params[i]);
            switch(i)
            {
                case 0: Compiler::emitVcpuAsm("STW", "drawRect_x1", false, codeLineIndex); break;
                case 1: Compiler::emitVcpuAsm("STW", "drawRect_y1", false, codeLineIndex); break;
                case 2: Compiler::emitVcpuAsm("STW", "drawRect_x2", false, codeLineIndex); break;
                case 3: Compiler::emitVcpuAsm("STW", "drawRect_y2", false, codeLineIndex); break;

                default: break;
            }
        }

        Compiler::emitVcpuAsm("%DrawRect", "", false, codeLineIndex);

        return true;
    }

    bool keywordRECTF(Compiler::CodeLine& codeLine, int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(tokenIndex);

        std::vector<std::string> tokens = Expression::tokenise(codeLine._code.substr(foundPos), ',', false);
        if(tokens.size() != 4)
        {
            fprintf(stderr, "Keywords::keywordRECTF() : Syntax error, 'RECTF <x1>, <y1>, <x2>, <y2>', in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex);
            return false;
        }

        std::vector<Expression::Numeric> params = {Expression::Numeric(), Expression::Numeric(), Expression::Numeric(), Expression::Numeric()};
        for(int i=0; i<int(tokens.size()); i++)
        {
            Compiler::parseExpression(codeLineIndex, tokens[i], params[i]);
            switch(i)
            {
                case 0: Compiler::emitVcpuAsm("STW", "drawRectF_x1", false, codeLineIndex); break;
                case 1: Compiler::emitVcpuAsm("STW", "drawRectF_y1", false, codeLineIndex); break;
                case 2: Compiler::emitVcpuAsm("STW", "drawRectF_x2", false, codeLineIndex); break;
                case 3: Compiler::emitVcpuAsm("STW", "drawRectF_y2", false, codeLineIndex); break;

                default: break;
            }
        }

        Compiler::emitVcpuAsm("%DrawRectF", "", false, codeLineIndex);

        return true;
    }

    bool keywordPOLY(Compiler::CodeLine& codeLine, int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(tokenIndex);

        std::vector<std::string> tokens = Expression::tokenise(codeLine._code.substr(foundPos), ',', false);
        if(tokens.size() != 1)
        {
            fprintf(stderr, "Keywords::keywordPOLY() : Syntax error, 'POLY <coords address>', in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex);
            return false;
        }

        Expression::Numeric param;
        Compiler::parseExpression(codeLineIndex, tokens[0], param);
        Compiler::emitVcpuAsm("STW", "drawPoly_addr", false, codeLineIndex);
        Compiler::emitVcpuAsm("%DrawPoly", "", false, codeLineIndex);

        return true;
    }

    bool keywordSCROLL(Compiler::CodeLine& codeLine, int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(tokenIndex);

        std::vector<std::string> tokens = Expression::tokenise(codeLine._code.substr(foundPos), ' ', false);
        if(tokens.size() != 1)
        {
            fprintf(stderr, "Keywords::keywordSCROLL() : Syntax error, 'SCROLL ON' or 'SCROLL OFF', in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex);
            return false;
        }

        std::string scrollToken = Expression::strToUpper(tokens[0]);
        Expression::stripWhitespace(scrollToken);
        if(scrollToken != "ON"  &&  scrollToken != "OFF")
        {
            fprintf(stderr, "Keywords::keywordSCROLL() : Syntax error, 'SCROLL ON' or 'SCROLL OFF', in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex);
            return false;
        }

        if(scrollToken == "ON")
        {
            Compiler::emitVcpuAsm("LDWI", "0x0001", false, codeLineIndex);
            Compiler::emitVcpuAsm("ORW", "miscFlags", false, codeLineIndex);
        }
        else
        {
            Compiler::emitVcpuAsm("LDWI", "0xFFFE", false, codeLineIndex);
            Compiler::emitVcpuAsm("ANDW", "miscFlags", false, codeLineIndex);
        }
        Compiler::emitVcpuAsm("STW", "miscFlags", false, codeLineIndex);

        return true;
    }

    bool keywordPOKE(Compiler::CodeLine& codeLine, int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(tokenIndex);

        std::vector<std::string> tokens = Expression::tokenise(codeLine._code.substr(foundPos), ',', false);
        if(tokens.size() != 2)
        {
            fprintf(stderr, "Keywords::keywordPOKE() : Syntax error, 'POKE <address>, <value>', in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex);
            return false;
        }

        std::vector<std::string> operands = {"", ""};
        std::vector<Expression::Numeric> numerics = {Expression::Numeric(), Expression::Numeric()};
        std::vector<Compiler::OperandType> operandTypes {Compiler::OperandConst, Compiler::OperandConst};

        for(int i=0; i<int(tokens.size()); i++)
        {
            operandTypes[i] = Compiler::parseExpression(codeLineIndex, tokens[i], operands[i], numerics[i]);
        }

        std::string opcode, operand;
        switch(numerics[1]._int16Byte)
        {
            case Expression::Int16Low:  opcode = "LD";  operand = "_" + operands[1];          break;
            case Expression::Int16High: opcode = "LD";  operand = "_" + operands[1] + " + 1"; break;
            case Expression::Int16Both: opcode = "LDW"; operand = "_" + operands[1];          break;

            default: break;
        }

        if((operandTypes[0] == Compiler::OperandVar  ||  operandTypes[0] == Compiler::OperandTemp)  &&  (operandTypes[1] == Compiler::OperandVar  ||  operandTypes[1] == Compiler::OperandTemp))
        {
            (operandTypes[1] == Compiler::OperandVar) ? Compiler::emitVcpuAsm(opcode, operand, false, codeLineIndex) : Compiler::emitVcpuAsm("LDW", operands[1], false, codeLineIndex);
            (operandTypes[0] == Compiler::OperandVar) ? Compiler::emitVcpuAsm("POKE", "_" + operands[0], false, codeLineIndex) : Compiler::emitVcpuAsm("POKE", operands[0], false, codeLineIndex);
        }
        else if((operandTypes[0] == Compiler::OperandVar  ||  operandTypes[0] == Compiler::OperandTemp)  &&  operandTypes[1] == Compiler::OperandConst)
        {
            Compiler::emitVcpuAsm("LDI", operands[1], false, codeLineIndex);
            (operandTypes[0] == Compiler::OperandVar) ? Compiler::emitVcpuAsm("POKE", "_" + operands[0], false, codeLineIndex) : Compiler::emitVcpuAsm("POKE", operands[0], false, codeLineIndex);
        }
        else if(operandTypes[0] == Compiler::OperandConst  &&  (operandTypes[1] == Compiler::OperandVar  ||  operandTypes[1] == Compiler::OperandTemp))
        {
            uint16_t addr;
            if(Expression::stringToU16(operands[0], addr)  &&  addr < 0x0100)
            {
                (operandTypes[1] == Compiler::OperandVar) ? Compiler::emitVcpuAsm(opcode, operand, false, codeLineIndex) : Compiler::emitVcpuAsm("LDW", operands[1], false, codeLineIndex);
                Compiler::emitVcpuAsm("ST", operands[0], false, codeLineIndex);
            }
            else
            {
                Compiler::emitVcpuAsm("LDWI", operands[0], false, codeLineIndex);
                Compiler::emitVcpuAsm("STW", "register0", false, codeLineIndex);
                (operandTypes[1] == Compiler::OperandVar) ? Compiler::emitVcpuAsm(opcode, operand, false, codeLineIndex) : Compiler::emitVcpuAsm("LDW", operands[1], false, codeLineIndex);
                Compiler::emitVcpuAsm("POKE", "register0", false, codeLineIndex);
            }
        }
        else
        {
            // Optimise for page 0
            uint16_t addr;
            if(Expression::stringToU16(operands[0], addr)  &&  addr < 0x0100)
            {
                Compiler::emitVcpuAsm("LDI", operands[1], false, codeLineIndex);
                Compiler::emitVcpuAsm("ST",  operands[0], false, codeLineIndex);
            }
            // All other pages
            else
            {
                Compiler::emitVcpuAsm("LDWI", operands[0], false, codeLineIndex);
                Compiler::emitVcpuAsm("STW",  "register0", false, codeLineIndex);
                Compiler::emitVcpuAsm("LDI",  operands[1], false, codeLineIndex);
                Compiler::emitVcpuAsm("POKE", "register0", false, codeLineIndex);
            }
        }

        return true;
    }

    bool keywordDOKE(Compiler::CodeLine& codeLine, int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(tokenIndex);

        std::vector<std::string> tokens = Expression::tokenise(codeLine._code.substr(foundPos), ',', false);
        if(tokens.size() != 2)
        {
            fprintf(stderr, "Keywords::keywordDOKE() : syntax error, 'DOKE <address>, <value>', in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex);
            return false;
        }

        std::vector<std::string> operands = {"", ""};
        std::vector<Expression::Numeric> numerics = {Expression::Numeric(), Expression::Numeric()};
        std::vector<Compiler::OperandType> operandTypes {Compiler::OperandConst, Compiler::OperandConst};

        for(int i=0; i<int(tokens.size()); i++)
        {
            operandTypes[i] = Compiler::parseExpression(codeLineIndex, tokens[i], operands[i], numerics[i]);
        }

        std::string opcode, operand;
        switch(numerics[1]._int16Byte)
        {
            case Expression::Int16Low:  opcode = "LD";  operand = "_" + operands[1];          break;
            case Expression::Int16High: opcode = "LD";  operand = "_" + operands[1] + " + 1"; break;
            case Expression::Int16Both: opcode = "LDW"; operand = "_" + operands[1];          break;

            default: break;
        }

        if((operandTypes[0] == Compiler::OperandVar  ||  operandTypes[0] == Compiler::OperandTemp)  &&  (operandTypes[1] == Compiler::OperandVar  ||  operandTypes[1] == Compiler::OperandTemp))
        {
            (operandTypes[1] == Compiler::OperandVar) ? Compiler::emitVcpuAsm(opcode, operand, false, codeLineIndex) : Compiler::emitVcpuAsm("LDW", "" + operands[1], false, codeLineIndex);
            (operandTypes[0] == Compiler::OperandVar) ? Compiler::emitVcpuAsm("DOKE", "_" + operands[0], false, codeLineIndex) : Compiler::emitVcpuAsm("DOKE", "" + operands[0], false, codeLineIndex);
        }
        else if((operandTypes[0] == Compiler::OperandVar  ||  operandTypes[0] == Compiler::OperandTemp)  &&  operandTypes[1] == Compiler::OperandConst)
        {
            Compiler::emitVcpuAsm("LDWI", operands[1], false, codeLineIndex);
            (operandTypes[0] == Compiler::OperandVar) ? Compiler::emitVcpuAsm("DOKE", "_" + operands[0], false, codeLineIndex) : Compiler::emitVcpuAsm("DOKE", "" + operands[0], false, codeLineIndex);
        }
        else if(operandTypes[0] == Compiler::OperandConst  &&  (operandTypes[1] == Compiler::OperandVar  ||  operandTypes[1] == Compiler::OperandTemp))
        {
            uint16_t addr;
            if(Expression::stringToU16(operands[0], addr)  &&  addr < 0x0100)
            {
                (operandTypes[1] == Compiler::OperandVar) ? Compiler::emitVcpuAsm(opcode, operand, false, codeLineIndex) : Compiler::emitVcpuAsm("LDW", "" + operands[1], false, codeLineIndex);
                Compiler::emitVcpuAsm("STW", operands[0], false, codeLineIndex);
            }
            else
            {
                Compiler::emitVcpuAsm("LDWI", operands[0], false, codeLineIndex);
                Compiler::emitVcpuAsm("STW", "register0", false, codeLineIndex);
                (operandTypes[1] == Compiler::OperandVar) ? Compiler::emitVcpuAsm(opcode, operand, false, codeLineIndex) : Compiler::emitVcpuAsm("LDW", "" + operands[1], false, codeLineIndex);
                Compiler::emitVcpuAsm("DOKE", "register0", false, codeLineIndex);
            }
        }
        else
        {
            // Optimise for page 0
            uint16_t addr;
            if(Expression::stringToU16(operands[0], addr)  &&  addr < 0x0100)
            {
                Compiler::emitVcpuAsm("LDWI", operands[1], false, codeLineIndex);
                Compiler::emitVcpuAsm("STW",  operands[0], false, codeLineIndex);
            }
            // All other pages
            else
            {
                Compiler::emitVcpuAsm("LDWI", operands[0], false, codeLineIndex);
                Compiler::emitVcpuAsm("STW",  "register0", false, codeLineIndex);
                Compiler::emitVcpuAsm("LDWI", operands[1], false, codeLineIndex);
                Compiler::emitVcpuAsm("DOKE", "register0", false, codeLineIndex);
            }
        }

        return true;
    }

    void usagePLAY(Compiler::CodeLine& codeLine, int codeLineIndex)
    {
        fprintf(stderr, "Keywords::keywordPLAY() : Syntax error, use 'PLAY MIDI, <address>, <waveType>', where <address> and <waveType> are optional; in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex);
        fprintf(stderr, "Keywords::keywordPLAY() : Syntax error, use 'PLAY MIDIV, <address>, <waveType>', where <address> and <waveType> are optional; in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex);
    }
    bool keywordPLAY(Compiler::CodeLine& codeLine, int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(tokenIndex);

        std::vector<std::string> tokens = Expression::tokenise(codeLine._code.substr(foundPos), ",", false);
        if(tokens.size() < 1  ||  tokens.size() > 3)
        {
            usagePLAY(codeLine, codeLineIndex);
            return false;
        }

        std::string midiToken = Expression::strToUpper(tokens[0]);
        Expression::stripWhitespace(midiToken);
        if(midiToken != "MIDI"  &&  midiToken != "MIDIV")
        {
            usagePLAY(codeLine, codeLineIndex);
            return false;
        }

        // Tick Midi
        if(tokens.size() == 1)
        {
            (midiToken == "MIDI") ? Compiler::emitVcpuAsm("%TickMidi", "", false, codeLineIndex) : Compiler::emitVcpuAsm("%TickMidiV", "", false, codeLineIndex);
            return true;
        }

        // Default wave type
        if(tokens.size() == 2)
        {
            Compiler::emitVcpuAsm("LDI", "2",       false, codeLineIndex);
            Compiler::emitVcpuAsm("ST", "waveType", false, codeLineIndex);
        }
        // Midi wave type, (optional)
        else if(tokens.size() == 3)
        {
            std::string waveTypeToken = tokens[2];
            Expression::stripWhitespace(waveTypeToken);
            Expression::Numeric param;
            Compiler::parseExpression(codeLineIndex, waveTypeToken, param);
            Compiler::emitVcpuAsm("ST", "waveType", false, codeLineIndex);
        }

        // Midi stream address
        std::string addressToken = tokens[1];
        Expression::stripWhitespace(addressToken);
        Expression::Numeric param;
        Compiler::parseExpression(codeLineIndex, addressToken, param);
        (midiToken == "MIDI") ? Compiler::emitVcpuAsm("%PlayMidi", "", false, codeLineIndex) : Compiler::emitVcpuAsm("%PlayMidiV", "", false, codeLineIndex);

        return true;
    }

    void usageLOAD(Compiler::CodeLine& codeLine, int codeLineIndex)
    {
            fprintf(stderr, "Keywords::keywordLOAD() : Syntax error, use 'LOAD IMAGE, <filename>, <optional address>', in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex);
            fprintf(stderr, "Keywords::keywordLOAD() : Syntax error, use 'LOAD SPRITE, <filename>, <id>, <optional flip>', in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex);
            fprintf(stderr, "Keywords::keywordLOAD() : Syntax error, use 'LOAD FONT, <filename>, <id>, <optional 16 bit bg:fg colours>', in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex);
    }
    bool keywordLOAD(Compiler::CodeLine& codeLine, int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(tokenIndex);

        std::vector<std::string> tokens = Expression::tokenise(codeLine._code.substr(foundPos), ",", false);
        if(tokens.size() < 2  ||  tokens.size() > 4)
        {
            usageLOAD(codeLine, codeLineIndex);
            return false;
        }

        // Type
        Expression::strToUpper(tokens[0]);
        Expression::stripWhitespace(tokens[0]);
        if(tokens[0] != "MIDI"  &&  tokens[0] != "IMAGE"  &&  tokens[0] != "SPRITE"  &&  tokens[0] != "FONT")
        {
            usageLOAD(codeLine, codeLineIndex);
            return false;
        }

        // Handle Image, Sprite and Font
        if(tokens[0] == "IMAGE"  ||  tokens[0] == "SPRITE"  ||  tokens[0] == "FONT")
        {
            std::string filename = tokens[1];
            Expression::stripWhitespace(filename);
            std::string ext = filename;
            Expression::strToUpper(ext);
            if(ext.find(".TGA") != std::string::npos)
            {
                std::string filepath = Loader::getFilePath();
                size_t slash = filepath.find_last_of("\\/");
                filepath = (slash != std::string::npos) ? filepath.substr(0, slash) : ".";
                filename = filepath + "/" + filename;
                Image::TgaFile tgaFile;

                //fprintf(stderr, "\nKeywords::keywordLOAD() : %s\n", filename.c_str());

                if(!Image::loadTgaFile(filename, tgaFile))
                {
                    fprintf(stderr, "Keywords::keywordLOAD() : File '%s' failed to load, in '%s' on line %d\n", filename.c_str(), codeLine._text.c_str(), codeLineIndex);
                    return false;
                }

                // Load image/sprite/font
                std::vector<uint8_t> data;
                std::vector<uint16_t> optional;
                Image::GtRgbFile gtRgbFile{GTRGB_IDENTIFIER, Image::GT_RGB_222, tgaFile._header._width, tgaFile._header._height, data, optional};
                Image::convertRGB8toRGB2(tgaFile._data, gtRgbFile._data, tgaFile._header._width, tgaFile._header._height, tgaFile._imageOrigin);

                // Image
                if(tokens[0] == "IMAGE")
                {
                    const uint16_t stride = 256;

                    if(tokens.size() > 3)
                    {
                        fprintf(stderr, "Keywords::keywordLOAD() : Syntax error, use 'LOAD IMAGE, <filename>, <optional address>', in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex);
                        return false;
                    }

                    // Parse optional address
                    uint16_t address = RAM_VIDEO_START;
                    if(tokens.size() == 3)
                    {
                        std::string addrToken = tokens[2];
                        Expression::stripWhitespace(addrToken);
                        Expression::Numeric addrNumeric;
                        std::string addrOperand;
                        Compiler::parseExpression(codeLineIndex, addrToken, addrOperand, addrNumeric);
                        address = uint16_t(std::lround(addrNumeric._value));
                        if(address < DEFAULT_START_ADDRESS)
                        {
                            fprintf(stderr, "Keywords::keywordLOAD() : Address field must be above %04x, found %s in '%s' on line %d\n", DEFAULT_START_ADDRESS, addrToken.c_str(),
                                                                                                                                         codeLine._text.c_str(), codeLineIndex);
                            return false;
                        }
                    }

                    if(gtRgbFile._header._width > stride  ||  gtRgbFile._header._width + (address & 0x00FF) > stride)
                    {
                        fprintf(stderr, "Keywords::keywordLOAD() : Image width %d + starting address 0x%04x overflow, for %s; in '%s' on line %d\n", gtRgbFile._header._width, address, filename.c_str(),
                                                                                                                                                     codeLine._text.c_str(), codeLineIndex);
                        return false;
                    }

                    Compiler::DefDataImage defDataImage = {address, tgaFile._header._width, tgaFile._header._height, stride, gtRgbFile._data};
                    Compiler::getDefDataImages().push_back(defDataImage);

                    // Take offscreen memory from compiler for images wider than visible screen resolution
                    int size = gtRgbFile._header._width;
                    for(int y=0; y<gtRgbFile._header._height; y++)
                    {
                        if(address >= RAM_VIDEO_START  &&  address <= 0x7FFF)
                        {
                            size = gtRgbFile._header._width + (address & 0x00FF) - RAM_SCANLINE_SIZE;
                            if(size > 0)
                            {
                                if(!Memory::takeFreeRAM(address + RAM_SCANLINE_SIZE, size))
                                {
                                    fprintf(stderr, "Keywords::keywordLOAD() : Allocating RAM for pixel row %d failed, in '%s' on line %d\n", y, codeLine._text.c_str(), codeLineIndex);
                                    return false;
                                }
                            }
                        }

                        // Next destination row
                        address += stride; 
                    }
                }
                // Sprite
                else if(tokens[0] == "SPRITE")
                {
                    if(tokens.size() > 4)
                    {
                        fprintf(stderr, "Keywords::keywordLOAD() : Syntax error, use 'LOAD SPRITE, <filename>, <id>, <optional flip>', in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex);
                        return false;
                    }

                    if(gtRgbFile._header._width % SPRITE_CHUNK_SIZE != 0)
                    {
                        fprintf(stderr, "Keywords::keywordLOAD() : Sprite width not a multiple of %d, (%d x %d), for %s; in '%s' on line %d\n", SPRITE_CHUNK_SIZE, gtRgbFile._header._width,
                                                                                                                                                                   gtRgbFile._header._height, 
                                                                                                                                                                   filename.c_str(),
                                                                                                                                                                   codeLine._text.c_str(), codeLineIndex);
                        return false;
                    }

                    if(tokens.size() < 3)
                    {
                        fprintf(stderr, "Keywords::keywordLOAD() : Syntax error, use 'LOAD SPRITE, <filename>, <id>, <optional flip>', in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex);
                        return false;
                    }

                    // Unique sprite ID
                    std::string idToken = tokens[2];
                    Expression::stripWhitespace(idToken);
                    Expression::Numeric idNumeric;
                    std::string idOperand;
                    Compiler::parseExpression(codeLineIndex, idToken, idOperand, idNumeric);
                    int spriteId = int(std::lround(idNumeric._value));
                    if(Compiler::getDefDataSprites().find(spriteId) != Compiler::getDefDataSprites().end())
                    {
                        fprintf(stderr, "Keywords::keywordLOAD() : Sprite id not unique, %d; in '%s' on line %d\n", spriteId, codeLine._text.c_str(), codeLineIndex);
                        return false;
                    }

                    // Parse optional sprite flip
                    Compiler::SpriteFlipType flipType = Compiler::NoFlip;
                    if(tokens.size() == 4)
                    {
                        std::string flipToken = tokens[3];
                        Expression::stripWhitespace(flipToken);
                        Expression::strToUpper(flipToken);
                        if(flipToken == "FLIPX")       flipType = Compiler::FlipX;
                        else if(flipToken == "FLIPY")  flipType = Compiler::FlipY;
                        else if(flipToken == "FLIPXY") flipType = Compiler::FlipXY;
                    }

                    // Build sprite data from image data
                    uint16_t numColumns = gtRgbFile._header._width / SPRITE_CHUNK_SIZE;
                    uint16_t remStripeChunks = gtRgbFile._header._height % Compiler::getSpriteStripeChunks();
                    uint16_t numStripesPerCol = gtRgbFile._header._height / Compiler::getSpriteStripeChunks() + int(remStripeChunks > 0);
                    uint16_t numStripeChunks = (numStripesPerCol == 1) ? gtRgbFile._header._height : Compiler::getSpriteStripeChunks();
                    std::vector<uint16_t> stripeAddrs;
                    std::vector<uint8_t> spriteData;

                    // Search sprite image for instancing
                    int parentInstance = 0;
                    bool isInstanced = false;
                    for(auto it=Compiler::getDefDataSprites().begin(); it!=Compiler::getDefDataSprites().end(); ++it)
                    {
                        if(it->second._filename == filename)
                        {
                            spriteData = it->second._data;
                            parentInstance = it->first;
                            isInstanced = true;
                            break;
                        }
                    }

                    // Allocate sprite memory
                    int addrIndex = 0;
                    uint16_t address = 0x0000;
                    for(int i=0; i<numColumns; i++)
                    {
                        // One stripe per column
                        if(numStripesPerCol == 1)
                        {
                            if(isInstanced)
                            {
                                address = Compiler::getDefDataSprites()[parentInstance]._stripeAddrs[addrIndex];
                                addrIndex += 2;
                            }
                            else
                            {
                                if(!Memory::getFreeRAM(Compiler::getSpriteStripeFitType(), numStripeChunks*SPRITE_CHUNK_SIZE + 1, Compiler::getSpriteStripeMinAddress(), Compiler::getRuntimeStart(), address))
                                {
                                    fprintf(stderr, "Keywords::keywordLOAD() : Getting Sprite memory for stripe %d failed, in '%s' on line %d\n", int(stripeAddrs.size()/2 + 1), codeLine._text.c_str(), codeLineIndex);
                                    return false;
                                }
                            }
                            stripeAddrs.push_back(address);
                            switch(flipType)
                            {
                                // destination offsets
                                case Compiler::NoFlip: stripeAddrs.push_back(uint16_t(0 + i*6));                                      break;
                                case Compiler::FlipX:  stripeAddrs.push_back(uint16_t(0 + (numColumns-1-i)*6));                       break;
                                case Compiler::FlipY:  stripeAddrs.push_back(uint16_t((numStripeChunks-1)*256 + i*6));                break;
                                case Compiler::FlipXY: stripeAddrs.push_back(uint16_t((numStripeChunks-1)*256 + (numColumns-1-i)*6)); break;
                                break;
                            }

                            // Copy sprite data and delimiter
                            for(int j=0; j<numStripeChunks; j++)
                            {
                                for(int k=0; k<SPRITE_CHUNK_SIZE; k++)
                                {
                                    spriteData.push_back(gtRgbFile._data[i*SPRITE_CHUNK_SIZE + j*SPRITE_CHUNK_SIZE*numColumns + k]);
                                }
                            }
                            spriteData.push_back(uint8_t(-gtRgbFile._header._height));
                        }
                        // Multiple stripes per column
                        else
                        {
                            // MAX_SPRITE_CHUNKS_PER_STRIPE stripes
                            for(int j=0; j<numStripesPerCol-1; j++)
                            {
                                if(isInstanced)
                                {
                                    address = Compiler::getDefDataSprites()[parentInstance]._stripeAddrs[addrIndex];
                                    addrIndex += 2;
                                }
                                else
                                {
                                    if(!Memory::getFreeRAM(Compiler::getSpriteStripeFitType(), numStripeChunks*SPRITE_CHUNK_SIZE + 1, Compiler::getSpriteStripeMinAddress(), Compiler::getRuntimeStart(), address))
                                    {
                                        fprintf(stderr, "Keywords::keywordLOAD() : Getting Sprite memory failed for stripe %d, in '%s' on line %d\n", int(stripeAddrs.size()/2 + 1), codeLine._text.c_str(), codeLineIndex);
                                        return false;
                                    }
                                }
                                stripeAddrs.push_back(address);
                                switch(flipType)
                                {
                                    // destination offsets
                                    case Compiler::NoFlip: stripeAddrs.push_back(uint16_t(j*numStripeChunks*256 + i*6));                                                         break;
                                    case Compiler::FlipX:  stripeAddrs.push_back(uint16_t(j*numStripeChunks*256 + (numColumns-1-i)*6));                                          break;
                                    case Compiler::FlipY:  stripeAddrs.push_back(uint16_t(((numStripesPerCol-1-j)*numStripeChunks+remStripeChunks-1)*256 + i*6));                break;
                                    case Compiler::FlipXY: stripeAddrs.push_back(uint16_t(((numStripesPerCol-1-j)*numStripeChunks+remStripeChunks-1)*256 + (numColumns-1-i)*6)); break;
                                    break;
                                }

                                // Copy sprite data and delimiter
                                for(int k=0; k<numStripeChunks; k++)
                                {
                                    for(int l=0; l<SPRITE_CHUNK_SIZE; l++)
                                    {
                                        spriteData.push_back(gtRgbFile._data[i*SPRITE_CHUNK_SIZE + j*numStripeChunks*SPRITE_CHUNK_SIZE*numColumns + k*SPRITE_CHUNK_SIZE*numColumns + l]);
                                    }
                                }
                                spriteData.push_back(255);
                            }

                            // Remainder stripe
                            if(isInstanced)
                            {
                                address = Compiler::getDefDataSprites()[parentInstance]._stripeAddrs[addrIndex];
                                addrIndex += 2;
                            }
                            else
                            {
                                if(!Memory::getFreeRAM(Compiler::getSpriteStripeFitType(), remStripeChunks*SPRITE_CHUNK_SIZE + 1, Compiler::getSpriteStripeMinAddress(), Compiler::getRuntimeStart(), address))
                                {
                                    fprintf(stderr, "Keywords::keywordLOAD() : Getting Sprite memory failed for stripe %d, in '%s' on line %d\n", int(stripeAddrs.size()/2 + 1), codeLine._text.c_str(), codeLineIndex);
                                    return false;
                                }
                            }
                            stripeAddrs.push_back(address);
                            switch(flipType)
                            {
                                // Destination offsets
                                case Compiler::NoFlip: stripeAddrs.push_back(uint16_t((numStripesPerCol-1)*numStripeChunks*256 + i*6));                break;
                                case Compiler::FlipX:  stripeAddrs.push_back(uint16_t((numStripesPerCol-1)*numStripeChunks*256 + (numColumns-1-i)*6)); break;
                                case Compiler::FlipY:  stripeAddrs.push_back(uint16_t((remStripeChunks-1)*256 + i*6));                                 break;
                                case Compiler::FlipXY: stripeAddrs.push_back(uint16_t((remStripeChunks-1)*256 + (numColumns-1-i)*6));                  break;
                                break;
                            }

                            // Copy sprite data and delimiter
                            for(int j=0; j<remStripeChunks; j++)
                            {
                                for(int k=0; k<SPRITE_CHUNK_SIZE; k++)
                                {
                                    spriteData.push_back(gtRgbFile._data[i*SPRITE_CHUNK_SIZE + (numStripesPerCol-1)*numStripeChunks*SPRITE_CHUNK_SIZE*numColumns + j*SPRITE_CHUNK_SIZE*numColumns + k]);
                                }
                            }
                            spriteData.push_back(255);
                        }
                    }

                    Compiler::DefDataSprite defDataSprite = {spriteId, filename, tgaFile._header._width, tgaFile._header._height, numColumns, numStripesPerCol, numStripeChunks, remStripeChunks,
                                                             stripeAddrs, spriteData, flipType, isInstanced};
                    Compiler::getDefDataSprites()[spriteId] = defDataSprite;
                }
                else if(tokens[0] == "FONT")
                {
                    if(tokens.size() < 3  ||  tokens.size() > 4)
                    {
                        fprintf(stderr, "Keywords::keywordLOAD() : Syntax error, use 'LOAD FONT, <filename>, <id>, <optional 16 bit fg:bg colours>', in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex);
                        return false;
                    }

                    // Unique font ID
                    std::string idToken = tokens[2];
                    Expression::stripWhitespace(idToken);
                    Expression::Numeric idNumeric;
                    std::string idOperand;
                    Compiler::parseExpression(codeLineIndex, idToken, idOperand, idNumeric);
                    int fontId = int(std::lround(idNumeric._value));
                    if(Compiler::getDefDataFonts().find(fontId) != Compiler::getDefDataFonts().end())
                    {
                        fprintf(stderr, "Keywords::keywordLOAD() : Font id not unique, %d; in '%s' on line %d\n", fontId, codeLine._text.c_str(), codeLineIndex);
                        return false;
                    }

                    // Foreground/background colours
                    uint16_t fgbgColour = 0x0000;
                    if(tokens.size() == 4)
                    {
                        std::string fgbgToken = tokens[3];
                        Expression::stripWhitespace(fgbgToken);
                        Expression::Numeric fgbgNumeric;
                        std::string fgbgOperand;
                        Compiler::parseExpression(codeLineIndex, fgbgToken, fgbgOperand, fgbgNumeric);
                        fgbgColour = uint16_t(std::lround(fgbgNumeric._value));
                    }

                    // Width
                    if(gtRgbFile._header._width % FONT_WIDTH != 0)
                    {
                        fprintf(stderr, "Keywords::keywordLOAD() : Font width %d is not a multiple of %d; in '%s' on line %d\n", gtRgbFile._header._width, FONT_WIDTH, codeLine._text.c_str(), codeLineIndex);
                        return false;
                    }

                    // Height
                    if(gtRgbFile._header._height % FONT_HEIGHT != 0)
                    {
                        fprintf(stderr, "Keywords::keywordLOAD() : Font height %d is not a multiple of %d; in '%s' on line %d\n", gtRgbFile._header._height, FONT_HEIGHT, codeLine._text.c_str(), codeLineIndex);
                        return false;
                    }

                    // Load font mapping file
                    bool foundMapFile = true;
                    size_t nameSuffix = filename.find_last_of(".");
                    filename = filename.substr(0, nameSuffix) + ".map";
                    std::ifstream infile(filename, std::ios::in);
                    if(!infile.is_open())
                    {
                        foundMapFile = false;
                    }

                    // Parse font mapping file
                    int maxIndex = -1;
                    uint16_t mapAddr = 0x0000;
                    std::vector<uint8_t> mapping(MAPPING_SIZE);
                    if(foundMapFile)
                    {
                        int ascii, index, line = 0;
                        while(!infile.eof())
                        {
                            infile >> ascii >> index;
                            if(index > maxIndex) maxIndex = index;
                            if(!infile.good() && !infile.eof())
                            {
                                fprintf(stderr, "Keywords::keywordLOAD() : error in Mapping file %s on line %d; in '%s' on line %d\n", filename.c_str(), line + 1, codeLine._text.c_str(), codeLineIndex);
                                return false;
                            }

                            if(line >= MAPPING_SIZE) break;
                            mapping[line++] = uint8_t(index);
                        }

                        if(line != MAPPING_SIZE)
                        {
                            fprintf(stderr, "Keywords::keywordLOAD() : warning, found an incorrect number of map entries %d for file %s, should be %d; in '%s' on line %d\n", line - 1, filename.c_str(), MAPPING_SIZE,
                                                                                                                                                                              codeLine._text.c_str(), codeLineIndex);
                            return false;
                        }

                        if(!Memory::getFreeRAM(Memory::FitDescending, MAPPING_SIZE, 0x0200, Compiler::getRuntimeStart(), mapAddr))
                        {
                            fprintf(stderr, "Keywords::keywordLOAD() : Getting Mapping memory for Map size of %d failed, in '%s' on line %d\n", MAPPING_SIZE, codeLine._text.c_str(), codeLineIndex);
                            return false;
                        }
                    }

                    // 8th line is implemented as a separate sprite call, to save memory and allow for more efficient memory packing
                    const int kCharHeight = FONT_HEIGHT-1;

                    // Copy font data and create delimiter
                    std::vector<uint8_t> charData;
                    std::vector<uint16_t> charAddrs;
                    std::vector<std::vector<uint8_t>> fontData;
                    for(int j=0; j<tgaFile._header._height; j+=FONT_HEIGHT)
                    {
                        for(int i=0; i<tgaFile._header._width; i+=FONT_WIDTH)
                        {

                            for(int l=0; l<kCharHeight; l++)
                            {
                                for(int k=0; k<FONT_WIDTH; k++)
                                {
                                    uint8_t pixel = gtRgbFile._data[j*tgaFile._header._width + i + l*tgaFile._header._width + k];
                                    if(fgbgColour)
                                    {
                                        if(pixel == 0x00) pixel = fgbgColour & 0x00FF;
                                        if(pixel == 0x3F) pixel = fgbgColour >> 8;
                                    }
                                    charData.push_back(pixel);
                                }
                            }
                            charData.push_back(uint8_t(-(kCharHeight)));
                            fontData.push_back(charData);
                            charData.clear();

                            uint16_t address = 0x0000;
                            if(!Memory::getFreeRAM(Memory::FitDescending, (kCharHeight)*FONT_WIDTH + 1, 0x0200, Compiler::getRuntimeStart(), address))
                            {
                                fprintf(stderr, "Keywords::keywordLOAD() : Getting font memory for char %d failed, in '%s' on line %d\n", int(fontData.size() - 1), codeLine._text.c_str(), codeLineIndex);
                                return false;
                            }

                            charAddrs.push_back(address);
                        }
                    }

                    if(foundMapFile  &&  maxIndex + 1 != int(fontData.size()))
                    {
                        fprintf(stderr, "Keywords::keywordLOAD() : Font mapping table does not match font data, found a mapping count of %d and a chars count of %d, in '%s' on line %d\n", 
                                        maxIndex + 1, int(fontData.size()), codeLine._text.c_str(), codeLineIndex);
                        return false;
                    }

                    // Create baseline for all chars in each font
                    uint16_t baseAddr = 0x0000;
                    if(!Memory::getFreeRAM(Memory::FitDescending, FONT_WIDTH + 1, 0x0200, Compiler::getRuntimeStart(), baseAddr))
                    {
                        fprintf(stderr, "Keywords::keywordLOAD() : Getting font memory for char %d failed, in '%s' on line %d\n", int(fontData.size() - 1), codeLine._text.c_str(), codeLineIndex);
                        return false;
                    }

                    Compiler::DefDataFont defDataFont = {fontId, filename, tgaFile._header._width, tgaFile._header._height, charAddrs, fontData, mapAddr, mapping, baseAddr, fgbgColour};
                    Compiler::getDefDataFonts()[fontId] = defDataFont;

                    Linker::enableFontLinking();
                }
            }
        }

        return true;
    }

    bool keywordSPRITE(Compiler::CodeLine& codeLine, int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(tokenIndex);

        std::vector<std::string> tokens = Expression::tokenise(codeLine._code.substr(foundPos), ",", false);
        if(tokens.size() != 4)
        {
            fprintf(stderr, "Keywords::keywordSPRITE() : Syntax error, use 'SPRITE <NOFLIP/FLIPX/FLIPY/FLIPXY>, <id>, <x postion>, <y position>'; in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex);
            return false;
        }

        // Flip type
        static std::map<std::string, int> flipType = {{"NOFLIP", Compiler::NoFlip}, {"FLIPX", Compiler::FlipX}, {"FLIPY", Compiler::FlipY}, {"FLIPXY", Compiler::FlipXY}};
        std::string flipToken = tokens[0];
        Expression::stripWhitespace(flipToken);
        Expression::strToUpper(flipToken);
        if(flipType.find(flipToken) == flipType.end())
        {
            fprintf(stderr, "Keywords::keywordSPRITE() : Syntax error, use one of the correct flip types, 'SPRITE <NOFLIP/FLIPX/FLIPY/FLIPXY>, <id>, <x postion>, <y position>'; in '%s' on line %d\n",
                            codeLine._text.c_str(), codeLineIndex);
            return false;
        }

        // Sprite identifier
        std::string idToken = tokens[1];
        Expression::stripWhitespace(idToken);
        Expression::Numeric idParam;
        Compiler::parseExpression(codeLineIndex, idToken, idParam);
        Compiler::emitVcpuAsm("STW", "spriteId", false, codeLineIndex);

        // Sprite X position
        std::string xposToken = tokens[2];
        Expression::stripWhitespace(xposToken);
        Expression::Numeric xposParam;
        Compiler::parseExpression(codeLineIndex, xposToken, xposParam);
        Compiler::emitVcpuAsm("ST", "spriteXY", false, codeLineIndex);

        // Sprite Y position
        std::string yposToken = tokens[3];
        Expression::stripWhitespace(yposToken);
        Expression::Numeric yposParam;
        Compiler::parseExpression(codeLineIndex, yposToken, yposParam);
        Compiler::emitVcpuAsm("ADDI", "8", false, codeLineIndex);
        Compiler::emitVcpuAsm("ST", "spriteXY + 1", false, codeLineIndex);

        // Draw sprite
        switch(flipType[flipToken])
        {
            case Compiler::NoFlip: Compiler::emitVcpuAsm("%DrawSprite",   "", false, codeLineIndex); break;
            case Compiler::FlipX:  Compiler::emitVcpuAsm("%DrawSpriteX",  "", false, codeLineIndex); break;
            case Compiler::FlipY:  Compiler::emitVcpuAsm("%DrawSpriteY",  "", false, codeLineIndex); break;
            case Compiler::FlipXY: Compiler::emitVcpuAsm("%DrawSpriteXY", "", false, codeLineIndex); break;
        }
 
        return true;
    }

    void usageSOUND(Compiler::CodeLine& codeLine, int codeLineIndex)
    {
        fprintf(stderr, "Keywords::keywordSOUND() : Syntax error, use 'SOUND ON, <channel>, <frequency>, <optional volume>, <optional waveform>'; in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex);
        fprintf(stderr, "Keywords::keywordSOUND() : Syntax error, use 'SOUND MOD, <channel>, <wavX>, <optional wavA>'; in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex);
        fprintf(stderr, "Keywords::keywordSOUND() : Syntax error, use 'SOUND OFF, <optional channel>'; in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex);
    }
    bool keywordSOUND(Compiler::CodeLine& codeLine, int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(tokenIndex);

        std::vector<std::string> tokens = Expression::tokenise(codeLine._code.substr(foundPos), ",", false);
        if(tokens.size() < 1  ||  tokens.size() > 5)
        {
            usageSOUND(codeLine, codeLineIndex);
            return false;
        }

        // Sound state
        std::string stateToken = tokens[0];
        Expression::stripWhitespace(stateToken);
        Expression::strToUpper(stateToken);

        // Sound channel, (has to be between 1 and 4, saves an ADD/INC, no checking done)
        if(tokens.size() >= 2)
        {
            std::string chanToken = tokens[1];
            Expression::stripWhitespace(chanToken);
            Expression::Numeric chanParam;
            Compiler::parseExpression(codeLineIndex, chanToken, chanParam);
            Compiler::emitVcpuAsm("ST", "sndChannel + 1", false, codeLineIndex);
        }
        
        // Sound channels off
        if(stateToken == "OFF")
        {
            if(tokens.size() > 2)
            {
                fprintf(stderr, "Keywords::keywordSOUND() : Syntax error, use 'SOUND OFF, <optional channel>'; in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex);
                return false;
            }

            // All sound channels off
            if(tokens.size() == 1)
            {
                Compiler::emitVcpuAsm("%SoundAllOff", "", false, codeLineIndex);
                return true;
            }
            // Single channel off
            else
            {
                Compiler::emitVcpuAsm("%SoundOff", "", false, codeLineIndex);
                return true;
            }
        }

        // Sound channels on
        if(stateToken == "ON")
        {
            if(tokens.size() < 3)
            {
                fprintf(stderr, "Keywords::keywordSOUND() : Syntax error, use 'SOUND ON, <channel>, <frequency>, <optional volume>, <optional waveform>'; in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex);
                return false;
            }

            std::string freqToken = tokens[2];
            Expression::stripWhitespace(freqToken);
            Expression::Numeric freqParam;
            Compiler::parseExpression(codeLineIndex, freqToken, freqParam);
            Compiler::emitVcpuAsm("STW", "sndFrequency", false, codeLineIndex);

            if(tokens.size() == 3)
            {
                Compiler::emitVcpuAsm("%SoundOn", "", false, codeLineIndex);
                return true;
            }

            std::string volToken = tokens[3];
            Expression::stripWhitespace(volToken);
            Expression::Numeric volParam;
            Compiler::parseExpression(codeLineIndex, volToken, volParam);
            Compiler::emitVcpuAsm("STW", "sndVolume", false, codeLineIndex);

            if(tokens.size() == 4)
            {
                Compiler::emitVcpuAsm("LDI", "2", false, codeLineIndex);
                Compiler::emitVcpuAsm("STW", "sndWaveType", false, codeLineIndex);
                Compiler::emitVcpuAsm("%SoundOnV", "", false, codeLineIndex);
                return true;
            }

            std::string wavToken = tokens[4];
            Expression::stripWhitespace(wavToken);
            Expression::Numeric wavParam;
            Compiler::parseExpression(codeLineIndex, wavToken, wavParam);
            Compiler::emitVcpuAsm("STW", "sndWaveType", false, codeLineIndex);
            Compiler::emitVcpuAsm("%SoundOnV", "", false, codeLineIndex);

            return true;
        }

        // Sound channels modulation
        if(stateToken == "MOD")
        {
            if(tokens.size() < 3  ||  tokens.size() > 4)
            {
                fprintf(stderr, "Keywords::keywordSOUND() : Syntax error, use 'SOUND MOD, <channel>, <wavX>, <optional wavA>'; in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex);
                return false;
            }

            std::string waveXToken = tokens[2];
            Expression::stripWhitespace(waveXToken);
            Expression::Numeric waveXParam;
            Compiler::parseExpression(codeLineIndex, waveXToken, waveXParam);
            Compiler::emitVcpuAsm("ST", "sndWaveType + 1", false, codeLineIndex);

            if(tokens.size() == 4)
            {
                std::string waveAToken = tokens[3];
                Expression::stripWhitespace(waveAToken);
                Expression::Numeric waveAParam;
                Compiler::parseExpression(codeLineIndex, waveAToken, waveAParam);
                Compiler::emitVcpuAsm("ST", "sndWaveType", false, codeLineIndex);
            }
            // Reset waveA
            else
            {
                Compiler::emitVcpuAsm("LDI", "0", false, codeLineIndex);
                Compiler::emitVcpuAsm("ST", "sndWaveType", false, codeLineIndex);
            }

            Compiler::emitVcpuAsm("%SoundMod", "", false, codeLineIndex);
            return true;
        }

        usageSOUND(codeLine, codeLineIndex);
        return false;
    }

    void usageSET(Compiler::CodeLine& codeLine, int codeLineIndex)
    {
        fprintf(stderr, "Keywords::keywordSET() : Syntax error, use 'SET FONTID, <fontId>'; in '%s' on line %d\n", codeLine._text.c_str(), codeLineIndex);
    }
    bool keywordSET(Compiler::CodeLine& codeLine, int codeLineIndex, int tokenIndex, size_t foundPos, KeywordFuncResult& result)
    {
        UNREFERENCED_PARAM(result);
        UNREFERENCED_PARAM(tokenIndex);

        std::vector<std::string> tokens = Expression::tokenise(codeLine._code.substr(foundPos), ",", false);
        if(tokens.size() < 1  ||  tokens.size() > 3)
        {
            usageSET(codeLine, codeLineIndex);
            return false;
        }

        // System variable to set
        std::string sysVarName = tokens[0];
        Expression::stripWhitespace(sysVarName);
        Expression::strToUpper(sysVarName);

        // First parameter after system var name
        std::string token1;
        Expression::Numeric param1;
        if(tokens.size() >= 2)
        {
            token1 = tokens[1];
            Expression::stripWhitespace(token1);
        }

        // Second parameter after system var name
        std::string token2;
        Expression::Numeric param2;
        if(tokens.size() >= 3)
        {
            token2 = tokens[2];
            Expression::stripWhitespace(token2);
        }

        // Font id variable
        if(sysVarName == "FONTID"  &&  tokens.size() == 2)
        {
            Compiler::parseExpression(codeLineIndex, token1, param1);
            Compiler::emitVcpuAsm("STW", "fontLutId", false, codeLineIndex);
            return true;
        }
        else if(sysVarName == "CURSORX"  &&  tokens.size() == 2)
        {
            Compiler::parseExpression(codeLineIndex, token1, param1);
            Compiler::emitVcpuAsm("ST", "cursorXY", false, codeLineIndex);
            return true;
        }
        else if(sysVarName == "CURSORY"  &&  tokens.size() == 2)
        {
            Compiler::parseExpression(codeLineIndex, token1, param1);
            Compiler::emitVcpuAsm("ST", "cursorXY + 1", false, codeLineIndex);
            return true;
        }
        else if(sysVarName == "CURSORXY"  &&  tokens.size() == 2)
        {
            Compiler::parseExpression(codeLineIndex, token1, param1);
            Compiler::emitVcpuAsm("STW", "cursorXY", false, codeLineIndex);
            return true;
        }
        else if(sysVarName == "FGCOLOUR"  &&  tokens.size() == 2)
        {
            Compiler::parseExpression(codeLineIndex, token1, param1);
            Compiler::emitVcpuAsm("ST", "fgbgColour + 1", false, codeLineIndex);
            return true;
        }
        else if(sysVarName == "BGCOLOUR"  &&  tokens.size() == 2)
        {
            Compiler::parseExpression(codeLineIndex, token1, param1);
            Compiler::emitVcpuAsm("ST", "fgbgColour", false, codeLineIndex);
            return true;
        }
        else if(sysVarName == "FGBGCOLOUR"  &&  tokens.size() == 2)
        {
            Compiler::parseExpression(codeLineIndex, token1, param1);
            Compiler::emitVcpuAsm("STW", "fgbgColour", false, codeLineIndex);
            return true;
        }
        else if(sysVarName == "MIDISTREAM"  &&  tokens.size() == 2)
        {
            Compiler::parseExpression(codeLineIndex, token1, param1);
            Compiler::emitVcpuAsm("STW", "midiStream", false, codeLineIndex);
            return true;
        }
        else if(sysVarName == "VIDEOTOP"  &&  tokens.size() == 2)
        {
            Compiler::emitVcpuAsm("LDWI", "giga_videoTop", false, codeLineIndex);
            Compiler::emitVcpuAsm("STW", "register0", false, codeLineIndex);
            Compiler::parseExpression(codeLineIndex, token1, param1);
            Compiler::emitVcpuAsm("POKE", "register0", false, codeLineIndex);
            return true;
        }
        else if(sysVarName == "LEDTEMPO"  &&  tokens.size() == 2)
        {
            Compiler::parseExpression(codeLineIndex, token1, param1);
            Compiler::emitVcpuAsm("ST", "giga_ledTempo", false, codeLineIndex);
            return true;
        }
        else if(sysVarName == "LEDSTATE"  &&  tokens.size() == 2)
        {
            Compiler::parseExpression(codeLineIndex, token1, param1);
            Compiler::emitVcpuAsm("ST", "giga_ledState", false, codeLineIndex);
            return true;
        }
        else if(sysVarName == "SOUNDTIMER"  &&  tokens.size() == 2)
        {
            Compiler::parseExpression(codeLineIndex, token1, param1);
            Compiler::emitVcpuAsm("ST", "giga_soundTimer", false, codeLineIndex);
            return true;
        }
        else if(sysVarName == "CHANNELMASK"  &&  tokens.size() == 2)
        {
            Compiler::parseExpression(codeLineIndex, token1, param1);
            Compiler::emitVcpuAsm("ST", "giga_channelMask", false, codeLineIndex);
            return true;
        }
        else if(sysVarName == "XOUTMASK"  &&  tokens.size() == 2)
        {
            Compiler::parseExpression(codeLineIndex, token1, param1);
            Compiler::emitVcpuAsm("ST", "giga_xoutMask", false, codeLineIndex);
            return true;
        }
        else if(sysVarName == "BUTTONSTATE"  &&  tokens.size() == 2)
        {
            Compiler::parseExpression(codeLineIndex, token1, param1);
            Compiler::emitVcpuAsm("ST", "giga_buttonState", false, codeLineIndex);
            return true;
        }
        else if(sysVarName == "FRAMECOUNT"  &&  tokens.size() == 2)
        {
            Compiler::parseExpression(codeLineIndex, token1, param1);
            Compiler::emitVcpuAsm("ST", "giga_frameCount", false, codeLineIndex);
            return true;
        }

        usageSET(codeLine, codeLineIndex);
        return false;
    }
}