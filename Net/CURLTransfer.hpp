#ifndef CURLTransfer_hpp
#define CURLTransfer_hpp

#include <curl/curl.h>
#include <limits>
#include "../stlauxiliary.hpp"
#include "../Timer.hpp"


namespace NWLib
{
///////////////////////////////////////////////////////////////////////////////
// ����� �������������� � ������������ ���������� ������ ��� ����������� 
// �������� �� ����, � � ����������� ������� ��
// �������������� ������� ���������� �������������, ��� ��� ���������������� 
// ����� ��������� ���
///////////////////////////////////////////////////////////////////////////////
class TTransferInit
{
public:
   TTransferInit() { curl_global_init(CURL_GLOBAL_ALL); }
   ~TTransferInit() { curl_global_cleanup(); }
};

///////////////////////////////////////////////////////////////////////////////
// ����� ��������������� ������������ ������ � ��������� ������ �� ����
// � ����� ����� ��� � ������������� �������� ������ ������������ ��� ������ 
// CallBack'��
// ���������� �������� ������������� ��������� �������: ������ ���������
// �������� ������� (�� ����� ������ ������������) �������� � ��������� 
// ��������� ������� ����� ������� ����� ��������� ����������� �� ���� 
// ���������� �������. ����� ������ ����� ��������� ���������:
//   interface TCallbackObject 
//   {
//      //������� ���������� ����� ����� �� ������� ��������� ������ ������
//      //pData - ��������� �� ������ ������
//      //Size - ���������� �������� ����
//      void OnReceive( const void *pData, size_t Size );
//      
//      //������� ���������� �� �������� ������ �� ������. ��� ������ �������� �
//      //pData �� ����� Size ���� � ������� ��� ��������� ���������� ���� �������
//      //��� ��������. ��� ���� ����� ��������� �� ��������� �������� ������ 
//      //(end-of-file) ���������� ������� 0
//      size_t OnSend( void *pData, size_t Size );
//   };
// ��� ���������� ������� ��������� �������� Perform() �� ��� ��� ���� �� 
// ���������� true. Perform() ��������� ������ �� ������ ������� ����� �������� 
// ��� ����� �� ������ � ����� ��������� ������ ������ ������ � �������� �����
// ������ TCallbackObject. 
//
// ��������� WaitStrategyT, ���������� ������� �� ����� � ������� Perform() 
// ���������� �������� ������� ��� ���:
// TTransferNoWait - Perform() �� ����������� � �������� ����������� 
// �����/��������� ������ � ����������� ���������� ��������� �����. ���� ����� 
// ������� ��� ����� ����� ����������� ���������� �������. ����������� ������ � 
// ����� ������ ���� ������������� ��������� ������������ �����
// TTransferWait   - � ������ Perform() ����� �������� ����������� ���������� 
// ����������� (��-��������� 500, �� ����� �������������� ������� 
// SetPerformTimeout())
///////////////////////////////////////////////////////////////////////////////
class TTransferNoWait
{
protected:
   int PerformWait( CURLM * ) { return 1; }
};
///////////////////////////////////////////////////////////////////////////////

class TTransferWait
{
public:
   //�������� ������� ����������� �������������
   static inline DWORD TimerInfinityVal() { return std::numeric_limits<DWORD>::max(); }

   //���������� ����� �������� �������� �������, ����� ���������� TimerInfinityVal(), 
   //��� ������������ ��������
   void SetPerformTimeout( DWORD dwMilliseconds );
    
protected:
   TTransferWait() { SetPerformTimeout(500); }
   int PerformWait( CURLM *hMulti );

private:
private:
   fd_set m_fdRead;
   fd_set m_fdWrite;
   fd_set m_fdExcep;
   TIMEVAL m_TimeOut;
   TIMEVAL *m_pTimeOut;
};
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
template< class WaitStrategyT >
class TTransfer: public WaitStrategyT, private NWLib::NonCopyable
{
public:
   TTransfer();
   ~TTransfer();

public: //��������� 
   //URL ������� � ������� ������ ��������������
   void SetURL( const std::string &Url )
   { 
      m_Url = Url; //The string must remain present until curl no longer needs it, as it doesn't copy the string.
      APL_CHECK( curl_easy_setopt(m_hEasy, CURLOPT_URL, m_Url.c_str()) == CURLE_OK ); 
   }

