#include "string_convert.h"
#include <iostream>
#include <codecvt>
#include <locale>

#include <wincrypt.h>
#pragma comment(lib, "crypt32.lib")

#pragma warning(disable:4996)

namespace Utils
{
	std::u16string to_utf16(std::string str) // utf-8 to utf16
	{
		return std::wstring_convert< std::codecvt_utf8_utf16<char16_t>, char16_t >{}.from_bytes(str);
	}

	
	std::string to_utf8(std::u16string str16)
	{
		return std::wstring_convert< std::codecvt_utf8_utf16<char16_t>, char16_t >{}.to_bytes(str16);
	}

	std::u32string to_utf32(std::string str)
	{
		return std::wstring_convert< std::codecvt_utf8<char32_t>, char32_t >{}.from_bytes(str);
	}

	std::string to_utf8(std::u32string str32)
	{
		return std::wstring_convert< std::codecvt_utf8<char32_t>, char32_t >{}.to_bytes(str32);
	}

	/// <summary>
	/// utf8 -> wchar
	/// </summary>
	/// <param name="str"></param>
	/// <returns></returns>
	std::wstring to_wchar_t(std::string str)
	{
		return std::wstring_convert< std::codecvt_utf8<wchar_t>, wchar_t >{}.from_bytes(str);
	}

	std::string to_utf8(std::wstring wstr)
	{
		return std::wstring_convert< std::codecvt_utf8<wchar_t>, wchar_t >{}.to_bytes(wstr);
	}

	// ����˭���Ұ����㵺�� -> 6211662f8c01ff1f6211723194939c7c5c9bff01
	std::string StringToUnicode(std::string& str)
	{
		if (str.length() == 0)
		{
			return "";
		}

		std::string unicodeStr;

		size_t length = strlen(str.c_str()) + 1;
		// char * setlocale ( int category, const char * locale );
		// �������������õ������Ϣ�����õ�ǰ����ʹ�õı��ػ���Ϣ.���� locale ���ǿ��ַ��� ""�����ʹ��ϵͳ���������� locale
		// ˵��string�е��ַ������Ǳ���Ĭ���ַ�������GB�ַ���
		setlocale(LC_ALL, "");
		wchar_t* wstr = new wchar_t[length];
		mbstowcs(wstr, str.c_str(), length);
		char charUnicode[5];

		for (size_t i = 0; i < wcslen(wstr); i++)
		{
			memset(charUnicode, '\0', 5);
			WCHAR wchar = wstr[i];
			sprintf_s(charUnicode, "%04x", wchar);
			unicodeStr.append(charUnicode);
		}

		delete[]wstr;

		return unicodeStr;
	}

	// L"����˭���Ұ����㵺��" -> 6211662f8c01ff1f6211723194939c7c5c9bff01
	std::string WStringToUnicode(std::wstring& str)
	{
		if (str.length() == 0)
		{
			return "";
		}
		char charUnicode[5];
		std::string unicodeStr;
		for (size_t i = 0; i < str.length(); i++)
		{
			memset(charUnicode, '\0', 5);
			WCHAR wchar = str[i];
			sprintf_s(charUnicode, "%04x", wchar);
			unicodeStr.append(charUnicode);
		}


		return unicodeStr;
	}

	// 6211662f8c01ff1f6211723194939c7c5c9bff01 -> L"����˭���Ұ����㵺��" 
	std::wstring UnicodeToWString(std::string& unicodeStr)
	{
		std::u32string u32Str;
		std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> converter;
		for (size_t i = 0; i < unicodeStr.length(); ) {
			char32_t uhan = strtol(unicodeStr.substr(i, 4).c_str(), nullptr, 16);
			u32Str += uhan;

			i += 4;
		}

		std::string utf8Str = to_utf8(u32Str);
		std::wstring wstr = to_wchar_t(utf8Str);

		return wstr;
	}

#if 0
	//��stringת����wstring  
	std::wstring string2wstring(std::string str)
	{
		std::wstring result;
		//��ȡ��������С��������ռ䣬��������С���ַ�����  
		int len = MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.size(), NULL, 0);
		WCHAR* buffer = new WCHAR[len + 1];
		//���ֽڱ���ת���ɿ��ֽڱ���  
		MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.size(), buffer, len);
		buffer[len] = '\0';             //����ַ�����β  
										//ɾ��������������ֵ  
		result.append(buffer);
		delete[] buffer;
		return result;
	}

	//��wstringת����string  
	std::string wstring2string(std::wstring wstr)
	{
		std::string result;
		//��ȡ��������С��������ռ䣬��������С�°��ֽڼ����  
		int len = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), wstr.size(), NULL, 0, NULL, NULL);
		char* buffer = new char[len + 1];
		//���ֽڱ���ת���ɶ��ֽڱ���  
		WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), wstr.size(), buffer, len, NULL, NULL);
		buffer[len] = '\0';
		//ɾ��������������ֵ  
		result.append(buffer);
		delete[] buffer;
		return result;
	}
#endif

	/// <summary>
	/// �� string תΪ wstring 
	/// </summary>
	/// <param name="input"></param>
	/// <returns></returns>
