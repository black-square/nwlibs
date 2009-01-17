//Автор: Шестеркин Дмитрий(NW) 2005

#include "stdafx.h"
#include "Auxiliary.h"
#include <shlwapi.h>

#pragma comment(lib, "shlwapi") //подлинкуем shlwapi.lib для PathFindFileName

//При конвертации использовать предположение о том что метод std::basic_string<T>::c_str()
//возвращает массив который можно изменять непосредственно применив const_cast
//Если необходимо включить данный максрос то стоит проверить что методы работают нормально
//при строках нулевого размера
#define USE_CONST_CAST_IN_STD_STRING_C_STR

namespace NWLib {

std::wstring &Convert(const std::string &In, std::wstring &Out)
{
#ifndef USE_CONST_CAST_IN_STD_STRING_C_STR
   int Rez = static_cast<int>(In.length()); 

   if( Rez == 0 )
   {
      Out.clear();
      return;
   }

   std::vector<WCHAR> Vec(Rez);

   APL_CHECK( MultiByteToWideChar(CP_ACP, 0, In.c_str(), Rez, &Vec.front(), Rez) == Rez );

   Out.assign( Vec.begin(), Vec.end() );

   return Out;
#else
   Out.resize( In.size() );

   APL_CHECK( MultiByteToWideChar(CP_ACP, 0, In.c_str(), (int)In.size(), const_cast<WCHAR *>(Out.c_str()), (int)Out.size()) == Out.size() );
   return Out;
#endif
}
///////////////////////////////////////////////////////////////////////////////

std::string &Convert(const std::wstring &In, std::string &Out)
{
#ifndef USE_CONST_CAST_IN_STD_STRING_C_STR
   int Rez = static_cast<int>(In.length()); 

   if( Rez == 0 )
   {
      Out.clear();
      return;
   }

   std::vector<CHAR> Vec(Rez);

   APL_CHECK( WideCharToMultiByte(CP_ACP, 0, In.c_str(), Rez, &Vec.front(), Rez, NULL, NULL) == Rez );

   Out.assign( Vec.begin(), Vec.end() );

   return Out;
#else
   Out.resize( In.size() );
   APL_CHECK( WideCharToMultiByte(CP_ACP, 0, In.c_str(), (int)In.size(), const_cast<CHAR *>(Out.c_str()), (int)Out.size(), NULL, NULL) == Out.size() );

   return Out;
#endif
}
///////////////////////////////////////////////////////////////////////////////

std::wstring &ToLower( const std::wstring &Src, std::wstring &Dst )
{
#ifndef USE_CONST_CAST_IN_STD_STRING_C_STR
   if( Src.empty() )
   {
      Dst.clear();
      return;
   }

   std::vector<wchar_t> Buff( Src.begin(), Src.end() ); 
   CharLowerBuffW( &Buff.front(), static_cast<DWORD>(Buff.size()) );
   Dst.assign( Buff.begin(), Buff.end() );

   return Dst;
#else
   Dst = Src;
   CharLowerBuffW( const_cast<WCHAR *>(Dst.c_str()), static_cast<DWORD>(Dst.size()) );
   return Dst;
#endif
}
///////////////////////////////////////////////////////////////////////////////

std::string &ToLower( const std::string &Src, std::string &Dst )
{
#ifndef USE_CONST_CAST_IN_STD_STRING_C_STR
   if( Src.empty() )
   {
      Dst.clear();
      return;
   }

   std::vector<char> Buff( Src.begin(), Src.end() ); 
   CharLowerBuffA( &Buff.front(), static_cast<DWORD>(Buff.size()) );
   Dst.assign( Buff.begin(), Buff.end() );

   return Dst;
#else
   Dst = Src;
   CharLowerBuffA( const_cast<CHAR *>(Dst.c_str()), static_cast<DWORD>(Dst.size()) );
   return Dst;
#endif
}
///////////////////////////////////////////////////////////////////////////////

std::basic_string<TCHAR> GetExeDirPath( HMODULE hModule /*= NULL*/ )
{
   TCHAR Buf[MAX_PATH];

   APL_CHECK( ::GetModuleFileName(hModule, Buf, MAX_PATH) );

   //TCHAR *pRez = _tcsrchr( Buf, _T('\\') );

   //if( pRez != NULL )
   //   *(pRez + 1) = _T('\0'); //Сохраняем  '\\'

   *PathFindFileName(Buf) = _T('\0');

   return Buf;
}
///////////////////////////////////////////////////////////////////////////////

std::basic_string<TCHAR> GetTimeStampString()
{
   SYSTEMTIME ST;
   const size_t BuffSize = 10;
   TCHAR Buff[BuffSize];

   GetLocalTime( &ST );

   APL_CHECK( GetTimeFormat(LOCALE_USER_DEFAULT, TIME_FORCE24HOURFORMAT | LOCALE_NOUSEROVERRIDE, &ST, NULL, Buff, BuffSize) );

   return Buff;
}
///////////////////////////////////////////////////////////////////////////////

std::basic_string<TCHAR> GetComputerName()
{
   TCHAR Buff[MAX_COMPUTERNAME_LENGTH + 1];
   DWORD Size = MAX_COMPUTERNAME_LENGTH + 1;
   
   APL_CHECK( ::GetComputerName(Buff, &Size) );

   return Buff;
}
///////////////////////////////////////////////////////////////////////////////

std::basic_string<TCHAR> GetHRErrorInfo( HRESULT hr )
{
	LPTSTR msg = 0;
	std::basic_string<TCHAR> Str;

	DWORD Rez = FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM, 0, hr,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&msg, 0, 0 );

	if( Rez )
	{
		//В зависимости от того определён UNICODE Нам необходимо преобразовать
		//msg в обычную строку
		try
		{
			Str = msg;

			//Часто функция FormatMessage вставляет символы перевода строки и точки,
			//а нам это не надо
			std::basic_string<TCHAR>::reverse_iterator I;

			for( I = Str.rbegin();
				I != Str.rend() && (*I == _T('\n') || *I == _T('\r') || *I == _T('.') );
				++I
				) { ; }

		   Str.erase( I.base(), Str.end() );

         LocalFree( msg );	
		}
		catch(...)
		{
			LocalFree( msg );
         throw;
		}
	}
	else
		Str = _T("Невозможно получить описание ошибки");


	return Str;
}

///////////////////////////////////////////////////////////////////////////////

} //namespace NWLib 

#ifdef NWLIB_STOCONA_FRAMEWORK
#include "StoconaFramework/StoconaAuxiliary.cpp"
#endif