   //���������� PROXY
   void SetProxy( const std::string &Proxy )
   {
      m_Proxy = Proxy;
      APL_CHECK( curl_easy_setopt(m_hEasy, CURLOPT_PROXY, m_Proxy.c_str()) == CURLE_OK ); 
   }

   const std::string &GetUrl() const { return m_Url; }

   //�������� �������������� ���� � ��������� ������� POST, �������� "Content-type: text/xml"
   void AddFieldToPostHeader( const char *szData )
   {
      //�������� curl_slist_append ������� �������� ������ szData
      m_Headers = curl_slist_append(m_Headers, szData);
      APL_ASSERT_PTR(m_Headers);
   }

   //�������� ��� ��������� �����������, ������� ��� �������, ����� � stderr
   void SetVerbose( int flag )
   {
      APL_CHECK( curl_easy_setopt(m_hEasy, CURLOPT_VERBOSE, flag) == CURLE_OK );
   }

public: //�������� ��. ��������� � �������� ������
   //��������� ������� �� ���������� ����� � �������
   //CallbackObject - ����� ������� ����� �������� ����������� � ���, ��� 
   //��������� ������ ������ ���� �������
   //������ ���������� NWLib::TAplThrowException
   template < class TCallbackObject >
   void Get( TCallbackObject &CallbackObject );

   //��������� ������� �� ������ � �������
   //������� �� ������ ������������ ������ - SendDataSize ���� � ����� 
   //����������� ����� �������
   //CallbackObject - ����� ������� ����� �������� ����������� � ���, ��� 
   //���������� ��������� ��� ������� ��������� ������ ������.
   //������ ���������� NWLib::TAplThrowException
   template< class TCallbackObject >
   void Request( TCallbackObject &CallbackObject, size_t SendDataSize );

   //��������� ��������� ����� �������� ������������ �������.
   //�����: true - ������� ��� �� ���������, ���������� ��������� �����.
   //       false - ������� ���������, ������� ������ ���
   //������ ���������� NWLib::TAplThrowException
   bool Perform();

   //���������� ������������ �������� � ������/��� ����������/�������� ������
   //�������� 0 �������� ��, ��� ����������� ���������
   void SetMaxDownloadSpeed( size_t Value );
   void SetMaxUploadSpeed( size_t Value );

   //��������� ��������� ������� �������
   void AbortTask();

private:
   //����� ������� ����������� ����� CallBack'� CURL � ���� ������ 
   template< class TCallbackObject> struct TCalbackFunc;

private:
   //���������� ��� ��������� ������� ��������� � m_hMulti
   //������ ���������� NWLib::TAplThrowException
   void ProcessMessages();

   //������� ��� ��������� �� ������� m_hMulti
   void RemoveMessages();

   //������� ������ ���������� ����� ������� ���������� �������
   //����������� m_hMulti
   void BeginNewTask();

   //��������� �������� ������ Perform, ������������� �������� m_RunnigHandlers
   //������ ���������� NWLib::TAplThrowException
   void PerformImpl();

