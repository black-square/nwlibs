#ifndef HTTP_HPP
#define HTTP_HPP

#include "TCPBasicServer.hpp"
#include "HTTPInterfaces.hpp"
#include "Detail/HTTPDetail.hpp"

#include <algorithm>
    
///////////////////////////////////////////////////////////////////////////////
// ������ ��������� HTTP ������ � HTTP ������ ������� ������������ ����������� 
// ����/�����
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// ���������� �������
///////////////////////////////////////////////////////////////////////////////
namespace NWLib
{
template< template <class FuncImplT> class ConnectionT = HTTP::TServerConnectionBasic, class GlobalDataT = HTTP::TServerGlobalDataBasic >
class THTTPServer: public GlobalDataT, public NonCopyable
{
   //���������� ������ ��� ���� ����������
   //����� ����������� �������� ������ � GlobalDataT �� ThreadsManager
   class TGlobalData: public AsyncIOConnection::TGlobalDataBasic
   {
      THTTPServer *m_pParent;

   public:
      //��� ����� ������� ���������� �������������� 
      //warning C4355: 'this' : used in base member initializer list
      //explicit TGlobalData( TTCPBasicClientServerBase *pParent ): m_pParent(pParent) {}
      TGlobalData(): m_pParent(0) {}
      void SetParent( THTTPServer *pParent ) { m_pParent = pParent; }
      THTTPServer *GetParent() { APL_ASSERT_PTR(m_pParent); return m_pParent; }
      const THTTPServer *GetParent() const { APL_ASSERT_PTR(m_pParent); return m_pParent; }
   };

   //����� ������������� ����������� ������� ��� TConnection
   class TConnectionBase: NonCopyable
   {
   private:
      TGlobalData &m_GlobalData;   //�������� � ���� ������ ��� ���� ����� 
      //����������� ConnectionT ��� ���������� � GlobalDataT

   public:
      TConnectionBase( TGlobalData &GlobalData ): m_GlobalData(GlobalData) {}
      
      TGlobalData &GetGlobalData() { return m_GlobalData; }
      const TGlobalData &GetGlobalData() const { return m_GlobalData; }
   };

   //��������� TConnection �� �������� ��������� ����������� �� �� ����� ������������
   //TConnection::TGlobalData � ������ TServerCallRedirection. ������� ������ �������������� ��������� 
   template<class UserConnectionT>
   struct TTypeDefinition
   {
      typedef UserConnectionT TConnection;
      typedef GlobalDataT TUserGlobalData;
   };

   //����� ���������� �������� ��� ����������� ������ ������������ � ������������ ��� ������ ����������
   template<class FuncImplT>
   class TConnection; 
   
   typedef TTCPBasicServer<TConnection, TGlobalData> TServer;  //���������� �������

public:
   //��������� �������
   typedef typename TServer::TSettings TSettings;

private:
   TServer m_Server;

public:
   THTTPServer() { m_Server.SetParent(this); }

   //����������� ���������� �������� � GlobalDataT
   template <class InitDataT>
   THTTPServer(const InitDataT &InitData): GlobalDataT(InitData) { m_Server.SetParent(this); }

   //���������� ������.
   //�������� ����� ���������� ���������� � ���������� ��������� � TSettings
   //������/������ �������� ��������� ������� ��� ���������� � ����� ����� ������� Run
   //���������� ����������
   void Stop() { m_Server.Stop(); }

   //��������� ��������� ����� �����������.
   //�������� ����� ���������� ���������� � ���������� ��������� � TSettings
   //������ �������� �������� ����� ����������� ���������� ����� ������������ ���������� 
   //����������� ���� � ����� ������� Run ���������� ����������
   void StopWaitConnection(){ m_Server.StopWaitConnection(); }

