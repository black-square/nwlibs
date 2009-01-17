#ifndef CURLTransfer_hpp
#define CURLTransfer_hpp

#include <curl/curl.h>
#include <limits>
#include "../stlauxiliary.hpp"
#include "../Timer.hpp"


namespace NWLib
{
///////////////////////////////////////////////////////////////////////////////
// Класс инициализирует в конструкторе внутренние данные для организации 
// передачи по сети, а в деструкторе удаляет их
// Поддерживается счётчик количества инициализаций, так что инициализировать 
// можно несколько раз
///////////////////////////////////////////////////////////////////////////////
class TTransferInit
{
public:
   TTransferInit() { curl_global_init(CURL_GLOBAL_ALL); }
   ~TTransferInit() { curl_global_cleanup(); }
};

///////////////////////////////////////////////////////////////////////////////
// Класс непосредственно занимающийся приёмом и передачей данный по сети
// О факте приёма или о необходимости передачи клиент уведомляется про помощи 
// CallBack'ов
// Выполнение операций организованно следующем образом: Клиент назначает
// некторое задание (не более одного одновременно) указывая в шаблонном 
// параметре функции класс который будет принимать уведомления по ходу 
// выполнения функции. Класс должен иметь следующий интерфейс:
//   interface TCallbackObject 
//   {
//      //Функция вызывается после приёма от сервера очередной порции данных
//      //pData - указатель на начало данных
//      //Size - количество принятых байт
//      void OnReceive( const void *pData, size_t Size );
//      
//      //Функция вызывается до отправки данных на сервер. Она должна записать в
//      //pData не более Size байт и вернуть как результат количество байт которое
//      //она записала. Для того чтобы уведомить об окончании передачи данных 
//      //(end-of-file) необходимо вернуть 0
//      size_t OnSend( void *pData, size_t Size );
//   };
// Для выполнение задания небходимо вызывать Perform() до тех пор пока он 
// возвращает true. Perform() проверяет готовы ли данные которые можно получить 
// или готов ли сервер к приёму очередной порции данных данных и вызывает соотв
// методы TCallbackObject. 
//
// Стратегия WaitStrategyT, определяет ожидает ли класс в функции Perform() 
// заполнения сетевого буффера или нет:
// TTransferNoWait - Perform() не блокируется в ожидании возможности 
// приёма/получения данных и выполняется минимально возможное время. Этот метод 
// заточен под вызов через определённые промежутки времени. Непрерывные вызовы в 
// цикле только лишь безсмыссленно расходуют процессорное время
// TTransferWait   - В вызове Perform() поток засыпает определённое количество 
// миллисекунд (по-умолчанию 500, но можно переопределить вызовом 
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
   //Значение таймера обозначющее бесконечность
   static inline DWORD TimerInfinityVal() { return std::numeric_limits<DWORD>::max(); }

   //Установить время ожидания сетевого события, можно установить TimerInfinityVal(), 
   //для бесконечного ожидания
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

public: //Настройка 
   //URL сервера с которым ведётся взаимодействие
   void SetURL( const std::string &Url )
   { 
      m_Url = Url; //The string must remain present until curl no longer needs it, as it doesn't copy the string.
      APL_CHECK( curl_easy_setopt(m_hEasy, CURLOPT_URL, m_Url.c_str()) == CURLE_OK ); 
   }

   //Установить PROXY
   void SetProxy( const std::string &Proxy )
   {
      m_Proxy = Proxy;
      APL_CHECK( curl_easy_setopt(m_hEasy, CURLOPT_PROXY, m_Proxy.c_str()) == CURLE_OK ); 
   }

   const std::string &GetUrl() const { return m_Url; }

   //Добавить дополнительное поле в заголовок запроса POST, например "Content-type: text/xml"
   void AddFieldToPostHeader( const char *szData )
   {
      //Функкция curl_slist_append сделает дубликат строки szData
      m_Headers = curl_slist_append(m_Headers, szData);
      APL_ASSERT_PTR(m_Headers);
   }

   //Включаем или выключаем болтливость, полезно при отладки, пишет в stderr
   void SetVerbose( int flag )
   {
      APL_CHECK( curl_easy_setopt(m_hEasy, CURLOPT_VERBOSE, flag) == CURLE_OK );
   }

public: //Действия см. подробнее в описании класса
   //Назначить задание на скачивания файла с сервера
   //CallbackObject - Класс который будет получать уведомления о том, что 
   //очередная порция данных была скачена
   //Кидает исключение NWLib::TAplThrowException
   template < class TCallbackObject >
   void Get( TCallbackObject &CallbackObject );

   //Назначить задание на запрос к серверу
   //Сначала на сервер отправляется запрос - SendDataSize байт а затем 
   //скачивается ответ сервера
   //CallbackObject - Класс который будет получать уведомления о том, что 
   //необходимо отправить или принять очередную порцию данных.
   //Кидает исключение NWLib::TAplThrowException
   template< class TCallbackObject >
   void Request( TCallbackObject &CallbackObject, size_t SendDataSize );

