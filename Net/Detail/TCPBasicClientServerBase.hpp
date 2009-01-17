#ifndef TCPBasicClientServerBase_HPP
#define TCPBasicClientServerBase_HPP

#include "../../ThreadsManager.hpp"


///////////////////////////////////////////////////////////////////////////////
// ������ �������� ������� ����� ��� ���������� TCP ������� � ������� ����������� 
// ��� ������ ������� Berkly-socket � ������������� ������� select ��� ������������� 
// �����/������
// �������� ����������� ������� �� AsyncIOConnection.hpp
///////////////////////////////////////////////////////////////////////////////

namespace NWLib
{
namespace Detail
{
   ///////////////////////////////////////////////////////////////////////////////
   // ����� �������������� � ������������ ��������� WSADATA � ������� � � �����������
   ///////////////////////////////////////////////////////////////////////////////
   class TWSAInitManager: NonCopyable
   {
      WSADATA wsaData;

   public:
      TWSAInitManager( WORD VersionRequired = MAKEWORD( 2, 2 ) )
      {
         int ErrorCode = WSAStartup( VersionRequired, &wsaData );
         if ( ErrorCode != 0 )
            APL_THROW( "Error at WSAStartup(), ErrorCode: " << ErrorCode );
      }

      ~TWSAInitManager()
      {
         APL_CHECK( WSACleanup() == 0 );
      }

      const WSADATA &GetData() const { return wsaData; }
      WSADATA &GetData() { return wsaData; }
   };

   ///////////////////////////////////////////////////////////////////////////////
   // ��������������� �� ������� � ������������� �� ����� � TIMEVAL
   // �������� pTimeVal ���������� ��������������� ���������� � select,
   // ����� ������ ������� �� ����� ��������� ���� �� TimeVal ���� �� NULL
   // (���� �������� dwMilliseconds == TimerInfinityVal())
   ///////////////////////////////////////////////////////////////////////////////
   inline void TimeConvert( DWORD dwMilliseconds, TIMEVAL &TimeVal, TIMEVAL *&pTimeVal )
   {
      if( dwMilliseconds == AsyncIOConnection::TimerInfinityVal() )
      {
         pTimeVal = NULL;
      }
      else
      {
         TimeVal.tv_sec = dwMilliseconds / 1000;
         TimeVal.tv_usec = dwMilliseconds % 1000 * 1000;
         pTimeVal = &TimeVal;
      }
   }

///////////////////////////////////////////////////////////////////////////////
// ������ ������������ ��� ���� ����� ������������ ������������� ������� ������ 
// TTCPBasicClientServerBase
///////////////////////////////////////////////////////////////////////////////
class TCPBasicClientTag {};
class TCPBasicServerTag {};

///////////////////////////////////////////////////////////////////////////////
// ���������� ��� ����� ������ TTCPBasicClientServerBase, ������� ���������� �
// ������� � �������
///////////////////////////////////////////////////////////////////////////////
template< class TagT > class TServerClientSideImpl;

template<>
class TServerClientSideImpl<TCPBasicServerTag>
{
protected:
   //�������������� ��������� ��� TTCPBasicClientServerBase::TConnectionInitData, ������� ���������� �� ����
   //����� �� �� ���� � �������� ��� ��������
   struct TBaseInitData
   {  
      /* � ������� ���� ��� ��� ������ */ 
   };

   //��� ��������� ����������
   enum TStopConnection 
   {
      SCContinue,        //���������� ��������� ������������ ����������
      SCStopListen,      //��������� ��������� ����� ���������� �� ���������� ��������� ������������
      SCFullStop         //��������� ��������� ����� ��������� � ��������� ������ � ��� ��� ���������� �� �������
   };

private:
   volatile TStopConnection m_StopConnection;   //���������� ���������� ������

protected:   
   //���������� �� ���������������
   TStopConnection NeadStop() const
   {
      return m_StopConnection;
   }

   //������� Run ������� ������ �������� ��� ������� ����� ����������� ����� ������
   void PrepareRun()
   {
      m_StopConnection = SCContinue;
   }


public:
   TServerClientSideImpl(): m_StopConnection(SCContinue) {}

public:
   //���������� ������
   //�������� ����� ���������� ���������� � ���������� ��������� � TSettings
   //������ �������� ��������� ������� ��� ���������� � ����� ����� ������� Run
   //���������� ����������
   void Stop()
   {
      m_StopConnection = SCFullStop;
   }

   //��������� ������� ��������� ����� �����������
   //�������� ����� ���������� ���������� � ���������� ��������� � TSettings
   //������ �������� �������� ����� ����������� ���������� ����� ������������ ���������� 
   //����������� ���� � ����� ������� Run ���������� ����������
   void StopWaitConnection()
   {
      m_StopConnection = SCStopListen;
   } 

};
///////////////////////////////////////////////////////////////////////////////

template<>
class TServerClientSideImpl<TCPBasicClientTag>
{
protected:
   //�������������� ��������� ��� TTCPBasicClientServerBase::TConnectionInitData, ������� ���������� �� ����
   //����� �� �� ���� � �������� ��� ��������
   struct TBaseInitData
   {  
      DWORD ConnectionWaitTime; //����� � ������������� �������� �����������, ���������� TSettings::ConnectionWaitTime
      sockaddr_in SockAddr;     //���������� ����������� ��� �����������
   };
};

///////////////////////////////////////////////////////////////////////////////
// ������� ����� ��� TCP ������� � ������� ����������� ��� ������ ������� 
// Berkly-socket � ������������� ������� select ��� ������������� �����/������.
// ��� ������� ������ ���������� �������� ��������� �����.
// TagT - ����� ���� ���� TCPBasicClientTag ���� TCPBasicServerTag
///////////////////////////////////////////////////////////////////////////////
template<class TagT, template <class FuncImplT> class ConnectionT = AsyncIOConnection::TConnectionBasic, class GlobalDataT = AsyncIOConnection::TGlobalDataBasic>
class TTCPBasicClientServerBase: public TServerClientSideImpl<TagT>, public GlobalDataT, public NonCopyable
{
public:
   //��������� �������/�������
   struct TSettings;

protected:
   typedef typename TServerClientSideImpl<TagT>::TBaseInitData TBaseInitData;

