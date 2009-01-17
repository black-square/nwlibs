//�����: ��������� �������(NW) 2006

#ifndef ThreadsManager_HPP
#define ThreadsManager_HPP

#include "Threads.h"
#include <vector>

/**
   ������ ������������� ������ ����������� ���������, ����������� � ����������� �������
*/ 

namespace NWLib {

///////////////////////////////////////////////////////////////////////////////
// ����� TThreadsManager ��������� ���������, ����������� � ����������� �������
// � ������ ������� ������������� ����� ����������� �� ThreadsManagerStrategy::TThreadBasic
// ������� �������� � ������������ ������ TThreadsManager. ThreadsManagerStrategy::TThreadBasic
// ������������� � ������� Run �������� ������� ���������� ��������� � ����� ������. 
// ����� ���� ��� ������ ��������� ����� ����� ���� ������� ������ ����������� �� 
// ThreadsManagerStrategy::TGlobalDataBasic ������� ����� ������������ �� ������ ����������.
///////////////////////////////////////////////////////////////////////////////
namespace ThreadsManagerStrategy
{
   ///////////////////////////////////////////////////////////////////////////////
   // ������� ����� ��� ��������� ����������� ������� ������.
   // TThreadsManager ������� ��������� ���������� � ��������� ������� GlobalDataT �����
   // ������� ������ �������� ����������� TGlobalDataBasic � ����� ������� ������ �� 
   // ���� ����� ������ ������. 
   // ����������� ����� ����� ��������� ����� ����� TGlobalDataBasic.
   // TThreadsManager �� ��������� ����� ���� ������������� ��� ������� � GlobalDataT
   // ������������ ��� ������ ��������� �� ����.
   // ����� ������� �����, ������� ����� ������������� �� TThreadsManager, � � 
   // ����������� �� TGlobalDataBasic ��������� ��������� this �� ����� �����.
   ///////////////////////////////////////////////////////////////////////////////
   class TGlobalDataBasic
   {
   public:   
      //����������� �� ��������� ���������� ������������� TThreadsManager ��� ����������
      TGlobalDataBasic() {}

      //� ����������� ����� �������� ����� ����������� ������������ �������� �� ��������� ������
      //���������� ������������ TThreadsManager
      //���������������� ����������� ����� �� ���� ��������� ���� ���� ��������.
      template <class InitDataT>
      TGlobalDataBasic(const InitDataT &InitData) {}

      //����������� � ��� ��� ��� ������ ����� TThreadsManager::Stop, � ��� ����� ������ ������������.
      //�������� ����� ����� ���� ��� ���������� pShutdown � ������� Run ������� �������� TRUE.
      //����� ������������, ��������, � ��� ������ ����� ����� �������� � ��������� ��������, ��� ����
      //����� ��������� ��������� ������ � ��������� ���������. ����� ����� ����� ��������, �������� 
      //��������� ���������� pShutdown � �������� ����������.
      void OnTryStopThreads() {}

      //���������� ��������� � ����������� TThreadsManager
      ~TGlobalDataBasic() {}
   };
   
   ///////////////////////////////////////////////////////////////////////////////
   // ������� ����� ������.
   // TThreadsManager ������ ���������� � ��������� ������� ThreadT �����
   // ������� ������ �������� ����������� TThreadBasic ������ ��� ��� ��������� ������ ������.
   // ����������� �� TThreadBasic ����� ������ ��������� ����������� ��� ������ ������ � ����� �����
   // TThreadsManager ������� ���.
   // ����������� ����� ����� ��������� ����� ����� TThreadBasic.
   ///////////////////////////////////////////////////////////////////////////////
   class TThreadBasic
   {
   public:
      // ��� ��������� �� ���������� ������� ������ TThreadsManager � ���� ������.
      // ���������� ����������� ����� ���� TRUE/FALSE � ���������� "������ ������ ��������� ���� ������".
      // �.�. ������� ������ ����� ��������� ��� �� ������������ ������� ���������� ���������� ����� ������ �
      // ��������� �����������
      typedef const volatile LONG *TShutdownPtr;

   protected:
      //����������� �� ��������� ������� �� ����� ������, �������������� ��� �� ����� �����
      TThreadBasic() {}

