#ifndef HTTPServerInterfaces_HPP
#define HTTPServerInterfaces_HPP

///////////////////////////////////////////////////////////////////////////////
// ���� �������� �������� ����������� ������� ��� HTTP �������
///////////////////////////////////////////////////////////////////////////////
namespace NWLib
{
namespace HTTP
{
///////////////////////////////////////////////////////////////////////////////
// ����� �� �������� ����������� TServerConnectionBasic. ������ ��� ��������������� 
// ������� ���������� ������� ������ HTTP �������.
// ������������ � ������ ����������� �� TServerConnectionBasic ������ �������� ������
// ������� ������ ��� �������� � �������� ������, � ����� ��� ���������� �����������
///////////////////////////////////////////////////////////////////////////////
template< class FuncImplT >
struct TServerCallRedirection
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

   //�������� ������ �� ����������� �� TServerGlobalDataBasic ������ ������� ������� � ��������� ������� 
   //GlobalDataT ������ HTTP �������
   TUserGlobalData &GetGlobalData() { return static_cast<TConnection *>(this)->GetGlobalData(); }
   const TUserGlobalData &GetGlobalData() const { return static_cast<const TConnection *>(this)->GetGlobalData(); }

   //���������� ����� �������� ������/������ ������� � �������������.
   //����� ��������������� ��������� TimerInfinityVal() ��� ������������ ��������
   void SetWaitableTimer( DWORD dwMilliseconds )
   {
      static_cast<TConnection *>(this)->SetWaitableTimer(dwMilliseconds);
   }

   //��������� ��������� ����� ����������. ����� ����� ������ ��� ���������� ����
   //��������� � ������� ������� ����������, ��� ���� �� ��������� ���� ���������� �������� ����������.
   //GlobalData ����� ������������ ��� ������� �����������
   //��� ���� ����� ������� ���������� ����� ��������� ���� � ������������ ������ ���������� ��
   //TServerConnectionBasic ������� ������ ������� (���� �������� � ������ OnConnection ��� ��� ��� ����, 
   //�������� �������� ���������� ����� ���������� ��-�� ���������������)
   void StopWaitConnection()
   {
      static_cast<TConnection *>(this)->StopWaitConnection();
   }

   //��������� ��� ������ ������� � �������� ������ � ������� ����������
   //��� ���� EC �� ����� ���� ����� ECOk
   //������� ����� ���������� ������ � ����������� ������� OnCompleate...Request
   //� �� � ����� ���� ������ ���������
   void SendErrorCode( TErrorCode EC )
   {
      static_cast<TConnection *>(this)->SendErrorCode( EC );
   }

   //��������� ����� ������� ��������� ������ �� ��������� (����� �� �������� ����).
   //��� ���� Header.GetContentLength() ������ ���� ����� TResponseHeader::ErrorContentLength()
   //������� ����� ���������� ������ � ����������� ������� OnCompleate...Request
   //� �� � ����� ���� ������ ���������
   void SendHeader( TErrorCode EC, const TResponseHeader &Header )
   {
      static_cast<TConnection *>(this)->SendHeader( EC, Header );
   }

   //��������� ����� ������� ������� ������� �� ��������� � ����.
   //��� ���� Header.GetContentLength() ������ ����� ��������������� ������� ���� ������
   //������� ����������� ��������.
   //����������� � ��� ��� ���������� �������� ��������� ������ ������ ����� ���������
   //��� ������ ������ OnSendData
   //������� ����� ���������� ������ � ����������� ������� OnCompleate...Request
   //� �� � ����� ���� ������ ���������
   void SendHeaderWithBody( TErrorCode EC, const TResponseHeader &Header )
   {
      static_cast<TConnection *>(this)->SendHeaderWithBody( EC, Header );
   }
};


///////////////////////////////////////////////////////////////////////////////
// ������� ����� ��� ��������� ����������� ������� ������ ����� ����� ������������.
// ����� HTTP ������ ������� ��������� ���������� � ��������� ������� GlobalDataT - �����
// ������� ������ �������� ����������� TServerGlobalDataBasic � ����� ������� ������ �� 
// ���� ����� ������ ����������. 
// ����������� ����� ����� ��������� ����� ����� TServerGlobalDataBasic.
// HTTP ������ �� ��������� ����� ���� ������������� ��� ������� � GlobalDataT
// ������������ ��� ������ ��������� �� ����.
// ����� ������� �����, ������� ����� ������������� �� ������ HTTP �������, � � 
// ����������� �� TServerGlobalDataBasic ��������� ��������� this �� ����� �����.
///////////////////////////////////////////////////////////////////////////////
class TServerGlobalDataBasic
{
public:   
   //����������� �� ��������� ���������� ������������� ������ HTTP ������� ��� ����������
   TServerGlobalDataBasic() {}

   //� ����������� ����� �������� ����� ����������� ������������ �������� �� ��������� ������
   //���������� ������������ ������ �������
   //���������������� ����������� ����� �� ���� ��������� ���� ���� ��������.
   template <class InitDataT>
   TServerGlobalDataBasic(const InitDataT &InitData) {}

