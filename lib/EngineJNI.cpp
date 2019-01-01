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
    std::filesystem::remove_all("result.html");
    while (!parser.end()) {
        try {
            parser.match()();
        } catch (const std::exception &e) {
            std::ofstream f{watery::RESULT_FILE_NAME, std::ios::app};
            watery::Printer::print(f, "<p style='color:red'>", e.what(), "</p>");
            parser.skip();
        }
    }
    
}

void Java_EngineJNI_finish(JNIEnv *, jclass) {
    watery::SystemManager::instance().finish();
    watery::Printer::println(std::cout, "Bye.");
}

jstring Java_EngineJNI_getCurrentDatabaseName(JNIEnv *env, jclass) {
    return env->NewStringUTF(watery::SystemManager::instance().current_database().c_str());
}