#if false
	std::wstring to_wstring(const std::string& input)
	{
		std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
		return converter.from_bytes(input);
	}

	/// <summary>
	///  �� wstring תΪ string 
	/// </summary>
	/// <param name="input"></param>
	/// <returns></returns>
	std::string to_string(const std::wstring& input)
	{
		std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
		return converter.to_bytes(input);
	}
#else 
	std::string to_string(const std::wstring& var)
	{
		static std::locale loc("");
		auto& facet = std::use_facet<std::codecvt<wchar_t, char, std::mbstate_t>>(loc);
		return std::wstring_convert<std::remove_reference<decltype(facet)>::type, wchar_t>(&facet).to_bytes(var);
	}

	std::wstring to_wstring(const std::string& var)
	{
		static std::locale loc("");
		auto& facet = std::use_facet<std::codecvt<wchar_t, char, std::mbstate_t>>(loc);
		return std::wstring_convert<std::remove_reference<decltype(facet)>::type, wchar_t>(&facet).from_bytes(var);
	}
#endif


	//�ַ����ָ��
	std::vector<std::string> SplitStr(std::string str, std::string pattern)
	{
		std::string::size_type pos;
		std::vector<std::string> result;
		str += pattern;//��չ�ַ����Է������
		int size = str.size();
		for (int i = 0; i < size; i++)
		{
			pos = str.find(pattern, i);
			if (pos < size)
			{
				std::string s = str.substr(i, pos - i);
				result.push_back(s);
				i = pos + pattern.size() - 1;
			}
		}
		return result;
	}

	std::vector<std::wstring> SplitStr(std::wstring str, std::wstring pattern)
	{
		std::wstring::size_type pos;
		std::vector<std::wstring> result;
		str += pattern;//��չ�ַ����Է������
		int size = str.size();
		for (int i = 0; i < size; i++)
		{
			pos = str.find(pattern, i);
			if (pos < size)
			{
				std::wstring s = str.substr(i, pos - i);
				result.push_back(s);
				i = pos + pattern.size() - 1;
			}
		}
		return result;
	}

	std::string& replace_all_distinct(std::string& str, const  std::string& old_value, const   std::string& new_value)
	{
		for (std::string::size_type pos(0); pos != std::string::npos; pos += new_value.length())
		{
			if ((pos = str.find(old_value, pos)) != std::string::npos)
			{
				str.replace(pos, old_value.length(), new_value);
			}
			else { break; }
		}
		return   str;
	}

	std::wstring& wstring_replace_all_distinct(std::wstring& str, const std::wstring& old_value, const std::wstring& new_value)
	{
		for (std::wstring::size_type pos(0); pos != std::wstring::npos; pos += new_value.length())
		{
			if ((pos = str.find(old_value, pos)) != std::wstring::npos)
			{
				str.replace(pos, old_value.length(), new_value);
			}
			else { break; }
		}
		return   str;
	}

	LPSTR Base64Encode(LPBYTE lpBuffer, DWORD dwLen)
	{
		DWORD dwNeed;
		LPSTR lpBase64Str = NULL;
		DWORD dwIndex;
		DWORD dwIndexJ;
		dwNeed = 0;
		lpBase64Str = NULL;

		CryptBinaryToStringA(lpBuffer, dwLen, CRYPT_STRING_BASE64, NULL, &dwNeed);
		if (dwNeed)
		{
			lpBase64Str = (LPSTR)malloc(dwNeed);
			ZeroMemory(lpBase64Str, dwNeed);
			CryptBinaryToStringA(lpBuffer, dwLen, CRYPT_STRING_BASE64, lpBase64Str, &dwNeed);
			dwIndex = 0;
			while (*(lpBase64Str + dwIndex) != 0)
			{
				if (*(lpBase64Str + dwIndex) == 0x0d || *(lpBase64Str + dwIndex) == 0x0a)
				{
					dwIndexJ = dwIndex + 1;

					while (*(lpBase64Str + dwIndexJ) != 0)
					{
						*(lpBase64Str + dwIndexJ - 1) = *(lpBase64Str + dwIndexJ);
						dwIndexJ++;
					}

					*(lpBase64Str + dwIndexJ - 1) = 0;

				}
				else
				{
					dwIndex++;
				}

			}
		}

		return lpBase64Str;
	}

	LPSTR Base64Decode(LPSTR lpBase64Str, LPDWORD lpdwLen)
	{
		DWORD dwLen;
		DWORD dwNeed;
		LPSTR lpBuffer = NULL;

		dwLen = strlen(lpBase64Str);
		dwNeed = 0;
		CryptStringToBinaryA(lpBase64Str, 0, CRYPT_STRING_BASE64, NULL, &dwNeed, NULL, NULL);
		if (dwNeed)
		{
			lpBuffer = (LPSTR)malloc(dwNeed);
			CryptStringToBinaryA(lpBase64Str, 0, CRYPT_STRING_BASE64, (LPBYTE)lpBuffer, &dwNeed, NULL, NULL);
			*lpdwLen = dwNeed;
		}

		return lpBuffer;
	}

}
