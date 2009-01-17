//�����: ��������� �������(NW) 2005

/** 
   ������ ������������� ��������� ������� ���������� ������ � ��������
*/
#ifndef TestHelper_H
#define TestHelper_H

#include <iostream>
#include <conio.h>
#include <string>
#include <vector>
#include <sstream>
#include "Threads.h"
#include "ThreadsManager.hpp"


//������ ������ ��� ��������� ��������� ������
#define APL_LINE "-------------------------------------------------------------------------------\n"

//���� arg ����� ����� � ������� ���������� � ������ TSilentException
#define APL_ERROR( arg )do {if( !(arg) ){ \
	std::stringstream Stream__; \
   Stream__ << APL_LINE;  \
	Stream__ << "   APL_ERROR( " ## #arg ## " )\n"; \
	Stream__ << "   " ## __FILE__ ## ", " << __LINE__ << std::endl;  \
	Stream__ << APL_LINE;  \
   std::cout << Stream__.str(); \
   throw NWLib::TSilentException(); \
} }while(false)

//���� ��� � APL_ERROR, �� �� ������ ����������
#define APL_WARNING( arg )	do { if( !(arg) ){ \
   std::stringstream Stream__; \
   Stream__ << APL_LINE;  \
   Stream__ << "   APL_WARNING( " ## #arg ## " )\n"; \
   Stream__ << "   " ## __FILE__ ## ", " << __LINE__ << std::endl;  \
   Stream__ << APL_LINE;  \
   std::cout << Stream__.str(); \
} }while(false)

#ifdef NWLIB_STOCONA_FRAMEWORK

#define APL_TRY() SS_TRY { try
#define APL_CATCH()\
catch (const TSilentException &) \
{}   \
catch( const std::exception &e )   \
{ \
	std::cout << "std::exception: " << e.what() << std::endl; \
	throw;  \
} }  \
SS_CATCH(L"")

#else

#define APL_TRY() try
#define APL_CATCH()\
catch (const NWLib::TSilentException &) \
{}   \
catch( const NWLib::TAplThrowException &e )   \
{ \
   NWLib::Console.Write( "NWLib::TAplThrowException ", e.File(), ", ", e.Line(), ":\n\t", e.what(), "\n" );  \
} \
catch( const std::exception &e )   \
{ \
	NWLib::Console.Write(  "std::exception: ", e.what() ); \
} \
catch(...) \
{ \
   NWLib::Console.Write( "<UNKNOWN_EXCEPTION>" ); \
} 

#endif

namespace NWLib {

//��������� ������������� ��������� ����� ������� �� �� � ������� ������������ ������ �������
class TConsoleAutoStop
{
   const char * const m_pMsg;

public:
   TConsoleAutoStop( const char *pMsg = "\nPress any Key...\n" ): m_pMsg( pMsg ) { APL_ASSERT_PTR(m_pMsg); }

	~TConsoleAutoStop()
	{
      std::cout << m_pMsg;
		while( _kbhit() ) _getch();

      _getch();
	}
};

///////////////////////////////////////////////////////////////////////////////
// ��������� �������� SHELL ��� ���� �� ���������� ��� ���� ���� ����� ����������
// �������� �������������� � ������� �������
// Command - �������� ��� �������, ��� ���� ����� ��������� �������� shell
//           ����� ������������ ��������� "cmd /c <��� ��������>", �������� "cmd /c dir /b"
// CurrentDirectory - ������� ���������� � ����������� ��������, ��� NULL, ��
//                    ������������ ���������� ������� � ������ ������ �������� �������
// ����� - ��� ������� ������� �������� ��� ������
///////////////////////////////////////////////////////////////////////////////
DWORD RunCommand( LPTSTR Command, LPCTSTR CurrentDirectory = NULL );

///////////////////////////////////////////////////////////////////////////////
// ��������� ���� FilePath �� ���������� � ���� FilePath ����������� ����
// ��������� ����� ��� ����������. ������� ������������� ��� ������������ �����, 
// � �������� �������� ��������, ������� � ������� ��������� ���� FilePath � 
// ������� ��� ������ �������� ������ DEBUG, ���� ��������� ������ DEBUG � 
// RELEASE ���� �� ���������.
// ��������� ������������ �������� ����� (��� bat ����� ��������� exit <���>)
// � ���� ��� != 0 �� ������ ����������
///////////////////////////////////////////////////////////////////////////////
void ExecutePreRunFile( const TCHAR *FilePath );

///////////////////////////////////////////////////////////////////////////////
// ����������� ����������� dll ��� LoadManager'a
///////////////////////////////////////////////////////////////////////////////
void CopyLoadManager();

///////////////////////////////////////////////////////////////////////////////
/**
	��������� �������� ��� �������� �������� ����-������.
	� ������ APL_TRY APL_CATCH ������ ���� ������� ���� ��� �����
	��� �������������� �������� ���������� � �������� ���������� � ��� �� �������.
	���-�� ������������� �������� ���������� TSilentException ������.
	APL_TRY()
	{
	   //��� ����-�����
	}
	APL_CATCH()


*/
//��������� ������� ������ ������ ������ ��� ������ �� �����.
//���������� � �� �� ������ ��������� ������������
class TSilentException: public std::exception  
{ 
public: 
	TSilentException():	std::exception("SilentException") {}
};  

//��������� ���������������� ����� �� �������
class ConsoleSynchronous
{
   typedef ClassLevelLockable<ConsoleSynchronous> TMutex;
   TMutex Mutex;

