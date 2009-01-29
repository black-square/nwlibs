//Автор: Шестеркин Дмитрий(NW) 2006

#ifndef ThreadsManager_HPP
#define ThreadsManager_HPP

#include "Threads.h"
#include <vector>

/**
   Модуль предоставляет классы управляющие созданием, выполнением и завершением потоков
*/ 

namespace NWLib {

///////////////////////////////////////////////////////////////////////////////
// Класс TThreadsManager управляет созданием, выполнением и завершением потоков
// С каждым потоком ассоциируется класс производный от ThreadsManagerStrategy::TThreadBasic
// который создаётся и уничтижается внутри TThreadsManager. ThreadsManagerStrategy::TThreadBasic
// инкапсулирует в функции Run действия которые необходимо выполнять в новом потоке. 
// Кроме того все потоки разделяют между собой одну область данных производную от 
// ThreadsManagerStrategy::TGlobalDataBasic которую могут использовать по своему усмотрению.
///////////////////////////////////////////////////////////////////////////////
namespace ThreadsManagerStrategy
{
   ///////////////////////////////////////////////////////////////////////////////
   // Базовый класс для стратегии разделяемой области памяти.
   // TThreadsManager открыто наследует переданный в параметре шаблона GlobalDataT класс
   // который должен являться наследником TGlobalDataBasic и затем передаёт ссылку на 
   // этот класс классу потока. 
   // Производный класс может заместить любой метод TGlobalDataBasic.
   // TThreadsManager не выполняет какую либо синхронизацию при доступе к GlobalDataT
   // пользователь сам должен заботится об этом.
   // Можно создать класс, который будет наследоваться от TThreadsManager, а в 
   // производном от TGlobalDataBasic приводить указатель this на такой класс.
   ///////////////////////////////////////////////////////////////////////////////
   class TGlobalDataBasic
   {
   public:   
      //Конструктор по умолчанию вызывается конструктором TThreadsManager без параметров
      TGlobalDataBasic() {}

      //В конструктор можно передать любой необходимой пользователю параметр по средством вызова
      //шаблонного конструктора TThreadsManager
      //Переопределяемый конструктор может не быть шаблонным если типы известны.
      template <class InitDataT>
      TGlobalDataBasic(const InitDataT &InitData) {}

      //Уведомление в том что был вызван метод TThreadsManager::Stop, и что потки должны остановиться.
      //Приходит сразу после того как переменная pShutdown в функции Run приняла значение TRUE.
      //Можно использовать, например, в том случае когда поток переведён в состояние ожидания, для того
      //чтобы перевести ожидаемый объект в свободное состояние. После этого поток проснётся, проверит 
      //состояние переменной pShutdown и спокойно завершится.
      void OnTryStopThreads() {}

      //Деструктор вызовется в деструкторе TThreadsManager
      ~TGlobalDataBasic() {}
   };
   
   ///////////////////////////////////////////////////////////////////////////////
   // Базовый класс потока.
   // TThreadsManager создаёт переданный в параметре шаблона ThreadT класс
   // который должен являться наследником TThreadBasic каждый раз при созданнии нового потока.
   // Производный от TThreadBasic класс должен выполнять необходимую для потока работу и после этого
   // TThreadsManager удаляет его.
   // Производный класс может заместить любой метод TThreadBasic.
   ///////////////////////////////////////////////////////////////////////////////
   class TThreadBasic
   {
   public:
      // Тип указателя на переменную которую меняет TThreadsManager в своём потоке.
      // Переменная пердсталяет собой флаг TRUE/FALSE и обозначает "Потоки должны завершить свою работу".
      // Т.е. функция потока может проверить дал ли пользователь команду завершения выполнения своей работу и
      // корректно завершиться
      typedef const volatile LONG *TShutdownPtr;

   protected:
      //Конструктор по умолчанию никогда не будет вызван, переопредилять его не имеет смысл
      TThreadBasic() {}

