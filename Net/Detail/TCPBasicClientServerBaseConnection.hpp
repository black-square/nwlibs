#ifndef TCPBasicClientServerBaseConnection_HPP
#define TCPBasicClientServerBaseConnection_HPP

///////////////////////////////////////////////////////////////////////////////
// Определение класса Detail::TTCPBasicClientServerBase::TConnection 
///////////////////////////////////////////////////////////////////////////////
namespace NWLib
{
namespace Detail
{

template < class TagT, template <class FuncImplT> class ConnectionT, class GlobalDataT >
class TTCPBasicClientServerBase<TagT, ConnectionT, GlobalDataT>::TConnection: 
   private TConnectionBase, 
   public ConnectionT<TTypeDefinition>, 
   public ThreadsManagerStrategy::TThreadBasic
{
   typedef ConnectionT<TTypeDefinition> TBase;

   //Указатель и размер буфера
   struct TBufPtrReceive
   {
      char* pBuf;
      int Length;

      TBufPtrReceive(): pBuf(0) {}
   };

   struct TBufPtrSend
   {
      const char* pBuf;
      int Length;

      TBufPtrSend(): pBuf(0) {}
   };

private:
   TBufPtrReceive m_ReadBufPtr; //Текущий буфер чтения
   TBufPtrSend m_WriteBufPtr;   //Текущий буфер записи
   fd_set m_FdSetRead;          //Множество сокетов чтения функции select
   fd_set m_FdSetWrite;         //Множество сокетов записи функции select
   TIMEVAL m_TimeVal;           //Время ожидания чтения/записи
   TIMEVAL *pTimeVal;           //Указатель на m_TimeVal или 0
   bool m_Disconnect;           //Необходимо ли отключить соединение

private:
   TConnection(); //Шаблонный конструктор не замещает конструктор по умолчанию

private:
   //Функция для Tag == TCPBasicClientTag открывает сокет и вызывает ProcessWithOpenSocket,
   //а для Tag == TCPBasicServerTag сразу вызыввает ProcessWithOpenSocket (т.к. в этом случае
   //сокет полностью подготовлен)
   void RunImpl( TGlobalData &GlobalData, TShutdownPtr pShutdown, TCPBasicServerTag );
   void RunImpl( TGlobalData &GlobalData, TShutdownPtr pShutdown, TCPBasicClientTag );

   //Функция управляет чтением и записью из/в сокет. Поведение сходно как для сервера так и для клиента
   void ProcessWithOpenSocket( TGlobalData &GlobalData, TShutdownPtr pShutdown );

public:
   //Конструктор 
   //InitData - Данные инициализации соединения
   template<class InitDataT>
   TConnection( const TConnectionInitData<InitDataT> &InitData, TGlobalData &GlobalData ): 
   TConnectionBase(InitData.Socket, GlobalData, InitData), TBase( InitData.m_InitData ),  pTimeVal(0), m_Disconnect(false)
   {                                                                        
      SetIOTimer( InitData.SettingsRef.DefaultIOWaitTime );
      SetCloseTimer( InitData.SettingsRef.DefaultCloseWaitTime );
   }

   //Установить таймер ожидания ввода/вывода
   void SetIOTimer( DWORD dwMilliseconds )
   {
      TimeConvert(dwMilliseconds, m_TimeVal, pTimeVal);
   }

   //Установить время ожидания передачи данных перед закрытием соединения
   void SetCloseTimer( DWORD dwMilliseconds )
   {
      /*
      If SO_DONTLINGER is enabled (the default setting) it always returns immediately—connection is gracefully closed in the background. 
      If SO_LINGER is enabled with a zero time-out: it always returns immediately —connection is reset/terminated. 
      If SO_LINGER is enabled with a nonzero time-out: 
      – with a blocking socket, it blocks until all data sent or time-out expires.
      – with a nonblocking socket, it returns immediately indicating failure.
      */
      const u_short TimeInSec = static_cast<u_short>(dwMilliseconds / 1000);

      linger li = { TimeInSec > 0, TimeInSec };

      if ( setsockopt( TConnectionBase::GetSocket(), SOL_SOCKET, SO_LINGER, (char *)&li, sizeof(linger) ) == SOCKET_ERROR )
         APL_THROW( _T("setsockopt(SO_LINGER) failed: ") << WSAGetLastError() );
   }

   //Отключится от соединения при следующем срабатыванию таймера ввода/вывода
   void TryDisconnect()
   {
      m_Disconnect = true;
   }

   //Находится ли соединение в состоянии отключения.
   //Т.е. пользователь вызвал метод TryDisconnect и вызов методов TrySend и TryReceive не
   //приведёт к передаче или получению данных
   bool IsDisconnecting() const
   {
      return m_Disconnect;
   }

   //Установить указатель на буфер чтения
   void TryReceive( char* pBuf, int Length )
   {
      APL_ASSERT_PTR( pBuf );
      APL_ASSERT( Length > 0 );

      //Нельзя быть уверенными что m_ReadBufPtr.pBuf равен нулю т.к. TryReceive может быть вызван 
      //в другом потоке (Например в потоке OnSend) пока поток вызвавший OnReceive пытается вернуть false

      m_ReadBufPtr.pBuf = pBuf;
      m_ReadBufPtr.Length = Length;
   }

   //Установить указатель на буфер записи
   void TrySend( const char* pBuf, int Length )
   {
      APL_ASSERT_PTR( pBuf );
      APL_ASSERT( Length > 0 );

      //Нельзя быть уверенными что m_WriteBufPtr.pBuf равен нулю т.к. TrySend может быть вызван 
      //в другом потоке (Например в потоке OnReceive) пока поток вызвавший OnSend пытается вернуть false

      m_WriteBufPtr.pBuf = pBuf;
      m_WriteBufPtr.Length = Length;
   }

   //Перестать слушать порт
   void StopWaitConnection()
   {
      TConnectionBase::GetGlobalData().GetParent()->StopWaitConnection();
   }

   //Завершить отправку данных
   void EndSend()
   {
      if( shutdown(TConnectionBase::GetSocket(), SD_SEND) == SOCKET_ERROR  )
         APL_THROW( _T("shutdown error: ") << WSAGetLastError() );
   }

   GlobalDataT &GetGlobalData() { return *TConnectionBase::GetGlobalData().GetParent(); }
   const GlobalDataT &GetGlobalData() const { return *TConnectionBase::GetGlobalData().GetParent(); }

   //Функция потока
   DWORD Run( TGlobalData &GlobalData, TShutdownPtr pShutdown );
};

///////////////////////////////////////////////////////////////////////////////
template < class TagT, template <class FuncImplT> class ConnectionT, class GlobalDataT >
void TTCPBasicClientServerBase<TagT, ConnectionT, GlobalDataT>::TConnection::RunImpl( TGlobalData &GlobalData, TShutdownPtr pShutdown, TCPBasicClientTag )
{
   //Сейчас нам необходимо попытаться установить соединение с удалённым хостом
   fd_set FdSetWrite;
   TIMEVAL TimeVal;
   TIMEVAL *pTimeVal;
   int SelectRet;

   Detail::TimeConvert( TConnectionBase::GetBaseInitData().ConnectionWaitTime, TimeVal, pTimeVal );

   //Устанавливаем режим non-blocking
   u_long BlockingMode = 1;

   if( ioctlsocket(TConnectionBase::GetSocket(), FIONBIO, &BlockingMode) == SOCKET_ERROR  )
      APL_THROW( _T("ioctlsocket() failed: ") << WSAGetLastError() );

   while( *pShutdown != TRUE && !m_Disconnect )
   {
      if( connect(TConnectionBase::GetSocket(), (SOCKADDR*) &TConnectionBase::GetBaseInitData().SockAddr, sizeof(TConnectionBase::GetBaseInitData().SockAddr)) != SOCKET_ERROR || WSAGetLastError() != WSAEWOULDBLOCK )
         APL_THROW( _T("connect() failed: ") << WSAGetLastError() );

      FD_ZERO(&FdSetWrite);
      FD_SET(TConnectionBase::GetSocket(), &FdSetWrite);

      SelectRet = select( 0, NULL, &FdSetWrite, NULL, pTimeVal );

      if( SelectRet == SOCKET_ERROR )
         APL_THROW( _T("select() failed: ") << WSAGetLastError() );

      if( SelectRet == 0 )
      {
         TBase::OnConnectTimer();
         continue;
      }

      APL_ASSERT( FD_ISSET(TConnectionBase::GetSocket(), &FdSetWrite) );

      //Если сокет готов для записи, то соединение установлено 

      BlockingMode = 0; //Возвращаем режим блокирования
      if( ioctlsocket(TConnectionBase::GetSocket(), FIONBIO, &BlockingMode) == SOCKET_ERROR  )
         APL_THROW( _T("ioctlsocket() failed: ") << WSAGetLastError() );

      ProcessWithOpenSocket(GlobalData, pShutdown);

      break;
   }
}
///////////////////////////////////////////////////////////////////////////////

template < class TagT, template <class FuncImplT> class ConnectionT, class GlobalDataT >
inline void TTCPBasicClientServerBase<TagT, ConnectionT, GlobalDataT>::TConnection::RunImpl( TGlobalData &GlobalData, TShutdownPtr pShutdown, TCPBasicServerTag )
{
   ProcessWithOpenSocket(GlobalData, pShutdown);
}
///////////////////////////////////////////////////////////////////////////////

template < class TagT, template <class FuncImplT> class ConnectionT, class GlobalDataT >
void TTCPBasicClientServerBase<TagT, ConnectionT, GlobalDataT>::TConnection::ProcessWithOpenSocket( TGlobalData &GlobalData, TShutdownPtr pShutdown )
{
   TBase::OnConnect();

   int Rezult;

   while( *pShutdown != TRUE && !m_Disconnect && (m_ReadBufPtr.pBuf != 0 || m_WriteBufPtr.pBuf != 0) )
   {
      //Настраиваем select
      FD_ZERO(&m_FdSetRead);
      FD_ZERO(&m_FdSetWrite);

      if( m_ReadBufPtr.pBuf != 0 )
         FD_SET(TConnectionBase::GetSocket(), &m_FdSetRead);

      if( m_WriteBufPtr.pBuf != 0 )
         FD_SET(TConnectionBase::GetSocket(), &m_FdSetWrite);

      Rezult = select(0, &m_FdSetRead, &m_FdSetWrite, NULL, pTimeVal);

      if( Rezult == SOCKET_ERROR )
         APL_THROW( _T("select error: ") << WSAGetLastError() );

      if( Rezult == 0 )
      {
         //Сработал Таймер
         TBase::OnTimer();
         continue;
      }

      if( FD_ISSET(TConnectionBase::GetSocket(), &m_FdSetRead) )
      {
         //Попробуем прочитать данные
         Rezult = recv( TConnectionBase::GetSocket(), m_ReadBufPtr.pBuf, m_ReadBufPtr.Length, 0  );

         if( Rezult == SOCKET_ERROR )
         {
            TBase::OnEndReceive( ERUnknownError );
            m_ReadBufPtr.pBuf = 0;
         }
         else if( Rezult == 0 || Rezult == WSAECONNRESET )
         {
            TBase::OnEndReceive( EROk );
            m_ReadBufPtr.pBuf = 0;
         }
         else
         {
            if( !TBase::OnReceive(Rezult, m_ReadBufPtr.pBuf, m_ReadBufPtr.Length) )
               m_ReadBufPtr.pBuf = 0;

            APL_ASSERT( m_ReadBufPtr.pBuf == 0 || m_ReadBufPtr.Length > 0 );
         }
      }

      if( FD_ISSET(TConnectionBase::GetSocket(), &m_FdSetWrite) )
      {
         //Попробуем отправить данные
         Rezult = send( TConnectionBase::GetSocket(), m_WriteBufPtr.pBuf, m_WriteBufPtr.Length, 0  );

         if( Rezult == SOCKET_ERROR )
         {
            TBase::OnEndSend( ERUnknownError );
            m_WriteBufPtr.pBuf = 0;
         }
         else
         {
            APL_ASSERT( Rezult > 0 );

            if( !TBase::OnSend(Rezult, m_WriteBufPtr.pBuf, m_WriteBufPtr.Length) )
               m_WriteBufPtr.pBuf = 0;

            APL_ASSERT( m_WriteBufPtr.pBuf == 0 || m_WriteBufPtr.Length > 0 );
         }
      }
   }

   OnDisconnect();
}
///////////////////////////////////////////////////////////////////////////////

template < class TagT, template <class FuncImplT> class ConnectionT, class GlobalDataT >
DWORD TTCPBasicClientServerBase<TagT, ConnectionT, GlobalDataT>::TConnection::Run( TGlobalData &GlobalData, TShutdownPtr pShutdown ) 
{         
   try
   {
      RunImpl( GlobalData, pShutdown, TagT() );
   }
   catch (std::exception &e)
   {
      APL_LOG( _T("Exception in TTCPBasicClientServer::Run: ") << ConvertToBuf< std::basic_string<TCHAR> >(e.what()) );
   }
   catch(...)
   {
      APL_LOG( _T("Unknown exception in TTCPBasicClientServer::Run") );
   }

   return 0;
}

} //namespace Detail
} //namespace NWLib


#endif