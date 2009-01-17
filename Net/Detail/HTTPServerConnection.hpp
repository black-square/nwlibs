#ifndef HTTPServerConnection_HPP
#define HTTPServerConnection_HPP

#include "HTTPBasicConnection.hpp"

///////////////////////////////////////////////////////////////////////////////
// ����������� ������ THTTPServer::TConnection  
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

   //��������� ��������� �� ������: ������ ��������� ������� ������� 
   void SendBufferToSmall() { SendErrorCode(HTTP::ECBadRequest); }

   //��������� ��������� �� ������: ������ ��������� ������������ 
   void SendErrorInHeader() { SendErrorCode(HTTP::ECBadRequest); }

   //��������� ��������� �� ������: ����� �� ����������
   void SendMethodNotImplemented() { SendErrorCode(HTTP::ECNotImplemented); }

   //��������� ��������� �� ������: ���������� �������� ������ ���� �������
   void SendLengthRequired() { SendErrorCode(HTTP::ECLengthRequired); }


public:
   //������� TStringParseFunc, ��������� SPMFirstLine
   //������������ ������ ������ ���������, �������� ������ ������ � m_RequestHeader  
   //� �������� ��������� �� ��������������� ������������� ������ � �������
   TStringParseResult ParseFirstHeaderString( const char *pBegin, const char *pEnd );

   //������� TStringParseFunc, ��������� SPMPOSTHeader
   //���������� ������ Post. ���������� ��������� �� ��� ��� ���� �� ����� ������ ������
   //� ����� ���������� ���� �������.
   TStringParseResult ParsePostHeaderString( const char *pBegin, const char *pEnd );

   //������� TStringParseFunc, ��������� SPMGETHeader
   //���������� ������ Post. ���������� ��������� �� ��� ��� ���� �� ����� ������ ������
   //� ����� ���������� ���� �������.�
   TStringParseResult ParseGetHeaderString( const char *pBegin, const char *pEnd );

   //������� TOnSendFunc, ��������� SMSendFromBufferAndDisconnect
   //���������� ���� ����� [pBuf, pBuf + Length), � ����� ��������� ����������
   bool OnSendFromBufferAndDisconnect( int SendSize, const char *&pBuf, int &Length );

   //������� TOnSendFunc, ��������� SMSendFromBufferOnly
   //���������� ���� ����� [pBuf, pBuf + Length)
   bool OnSendFromBufferOnly( int SendSize, const char *&pBuf, int &Length );

   //������� TOnSendFunc, ��������� SMSendFromBufferAndThenBody
   //���������� ���� ����� [pBuf, pBuf + Length), � ����������� ��������� �� SMSendBody
   bool OnSendFromBufferAndThenBody( int SendSize, const char *&pBuf, int &Length );

   //������� TOnSendFunc, ��������� SMSendBody
   //���������� � ������������ ������ ��� �������� � ����������, � ����� ������������� � ����� ������
   bool OnSendBody( int SendSize, const char *&pBuf, int &Length );

public:
   template<class InitDataT>
   TConnection( const InitDataT &InitData ): 
      TConnectionBase( TServerConnection::GetGlobalData() ),
      TBaseImpl( InitData )
   { }

   //������ ����������
   void OnDisconnect() { TBase::OnDisconnect(); }

   //�������� ����������
   void OnConnect();

   //�������� ������ �������� �����/������
   void OnTimer() { TBase::OnTimer(); }

   //��������� ��� ������ ������� � ������� ����������
   void SendErrorCode( HTTP::TErrorCode EC );

   //��������� ������ ��������� �������
   void SendHeader( HTTP::TErrorCode EC, const HTTP::TResponseHeader &Header );

   //��������� ���������, � ����� ����
   void SendHeaderWithBody( HTTP::TErrorCode EC, const HTTP::TResponseHeader &Header );

   //��������� � ������� ������ �� ������ ����������.
   //� ������� ���������� ����� ����������� �� TServerConnectionBasic ����� �������� ��� ������ ������
   //������ OnDisconnect.
   void TryDisconnect() { TServerConnection::TryDisconnect(); }

   //�������� ������ �� ����������� �� TServerGlobalDataBasic ����� ������� ������� � ��������� ������� 
   //GlobalDataT ������ HTTP �������
   GlobalDataT &GetGlobalData() { return *TConnectionBase::GetGlobalData().GetParent(); }
   const GlobalDataT &GetGlobalData() const { return *TConnectionBase::GetGlobalData().GetParent(); }

   //���������� ����� �������� ������/������ ������� � �������������.
   //����� ��������������� ��������� TimerInfinityVal() ��� ������������ ��������
   void SetWaitableTimer( DWORD dwMilliseconds ) { TServerConnection::SetWaitableTimer(dwMilliseconds); }

   //��������� ��������� ����� ����������. ����� ����� ������ ��� ���������� ����
   //��������� � ������� ������� ����������, ��� ���� �� ��������� ���� ���������� �������� ����������.
   //GlobalData ����� ������������ ��� ������� �����������
   //��� ���� ����� ������� ���������� ����� ��������� ���� � ������������ ������ ���������� ��
   //TServerConnectionBasic ������� ������ ������� (���� �������� � ������ OnConnection ��� ��� ��� ����, 
   //�������� �������� ���������� ����� ���������� ��-�� ���������������)
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
   
   //����������� ����� ��������
   m_CurSendMode = SMSendFromBufferAndDisconnect;

   m_SendBuf.MakeErrorCode(EC);

   //DEBUG_MSG( "���������� ������: " <<  m_SendBuf.Begin() );
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
   
   //����� ������������ ���������� m_NeadToReceive ��� �������� ���� ��� ������������ ������ � ���� �������
   //����� ������� ������ ������� ������ � Header.GetContentLength()
   APL_DEBUG_OPERATION( m_NeadToReceive = Header.GetContentLength() );

   m_CurSendMode = SMSendFromBufferAndThenBody;

   m_SendBuf.MakeHeader( EC, Header );

   TServerConnection::TrySend( m_SendBuf.Begin(), m_SendBuf.Length() );
}

} //namespace NWLib

#include "HTTPServerConnectionStateFuction.hpp"

#endif