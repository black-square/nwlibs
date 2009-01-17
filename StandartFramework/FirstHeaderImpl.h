#ifndef FirstHeaderImplH
#define FirstHeaderImplH

namespace NWLib
{
   class TAplThrowException: public std::exception
   {
      //BoundsChecker ����� �������� �� ������ ������ ��� �������� ���������� � ����� std::string
      //� ������ ����������� Multi-threaded Debug /MTd
      //������ BoundsChe�ker �������� ������� ����� ���� ���������� ���� Multi-threaded Debug DLL (/MDd)
      std::string m_File;  
      int m_Line;

   public:
      TAplThrowException() {}
      TAplThrowException( const char *Msg ): std::exception(Msg) { }
      TAplThrowException( const char *Msg, const char *File, int Line ): 
         std::exception(Msg), m_File(File), m_Line(Line) { }

      const char *File() const { return m_File.c_str(); }
      int Line() const { return m_Line; }
   };
}

#define APL_THROW( arg ) do {                                                                       \
   std::basic_stringstream<TCHAR> Stream__; Stream__ << arg;                                    \
   throw NWLib::TAplThrowException( NWLib::ConvertToBuf<std::string>(Stream__.str()).c_str(), __FILE__, __LINE__ );                   \
} while(false)


#define APL_LOG( arg )  \
do {                    \
   std::basic_stringstream<TCHAR> Stream__; Stream__ << _T(APP_ID) _T("_LOG\t") << arg << std::endl << _T(APP_ID) _T("_LOG\t\t") << _T(__FILE__) _T(", ") << __LINE__ << std::endl;   \
   OutputDebugString( Stream__.str().c_str() );  \
} while(false)


#include "Auxiliary.h" //��� ������� Convert

#endif