   void StdData();

public:
   template< class T1 >
      void Write( const T1 &o1 )
   {
      TMutex::Lock L(Mutex);
      StdData();
      std::cout << o1 << std::endl;
   }

   template< class T1, class T2 >
      void Write( const T1 &o1, const T2 &o2 )
   {
      TMutex::Lock L(Mutex);
      StdData();
      std::cout << o1 << o2 << std::endl;
   }

   template< class T1, class T2, class T3 >
      void Write( const T1 &o1, const T2 &o2, const T3 &o3 )
   {
      TMutex::Lock L(Mutex);
      StdData();
      std::cout << o1 << o2 << o3 <<std::endl;
   }

   template< class T1, class T2, class T3, class T4 >
      void Write( const T1 &o1, const T2 &o2, const T3 &o3, const T4 &o4 )
   {
      TMutex::Lock L(Mutex);
      StdData();
      std::cout << o1 << o2 << o3 << o4 << std::endl;
   }

   template< class T1, class T2, class T3, class T4, class T5 >
      void Write( const T1 &o1, const T2 &o2, const T3 &o3, const T4 &o4, const T5 &o5 )
   {
      TMutex::Lock L(Mutex);
      StdData();
      std::cout << o1 << o2 << o3 << o4 << o5 << std::endl;
   }

   template< class T1, class T2, class T3, class T4, class T5, class T6 >
      void Write( const T1 &o1, const T2 &o2, const T3 &o3, const T4 &o4, const T5 &o5, const T6 &o6 )
   {
      TMutex::Lock L(Mutex);
      StdData();
      std::cout << o1 << o2 << o3 << o4 << o5 << o6 << std::endl;
   }

   template< class T1, class T2, class T3, class T4, class T5, class T6, class T7 >
   void Write( const T1 &o1, const T2 &o2, const T3 &o3, const T4 &o4, const T5 &o5, const T6 &o6, const T7 &o7 )
   {
      TMutex::Lock L(Mutex);
      StdData();
      std::cout << o1 << o2 << o3 << o4 << o5 << o6 << o7 << std::endl;
   }
   
   template< class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8 >
   void Write( const T1 &o1, const T2 &o2, const T3 &o3, const T4 &o4, const T5 &o5, const T6 &o6, const T7 &o7, const T8 &o8 )
   {
      TMutex::Lock L(Mutex);
      StdData();
      std::cout << o1 << o2 << o3 << o4 << o5 << o6 << o7 << o8 << std::endl;
   }

   template< class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9 >
   void Write( const T1 &o1, const T2 &o2, const T3 &o3, const T4 &o4, const T5 &o5, const T6 &o6, const T7 &o7, const T8 &o8, const T9 &o9 )
   {
      TMutex::Lock L(Mutex);
      StdData();
      std::cout << o1 << o2 << o3 << o4 << o5 << o6 << o7 << o8 << o9 << std::endl;
   }

