//Автор: Шестеркин Дмитрий(NW) 2005

/** 
   Модуль предоставляет несколько функций упрощающих работу с консолью
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


//Просто строка для отделения различных частей
#define APL_LINE "-------------------------------------------------------------------------------\n"

//Если arg ложно выдаёт в консоль информацию и кидает TSilentException
#define APL_ERROR( arg )do {if( !(arg) ){ \
	std::stringstream Stream__; \
   Stream__ << APL_LINE;  \
	Stream__ << "   APL_ERROR( " ## #arg ## " )\n"; \
	Stream__ << "   " ## __FILE__ ## ", " << __LINE__ << std::endl;  \
	Stream__ << APL_LINE;  \
   std::cout << Stream__.str(); \
   throw NWLib::TSilentException(); \
} }while(false)

//Тоже что и APL_ERROR, но не кидает исключение
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

//Позволяет останавливать программу перед выходом из неё и просить пользователя нажать клавишу
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
// запустить комманду SHELL или файл на выполнение при этом весь вывод запущенной
// комманды осуществляется в текущую консоль
// Command - Комманда для запуска, для того чтобы выполнить комманду shell
//           нужно использовать синтаксис "cmd /c <имя комманды>", например "cmd /c dir /b"
// CurrentDirectory - Текущая директория у запущеснной комманды, ели NULL, то
//                    используется директория которая в данный момент является текущей
// ВОЗВР - Код который вернула комманда при выходе
///////////////////////////////////////////////////////////////////////////////
DWORD RunCommand( LPTSTR Command, LPCTSTR CurrentDirectory = NULL );

///////////////////////////////////////////////////////////////////////////////
// Запустить файл FilePath на выполнение и если FilePath выполняемый файл
// дождаться конца его выполнения. Функция устанавливает для запускаемого файла, 
// в качестве текущего каталога, каталог в котором находится файл FilePath и 
// передаёт как первый аргумент строку DEBUG, если подключен макрос DEBUG и 
// RELEASE если не подключён.
// Проверяет возвращаемое значение файла (для bat файла коммандой exit <код>)
// и если оно != 0 то кидаем исключение
///////////////////////////////////////////////////////////////////////////////
void ExecutePreRunFile( const TCHAR *FilePath );

///////////////////////////////////////////////////////////////////////////////
// Скопировать необходимую dll для LoadManager'a
///////////////////////////////////////////////////////////////////////////////
void CopyLoadManager();

///////////////////////////////////////////////////////////////////////////////
/**
	Несколько макросов для удобного создания Юнит-тестов.
	В обёртку APL_TRY APL_CATCH должен быть положен весь код теста
	Она перехватывпает основные исключения и выводить информацию о них на консоль.
	Так-же предотвращает передачц исключения TSilentException дальше.
	APL_TRY()
	{
	   //Код Юнит-теста
	}
	APL_CATCH()


*/
//Искючение которое должно кидать только для выхода из блока.
//информация о нём не должна поступать пользователю
class TSilentException: public std::exception  
{ 
public: 
	TSilentException():	std::exception("SilentException") {}
};  

//Позволяет иснхронизировать вывод на консоль
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
// Класс предоставляет удобный механизм для многопоточного тестирования (или просто 
// выполнения, некоторой тестовой задачи в отдельном потоке
// TesterT - Стратегия, которая наследуется открыто, со следующем интерфейсом
//         interface TesterT
//         {
//            //Конструктор принимающий как параметр количество создаваемых потоков
//            TesterT(int ThreadCount);   
//
//            //Функция для тестирования, которая вызывается циклически во всех созданных потоках
//            //до тех пор пока она не вернёт false или пользователь не остановит выполнение потоков
//            //ThreadNum - Номер текущего потока
//            //pShutdown - Если значение по этому указателю становится равным TRUE, то пользователь
//            //            хочет остановить тестирование (Если значение равно TRUE, то нет необходимости возвращаеть
//            //            false, т.к. функция всё равно больше вызываться не будет
//            bool Run( int ThreadNum, volatile LONG *pShutdown );
//         };  
///////////////////////////////////////////////////////////////////////////////
template< class TesterT >
class CThreadTestBase: public TesterT, public NonCopyable
{
   //Реализация Глобальных данных
   class TGlobalData: public ThreadsManagerStrategy::TGlobalDataBasic
   {
      CThreadTestBase *m_pParent;

   public:
      //При таком подходе появляется предупреждение 
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
               Console.Write( ThreadNum, " Создаём поток" );

               while( !*pShutdown && static_cast<TesterT *>(m_pParent)->Run( ThreadNum, const_cast<volatile LONG *>(pShutdown) ) )
               {   
                  //Ничего делать не надо
               }

               Console.Write( ThreadNum, " Завершаем поток" );
            }
            catch(...)
            {
               Console.Write( ThreadNum, " ВОЗНИКЛО ИСКЛЮЧЕНИЕ. ЗАВЕРШАЕМ ПОТОК" );
               m_pParent->m_ThreadsManager.Stop();
               throw;
            }
         }			
         APL_CATCH()
      }
   };

   //Реализация Потока
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


   //Запустить многопоточный тест
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