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

//SS_THROW �� ������ ������ ����������, � ��������� ������ ��� ������ ���������� �������� SS_THROW
//����� ��������� ������ ����� ������� ���������. � ����� ������� �������� ����������� ��� ����������
//std::exception 
#include "../ASCInterface/defines.h"

SET_LOAD_MANAGER_DEFINE;
USES_LOAD_MANAGER;

///////////////////////////////////////////////////////////////////////////////
// !!! ���������� � ����� �� ������� CPP ���������� SET_LOAD_MANAGER_IMPLEMENTATION; !!!
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

#include "Auxiliary.h" //��� ������� Convert

#endif