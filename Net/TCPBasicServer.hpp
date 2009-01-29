#ifndef TCPBasicServer_HPP
#define TCPBasicServer_HPP

#include "TCPInterfaces.hpp"
#include "Detail/TCPBasicClientServerBase.hpp"
#include "Detail/TCPBasicClientServerBaseConnection.hpp"

///////////////////////////////////////////////////////////////////////////////
// Модуль содержит реализации TCP сервера и клиента работающего при помощи 
// функций Berkly-socket и использующего функцию select для синхронизации 
// ввода/вывода
// Сейчас каждое соедиенение создаётся в отдельном потоке, но если при помощи
// изменения класса TThreadsManager на сходный по интерфейсу но другой класс
// можно реализаовать множество других стратегий, таких как "потоки вобще не 
// создаются", "создаётся и работает постоянно определённое количество потоков",
// и т.п.
// Кроме того в последствии имеет смысл создать класс со схожим с TTCPBasicServer
// интерфейсом, но работающем на Overlaped I/O
///////////////////////////////////////////////////////////////////////////////
namespace NWLib
{
///////////////////////////////////////////////////////////////////////////////
// Реализация сервера
///////////////////////////////////////////////////////////////////////////////
template< template <class FuncImplT> class ConnectionT = AsyncIOConnection::TConnectionBasic, class GlobalDataT = AsyncIOConnection::TGlobalDataBasic >
class TTCPBasicServer: public Detail::TTCPBasicClientServerBase<Detail::TCPBasicServerTag, ConnectionT, GlobalDataT>
{
   typedef Detail::TTCPBasicClientServerBase<Detail::TCPBasicServerTag, ConnectionT, GlobalDataT> TBase;
   typedef typename TBase::TThreadsManager TThreadsManager;

public:
   //Конструктор
   TTCPBasicServer() {}

   //Конструктор передающий параметр в GlobalDataT
   template <class InitDataT>
   TTCPBasicServer(const InitDataT &InitData): TBase(InitData) {}

   //Запустить сервер с настройками Settings.
   //Сервер начинает слушать порт и как только поступает входящие соединение создаёт ConnectionT
   //который обслуживает соединение
   //Поток вызвавший Run блокируется пока не будет вызван из другого потока метод Stop()
   //ConnectionInitData - Параметр который передаётся конструктору производного от TConnectionBasic классу
   //                     если нет необходимости передавать параметр можно, например, передать NullType
   template<class InitDataT>
   void Run( const TSettings &Settings, const InitDataT &InitData );
};
///////////////////////////////////////////////////////////////////////////////

template < template <class FuncImplT> class ConnectionT, class GlobalDataT >
template<class InitDataT>
void TTCPBasicServer<ConnectionT, GlobalDataT>::Run( const TSettings &Settings, const InitDataT &ConnectionInitData )
{
   TBase::PrepareRun( );
   TThreadsManager ThreadsManager; 
   ThreadsManager.SetParent(this);
   
   //Автоматически освобождает данные при выходе из функции
   struct THolder: public NonCopyable
   {
      SOCKET Socket;

      THolder()
      { 
         Socket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );

         if ( Socket == INVALID_SOCKET )
            APL_THROW( _T("socket() failed: ") << WSAGetLastError() );
      }

      ~THolder()
      {
         APL_CHECK( closesocket(Socket) == 0 );
      }
   };

   {
      THolder Holder;
      sockaddr_in SockAddr;

      //Заполняем данные об адресе и порте
      SockAddr.sin_family = AF_INET;

      if( Settings.Adress.empty() )
         SockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
      else
      {
         SockAddr.sin_addr.s_addr = inet_addr( Settings.Adress.c_str() );
         if(SockAddr.sin_addr.s_addr == INADDR_NONE)
            APL_THROW( _T("Error in IP Adress: ") << ConvertToBuf<std::basic_string<TCHAR> >(Settings.Adress) );
      }

      SockAddr.sin_port = htons( Settings.Port );

      BOOL bReuseAddress = TRUE;
      
      if ( setsockopt(Holder.Socket, SOL_SOCKET, SO_REUSEADDR, (const char *)&bReuseAddress, sizeof(BOOL)) == SOCKET_ERROR )
         APL_THROW( _T("setsockopt(SO_REUSEADDR) failed: ") << WSAGetLastError() );

      if ( bind( Holder.Socket, (SOCKADDR*) &SockAddr, sizeof(SockAddr) ) == SOCKET_ERROR )
         APL_THROW( _T("bind() failed: ") << WSAGetLastError() );

      if ( listen( Holder.Socket, SOMAXCONN ) == SOCKET_ERROR )
         APL_THROW( _T("listen() failed: ") << WSAGetLastError() );

      //Принимаем входящие соединения
      fd_set FdSetRead;
      TIMEVAL TimeVal;
      TIMEVAL *pTimeVal;
      int SelectRet;
      TBase::TConnectionInitData<InitDataT> InitData(ConnectionInitData, Settings);

      Detail::TimeConvert( Settings.ConnectionWaitTime, TimeVal, pTimeVal );

      while( TBase::NeadStop() == TBase::SCContinue )
      {
         FD_ZERO(&FdSetRead);
         FD_SET(Holder.Socket, &FdSetRead);

         SelectRet = select( 0, &FdSetRead, NULL, NULL, pTimeVal );

         if( SelectRet == SOCKET_ERROR )
            APL_THROW( _T("select() failed: ") << WSAGetLastError() );

         if( SelectRet == 0 )
         {
            //Не дождались соединения, проверим может необходимо остановиться
            continue;
         }

         APL_ASSERT( FD_ISSET(Holder.Socket, &FdSetRead) );

         InitData.Socket = accept( Holder.Socket, NULL, NULL );

         if( InitData.Socket == INVALID_SOCKET )
            APL_THROW( _T("accept() failed: ") << WSAGetLastError() );

         ThreadsManager.CreateThread( InitData );
      }
   }

