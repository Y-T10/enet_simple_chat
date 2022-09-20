#include "basic_script.hpp"

#include<iostream>
#include"scriptarray/scriptarray.h"
#include"scriptstdstring/scriptstdstring.h"

/// @brief 
/// @param msg 
/// @param param 
void CompileInfo(const asSMessageInfo *msg, void *param){
    //エラーの種類
    const std::string type = [](const asEMsgType& type) -> std::string {
        if (type == asMSGTYPE_WARNING){
            return "WARN";
        }
        if (type == asMSGTYPE_INFORMATION){
            return "INFO";
        }
        return "ERR ";
    }(msg->type);
    //エラー分
    std::cerr << "[" << type << "] "
              << "[" << msg->section << "] "
              << "at (" << msg->row << ", " << msg->col << ")" << std::endl
              <<  msg->message << std::endl;
}

/// @brief スクリプトエンジンを作成する
/// @return スクリプトエンジン
asIScriptEngine* InitScript(){
    //ジェネリック呼び出しが必要かを判断する
    if(std::string(asGetLibraryOptions()).find("AS_MAX_PORTABILITY") != std::string::npos){
        return NULL;
    }

    //エンジンを生成する
    asIScriptEngine *engine = asCreateScriptEngine();
    if(engine == NULL){
        return NULL;
    }

    //コンパイル情報を印字する関数を追加
    engine->SetMessageCallback(asFUNCTION(CompileInfo), 0, asCALL_CDECL);

    //標準ライブラリを追加
    RegisterStdString(engine);
    RegisterStdStringUtils(engine);
    RegisterScriptArray(engine, true);

    //エンジンを返す
    return engine;
}