   //����������� ������ Callback ������� ��� ���������� ��������
   static size_t AbortReceiveCallback( void *ptr, size_t size, size_t nmemb, void *stream ) { return std::numeric_limits<size_t>::max(); }
   static size_t AbortSendCallback( void *ptr, size_t size, size_t nmemb, void *stream ) { return CURL_READFUNC_ABORT; }

private:
   CURL *m_hEasy;
   CURLM *m_hMulti;
   curl_slist *m_Headers; //�������������� ���� ��������� ��� POST 
   int m_RunnigHandlers;  //���������� �������� �������
   std::string m_Url;     //URL ��������������
   std::string m_Proxy;   //Proxy ��������������
};
///////////////////////////////////////////////////////////////////////////////

template< class WaitStrategyT >
template< class TCallbackObject >
struct TTransfer<WaitStrategyT>::TCalbackFunc
{
   static size_t ReceiveCallback( void *ptr, size_t size, size_t nmemb, void *stream )
   {
      APL_ASSERT_PTR(stream);
      APL_ASSERT_PTR(ptr);

      size_t Size = size * nmemb;
      static_cast<TCallbackObject *>(stream)->OnReceive( ptr, Size ); 

      return Size;
   }

   static size_t SendCallback( void *ptr, size_t size, size_t nmemb, void *stream )
   {
      APL_ASSERT_PTR(stream);
      APL_ASSERT_PTR(ptr);

      size_t Size = size * nmemb;
      return static_cast<TCallbackObject *>(stream)->OnSend( ptr, Size ); 
   }
};
///////////////////////////////////////////////////////////////////////////////

template< class WaitStrategyT >
template < class TCallbackObject >
void TTransfer<WaitStrategyT>::Get( TCallbackObject &CallbackObject )
{
   BeginNewTask();

   APL_CHECK( curl_easy_setopt(m_hEasy, CURLOPT_WRITEFUNCTION, TCalbackFunc<TCallbackObject>::ReceiveCallback ) == CURLE_OK );
   APL_CHECK( curl_easy_setopt(m_hEasy, CURLOPT_WRITEDATA, &CallbackObject) == CURLE_OK );
   APL_CHECK( curl_easy_setopt(m_hEasy, CURLOPT_HTTPGET, TRUE) == CURLE_OK );
   APL_CHECK( curl_easy_setopt(m_hEasy, CURLOPT_HTTPHEADER, NULL) == CURLE_OK );

   PerformImpl();
}
///////////////////////////////////////////////////////////////////////////////

template< class WaitStrategyT >
template < class TCallbackObject >
void TTransfer<WaitStrategyT>::Request( TCallbackObject &CallbackObject, size_t SendDataSize )
{
   BeginNewTask();

   APL_CHECK( curl_easy_setopt(m_hEasy, CURLOPT_WRITEFUNCTION, TCalbackFunc<TCallbackObject>::ReceiveCallback ) == CURLE_OK );
   APL_CHECK( curl_easy_setopt(m_hEasy, CURLOPT_WRITEDATA, &CallbackObject) == CURLE_OK );
   APL_CHECK( curl_easy_setopt(m_hEasy, CURLOPT_READFUNCTION, TCalbackFunc<TCallbackObject>::SendCallback ) == CURLE_OK );
   APL_CHECK( curl_easy_setopt(m_hEasy, CURLOPT_READDATA, &CallbackObject) == CURLE_OK );
   APL_CHECK( curl_easy_setopt(m_hEasy, CURLOPT_POST, TRUE) == CURLE_OK );
   APL_CHECK( curl_easy_setopt(m_hEasy, CURLOPT_POSTFIELDSIZE_LARGE, curl_off_t(SendDataSize) ) == CURLE_OK );
   APL_CHECK( curl_easy_setopt(m_hEasy, CURLOPT_HTTPHEADER, m_Headers) == CURLE_OK );

   PerformImpl();
}
///////////////////////////////////////////////////////////////////////////////

template< class WaitStrategyT >
TTransfer<WaitStrategyT>::TTransfer(): m_RunnigHandlers(0), m_Headers(NULL)
{
   m_hMulti = curl_multi_init();
   m_hEasy = curl_easy_init();

   APL_ASSERT_PTR(m_hMulti);
   APL_ASSERT_PTR(m_hEasy);

   APL_CHECK( curl_multi_add_handle(m_hMulti, m_hEasy) == CURLE_OK );
   APL_CHECK( curl_easy_setopt(m_hEasy, CURLOPT_NOPROGRESS, TRUE) == CURLE_OK );
}
///////////////////////////////////////////////////////////////////////////////

template< class WaitStrategyT >
TTransfer<WaitStrategyT>::~TTransfer()
{
   curl_slist_free_all(m_Headers);
   APL_CHECK( curl_multi_remove_handle(m_hMulti, m_hEasy) == CURLE_OK );
   curl_easy_cleanup(m_hEasy);  //����� ��������� Multi ���� ������� ��� Easy
   APL_CHECK( curl_multi_cleanup(m_hMulti) == CURLE_OK );
}   
///////////////////////////////////////////////////////////////////////////////

template< class WaitStrategyT >
void TTransfer<WaitStrategyT>::ProcessMessages()
{
   int  MsgInQueue;
   CURLMsg *pCurlMsg;

   //������������� ��������� � ��������� ������
   while ( (pCurlMsg = curl_multi_info_read(m_hMulti, &MsgInQueue)) != NULL ) 
      if (pCurlMsg->msg == CURLMSG_DONE)
      {
         if( pCurlMsg->data.result != CURLE_OK )
         {
            AbortTask();
            APL_THROW( _T("Transfer error: ") << NWLib::ConvertToTStr(curl_easy_strerror(pCurlMsg->data.result)) << 
               _T(" URL: ") << NWLib::ConvertToTStr(m_Url) );
         }
         else
         {
            long ServerRezult;

            APL_CHECK( curl_easy_getinfo(pCurlMsg->easy_handle, CURLINFO_RESPONSE_CODE, &ServerRezult) == CURLE_OK );

            if( ServerRezult != 200 )
            {
               AbortTask();
               APL_THROW( _T("Transfer error: server return error code ") << ServerRezult <<
                  _T(" URL: ") << NWLib::ConvertToTStr(m_Url) );
            }
         }
      }
}
///////////////////////////////////////////////////////////////////////////////

template< class WaitStrategyT >
void TTransfer<WaitStrategyT>::BeginNewTask()
{
   //����� �������� curl_multi_remove_handle � ProcessMessages, � ����� �� �������� ������,
   //�� �� ���� ��� ��� ����� ������ � ��� ����� ��������� � ProcessMessages � ������� EasyHandle
   //�������� ��� �������� ��� ������ � curl_multi_perform
   //������� � ������ ������ �� ����� ������������ ���������� ������� ����������� EasyHandle � MultiHandle
   APL_CHECK( curl_multi_remove_handle(m_hMulti, m_hEasy) == CURLE_OK );
   APL_CHECK( curl_multi_add_handle(m_hMulti, m_hEasy) == CURLE_OK );

   RemoveMessages();
}
///////////////////////////////////////////////////////////////////////////////

template< class WaitStrategyT >
void TTransfer<WaitStrategyT>::AbortTask()
{
   //������������ ������ �������� �������� ��� �������� ������, ��� ��������� Callback'� :(
   APL_CHECK( curl_easy_setopt(m_hEasy, CURLOPT_WRITEFUNCTION, AbortReceiveCallback ) == CURLE_OK );
   APL_CHECK( curl_easy_setopt(m_hEasy, CURLOPT_READFUNCTION, AbortSendCallback ) == CURLE_OK );
   //���� ��� CURLOPT_PROGRESSFUNCTION, ��
   //Usage of the CURLOPT_PROGRESSFUNCTION callback is not recommended when using the multi interface.

   APL_DEBUG_VARIABLE( NWLib::TTimer CycleTime );

   do 
   {
      curl_multi_perform( m_hMulti, &m_RunnigHandlers );
      APL_DEBUG_OPERATION( CycleTime.Stop() );
      APL_ASSERT( ("������� �������� �������� ������ ������ ������� ����� �������", CycleTime.InSec() < 1.5) );
      APL_DEBUG_OPERATION( CycleTime.Resume() );
   } 
   while( m_RunnigHandlers );

   RemoveMessages();
}
///////////////////////////////////////////////////////////////////////////////

template< class WaitStrategyT >
void TTransfer<WaitStrategyT>::RemoveMessages()
{
   int MsgInQueue;

   while ( curl_multi_info_read(m_hMulti, &MsgInQueue) != NULL ) { /*������*/ }
}
///////////////////////////////////////////////////////////////////////////////

template< class WaitStrategyT >
void TTransfer<WaitStrategyT>::PerformImpl()
{
   CURLMcode CurlRez;

   while( (CurlRez = curl_multi_perform(m_hMulti, &m_RunnigHandlers)) == CURLM_CALL_MULTI_PERFORM ) { /*������*/ }

   if( CurlRez != CURLM_OK )
   {
      AbortTask();
      APL_THROW( _T("curl_multi_perform error: ") << NWLib::ConvertToTStr(curl_multi_strerror(CurlRez)) );
   }

   ProcessMessages();
}
///////////////////////////////////////////////////////////////////////////////

template< class WaitStrategyT >
bool TTransfer<WaitStrategyT>::Perform()
{
   if( !m_RunnigHandlers )
      return false;

   if( WaitStrategyT::PerformWait(m_hMulti) == SOCKET_ERROR )
   {
      AbortTask();
      APL_THROW( _T("select from socket error: ") << WSAGetLastError() );
   }

   //���� PerformWait ���������� 0 (������������� select ���������� 0)
   //�� �������� ������, �.�. ��� ������������� ����������� m_RunnigHandlers ������� 
   //�� ����� ������ 0 � �� ������� �� ������ �� ��������
   //return true; 
      
   PerformImpl();

   return m_RunnigHandlers != 0;
}
///////////////////////////////////////////////////////////////////////////////

template< class WaitStrategyT >
inline void TTransfer<WaitStrategyT>::SetMaxDownloadSpeed( size_t Value )
{
   curl_off_t Tmp(Value);
   APL_CHECK( curl_easy_setopt(m_hEasy, CURLOPT_MAX_RECV_SPEED_LARGE, Tmp) == CURLE_OK );
}
///////////////////////////////////////////////////////////////////////////////

template< class WaitStrategyT >
inline void TTransfer<WaitStrategyT>::SetMaxUploadSpeed( size_t Value )
{
   curl_off_t Tmp(Value);
   APL_CHECK( curl_easy_setopt(m_hEasy, CURLOPT_MAX_SEND_SPEED_LARGE, Tmp) == CURLE_OK );
}

///////////////////////////////////////////////////////////////////////////////
// class TTransferWait
///////////////////////////////////////////////////////////////////////////////
inline void TTransferWait::SetPerformTimeout( DWORD dwMilliseconds )
{
   if( dwMilliseconds == TimerInfinityVal() )
   {
      m_pTimeOut = NULL;
   }
   else
   {
      m_TimeOut.tv_sec = dwMilliseconds / 1000;
      m_TimeOut.tv_usec = dwMilliseconds % 1000 * 1000;
      m_pTimeOut = &m_TimeOut;
   }
}
///////////////////////////////////////////////////////////////////////////////

inline int TTransferWait::PerformWait( CURLM *hMulti )
{
   FD_ZERO(&m_fdRead);
   FD_ZERO(&m_fdWrite);
   FD_ZERO(&m_fdExcep);

   int MaxFD;

   APL_CHECK( curl_multi_fdset(hMulti, &m_fdRead, &m_fdWrite, &m_fdExcep, &MaxFD) == CURLE_OK );
   APL_ASSERT( MaxFD > -1 );

   return select( MaxFD + 1, &m_fdRead, &m_fdWrite, &m_fdExcep, &m_TimeOut );
}

} //namespace NWLib

#endif
