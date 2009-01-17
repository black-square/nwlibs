//Автор: Шестеркин Дмитрий(NW) 2005

#include "stdafx.h"
#include "TestHelper.h"
#include "CoutConvert.h"

#include <shellapi.h>

namespace NWLib {

ConsoleSynchronous Console;

///////////////////////////////////////////////////////////////////////////////

DWORD RunCommand( LPTSTR Command, LPCTSTR CurrentDirectory /*= NULL*/ )
{
   STARTUPINFO si;
   PROCESS_INFORMATION pi;

   ZeroMemory( &si, sizeof(si) );
   si.cb = sizeof(si);
   ZeroMemory( &pi, sizeof(pi) );

   // Start the child process. 
   if( !CreateProcess( NULL, // No module name (use command line). 
      Command,          // Command line. 
      NULL,             // Process handle not inheritable. 
      NULL,             // Thread handle not inheritable. 
      FALSE,            // Set handle inheritance to FALSE. 
      0,                // No creation flags. 
      NULL,             // Use parent's environment block. 
      CurrentDirectory, // Starting directory. 
      &si,              // Pointer to STARTUPINFO structure.
      &pi )             // Pointer to PROCESS_INFORMATION structure.
      ) 
   {
      APL_THROW( 
         _T("Error on executing command '") << ConvertToBuf<std::basic_string<TCHAR> >(Command)
         << _T("' with starting dir '") << (CurrentDirectory == NULL ? std::basic_string<TCHAR>(_T("NULL")) : ConvertToBuf<std::basic_string<TCHAR> >(CurrentDirectory)) 
         << _T("': ") << GetDWErrorInfo( GetLastError() )
      );      
   }

   DWORD ExitCode;

   APL_ERROR( WaitForSingleObject(pi.hProcess, INFINITE) != WAIT_FAILED );
   APL_ERROR( GetExitCodeProcess(pi.hProcess, &ExitCode) );       
   APL_ERROR( CloseHandle(pi.hProcess) );
   APL_ERROR( CloseHandle(pi.hThread) );

   return ExitCode;
}
///////////////////////////////////////////////////////////////////////////////

void ExecutePreRunFile( const TCHAR *FilePath )
{
#if 0
   SHELLEXECUTEINFO ShExecInfo;
   
   TCHAR FullPath[MAX_PATH], FileDir[MAX_PATH];
   LPTSTR FileName;

   APL_ERROR( GetFullPathName(FilePath, MAX_PATH, FullPath, &FileName) != 0 );

   memcpy( FileDir, FullPath, (FileName - FullPath) * sizeof(*FileDir) );
   FileDir[FileName - FullPath] = _T('\0');

   ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
   ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
   ShExecInfo.hwnd = NULL;     o
   ShExecInfo.lpVerb = NULL;
   ShExecInfo.lpFile = FullPath;
   
#ifdef DEBUG
   ShExecInfo.lpParameters = _T( "DEBUG" );
#else
   ShExecInfo.lpParameters = _T( "RELEASE" );
#endif

   ShExecInfo.lpDirectory = FileDir;
   ShExecInfo.nShow = nShowCmd;
   ShExecInfo.hInstApp = NULL;

   cout << "Запуск файла " << FullPath;
   if( ShellExecuteEx(&ShExecInfo) != TRUE )
   {
      cout << "ERROR: " << GetDWErrorInfo( GetLastError() );
      throw TSilentException();
   }

   if( ShExecInfo.hProcess != NULL )
   {
      DWORD ExitCode;

      APL_ERROR( WaitForSingleObject(ShExecInfo.hProcess, INFINITE) != WAIT_FAILED );
      APL_ERROR( GetExitCodeProcess(ShExecInfo.hProcess, &ExitCode) );       
      APL_ERROR( CloseHandle(ShExecInfo.hProcess) != 0 );

      if( ExitCode != 0 )
      {
         cout << " ERROR ExitCode != 0!"<< endl;
         throw TSilentException();
      }
      else
         cout << " OK!" << endl;
   }
   else
      cout << " OK (no hProcess)!" << endl;
#else

   #ifdef DEBUG
      #define CMD_LINE _T( " DEBUG" )
   #else
      #define CMD_LINE _T( " RELEASE" )
   #endif

   TCHAR FullPath[MAX_PATH], FileDir[MAX_PATH];
   LPTSTR FileName;

   APL_ERROR( GetFullPathName(FilePath, MAX_PATH, FullPath, &FileName) != 0 );

   memcpy( FileDir, FullPath, (FileName - FullPath) * sizeof(*FileDir) );
   FileDir[FileName - FullPath] = _T('\0');
   
   lstrcat( FullPath, CMD_LINE );

   std::cout << "Запуск файла: " << FullPath << std::endl << APL_LINE;

   DWORD ExitCode = RunCommand( FullPath, FileDir );
   std::cout << APL_LINE;

   if( ExitCode != 0)
   {
      std::cout << " ERROR ExitCode: " << ExitCode << std::endl;
      throw TSilentException();
   }
   else
      std::cout << "OK!" << std::endl << std::endl;

#endif
}
///////////////////////////////////////////////////////////////////////////////

void CopyLoadManager()
{
#ifdef DEBUG
   std::basic_string<TCHAR> CopyFrom(GetExeDirPath() + _T("..\\..\\..\\..\\Bin.debug\\Core\\LoadManager.dll") );
#else
   std::basic_string<TCHAR> CopyFrom(GetExeDirPath() + _T("..\\..\\..\\..\\Bin\\Core\\LoadManager.dll") );
#endif

   std::basic_string<TCHAR> CopyTo(GetExeDirPath() + _T("LoadManager.dll") );

   std::cout << "Копируем\n\t" << CopyFrom << "\n\t" << CopyTo << std::endl << std::endl;
   if( CopyFile(CopyFrom.c_str(), CopyTo.c_str(), FALSE) == FALSE )
   {
      std::cout << GetDWErrorInfo(GetLastError());
      throw TSilentException();
   }
}

///////////////////////////////////////////////////////////////////////////////
// class ConsoleSynchronous
///////////////////////////////////////////////////////////////////////////////
void ConsoleSynchronous::StdData()
{
   std::cout << GetTimeStampString() << " ";
}



} //namespace NWLib 