   public:
      //При создании потока в конструктор можно передать любой необходимой пользователю 
      //параметр по средством вызова TThreadsManager::CreateThread с параметром InitData. 
      //Так же в конструктор передаётся ссылка на производный от TGlobalDataBasic объект.
      //Переопределяемый конструктор может не быть шаблонным если типы известны.
      //Если нет необходимости передавать параметр в конструктор то можно в качестве параметра 
      //функции TThreadsManager::CreateThread передать например NullType
      //Конструктор вызывается в потоке в котором вызвался метод TThreadsManager::CreateThread и
      //может кидать исключения.
      template <class InitDataT>
      TThreadBasic(const InitDataT &InitData, TGlobalDataBasic &GlobalDataBasic) {}

      //Функция потока
      //Основное что необходимо переопредилять пользователю. Функция вызывается
      //в новом потоке и после того как она вернёт управление поток завершается.
      //Получает ссылку на производный от TGlobalDataBasic объект. И на флаг
      //завершения(подробнее см выше)
      //При успешном завершение должна возвращать 0
      //Функция не должна кидать исключения
      DWORD Run( TGlobalDataBasic &GlobalDataBasic, TShutdownPtr pShutdown ) 
      {
         return 0;
      }

      //Деструктор объекта вызывается в созданном потоке сразу после того как
      //функция Run вернула управление и не может кидать исключения
      ~TThreadBasic()
      {

      }
   };
}
///////////////////////////////////////////////////////////////////////////////

template<class ThreadT = ThreadsManagerStrategy::TThreadBasic, class GlobalDataT = ThreadsManagerStrategy::TGlobalDataBasic>
class TThreadsManager: private NonCopyable, public GlobalDataT
{
   //Список открытых хендлов потока
   //Создаваемы потоки никак не обращаются к этому списку, за корректным освобождением хендлов следит
   //сам TThreadsManager в основном потоке
   //Нет необходимости вводить мьютекс для защиты этого списка т.к. он используется только в одном потоке
   typedef std::vector<HANDLE> TThreadsHandles;

   class TThreadDataList;

   //Внутренняя информация по потоку. 
   //Структуры выстраиваются в связанный список.
   class TThreadData: NonCopyable
   {  
   private:
      ThreadT m_Thread;
      TThreadData *m_pNext;
      TThreadData *m_pPrev;
     
      //Поскольку в функцию потока мы можем передать только один указатель и любая стековая переменная
      //может быть разрушена к моменту обращения к ней потока, необходимо передавать указатель на m_ThreadsManager
      //в динамической памяти. А здесь это сделать наиболее удобно.
      TThreadsManager &m_ThreadsManager;

   private:
      TThreadData();
      friend class TThreadDataList;

   public:    
      template <class InitDataT>
      explicit TThreadData( const InitDataT &InitData, TThreadsManager &ThreadsManager ): 
         m_Thread(InitData, static_cast<GlobalDataT &>(ThreadsManager)), m_ThreadsManager(ThreadsManager)
      {} 

      //Ссылка на TThreadsManager
      TThreadsManager &GetManager() { return m_ThreadsManager; }
      ThreadT &GetThread() { return m_Thread; }
   };

   //Связанный список объектов TThreadData. Владеет объектами TThreadData (создаёт и удаляет)
   //Потокобезопасен                                                              
   class TThreadDataList: NonCopyable
   {
      //Мьютьекс защищающий список объектов TThreadData от одновременного изменения в разных потоках
      typedef ObjectLevelLockable<TThreadDataList> TMutex;

   private:  
      TThreadData *m_pFirstThreadData;
      TMutex m_Mutex;
   
   public:
      typedef TThreadData *TPointer;
      typedef const TThreadData *TConstPointer;