   //���������� ��������� � ����������� ������ �������
   ~TServerGlobalDataBasic() {}
};


///////////////////////////////////////////////////////////////////////////////
// ������� ����� ����������.
// ����� HTTP ������� ������ ��� ����������� ������ ���������� ����� ����������� 
// �� TServerConnectionBasic � ���������� � ��������� ConnectionT.
// ����������� ����� ������ ���� �������� ���������������� ���� �������� ������
// TServerConnectionBasic (������� ������ ������� �������� ������� �� ������������ � 
// ����������� �������)
// ��� �������� ���������� ����� �����������.
///////////////////////////////////////////////////////////////////////////////
template< class FuncImplT >
class TServerConnectionBasic: public TServerCallRedirection<FuncImplT>
{
public:
   //��������� ������������� ������� ����� ���� �������������� � ����������� �������
   enum TInitConsts
   {
      //������ ������ ������������� ��� ����� ������. ���� ������ ��������� ����� �������
      //������� ������, �� ������� ����� ����� ��� ������ 400 (����������� ������, Bad Request)
      //� ���������� ���������.
      ICReceiveBufferSize = 1024
   };

protected:
   //����������� �� ��������� ������� �� ����� ������, �������������� ��� �� ����� �����
   TServerConnectionBasic() {}

public:
   //����������� ���������� ��� �������� �����������
   //������������ ������� GetGlobalData ��� �����
   //��� �������� ���������� � ����������� ����� �������� ����� ����������� ������������ 
   //�������� �� ��������� ������ Run � ������� � ���������� InitData. 
   //���������������� ����������� ����� �� ���� ��������� ���� ��� InitDataT �������.
   //���� ��� ������������� ���������� �������� � ������������ �� ����� � �������� ��������� 
   //InitDataT ������ Run � ������� � �������� �������� NullType
   template <class InitDataT>
   TServerConnectionBasic(const InitDataT &InitData) {}

   //����������� � ��� ��� ���������� �����������
   //������ �������� ��������� ������� �������
   void OnConnect() {}

   //����������� � ��� ��� ���������� ���������.
   void OnDisconnect() {}

   //����������� � ��� ��� �������� ������ �������� �������� ������ ��� ������
   //����� ������� TryDisconnect, ��� ���������� ���������� ��� ��������� ������ ��������,
   //�������� ���������� ��������� ������ KEEP ALIVE (���� �� ���� �������� �������� ������)
   void OnTimer() {}

   //����������� � ��� ��� ���� ��������� ��� ������� ������.
   //����� ������������ ��� ������� ��� ������ �������� �������� �������� �����/������
   void OnIOOperation() {}

   //����������� � ��� ��� ��� ������ ��������� RequestHeader ������� POST
   //������������ ������ ������� ECOk, ���� �� ��������� ��������� ���� �������.
   //��� ����� ������ �� ����� �������� ������� ������� OnReceiveData;
   //����� ������ �����. �������� ������� � ���� ��� ��� ����� ���������� ������� ���
   //������ � ���������� ���������
   TErrorCode OnBeginPostBodyReceive( const TRequestHeader &RequestHeader ) { return ECInternalServerError; }

   //����������� � ��� ��� ��� ��������� ������ ������ POST.
   //��������� ������� ��� ������� � ������ OnBeginPostBodyReceive, � ���� � OnReceiveData
   //������������ ������ ������� ���� �� Send... ������� ��� ����������� �������
   void OnCompleatePostRequest() { SendErrorCode(ECInternalServerError); }

   //����������� � ��� ��� ��� ��������� ������ ������ GET
   //m_RequestHeader - ��������� �������
   //������������ ������ ������� ���� �� Send... ������� ��� ����������� �������
   void OnCompleateGetRequest( const TRequestHeader &RequestHeader ) { SendErrorCode(ECInternalServerError); }

   //����������� � ��� ��� �� ������ ���� ������� ��������� ������ ������ ���� ������� POST
   //����� ���� ��� ��� ������ ����� ������� ��������� ����� OnCompleatePostRequest
   //Length ����� ���� = 0
   void OnReceiveData( const char *pBegin, int Length ) {}

   //����������� � ��� ��� �� ������������ ��������� ��������� ������ ������ ��� ���� ������
   //�����: true -  ��������� pBegin ��������� �� ������ ������� ���������� ��������, � Length �� ����� ����
   //������(������ ���� > 0), false - ���������� ������ �� ���� (� ���������� ������ ������� ���� �������� ��������� ������ ������)
   //������ ������ �������� ����� ������� ������ ������� ���� ������� � ��������� Header.GetContentLength()
   //������� SendHeaderWithBody
   bool OnSendData( const char *&pBegin, int &Length ) { return false; }

   //���������� ������� ���������� � ����� ����� ���� ��� ���������� ���������
   ~TServerConnectionBasic() {}

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
   //��� ������ ��������� ����� ���������� ������ ���������� � ����������� ���������� 
   //����� �������� � TRequestHeader
   //    [pBeginMethod, pEndMethod) - �������� HTTP ������
   //    [pBeginURL, pEndURL) - URL �� ������� �������� ������
   void OnStartHeaderLine( const char *pBeginMethod, const char *pEndMethod, const char *pBeginURL, const char *pEndURL ) {}

   //����������� � ��� ��� �� ������� ���� �������� ������ � ����� ������ ���������
   //��� ������ ��������� ����� ���������� ������ ���������� � ����������� ���������� ����� ��������
   //� TRequestHeader
   //    [pBeginFieldName, pEndFieldName) - ��� ���� ���������
   //    [pBeginFieldValue, pEndFieldValue) - �������� ���� ���������
   //������� � ���� � OnStartHeaderLine ����� ���� ������������ ��� ����������� ���������� ����� ������� ������ ����� �������� �
   //���������
   void OnHeaderLine( const char *pBeginFieldName, const char *pEndFieldName, const char *pBeginFieldValue, const char *pEndFieldValue ) {}
};
} //namespace HTTP
} //namespace NWLib





#endif