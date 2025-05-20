#include <jni.h>
#include "sap_decompressor.h"

extern "C" {
    JNIEXPORT jint JNICALL Java_SapDecompressor_decompressSapSource
    (JNIEnv* env, jobject obj, jstring inputPath, jstring outputPath, jint useUnicode, jint isNonUnicodeSap) {
        
        // Convert Java strings to C strings
        const char* inPath = env->GetStringUTFChars(inputPath, NULL);
        const char* outPath = env->GetStringUTFChars(outputPath, NULL);
        
        // Call our DLL function
        int result = DecompressSapSource(inPath, outPath, useUnicode, isNonUnicodeSap);
        
        // Release the Java strings
        env->ReleaseStringUTFChars(inputPath, inPath);
        env->ReleaseStringUTFChars(outputPath, outPath);
        
        return result;
    }
} 