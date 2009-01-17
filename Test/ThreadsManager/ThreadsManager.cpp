// ThreadsManager.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "../../CoutConvert.h"
#include "../../ThreadsManager.hpp"
#include "../../StlAuxiliary.hpp"
#include "../../TestHelper.h"
#include <ctime>
#include <cstdlib>

class TThreadTestBaseTester
{
public:
   TThreadTestBaseTester(int ThreadCount) { }

   bool Run( int ThreadNum, volatile LONG *pShutdown )
   {
      int WorkTime = 5 + std::rand() *  (ThreadNum + 1) % 15;
      int ExceptionTime = 5 + std::rand() * (ThreadNum + 1) % 20;
      NWLib::Console.Write( ThreadNum, " WorkTime: ", WorkTime,  " ExceptionTime: ", ExceptionTime );

      while( WorkTime-- && !*pShutdown )
      {
         if( !ExceptionTime-- )
            APL_THROW( _T("Исключение в потоке: ") << ThreadNum );

         NWLib::Console.Write( ThreadNum, " раз: ",  WorkTime + 1 );
         Sleep(1000);
      }
      
      return false;
   }
};  


                  
int _tmain(int argc, _TCHAR* argv[])
{
   NWLib::TConsoleAutoStop CAS;
   try
   {
      std::srand( (unsigned)std::time( NULL ) );
      
      NWLib::TThreadsManager<> TM;

      TM.CreateThread( NWLib::NullType() );
      TM.Stop();
      TM.Wait();

      NWLib::CThreadTestBase<TThreadTestBaseTester> TTB( 10 );
      TTB.ThreadTest();
   }
   catch(...)
   {
      std::cout << "catch(...)" << std::endl;
   }

   return 0;
}