   //��������� ������ � ����������� Settings.
   //������ �������� ������� ���� � ��� ������ ��������� �������� ���������� ������ ConnectionT
   //������� ����������� ����������
   //����� ��������� Run ����������� ���� �� ����� ������ �� ������� ������ ����� Stop()
   //ConnectionInitData - �������� ������� ��������� ������������ ������������ �� TConnectionBasic ������
   //                     ���� ��� ������������� ���������� �������� �����, ��������, �������� NullType
   template<class InitDataT>
   void Run( const TSettings &Settings, const InitDataT &InitData ){ m_Server.Run(Settings, InitData); }
};
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
// ���������� �������
// ���������� ���� ��������, ����� ������������ � ������� libcurl 
// http://curl.haxx.se/libcurl/
///////////////////////////////////////////////////////////////////////////////
template< template <class FuncImplT> class ConnectionT = HTTP::TClientConnectionBasic, class GlobalDataT = HTTP::TClientGlobalDataBasic >
class THTTPClient: public GlobalDataT, public NonCopyable
{
   //���������� ������ ��� ���� ����������
   //����� ����������� �������� ������ � GlobalDataT �� ThreadsManager
   class TGlobalData: public AsyncIOConnection::TGlobalDataBasic
   {
      THTTPClient *m_pParent;

   public:
      //��� ����� ������� ���������� �������������� 
      //warning C4355: 'this' : used in base member initializer list
      //explicit TGlobalData( TTCPBasicClientServerBase *pParent ): m_pParent(pParent) {}
      TGlobalData(): m_pParent(0) {}
      void SetParent( THTTPClient *pParent ) { m_pParent = pParent; }
      THTTPClient *GetParent() { APL_ASSERT_PTR(m_pParent); return m_pParent; }
      const THTTPClient *GetParent() const { APL_ASSERT_PTR(m_pParent); return m_pParent; }
   };

   //����� ������������� ����������� ������� ��� TConnection
   class TConnectionBase: NonCopyable
   {
   private:
      TGlobalData &m_GlobalData;   //�������� � ���� ������ ��� ���� ����� 
      //����������� ConnectionT ��� ���������� � GlobalDataT

   public:
      TConnectionBase( TGlobalData &GlobalData ): m_GlobalData(GlobalData) {}
      
      TGlobalData &GetGlobalData() { return m_GlobalData; }
      const TGlobalData &GetGlobalData() const { return m_GlobalData; }
   };

   //��������� TConnection �� �������� ��������� ����������� �� �� ����� ������������
   //TConnection::TGlobalData � ������ TServerCallRedirection. ������� ������ �������������� ��������� 
   template<class UserConnectionT>
   struct TTypeDefinition
   {
      typedef UserConnectionT TConnection;
      typedef GlobalDataT TUserGlobalData;
   };

   //����� ���������� �������� ��� �������� ���������� � ������������ ��� ������ ����������
   template<class FuncImplT>
   class TConnection; 
   
   typedef TTCPBasicClient<TConnection, TGlobalData> TClient;  //���������� �������

public:
   //��������� �������
   typedef typename TClient::TSettings TSettings;

private:
   TClient m_Client;

public:
   THTTPClient() { m_Client.SetParent(this); }

   //����������� ���������� �������� � GlobalDataT
   template <class InitDataT>
   THTTPClient(const InitDataT &InitData): GlobalDataT(InitData) { m_Server.SetParent(this); }

   //����� ���� ��� ���������� ���������
   void Wait() { m_Client.Wait(); }

   //��������� ���������� � ��� ��� �� ���������� ���������
   void Stop() { m_Client.Stop(); }

   //���������� ���������� � �������� ��������� ��c������ Settings.
   //������ �������� ���������� c �������� � ��������� ������ � ���������� ����� ���������� ����������
   //����������� � ��������� �������� ���������� �������� � ����������� HTTP::TClientConnectionBasic::OnConnectTimer  
   //ConnectionInitData - �������� ������� ��������� ������������ ������������ �� TConnectionBasic ������
   //                     ���� ��� ������������� ���������� �������� �����, ��������, �������� NullType
   template<class InitDataT>
   void Connect( const TSettings &Settings, const InitDataT &ConnectionInitData ) { m_Client.Connect(Settings, ConnectionInitData); }
};
///////////////////////////////////////////////////////////////////////////////

} //namespace NWLib


#include "Detail/HTTPServerConnection.hpp"

#endif