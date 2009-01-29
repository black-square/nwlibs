#ifndef HTTPServerConnection_HPP
#define HTTPServerConnection_HPP

#include "HTTPBasicConnection.hpp"

///////////////////////////////////////////////////////////////////////////////
// Определение класса THTTPServer::TConnection  
///////////////////////////////////////////////////////////////////////////////
namespace NWLib
{
template< template <class> class ConnectionT, class GlobalDataT >
template<class FuncImplT>
class THTTPServer<ConnectionT, GlobalDataT>::TConnection: 
   private TConnectionBase, 
   public Detail::THTTPBasicConnection< 
      TConnection<FuncImplT>, 
      Detail::TServerTraits, 
      ConnectionT< TTypeDefinition< TConnection<FuncImplT> > >, 
      FuncImplT 
   >
{
private:
   typedef Detail::THTTPBasicConnection< 
      TConnection<FuncImplT>, 
      Detail::TServerTraits, 
      ConnectionT< TTypeDefinition< TConnection<FuncImplT> > >, 
      FuncImplT 
   > TBaseImpl;

   typedef ConnectionT< TTypeDefinition<TConnection<FuncImplT> > > TBase;
   typedef AsyncIOConnection::TConnectionBasic<FuncImplT> TServerConnection;

public:

   //Отправить сообщение об ошибке: Строка заголовка слишком большая 
   void SendBufferToSmall() { SendErrorCode(HTTP::ECBadRequest); }

   //Отправить сообщение об ошибке: Формат заголовка неправильный 
   void SendErrorInHeader() { SendErrorCode(HTTP::ECBadRequest); }

   //Отправить сообщение об ошибке: Метод не реализован
   void SendMethodNotImplemented() { SendErrorCode(HTTP::ECNotImplemented); }

   //Отправить сообщение об ошибке: Необходимо передать размер тела запроса
   void SendLengthRequired() { SendErrorCode(HTTP::ECLengthRequired); }


public:
   //Функция TStringParseFunc, состояние SPMFirstLine
   //Расшифровать первую строку заголовка, дабавить нужные данные в m_RequestHeader  
   //и изменить состояние на соответствующие используемому методу в запросе
   TStringParseResult ParseFirstHeaderString( const char *pBegin, const char *pEnd );

   //Функция TStringParseFunc, состояние SPMPOSTHeader
   //Обработать запрос Post. Обработать заголовок до тех пор пока не придёт пустая строка
   //и затем обработать тело запроса.
   TStringParseResult ParsePostHeaderString( const char *pBegin, const char *pEnd );

   //Функция TStringParseFunc, состояние SPMGETHeader
   //Обработать запрос Post. Обработать заголовок до тех пор пока не придёт пустая строка
   //и затем обработать тело запроса.ы
   TStringParseResult ParseGetHeaderString( const char *pBegin, const char *pEnd );

   //Функция TOnSendFunc, состояние SMSendFromBufferAndDisconnect
   //Отправляем весь буфер [pBuf, pBuf + Length), а затем разорвать соединение
   bool OnSendFromBufferAndDisconnect( int SendSize, const char *&pBuf, int &Length );

   //Функция TOnSendFunc, состояние SMSendFromBufferOnly
   //Отправляем весь буфер [pBuf, pBuf + Length)
   bool OnSendFromBufferOnly( int SendSize, const char *&pBuf, int &Length );

   //Функция TOnSendFunc, состояние SMSendFromBufferAndThenBody
   //Отправляем весь буфер [pBuf, pBuf + Length), и переключаем состояние на SMSendBody
   bool OnSendFromBufferAndThenBody( int SendSize, const char *&pBuf, int &Length );

   //Функция TOnSendFunc, состояние SMSendBody
   //Спрашиваем у пользователя данные для отправки и отправляем, а затем переключаемся в режим чтения
   bool OnSendBody( int SendSize, const char *&pBuf, int &Length );

public:
   template<class InitDataT>
   TConnection( const InitDataT &InitData ): 
      TConnectionBase( TServerConnection::GetGlobalData() ),
      TBaseImpl( InitData )
   { }

   //Разрыв соединения
   void OnDisconnect() { TBase::OnDisconnect(); }

   //Создание соединения
   void OnConnect();

   //Сработал таймер ожидания ввода/вывода
   void OnTimer() { TBase::OnTimer(); }

   //Отправить код ошибки клиенту и закрыть соединение
   void SendErrorCode( HTTP::TErrorCode EC );

   //Отправить только заголовок клиенту
   void SendHeader( HTTP::TErrorCode EC, const HTTP::TResponseHeader &Header );

   //Отправить заголовок, а затем тело
   void SendHeaderWithBody( HTTP::TErrorCode EC, const HTTP::TResponseHeader &Header );

   //Поставить в очередь запрос на разрыв соединения.
   //О разрыве соединения класс производный от TServerConnectionBasic будет уведомлён при помощи вызова
   //метода OnDisconnect.
   void TryDisconnect() { TServerConnection::TryDisconnect(); }

   //Получить ссылку на производный от TServerGlobalDataBasic класс который передан в параметре шаблона 
   //GlobalDataT класса HTTP Сервера
   GlobalDataT &GetGlobalData() { return *TConnectionBase::GetGlobalData().GetParent(); }
   const GlobalDataT &GetGlobalData() const { return *TConnectionBase::GetGlobalData().GetParent(); }

   //Установить время ожидания чтения/записи таймера в миллисекундах.
   //Можно воспользоваться значением TimerInfinityVal() для бесконечного ожидания
   void SetWaitableTimer( DWORD dwMilliseconds ) { TServerConnection::SetWaitableTimer(dwMilliseconds); }

   //Перестать принимать новые соединения. После этого сервер ждёт завершения всех
   //созданных к данному моменту соединений, при этом не передаётся флаг досрочного закрытия соединений.
   //GlobalData можно использовать как счётчик соединенний
   //Для того чтобы текущее соединение стало последним надо в конструкторе класса наследника от
   //TServerConnectionBasic вызвать данную функцию (Если вызывать в методе OnConnection или ещё где либо, 
   //возможно создание нескольких новых соединений из-за многопоточности)
   void StopWaitConnection() { TServerConnection::StopWaitConnection(); }
};
///////////////////////////////////////////////////////////////////////////////

template< template <class FuncImplT> class ConnectionT, class GlobalDataT >
template<class FuncImplT>
void THTTPServer<ConnectionT, GlobalDataT>::TConnection<FuncImplT>::OnConnect()
{
   char *pReceiveBuf;
   int ReceiveLength;

   TBase::OnConnect();
   m_CurReceiveMode = RMReadString;
   m_CurStringParseMode = SPMFirstLine;
   m_ReceiveBuf.PrepareForReceiveFull();
   m_ReceiveBuf.BeforeRecieveData( pReceiveBuf, ReceiveLength );
   TServerConnection::TryReceive( pReceiveBuf, ReceiveLength );
}
///////////////////////////////////////////////////////////////////////////////

template< template <class FuncImplT> class ConnectionT, class GlobalDataT >
template<class FuncImplT>
void THTTPServer<ConnectionT, GlobalDataT>::TConnection<FuncImplT>::SendErrorCode( HTTP::TErrorCode EC )
{
   APL_ASSERT( EC != HTTP::ECOk ); 
   
   //Отключаемся после передачи
   m_CurSendMode = SMSendFromBufferAndDisconnect;

   m_SendBuf.MakeErrorCode(EC);

   //DEBUG_MSG( "Отправляем ошибку: " <<  m_SendBuf.Begin() );
   TServerConnection::TrySend( m_SendBuf.Begin(), m_SendBuf.Length() );
}
///////////////////////////////////////////////////////////////////////////////

template< template <class FuncImplT> class ConnectionT, class GlobalDataT >
template<class FuncImplT>
void THTTPServer<ConnectionT, GlobalDataT>::TConnection<FuncImplT>::SendHeader( HTTP::TErrorCode EC, const HTTP::TResponseHeader &Header )
{
   APL_ASSERT( Header.GetContentLength() == Header.ErrorContentLength() );

   m_CurSendMode = SMSendFromBufferOnly;

   m_SendBuf.MakeHeader( EC, Header );

   TServerConnection::TrySend( m_SendBuf.Begin(), m_SendBuf.Length() );
}
///////////////////////////////////////////////////////////////////////////////

template< template <class FuncImplT> class ConnectionT, class GlobalDataT >
template<class FuncImplT>
void THTTPServer<ConnectionT, GlobalDataT>::TConnection<FuncImplT>::SendHeaderWithBody( HTTP::TErrorCode EC, const HTTP::TResponseHeader &Header )
{
   APL_ASSERT( Header.GetContentLength() != Header.ErrorContentLength() );
   
   //Будем использовать переменную m_NeadToReceive для проверки того что пользователь послал в теле запроса
   //ровно столько данных сколько указал в Header.GetContentLength()
   APL_DEBUG_OPERATION( m_NeadToReceive = Header.GetContentLength() );

   m_CurSendMode = SMSendFromBufferAndThenBody;

   m_SendBuf.MakeHeader( EC, Header );

   TServerConnection::TrySend( m_SendBuf.Begin(), m_SendBuf.Length() );
}

} //namespace NWLib

#include "HTTPServerConnectionStateFuction.hpp"

#endif