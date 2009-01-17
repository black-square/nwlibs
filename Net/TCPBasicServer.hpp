#ifndef TCPBasicServer_HPP
#define TCPBasicServer_HPP

#include "TCPInterfaces.hpp"
#include "Detail/TCPBasicClientServerBase.hpp"
#include "Detail/TCPBasicClientServerBaseConnection.hpp"

///////////////////////////////////////////////////////////////////////////////
// ������ �������� ���������� TCP ������� � ������� ����������� ��� ������ 
// ������� Berkly-socket � ������������� ������� select ��� ������������� 
// �����/������
// ������ ������ ����������� �������� � ��������� ������, �� ���� ��� ������
// ��������� ������ TThreadsManager �� ������� �� ���������� �� ������ �����
// ����� ������������ ��������� ������ ���������, ����� ��� "������ ����� �� 
// ���������", "�������� � �������� ��������� ����������� ���������� �������",
// � �.�.
// ����� ���� � ����������� ����� ����� ������� ����� �� ������ � TTCPBasicServer
// �����������, �� ���������� �� Overlaped I/O
///////////////////////////////////////////////////////////////////////////////
namespace NWLib
{
///////////////////////////////////////////////////////////////////////////////
// ���������� �������
///////////////////////////////////////////////////////////////////////////////
template< template <class FuncImplT> class ConnectionT = AsyncIOConnection::TConnectionBasic, class GlobalDataT = AsyncIOConnection::TGlobalDataBasic >
class TTCPBasicServer: public Detail::TTCPBasicClientServerBase<Detail::TCPBasicServerTag, ConnectionT, GlobalDataT>
{
   typedef Detail::TTCPBasicClientServerBase<Detail::TCPBasicServerTag, ConnectionT, GlobalDataT> TBase;
   typedef typename TBase::TThreadsManager TThreadsManager;

public:
   //�����������
   TTCPBasicServer() {}

   //����������� ���������� �������� � GlobalDataT
   template <class InitDataT>
   TTCPBasicServer(const InitDataT &InitData): TBase(InitData) {}

   //��������� ������ � ����������� Settings.
   //������ �������� ������� ���� � ��� ������ ��������� �������� ���������� ������ ConnectionT
   //������� ����������� ����������
   //����� ��������� Run ����������� ���� �� ����� ������ �� ������� ������ ����� Stop()
   //ConnectionInitData - �������� ������� ��������� ������������ ������������ �� TConnectionBasic ������
   //                     ���� ��� ������������� ���������� �������� �����, ��������, �������� NullType
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
   
   //������������� ����������� ������ ��� ������ �� �������
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

      //��������� ������ �� ������ � �����
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

      //��������� �������� ����������
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
            //�� ��������� ����������, �������� ����� ���������� ������������
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
// ���������� �������
///////////////////////////////////////////////////////////////////////////////
template< template <class FuncImplT> class ConnectionT = AsyncIOConnection::TConnectionBasic, class GlobalDataT = AsyncIOConnection::TGlobalDataBasic >
class TTCPBasicClient: public Detail::TTCPBasicClientServerBase<Detail::TCPBasicClientTag, ConnectionT, GlobalDataT>
{
   typedef Detail::TTCPBasicClientServerBase<Detail::TCPBasicClientTag, ConnectionT, GlobalDataT> TBase;
   typedef typename TBase::TThreadsManager TThreadsManager;

private:
   TThreadsManager m_ThreadsManager;

public:
   //�����������
   TTCPBasicClient() { m_ThreadsManager.SetParent(this); }

   //����������� ���������� �������� � GlobalDataT
   template <class InitDataT>
   TTCPBasicClient(const InitDataT &InitData): TBase(InitData) { m_ThreadsManager.SetParent(this); }

   //����� ���� ��� ���������� ���������
   void Wait() 
   {
      m_ThreadsManager.Wait();
   }

   //��������� ��� ���������� ��� �� ����� ���������
   void Stop()
   {
      m_ThreadsManager.Stop();
   }

   //���������� ���������� � �������� ��������� ��c������ Settings.
   //������ �������� ���������� c �������� � ��������� ������ ���������� ����� ���������� ����������
   //����������� � ��������� �������� ���������� �������� � ����������� AsyncIOConnection::TConnectionBasic::OnConnectTimer 
   //ConnectionInitData - �������� ������� ��������� ������������ ������������ �� TConnectionBasic ������
   //                     ���� ��� ������������� ���������� �������� �����, ��������, �������� NullType
   template<class InitDataT>
   void Connect( const TSettings &Settings, const InitDataT &ConnectionInitData );
};
///////////////////////////////////////////////////////////////////////////////

template < template <class FuncImplT> class ConnectionT, class GlobalDataT >
template<class InitDataT>
void TTCPBasicClient<ConnectionT, GlobalDataT>::Connect( const TSettings &Settings, const InitDataT &ConnectionInitData )
{
   TBase::TConnectionInitData<InitDataT> InitData(ConnectionInitData, Settings);

   //��������� ������ �� ������ � �����
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