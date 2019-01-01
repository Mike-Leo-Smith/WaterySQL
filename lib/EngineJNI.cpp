//
// Created by Mike Smith on 2019-01-01.
//

#include "EngineJNI.h"
#include "../src/parsing/parser.h"
#include "../src/utility/io/printer.h"

void Java_EngineJNI_initialize(JNIEnv *, jclass) {
    
    watery::PageManager::instance();
    if (!std::filesystem::exists(watery::DATABASE_BASE_PATH)) {
        std::filesystem::create_directories(watery::DATABASE_BASE_PATH);
    }
    
}

void Java_EngineJNI_execute(JNIEnv *env, jclass, jstring command) {
    
    jboolean a{JNI_FALSE};
    watery::Parser parser{env->GetStringUTFChars(command, &a)};
    
    while (!parser.end()) {
        try {
            parser.match()();
        } catch (const std::exception &e) {
            watery::Printer::print(std::cerr, e.what());
            parser.skip();
        }
    }
    
}

void Java_EngineJNI_finish(JNIEnv *, jclass) {
    watery::SystemManager::instance().finish();
    watery::Printer::println(std::cout, "Bye.");
}
