#ifndef HTTPClientInterfaces_HPP
#define HTTPClientInterfaces_HPP

///////////////////////////////////////////////////////////////////////////////
// ���� �������� �������� ����������� ������� ��� HTTP �������
///////////////////////////////////////////////////////////////////////////////
namespace NWLib
{
namespace HTTP
{
///////////////////////////////////////////////////////////////////////////////
// ����� �� �������� ����������� TCLientConnectionBasic. ������ ��� ��������������� 
// ������� ���������� ������� ������ HTTP �������.
// ������������ � ������ ����������� �� TClientConnectionBasic ������ �������� ������
// ������� ������ ��� �������� � �������� ������, � ����� ��� ���������� �����������
///////////////////////////////////////////////////////////////////////////////
template< class FuncImplT >
struct TClientCallRedirection
{
   //��� ������ ������������� ��� ��������� ����������� ����� ������������ ������
   typedef typename FuncImplT::TUserGlobalData TUserGlobalData;
   typedef typename FuncImplT::TConnection TConnection;

public:
   typedef TErrorCode TErrorCode;
   typedef TRequestHeader TRequestHeader;
   typedef TResponseHeader TResponseHeader;

public:
   //��������� � ������� ������ �� ������ ����������.
   //� ������� ���������� ����� ����������� �� TServerConnectionBasic ����� �������� ��� ������ ������
   //������ OnDisconnect.
   void TryDisconnect()
   {
      static_cast<TConnection *>(this)->TryDisconnect();
   }

   //�������� ������ �� ����������� �� TClientGlobalDataBasic ������ ������� ������� � ��������� ������� 
   //GlobalDataT ������ HTTP �������
   TUserGlobalData &GetGlobalData() { return static_cast<TConnection *>(this)->GetGlobalData(); }
   const TUserGlobalData &GetGlobalData() const { return static_cast<const TConnection *>(this)->GetGlobalData(); }

   //���������� ����� �������� ������/������ ������� � �������������.
   //����� ��������������� ��������� TimerInfinityVal() ��� ������������ ��������
   void SetWaitableTimer( DWORD dwMilliseconds )
   {
      static_cast<TConnection *>(this)->SetWaitableTimer(dwMilliseconds);
   }

   //�������� ������� ����� GET (������� ������� ������ �� ���������)
   //��� ���� Header.GetContentLength() ������ ���� ����� TResponseHeader::ErrorContentLength()
   //����������� �� ������ ������� ����� ��������� ��� ������ ������� 
   //OnBeginResponseBodyReceive, OnCompleateResponse, OnReceiveData 
   void SendRequest_GET( const TRequestHeader &Header )
   {
      static_cast<TConnection *>(this)->SendRequest_GET( Header );
   }

   //�������� ������� ������ POST (������� ������� �� ��������� � ����)
   //��� ���� Header.GetContentLength() ������ ����� ��������������� ������� ���� ������
   //������� ����������� ��������.
   //����������� � ��� ��� ���������� �������� ��������� ������ ���� ������� ����� ���������
   //��� ������ ������ OnSendData
   //����������� �� ������ ������� ����� ��������� ��� ������ ������� 
   //OnBeginResponseBodyReceive, OnCompleateResponse, OnReceiveData 
   void SendRequest_POST( const TRequestHeader &Header )
   {
      static_cast<TConnection *>(this)->SendRequest_POST( Header );
   }
};


///////////////////////////////////////////////////////////////////////////////
// ������� ����� ��� ��������� ����������� ������� ������ ����� ����� ������������.
// ����� HTTP ������ ������� ��������� ���������� � ��������� ������� GlobalDataT - �����
// ������� ������ �������� ����������� TClientGlobalDataBasic � ����� ������� ������ �� 
// ���� ����� ������ ����������. 
// ����������� ����� ����� ��������� ����� ����� TClientGlobalDataBasic.
// HTTP ������ �� ��������� ����� ���� ������������� ��� ������� � GlobalDataT
// ������������ ��� ������ ��������� �� ����.
// ����� ������� �����, ������� ����� ������������� �� ������ HTTP �������, � � 
// ����������� �� TClientGlobalDataBasic ��������� ��������� this �� ����� �����.
///////////////////////////////////////////////////////////////////////////////
class TClientGlobalDataBasic
{
public:   
   //����������� �� ��������� ���������� ������������� ������ HTTP ������� ��� ����������
   TClientGlobalDataBasic() {}

   //� ����������� ����� �������� ����� ����������� ������������ �������� �� ��������� ������
   //���������� ������������ ������ �������
   //���������������� ����������� ����� �� ���� ��������� ���� ���� ��������.
   template <class InitDataT>
   TClientGlobalDataBasic(const InitDataT &InitData) {}