   template<class InitDataT>
   struct TConnectionInitData: TBaseInitData
   {
      //����� ������� ������ ��� ������, ��� ���������� ���������� ����������
      //����� ������� ��� ��������� ����� � ���� ����� ������ � ������ ������
      SOCKET Socket; 
      
      //�������� ������ ��� �������� �������� ��� ������������ � ����� ����������� �� TConnectionBasic
      const InitDataT &m_InitData; 

      //��������� �� ��������� 
      const TSettings &SettingsRef;

      TConnectionInitData(const InitDataT &InitData, const TSettings &Settings): 
         m_InitData( InitData ), SettingsRef(Settings) {}
   };

private:   
   //���������� ������ ��� ���� ����������
   //����� ����������� �������� ������ � GlobalDataT �� ThreadsManager
   class TGlobalData: public ThreadsManagerStrategy::TGlobalDataBasic
   {
      TTCPBasicClientServerBase *m_pParent;

   public:
      //��� ����� ������� ���������� �������������� 
      //warning C4355: 'this' : used in base member initializer list
      //explicit TGlobalData( TTCPBasicClientServerBase *pParent ): m_pParent(pParent) {}
      TGlobalData(): m_pParent(0) {}
      void SetParent( TTCPBasicClientServerBase *pParent ) { m_pParent = pParent; }
      TTCPBasicClientServerBase *GetParent() { APL_ASSERT_PTR(m_pParent); return m_pParent; }
      const TTCPBasicClientServerBase *GetParent() const { APL_ASSERT_PTR(m_pParent); return m_pParent; }
   };

   //����� ������������� ����������� ������� ��� TConnection
   class TConnectionBase: NonCopyable, private TBaseInitData
   {
      SOCKET m_Socket;             //����� ��� ������/������
      TGlobalData &m_GlobalData;   //�������� � ���� ������ ��� ���� ����� 
                                   //����������� ConnectionT ��� ���������� � GlobalDataT

   public:
      TConnectionBase( SOCKET Socket, TGlobalData &GlobalData, const TBaseInitData &BaseConnectionInitData ): 
         m_Socket(Socket), m_GlobalData(GlobalData), TBaseInitData(BaseConnectionInitData) {}

      ~TConnectionBase() { APL_CHECK( closesocket(m_Socket) == 0 ); }

      TGlobalData &GetGlobalData() { return m_GlobalData; }
      const TGlobalData &GetGlobalData() const { return m_GlobalData; }
      SOCKET &GetSocket() { return m_Socket; }
      const TBaseInitData &GetBaseInitData() const { return *this; }
   };

   //��������� TConnection �� �������� ��������� ����������� �� �� ����� ������������
   //TConnection::TGlobalData � ������������ ConnectionT. ������� ������ �������������� ���������
   class TConnection;
   struct TTypeDefinition
   {
      typedef TConnection TConnection;
      typedef GlobalDataT TUserGlobalData;
   };

   //����� ������������� � ���� ������ � ���������� �����������
   //����� ���� ��� ������ �������� ����� accept �������� ������ ����� ������� ��������� ��������� �����������
   class TConnection;

protected:
   typedef TThreadsManager<TConnection, TGlobalData> TThreadsManager;

private:
   Detail::TWSAInitManager m_WSAInitManager;

protected:
   TTCPBasicClientServerBase() {}

   //����������� ���������� �������� � GlobalDataT
   template <class InitDataT>
   TTCPBasicClientServerBase(const InitDataT &InitData): GlobalDataT(InitData) {}
};

///////////////////////////////////////////////////////////////////////////////

template < class TagT, template <class FuncImplT> class ConnectionT, class GlobalDataT >
struct TTCPBasicClientServerBase<TagT, ConnectionT, GlobalDataT>::TSettings
{
   //����� � ������������� �������� �����������. 
   //�� ��������� ����� �������, ���� ����������� �� ������������� ����������� �� ��������� �� ������������
   //���������� ������� ������������, � ���� ��� �� ���������� ������� �����������.
   //����� ������������ TimerInfinityVal()
   DWORD ConnectionWaitTime; 

   //����� � ������������� �������� �����/�������� ������
   //���� � ������� ����� ������� �� ������� ��������/������� ������, �� ���������� ������� OnTimer()
   //����� ������������ TimerInfinityVal()
   DWORD DefaultIOWaitTime;   

   //����� � ������������� �������� �������� ������ ����� ��������� ����������. 
   //���� � ������ �������� ���������� ���� �������������� ������ �� ��������� 
   //�� �������� � ������� ������� �������, � ������ ����� ����� ���������� �����������.
   DWORD DefaultCloseWaitTime; 

   //IP ����� ����������
   //������ ������ �������� ����� ������
   std::string Adress; 

   //���� ���������� 
   //������������� ���������� htons
   u_short Port;              

   TSettings():
   ConnectionWaitTime(5000),
      DefaultIOWaitTime(5000),
      DefaultCloseWaitTime(0),
      Adress(""),             
      Port(23)                //23 - ���� telnet
   {}
};

} //namespace Detail
} //namespace NWLib

#endif