   private:                      
      //Реализация Remove без блокирования списка
      void RemoveNonBlocked( TPointer I )
      {
         APL_ASSERT_PTR( I );           

         if( I->m_pNext != 0 )
            I->m_pNext->m_pPrev = I->m_pPrev;

         if( I->m_pPrev != 0 )
            I->m_pPrev->m_pNext = I->m_pNext;
         else
         {
            APL_ASSERT( I == m_pFirstThreadData );
            m_pFirstThreadData = I->m_pNext;
         }

         delete I;
      }


   public:
      TThreadDataList(): m_pFirstThreadData(0) {}
      ~TThreadDataList()
      {
         TMutex::Lock Guard( m_Mutex );

         while( m_pFirstThreadData != 0 )
            RemoveNonBlocked( m_pFirstThreadData );
      }

      //Пустой ли список
      bool IsEmpty() const { return m_pFirstThreadData == 0; }

      //Создать новый ThreadData и добавить его в списк
      template <class InitDataT>
      TPointer AddToList( const InitDataT &InitData, TThreadsManager &ThreadsManager )
      {
         TThreadData *pNew = new TThreadData( InitData, ThreadsManager ); 
         TMutex::Lock Guard( m_Mutex );

         if( m_pFirstThreadData != 0 )
            m_pFirstThreadData->m_pPrev = pNew;
         
         pNew->m_pNext = m_pFirstThreadData; 
         pNew->m_pPrev = 0;
         m_pFirstThreadData = pNew;

         return pNew;
      }
      ///////////////////////////////////////////////////////////////////////////////

      //Удалить из списка
      void Remove( TPointer I )
      {
         APL_ASSERT_PTR( I );
         TMutex::Lock Guard( m_Mutex );

         RemoveNonBlocked(I);
      }
   };


private:
   TThreadsHandles m_ThreadsHandles;
   TThreadDataList m_ThreadDataList;
   volatile LONG m_Shutdown; //Признак того что необходимо всем потокам корректно завершится

private:
  
   //Точка входа в поток
   static DWORD WINAPI ThreadFunction(PVOID pvParam)
   {
      APL_ASSERT_PTR(pvParam);
      
      //Автоматически освобождает данные потока при выходе из функции
      struct THolder: NonCopyable
      {
         TThreadDataList::TPointer pThreadData;

         THolder(TThreadDataList::TPointer pTD): pThreadData(pTD)
         {
            APL_ASSERT_PTR(pTD);
         }

         ~THolder()
         {
            pThreadData->GetManager().m_ThreadDataList.Remove(pThreadData);
         }
      } Holder( static_cast<TThreadDataList::TPointer>(pvParam) );

      //Вызываем функцию потока
      return Holder.pThreadData->GetThread().Run( static_cast<GlobalDataT &>(Holder.pThreadData->GetManager()), &Holder.pThreadData->GetManager().m_Shutdown );
   }

public:
   TThreadsManager(): m_Shutdown(FALSE) {}

   //Конструктов позволяет передавать произвольный инициализационный параметр конструктору GlobalDataT
   template< class InitDataT >
   TThreadsManager(const InitDataT &InitData ): GlobalDataT(InitData), m_Shutdown(FALSE) {}

   //В деструкторе устанавливаем флаг флаг "Потоки должны завершить свою работу" и ждём их завершения
   ~TThreadsManager()
   {
      Stop();
      Wait();
   }