   public:
      //��� �������� ������ � ����������� ����� �������� ����� ����������� ������������ 
      //�������� �� ��������� ������ TThreadsManager::CreateThread � ���������� InitData. 
      //��� �� � ����������� ��������� ������ �� ����������� �� TGlobalDataBasic ������.
      //���������������� ����������� ����� �� ���� ��������� ���� ���� ��������.
      //���� ��� ������������� ���������� �������� � ����������� �� ����� � �������� ��������� 
      //������� TThreadsManager::CreateThread �������� �������� NullType
      //����������� ���������� � ������ � ������� �������� ����� TThreadsManager::CreateThread �
      //����� ������ ����������.
      template <class InitDataT>
      TThreadBasic(const InitDataT &InitData, TGlobalDataBasic &GlobalDataBasic) {}

      //������� ������
      //�������� ��� ���������� �������������� ������������. ������� ����������
      //� ����� ������ � ����� ���� ��� ��� ����� ���������� ����� �����������.
      //�������� ������ �� ����������� �� TGlobalDataBasic ������. � �� ����
      //����������(��������� �� ����)
      //��� �������� ���������� ������ ���������� 0
      //������� �� ������ ������ ����������
      DWORD Run( TGlobalDataBasic &GlobalDataBasic, TShutdownPtr pShutdown ) 
      {
         return 0;
      }

      //���������� ������� ���������� � ��������� ������ ����� ����� ���� ���
      //������� Run ������� ���������� � �� ����� ������ ����������
      ~TThreadBasic()
      {

      }
   };
}
///////////////////////////////////////////////////////////////////////////////

template<class ThreadT = ThreadsManagerStrategy::TThreadBasic, class GlobalDataT = ThreadsManagerStrategy::TGlobalDataBasic>
class TThreadsManager: private NonCopyable, public GlobalDataT
{
   //������ �������� ������� ������
   //���������� ������ ����� �� ���������� � ����� ������, �� ���������� ������������� ������� ������
   //��� TThreadsManager � �������� ������
   //��� ������������� ������� ������� ��� ������ ����� ������ �.�. �� ������������ ������ � ����� ������
   typedef std::vector<HANDLE> TThreadsHandles;

   class TThreadDataList;

   //���������� ���������� �� ������. 
   //��������� ������������� � ��������� ������.
   class TThreadData: NonCopyable
   {  
   private:
      ThreadT m_Thread;
      TThreadData *m_pNext;
      TThreadData *m_pPrev;
     
      //��������� � ������� ������ �� ����� �������� ������ ���� ��������� � ����� �������� ����������
      //����� ���� ��������� � ������� ��������� � ��� ������, ���������� ���������� ��������� �� m_ThreadsManager
      //� ������������ ������. � ����� ��� ������� �������� ������.
      TThreadsManager &m_ThreadsManager;

   private:
      TThreadData();
      friend class TThreadDataList;

   public:    
      template <class InitDataT>
      explicit TThreadData( const InitDataT &InitData, TThreadsManager &ThreadsManager ): 
         m_Thread(InitData, static_cast<GlobalDataT &>(ThreadsManager)), m_ThreadsManager(ThreadsManager)
      {} 

      //������ �� TThreadsManager
      TThreadsManager &GetManager() { return m_ThreadsManager; }
      ThreadT &GetThread() { return m_Thread; }
   };

   //��������� ������ �������� TThreadData. ������� ��������� TThreadData (������ � �������)
   //���������������                                                              
   class TThreadDataList: NonCopyable
   {
      //�������� ���������� ������ �������� TThreadData �� �������������� ��������� � ������ �������
      typedef ObjectLevelLockable<TThreadDataList> TMutex;

   private:  
      TThreadData *m_pFirstThreadData;
      TMutex m_Mutex;
   
   public:
      typedef TThreadData *TPointer;
      typedef const TThreadData *TConstPointer;

   private:                      
      //���������� Remove ��� ������������ ������
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

      //������ �� ������
      bool IsEmpty() const { return m_pFirstThreadData == 0; }

      //������� ����� ThreadData � �������� ��� � �����
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

      //������� �� ������
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
   volatile LONG m_Shutdown; //������� ���� ��� ���������� ���� ������� ��������� ����������

private:
  
