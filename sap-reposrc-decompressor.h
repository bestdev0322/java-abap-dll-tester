#ifdef _WIN32
#define DLL_EXPORT extern "C" __declspec(dllexport)
#else
#define DLL_EXPORT extern "C"
#endif

DLL_EXPORT int decompress_sap_source(const char* input_path, const char* output_path); 