   //Создание нового потока.
   //Создатся новый объект ThreadT в текущем потоке, которому в конструкторе передаётся 
   //InitData и GlobalDataT и после этого создаётся новый поток и в нём вызвается ThreadT::Run.
   //После выхода из функции Run объект ThreadT уничтожается и вызывается его деструктор 
   //в новом потоке потоке. После этого новый поток уничтожается.
   //Может быть вызвана только в основном потоке
   template< class InitDataT >
   void CreateThread( const InitDataT &InitData )
   {
      if( m_Shutdown == TRUE ) //Потоки завершились или завершаются
      {  
         Wait(); //Ждём пока завершатся все потоки
         
         APL_ASSERT( m_ThreadsHandles.empty() );
         APL_ASSERT( m_ThreadDataList.IsEmpty() );

         InterlockedExchange(&m_Shutdown, FALSE);
      }

      //Добавляем данные по потоку
      TThreadDataList::TPointer pThreadData = m_ThreadDataList.AddToList( InitData, *this );
      DWORD dwThreadID;

      HANDLE hNewThread = chBEGINTHREADEX(NULL, 0, &ThreadFunction, pThreadData, 0, &dwThreadID); 

      if( hNewThread == NULL || hNewThread == INVALID_HANDLE_VALUE || hNewThread == (HANDLE)1L )
         APL_THROW( _T("Ошибка при созадании потока") );

      //Попытаемся сначала найти в m_ThreadDataMutex хендл потока который завершил свою работу и тогда 
      //на его место можно поместить только что созданный хендл
      TThreadsHandles::iterator I;
      DWORD ExitCode;

      for( I = m_ThreadsHandles.begin(); I != m_ThreadsHandles.end(); ++I )
      {
         APL_CHECK( GetExitCodeThread(*I, &ExitCode) );

         if( ExitCode != STILL_ACTIVE )
            break;
      }

      if( I != m_ThreadsHandles.end() )
      {
         //Нашли уже остановленный процесс
         APL_ASSERT( WaitForSingleObject(*I, 0) == WAIT_OBJECT_0 );
         APL_CHECK( CloseHandle(*I) );
         *I = hNewThread;
      }
      else
      {
         APL_ASSERT( m_ThreadsHandles.size() < MAXIMUM_WAIT_OBJECTS );

         //Все потоки находятся в активном состоянии
         m_ThreadsHandles.push_back(hNewThread);
      }
   }

   //Включить флаг "Потоки должны завершить свою работу"
   //После этого потоки самостоятельно должны его проанализировать и завершить свою работу 
   //Функция не дожидается завершения потоков и сразу возвращает управление 
   //Может быть вызвана в любом потоке
   void Stop()
   {
      InterlockedExchange(&m_Shutdown, TRUE);
      GlobalDataT::OnTryStopThreads();
   }

   //Подождать завершения всех потоков в течении времени dwMilliseconds.
   //Функция не устанавливает флаг "Потоки должны завершить свою работу" и потоки не узнают
   //о том что ожидается их завершение.
   //Для бесконечного ожидания необходимо передать в качестве параметра 
   //dwMilliseconds константу INFINITE
   //Возвр: true  - Все потоки корректно завершились
   //       false - Время ожидание истекло, не все потоки завершились. 
   //               (Только в случае dwMilliseconds != INFINITE)
   //Может быть вызвана только в основном потоке
   bool Wait( DWORD dwMilliseconds = INFINITE )
   {
      if( m_ThreadsHandles.empty() )
         return true;

      //TODO: Заменить вызыов WaitForMultipleObjects, на WaitForSingleObject в цикле для устранения зависимости от MAXIMUM_WAIT_OBJECTS
      DWORD Result = WaitForMultipleObjects( (DWORD)m_ThreadsHandles.size(), &m_ThreadsHandles[0], TRUE, dwMilliseconds );
      APL_ASSERT( Result != WAIT_FAILED );
      
      if( Result == WAIT_TIMEOUT )
         return false;
      
      APL_ASSERT( m_ThreadDataList.IsEmpty() );

      for( TThreadsHandles::iterator I = m_ThreadsHandles.begin(); I != m_ThreadsHandles.end(); ++I )
         APL_CHECK( CloseHandle(*I) );

      m_ThreadsHandles.clear();

      return true;
   }

   //Проверить состояние флага "Потоки должны завершить свою работу"
   bool IsShutdown() const { return m_Shutdown == TRUE; }
};

} //namespace NWLib 

#endif