   //Выполнить атомарную часть операции назначенного задания.
   //Возвр: true - задание ещё не выполнено, необходимо повторить вызов.
   //       false - задание выполнено, заданий больше нет
   //Кидает исключение NWLib::TAplThrowException
   bool Perform();

   //Установить максимальную скорость в байтах/сек скачивания/отправки данных
   //Значение 0 означает то, что ограничение снимается
   void SetMaxDownloadSpeed( size_t Value );
   void SetMaxUploadSpeed( size_t Value );

   //Экстренно завершить текущие задание
   void AbortTask();

private:
   //Класс который преобразует вызов CallBack'а CURL в свой формат 
   template< class TCallbackObject> struct TCalbackFunc;

private:
   //Обработать все сообщения которые появились в m_hMulti
   //Кидает исключение NWLib::TAplThrowException
   void ProcessMessages();

   //Извлечь все сообщения из очереди m_hMulti
   void RemoveMessages();

   //Функция должна вызываться перед началом выполнения задания
   //Настраивает m_hMulti
   void BeginNewTask();

   //Выполнить основную работу Perform, устанавливает значение m_RunnigHandlers
   //Кидает исключение NWLib::TAplThrowException
   void PerformImpl();

   //Специальные версии Callback функций для прерывания передачи
   static size_t AbortReceiveCallback( void *ptr, size_t size, size_t nmemb, void *stream ) { return std::numeric_limits<size_t>::max(); }
   static size_t AbortSendCallback( void *ptr, size_t size, size_t nmemb, void *stream ) { return CURL_READFUNC_ABORT; }

private:
   CURL *m_hEasy;
   CURLM *m_hMulti;
   curl_slist *m_Headers; //Дополнительные поля заголовка для POST 
   int m_RunnigHandlers;  //Количество активных заданий
   std::string m_Url;     //URL взаимодействия
   std::string m_Proxy;   //Proxy взаимодействия
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
   curl_easy_cleanup(m_hEasy);  //Перед удалением Multi надо удалить все Easy
   APL_CHECK( curl_multi_cleanup(m_hMulti) == CURLE_OK );
}   
///////////////////////////////////////////////////////////////////////////////

template< class WaitStrategyT >
void TTransfer<WaitStrategyT>::ProcessMessages()
{
   int  MsgInQueue;
   CURLMsg *pCurlMsg;

   //Просматриваем сообщения и проверяем ошибки
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
   //Можно вызывать curl_multi_remove_handle в ProcessMessages, а здесь не вызывать ничего,
   //но не факт что при любой ошибке к нам придёт сообщение в ProcessMessages с текущем EasyHandle
   //Например это возможно при ошибке в curl_multi_perform
   //Поэтому с самого начала мы будем поддерживать постоянное наличие регистрации EasyHandle в MultiHandle
   APL_CHECK( curl_multi_remove_handle(m_hMulti, m_hEasy) == CURLE_OK );
   APL_CHECK( curl_multi_add_handle(m_hMulti, m_hEasy) == CURLE_OK );

   RemoveMessages();
}
///////////////////////////////////////////////////////////////////////////////

template< class WaitStrategyT >
void TTransfer<WaitStrategyT>::AbortTask()
{
   //Единственный способ прервать передачу или отправку данных, это подменять Callback'и :(
   APL_CHECK( curl_easy_setopt(m_hEasy, CURLOPT_WRITEFUNCTION, AbortReceiveCallback ) == CURLE_OK );
   APL_CHECK( curl_easy_setopt(m_hEasy, CURLOPT_READFUNCTION, AbortSendCallback ) == CURLE_OK );
   //Есть ещё CURLOPT_PROGRESSFUNCTION, но
   //Usage of the CURLOPT_PROGRESSFUNCTION callback is not recommended when using the multi interface.

   APL_DEBUG_VARIABLE( NWLib::TTimer CycleTime );

   do 
   {
      curl_multi_perform( m_hMulti, &m_RunnigHandlers );
      APL_DEBUG_OPERATION( CycleTime.Stop() );
      APL_ASSERT( ("Попытка прервать передачу данных заняла слишком много времени", CycleTime.InSec() < 1.5) );
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

   while ( curl_multi_info_read(m_hMulti, &MsgInQueue) != NULL ) { /*ничего*/ }
}
///////////////////////////////////////////////////////////////////////////////

template< class WaitStrategyT >
void TTransfer<WaitStrategyT>::PerformImpl()
{
   CURLMcode CurlRez;

   while( (CurlRez = curl_multi_perform(m_hMulti, &m_RunnigHandlers)) == CURLM_CALL_MULTI_PERFORM ) { /*Ничего*/ }

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

   //Если PerformWait возвращает 0 (следовательно select возвращает 0)
   //то выходить нельзя, т.к. при невозможности подключения m_RunnigHandlers никогда 
   //не будет равным 0 и мы никогда не выйдем по таймауту
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