   if( TBase::NeadStop() == TBase::SCFullStop )
      ThreadsManager.Stop();
   
   ThreadsManager.Wait();
}

///////////////////////////////////////////////////////////////////////////////
// Реализация Клиента
///////////////////////////////////////////////////////////////////////////////
template< template <class FuncImplT> class ConnectionT = AsyncIOConnection::TConnectionBasic, class GlobalDataT = AsyncIOConnection::TGlobalDataBasic >
class TTCPBasicClient: public Detail::TTCPBasicClientServerBase<Detail::TCPBasicClientTag, ConnectionT, GlobalDataT>
{
   typedef Detail::TTCPBasicClientServerBase<Detail::TCPBasicClientTag, ConnectionT, GlobalDataT> TBase;
   typedef typename TBase::TThreadsManager TThreadsManager;

private:
   TThreadsManager m_ThreadsManager;

public:
   //Конструктор
   TTCPBasicClient() { m_ThreadsManager.SetParent(this); }

   //Конструктор передающий параметр в GlobalDataT
   template <class InitDataT>
   TTCPBasicClient(const InitDataT &InitData): TBase(InitData) { m_ThreadsManager.SetParent(this); }

   //Ждать пока все соединения закроются
   void Wait() 
   {
      m_ThreadsManager.Wait();
   }

   //Уведомить все соединения что им нужно закрыться
   void Stop()
   {
      m_ThreadsManager.Stop();
   }

   //Попытаться соединится с сервером используя наcтройки Settings.
   //Клиент пытается соединится c сервером в отдельном потоке вызывающий поток возвращает управление
   //Уведомления о неудачных попытках соединения приходят в уведомлении AsyncIOConnection::TConnectionBasic::OnConnectTimer 
   //ConnectionInitData - Параметр который передаётся конструктору производного от TConnectionBasic классу
   //                     если нет необходимости передавать параметр можно, например, передать NullType
   template<class InitDataT>
   void Connect( const TSettings &Settings, const InitDataT &ConnectionInitData );
};
///////////////////////////////////////////////////////////////////////////////

template < template <class FuncImplT> class ConnectionT, class GlobalDataT >
template<class InitDataT>
void TTCPBasicClient<ConnectionT, GlobalDataT>::Connect( const TSettings &Settings, const InitDataT &ConnectionInitData )
{
   TBase::TConnectionInitData<InitDataT> InitData(ConnectionInitData, Settings);

   //Заполняем данные об адресе и порте
   InitData.SockAddr.sin_family = AF_INET;

   if( Settings.Adress.empty() )
      InitData.SockAddr.sin_addr.s_addr = htonl(INADDR_ANY);
   else
   {
      InitData.SockAddr.sin_addr.s_addr = inet_addr( Settings.Adress.c_str() );
      
      if(InitData.SockAddr.sin_addr.s_addr == INADDR_NONE)
         APL_THROW( _T("Error in IP Adress: ") << Settings.Adress );
   }

   InitData.SockAddr.sin_port = htons( Settings.Port );

   InitData.ConnectionWaitTime = Settings.ConnectionWaitTime;

   InitData.Socket = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
   
   if ( InitData.Socket == INVALID_SOCKET )
      APL_THROW( _T("socket() failed: ") << WSAGetLastError() );

    m_ThreadsManager.CreateThread(InitData);
}

} //namespace NWLib


#endif