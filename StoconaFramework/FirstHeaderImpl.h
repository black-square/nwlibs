#ifndef FirstHeaderImplH
#define FirstHeaderImplH

#include <exception>
namespace StoconaSearch {}
namespace NWLib { using namespace StoconaSearch; }
namespace StoconaSearch
{ 
	namespace Lib = NWLib; 
	using std::exception;
}

//SS_THROW не всегда кидает исключения, в противном случае она просто игнорирует комманду SS_THROW
//Такое поведение врядле можно назвать корретным. В таких случаях кидается стандратный тип исключения
//std::exception 
#include "../ASCInterface/defines.h"

SET_LOAD_MANAGER_DEFINE;
USES_LOAD_MANAGER;

///////////////////////////////////////////////////////////////////////////////
// !!! Необходимо в одном из модулей CPP определить SET_LOAD_MANAGER_IMPLEMENTATION; !!!
///////////////////////////////////////////////////////////////////////////////

#ifndef USES_IBASE
#define APL_THROW( arg ) do {                                                                       \
   std::basic_stringstream<TCHAR> Stream__; Stream__ << arg;                                    \
   if( !(!spLoadManager ) ) {                                                                   \
   SS_THROW( const_cast<wchar_t*>(NWLib::ConvertToBuf<std::wstring>(Stream__.str()).c_str()) );     \
   } else {                                                                                     \
      throw std::exception( NWLib::ConvertToBuf<std::string>(Stream__.str()).c_str() );                \
   }                                                                                            \
} while(false)
#else
#define APL_THROW( arg ) do {                                                                       \
   std::basic_stringstream<TCHAR> Stream__; Stream__ << arg;                                    \
   SS_THROW( const_cast<wchar_t*>(NWLib::ConvertToBuf<std::wstring>(Stream__.str()).c_str()) );        \
}  while(false)
#endif

#define APL_LOG( arg ) do {                                                                         \
   std::basic_stringstream<TCHAR> Stream__; Stream__ << arg <<                                  \
       std::endl << _T("\t") << __WFUNCTION__ << std::endl;                                     \
   SAVE_LOG(SS_WARNING AND const_cast<wchar_t*>(                                                \
       NWLib::ConvertToBuf<std::wstring>(Stream__.str()).c_str()) );                                   \
} while(false)

#include "Auxiliary.h" //Для функции Convert

#endif