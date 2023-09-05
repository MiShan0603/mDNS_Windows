#pragma once
#include <windows.h>
#include <string>
#include <vector>

namespace Utils
{
	std::u16string to_utf16(std::string str); // utf-8 to utf16

	std::string to_utf8(std::u16string str16);

	std::u32string to_utf32(std::string str);

	std::string to_utf8(std::u32string str32);

	std::wstring to_wchar_t(std::string str);

	std::string to_utf8(std::wstring wstr);

	// 我是谁？我爱钓鱼岛！ -> 6211662f8c01ff1f6211723194939c7c5c9bff01
	std::string StringToUnicode(std::string& str);

	// L"我是谁？我爱钓鱼岛！" -> 6211662f8c01ff1f6211723194939c7c5c9bff01
	std::string WStringToUnicode(std::wstring& str);

	// 6211662f8c01ff1f6211723194939c7c5c9bff01 -> L"我是谁？我爱钓鱼岛！" 
	std::wstring UnicodeToWString(std::string& unicodeStr);


	// windows 下的转换

#if 0
	//将string转换成wstring  
	std::wstring string2wstring(std::string str);

	//将wstring转换成string  
	std::string wstring2string(std::wstring wstr);
#endif

	// 将 string 转为 wstring
	std::wstring to_wstring(const std::string& input);
	// 将 wstring 转为 string 
	std::string to_string(const std::wstring& input);


	//字符串分割函数
	std::vector<std::string> SplitStr(std::string str, std::string pattern);
	std::vector<std::wstring> SplitStr(std::wstring str, std::wstring pattern);

	std::string& replace_all_distinct(std::string& str, const  std::string& old_value, const   std::string& new_value);

	std::wstring& wstring_replace_all_distinct(std::wstring& str, const std::wstring& old_value, const std::wstring& new_value);

	LPSTR Base64Encode(LPBYTE lpBuffer, DWORD dwLen);

	LPSTR Base64Decode(LPSTR lpBase64Str, LPDWORD lpdwLen);

}