   //���������� ��������� � ����������� ������ �������
   ~TClientGlobalDataBasic() {}
};


///////////////////////////////////////////////////////////////////////////////
// ������� ����� ����������.
// ����� HTTP ������� ������ ������� ���������� ����� ����������� 
// �� TClientConnectionBasic � ���������� � ��������� ConnectionT.
// ����������� ����� ������ ���� �������� ���������������� ���� �������� ������
// TClientConnectionBasic (������� ������ ������� �������� ������� �� ������������ � 
// ����������� �������)
// ��� �������� ���������� ����� �����������.
///////////////////////////////////////////////////////////////////////////////
template< class FuncImplT >
class TClientConnectionBasic: public TClientCallRedirection<FuncImplT>
{
public:
   //��������� ������������� ������� ����� ���� �������������� � ����������� �������
   enum TInitConsts
   {
      //������ ������ ������������� ��� ����� ������. ���� ������ ��������� ����� �������
      //������� ������, �� ���������� ���������.
      ICReceiveBufferSize = 1024
   };

protected:
   //����������� �� ��������� ������� �� ����� ������, �������������� ��� �� ����� �����
   TClientConnectionBasic() {}

public:
   //����������� ���������� ��� �������� �����������
   //������������ ������� GetGlobalData ��� �����
   //��� �������� ���������� � ����������� ����� �������� ����� ����������� ������������ 
   //�������� �� ��������� ������ Connect � ������� � ���������� InitData. 
   //���������������� ����������� ����� �� ���� ��������� ���� ��� InitDataT �������.
   //���� ��� ������������� ���������� �������� � ������������ �� ����� � �������� ��������� 
   //InitDataT ������ Run � ������� � �������� �������� NullType
   template <class InitDataT>
   TClientConnectionBasic(const InitDataT &InitData) {}

   //����������� � ��� ��� ���������� �����������
   //���������� ��������� ������ � �������, ��� ������ ������� SendRequest_*
   void OnConnect() {}

   //����������� � ��� ��� ���������� ���������.
   void OnDisconnect() {}

   //����������� � ��� ��� �������� ������ �������� �������� ������ ��� ������
   //����� ������� TryDisconnect, ��� ���������� ���������� ��� ��������� ������ ��������,
   //�������� ���������� ��������� ������ KEEP ALIVE (���� �� ���� �������� �������� ������)
   void OnTimer() {}

   //����������� � ��� ��� �������� ������ �������� ����������� � �������
   //����� ������� TryDisconnect, ��� ���������� ������� ����������� ��� ��������� ������ ��������
   void OnConnectTimer() {}

   //����������� � ��� ��� ���� ��������� ��� ������� ������.
   //����� ������������ ��� ������� ��� ������ �������� �������� �������� �����/������
   void OnIOOperation() {}
   
   //����������� � ��� ��� ��� ������ ��������� ������ �������
   //������������ ����� ������� ����������� ��� ������ ������ TryDisconnect
   //� ����� ������ �� ����� �������� ������� ������� OnReceiveData
   void OnBeginResponseBodyReceive( const TResponseHeader &ResponseHeader ) { }

   //����������� � ��� ��� ��� ��������� ������ ����� �������.
   //��������� ������ ��� ������� � ������ OnBeginResponseBodyReceive, � ���� � OnReceiveData
   void OnCompleateResponse() { }

   //����������� � ��� ��� �� ������ ���� ������� ��������� ������ ������ ���� ������ �������
   //����� ���� ��� ��� ������ ����� ������� ��������� ����� OnCompleateResponse
   //Length ����� ���� = 0
   void OnReceiveData( const char *pBegin, int Length ) {}

   //����������� � ��� ��� �� ������������ ��������� ��������� ������ ������ ��� ���� ������
   //�����: true -  ��������� pBegin ��������� �� ������ ������� ���������� ��������, 
   //               � Length �� ����� ���� ������ (������ ���� > 0) 
   //       false - ���������� ������ �� ���� (� ���������� ������ ������� ���� �������� 
   //               ��������� ������ ������)
   //������ ������ �������� ����� ������� ������ ������� ���� ������� � ��������� Header.GetContentLength()
   //������� SendRequest_POST
   bool OnSendData( const char *&pBegin, int &Length ) { return false; }

   //���������� ������� ���������� � ����� ����� ���� ��� ���������� ���������
   ~TClientConnectionBasic() {}

   ///////////////////////////////////////////////////////////////////////////////
   // ������� ������������ � ����������� ����� �������������� ������, ����� ����� 
   // ������������ � �������� ��� �������
   ///////////////////////////////////////////////////////////////////////////////

   //����������� � ��� ��� ������� ���� ���������� ��������� ������ ������.
   //������� ��������� ������� �� ������ ������� "�����" ������ �� ������� � �������
   void OnRawDataSend( const char *pBegin, int Length ) {}

   //����������� � ��� ��� �� ������� ���� ������� ��������� ������ ������.
   //������� ��������� ������� �� ������ ������� "�����" ������ �� ������� � �������
   void OnRawDataReceive( const char *pBegin, int Length ) {}

   //����������� � ��� ��� �� ������� ���� �������� ������ ������ ���������
   //��� ������ ��������� ����� ���������� ������� ������ ���������� � ����������� 
   //���������� ����� �������� � TResponseHeader
   //    [pBeginMethod, pEndMethod) - �������� HTTP ������
   //    [pBeginURL, pEndURL) - URL �� ������� �������� ������
   void OnStartHeaderLine( const char *pBeginMethod, const char *pEndMethod, const char *pBeginURL, const char *pEndURL ) {}

   //����������� � ��� ��� �� ������� ���� �������� ������ � ����� ������ ���������
   //��� ������ ��������� ����� ���������� ������ ���������� � ����������� ���������� ����� ��������
   //� TResponseHeader
   //    [pBeginFieldName, pEndFieldName) - ��� ���� ���������
   //    [pBeginFieldValue, pEndFieldValue) - �������� ���� ���������
   //������� � ���� � OnStartHeaderLine ����� ���� ������������ ��� ����������� ���������� ����� ������� ������ ����� �������� �
   //���������
   void OnHeaderLine( const char *pBeginFieldName, const char *pEndFieldName, const char *pBeginFieldValue, const char *pEndFieldValue ) {}
};
} //namespace HTTP
} //namespace NWLib





#endif