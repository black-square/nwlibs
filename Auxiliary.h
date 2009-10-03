//Автор: Шестеркин Дмитрий(NW) 2005

#ifndef Auxiliary_H
#define Auxiliary_H

#include <WTypes.h>

///////////////////////////////////////////////////////////////////////////////
// Модуль содержит вспомогательные функции и макросы системного характера
///////////////////////////////////////////////////////////////////////////////

namespace NWLib {

//Преобразования строк char и wchar_t друг в друга
std::wstring &Convert(const std::string &In, std::wstring &Out);
std::string &Convert(const std::wstring &In, std::string &Out);

//Заменить следующие две функции на
//template< class T > inline T &Convert(const T &In, T &Out){ return Out = In; }
//нельзя т.к. при вызове Convert возможно неявное преобразование типов, и шаблонная 
//функция вызвана не будет
inline std::wstring &Convert(const std::wstring &In, std::wstring &Out){ return Out = In; }
inline std::string &Convert(const std::string &In, std::string &Out){ return Out = In; }

namespace Detail
{
   template< class TIn, class TOut >
   struct ConvertToBufImpl
   {
      typedef TOut TRetVal;
      static TRetVal F( const TIn &In ){ TOut Out; return Convert(In, Out); }
   };
   
   template< class T >
   struct ConvertToBufImpl< T, T >
   {
      typedef const T & TRetVal;
      static TRetVal F( const T &In ){ return In; }
   };
}

//Возвращение объекта по значению, с созданием внутреннего буфера. Медленная операция.
template< class TOut, class TIn >
inline typename Detail::ConvertToBufImpl<TIn, TOut>::TRetVal 
   ConvertToBuf( const TIn &In ){ return Detail::ConvertToBufImpl<TIn, TOut>::F(In); }

//Функции нужны для поддержания совместимости, лучше пользоваться ConvertToBuf
inline std::wstring ConvertToWide(const std::string &In){ return ConvertToBuf<std::wstring>(In); }
inline const std::wstring &ConvertToWide(const std::wstring &In){ return ConvertToBuf<std::wstring>(In); }
inline std::string ConvertToNarrow(const std::wstring &In){ return ConvertToBuf<std::string>(In); }
inline const std::string &ConvertToNarrow(const std::string &In){ return ConvertToBuf<std::string>(In); }

//Конвертировать строку в нижний регистр, Src и Dst могут совпадать
std::wstring &ToLower( const std::wstring &Src, std::wstring &Dst );
std::string &ToLower( const std::string &Src, std::string &Dst );

//Конвертировить строку в std::basic_string<TCHAR>
//Просто сокращаённая запись ConvertToBuf< std::basic_string<TCHAR> >(In);
inline Detail::ConvertToBufImpl< std::wstring, std::basic_string<TCHAR> >::TRetVal 
   ConvertToTStr( const std::wstring &In ){ return ConvertToBuf< std::basic_string<TCHAR> >(In); }

inline Detail::ConvertToBufImpl< std::string, std::basic_string<TCHAR> >::TRetVal 
   ConvertToTStr( const std::string &In ){ return ConvertToBuf< std::basic_string<TCHAR> >(In); } 

///////////////////////////////////////////////////////////////////////////////

//Путь к текущему модулю
//В конце сохраняется '\\'
std::basic_string<TCHAR> GetExeDirPath( HMODULE hModule = NULL );

//Имя компьютера
std::basic_string<TCHAR> GetComputerName();

//Строка с текущем временем
std::basic_string<TCHAR> GetTimeStampString();

// Возвращает текстовое описание HRESULT
std::basic_string<TCHAR> GetHRErrorInfo( HRESULT hr );

// Возвращает текстовое описание DWORD
inline std::basic_string<TCHAR> GetDWErrorInfo( DWORD dwErr )
{
   return GetHRErrorInfo( HRESULT_FROM_WIN32(dwErr) );
}

} //namespace NWLib 

#ifdef NWLIB_STOCONA_FRAMEWORK
#include "StoconaFramework/StoconaAuxiliary.h"
#endif


#endif