   //����� ����� � �����
   static DWORD WINAPI ThreadFunction(PVOID pvParam)
   {
      APL_ASSERT_PTR(pvParam);
      
      //������������� ����������� ������ ������ ��� ������ �� �������
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

      //�������� ������� ������
      return Holder.pThreadData->GetThread().Run( static_cast<GlobalDataT &>(Holder.pThreadData->GetManager()), &Holder.pThreadData->GetManager().m_Shutdown );
   }

public:
   TThreadsManager(): m_Shutdown(FALSE) {}

   //����������� ��������� ���������� ������������ ����������������� �������� ������������ GlobalDataT
   template< class InitDataT >
   TThreadsManager(const InitDataT &InitData ): GlobalDataT(InitData), m_Shutdown(FALSE) {}

   //� ����������� ������������� ���� ���� "������ ������ ��������� ���� ������" � ��� �� ����������
   ~TThreadsManager()
   {
      Stop();
      Wait();
   }

   //�������� ������ ������.
   //�������� ����� ������ ThreadT � ������� ������, �������� � ������������ ��������� 
   //InitData � GlobalDataT � ����� ����� �������� ����� ����� � � �� ��������� ThreadT::Run.
   //����� ������ �� ������� Run ������ ThreadT ������������ � ���������� ��� ���������� 
   //� ����� ������ ������. ����� ����� ����� ����� ������������.
   //����� ���� ������� ������ � �������� ������
   template< class InitDataT >
   void CreateThread( const InitDataT &InitData )
   {
      if( m_Shutdown == TRUE ) //������ ����������� ��� �����������
      {  
         Wait(); //��� ���� ���������� ��� ������
         
         APL_ASSERT( m_ThreadsHandles.empty() );
         APL_ASSERT( m_ThreadDataList.IsEmpty() );

         InterlockedExchange(&m_Shutdown, FALSE);
      }

      //��������� ������ �� ������
      TThreadDataList::TPointer pThreadData = m_ThreadDataList.AddToList( InitData, *this );
      DWORD dwThreadID;

      HANDLE hNewThread = chBEGINTHREADEX(NULL, 0, &ThreadFunction, pThreadData, 0, &dwThreadID); 

      if( hNewThread == NULL || hNewThread == INVALID_HANDLE_VALUE || hNewThread == (HANDLE)1L )
         APL_THROW( _T("������ ��� ��������� ������") );

      //���������� ������� ����� � m_ThreadDataMutex ����� ������ ������� �������� ���� ������ � ����� 
      //�� ��� ����� ����� ��������� ������ ��� ��������� �����
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
         //����� ��� ������������� �������
         APL_ASSERT( WaitForSingleObject(*I, 0) == WAIT_OBJECT_0 );
         APL_CHECK( CloseHandle(*I) );
         *I = hNewThread;
      }
      else
      {
         APL_ASSERT( m_ThreadsHandles.size() < MAXIMUM_WAIT_OBJECTS );

         //��� ������ ��������� � �������� ���������
         m_ThreadsHandles.push_back(hNewThread);
      }
   }

   //�������� ���� "������ ������ ��������� ���� ������"
   //����� ����� ������ �������������� ������ ��� ���������������� � ��������� ���� ������ 
   //������� �� ���������� ���������� ������� � ����� ���������� ���������� 
   //����� ���� ������� � ����� ������
   void Stop()
   {
      InterlockedExchange(&m_Shutdown, TRUE);
      GlobalDataT::OnTryStopThreads();
   }

   //��������� ���������� ���� ������� � ������� ������� dwMilliseconds.
   //������� �� ������������� ���� "������ ������ ��������� ���� ������" � ������ �� ������
   //� ��� ��� ��������� �� ����������.
   //��� ������������ �������� ���������� �������� � �������� ��������� 
   //dwMilliseconds ��������� INFINITE
   //�����: true  - ��� ������ ��������� �����������
   //       false - ����� �������� �������, �� ��� ������ �����������. 
   //               (������ � ������ dwMilliseconds != INFINITE)
   //����� ���� ������� ������ � �������� ������
   bool Wait( DWORD dwMilliseconds = INFINITE )
   {
      if( m_ThreadsHandles.empty() )
         return true;

      //TODO: �������� ������ WaitForMultipleObjects, �� WaitForSingleObject � ����� ��� ���������� ����������� �� MAXIMUM_WAIT_OBJECTS
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

   //��������� ��������� ����� "������ ������ ��������� ���� ������"
   bool IsShutdown() const { return m_Shutdown == TRUE; }
};

} //namespace NWLib 

#endif