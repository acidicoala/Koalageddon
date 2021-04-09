#include "pch.h"
#include "util.h"
#include "version_exports.h"
#include <boost/preprocessor/tuple/to_seq.hpp>
#include <boost/preprocessor/seq/for_each_i.hpp>
#include <boost/preprocessor/comma_if.hpp>

HMODULE hOriginal = NULL;

#define WRAP_declare_param(r, data, i, paramType) BOOST_PP_COMMA_IF(i) paramType _ ## i
#define WRAP_forward_param(r, data, i, paramType) BOOST_PP_COMMA_IF(i) _ ## i

#ifdef _WIN64
#define PREFIX ""
#else
#define PREFIX "_"
#endif

#define ARGS(...) __VA_ARGS__

#define FUNC_IMPL(TYPE, NAME, ...) \
extern "C" TYPE __cdecl __##NAME(BOOST_PP_SEQ_FOR_EACH_I(WRAP_declare_param, ~, BOOST_PP_TUPLE_TO_SEQ((__VA_ARGS__)))) { \
	__pragma(comment(linker, "/EXPORT:"#NAME"=__" PREFIX #NAME)) \
	static auto func = PLH::FnCast(GetProcAddress(hOriginal, #NAME), __##NAME); \
	return func(BOOST_PP_SEQ_FOR_EACH_I(WRAP_forward_param, ~, BOOST_PP_TUPLE_TO_SEQ((__VA_ARGS__)))); \
}

FUNC_IMPL(BOOL, GetFileVersionInfoA, ARGS(LPCSTR, DWORD, DWORD, LPVOID));
FUNC_IMPL(BOOL, GetFileInformationByHandle, ARGS(HANDLE, LPBY_HANDLE_FILE_INFORMATION));
FUNC_IMPL(BOOL, GetFileVersionInfoExA, ARGS(DWORD, LPCSTR, DWORD, DWORD, LPVOID));
FUNC_IMPL(BOOL, GetFileVersionInfoExW, ARGS(DWORD, LPCWSTR, DWORD, DWORD, LPVOID));
FUNC_IMPL(DWORD, GetFileVersionInfoSizeA, ARGS(LPCSTR, LPDWORD));
FUNC_IMPL(DWORD, GetFileVersionInfoSizeExA, ARGS(DWORD, LPCSTR, LPDWORD));
FUNC_IMPL(DWORD, GetFileVersionInfoSizeExW, ARGS(DWORD, LPCWSTR, LPDWORD));
FUNC_IMPL(DWORD, GetFileVersionInfoSizeW, ARGS(LPCWSTR, LPDWORD));
FUNC_IMPL(BOOL, GetFileVersionInfoW, ARGS(LPCWSTR, DWORD, DWORD, LPVOID));
FUNC_IMPL(DWORD, VerFindFileA, ARGS(DWORD, LPCSTR, LPCSTR, LPCSTR, LPSTR, PUINT, LPSTR, PUINT));
FUNC_IMPL(DWORD, VerFindFileW, ARGS(DWORD, LPCWSTR, LPCWSTR, LPCWSTR, LPWSTR, PUINT, LPWSTR, PUINT));
FUNC_IMPL(DWORD, VerInstallFileA, ARGS(DWORD, LPCSTR, LPCSTR, LPCSTR, LPCSTR, LPCSTR, LPSTR, PUINT));
FUNC_IMPL(DWORD, VerInstallFileW, ARGS(DWORD, LPCWSTR, LPCWSTR, LPCWSTR, LPCWSTR, LPCWSTR, LPWSTR, PUINT));
FUNC_IMPL(DWORD, VerLanguageNameA, ARGS(DWORD, LPSTR, DWORD));
FUNC_IMPL(DWORD, VerLanguageNameW, ARGS(DWORD, LPWSTR, DWORD));
FUNC_IMPL(BOOL, VerQueryValueA, ARGS(LPCVOID, LPCSTR, LPVOID*, PUINT));
FUNC_IMPL(BOOL, VerQueryValueW, ARGS(LPCVOID, LPCWSTR, LPVOID*, PUINT));