   template< class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9, class T10 >
   void Write( const T1 &o1, const T2 &o2, const T3 &o3, const T4 &o4, const T5 &o5, const T6 &o6, const T7 &o7, const T8 &o8, const T9 &o9, const T10 &o10 )
   {
      TMutex::Lock L(Mutex);
      StdData();
      std::cout << o1 << o2 << o3 << o4 << o5 << o6 << o7 << o8 << o9 << o10 << std::endl;
   }
};
extern ConsoleSynchronous Console;

///////////////////////////////////////////////////////////////////////////////
// ����� ������������� ������� �������� ��� �������������� ������������ (��� ������ 
// ����������, ��������� �������� ������ � ��������� ������
// TesterT - ���������, ������� ����������� �������, �� ��������� �����������
//         interface TesterT
//         {
//            //����������� ����������� ��� �������� ���������� ����������� �������
//            TesterT(int ThreadCount);   
//
//            //������� ��� ������������, ������� ���������� ���������� �� ���� ��������� �������
//            //�� ��� ��� ���� ��� �� ����� false ��� ������������ �� ��������� ���������� �������
//            //ThreadNum - ����� �������� ������
//            //pShutdown - ���� �������� �� ����� ��������� ���������� ������ TRUE, �� ������������
//            //            ����� ���������� ������������ (���� �������� ����� TRUE, �� ��� ������������� �����������
//            //            false, �.�. ������� �� ����� ������ ���������� �� �����
//            bool Run( int ThreadNum, volatile LONG *pShutdown );
//         };  
///////////////////////////////////////////////////////////////////////////////
template< class TesterT >
class CThreadTestBase: public TesterT, public NonCopyable
{
   //���������� ���������� ������
   class TGlobalData: public ThreadsManagerStrategy::TGlobalDataBasic
   {
      CThreadTestBase *m_pParent;

   public:
      //��� ����� ������� ���������� �������������� 
      //warning C4355: 'this' : used in base member initializer list
      //explicit TGlobalData( CThreadTestBase *pParent ): m_pParent(pParent) {}

      TGlobalData(): m_pParent(0) {}
      void SetParent( CThreadTestBase *pParent ) { m_pParent = pParent; }

      void Run( int ThreadNum, ThreadsManagerStrategy::TThreadBasic::TShutdownPtr pShutdown )
      {
         APL_ASSERT_PTR(m_pParent);

         APL_TRY()
         {
            try
            {
               Console.Write( ThreadNum, " ������ �����" );

               while( !*pShutdown && static_cast<TesterT *>(m_pParent)->Run( ThreadNum, const_cast<volatile LONG *>(pShutdown) ) )
               {   
                  //������ ������ �� ����
               }

               Console.Write( ThreadNum, " ��������� �����" );
            }
            catch(...)
            {
               Console.Write( ThreadNum, " �������� ����������. ��������� �����" );
               m_pParent->m_ThreadsManager.Stop();
               throw;
            }
         }			
         APL_CATCH()
      }
   };

   //���������� ������
   class TThread: public ThreadsManagerStrategy::TThreadBasic
   {
      int m_ThreadNum;
   
   public:
      TThread( int ThreadNum, TGlobalData &GD ): m_ThreadNum(ThreadNum){}

      DWORD Run( TGlobalData &GD, TShutdownPtr pShutdown ) 
      {
         GD.Run(m_ThreadNum, pShutdown);
         return 0;
      }
   };

   typedef TThreadsManager<TThread, TGlobalData> TThreadsManager;

private:
   TThreadsManager m_ThreadsManager;
   int m_ThreadCount;

   friend class TGlobalData;

public:
   explicit CThreadTestBase( int ThreadCount ): TesterT(ThreadCount), m_ThreadCount(ThreadCount)
   {
      APL_ASSERT( m_ThreadCount > 0 );

      m_ThreadsManager.SetParent( this );
   }


   //��������� ������������� ����
   void ThreadTest()
   {
      for( int i = 0; i < m_ThreadCount; ++i )
         m_ThreadsManager.CreateThread( i );

      _getch();
      m_ThreadsManager.Stop();
      m_ThreadsManager.Wait();
   }
};

} //namespace NWLib 

#endif