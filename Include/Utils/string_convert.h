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

	// ����˭���Ұ����㵺�� -> 6211662f8c01ff1f6211723194939c7c5c9bff01
	std::string StringToUnicode(std::string& str);

	// L"����˭���Ұ����㵺��" -> 6211662f8c01ff1f6211723194939c7c5c9bff01
	std::string WStringToUnicode(std::wstring& str);

	// 6211662f8c01ff1f6211723194939c7c5c9bff01 -> L"����˭���Ұ����㵺��" 
	std::wstring UnicodeToWString(std::string& unicodeStr);


	// windows �µ�ת��

#if 0
	//��stringת����wstring  
	std::wstring string2wstring(std::string str);

	//��wstringת����string  
	std::string wstring2string(std::wstring wstr);
#endif

	// �� string תΪ wstring
	std::wstring to_wstring(const std::string& input);
	// �� wstring תΪ string 
	std::string to_string(const std::wstring& input);


	//�ַ����ָ��
	std::vector<std::string> SplitStr(std::string str, std::string pattern);
	std::vector<std::wstring> SplitStr(std::wstring str, std::wstring pattern);

	std::string& replace_all_distinct(std::string& str, const  std::string& old_value, const   std::string& new_value);

	std::wstring& wstring_replace_all_distinct(std::wstring& str, const std::wstring& old_value, const std::wstring& new_value);

	LPSTR Base64Encode(LPBYTE lpBuffer, DWORD dwLen);

	LPSTR Base64Decode(LPSTR lpBase64Str, LPDWORD lpdwLen);

}
