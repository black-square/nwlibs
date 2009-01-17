#ifndef HTTPBasicConnection_HPP
#define HTTPBasicConnection_HPP

///////////////////////////////////////////////////////////////////////////////
// ���������� �������� ������ ��� THTTPClient::TConnection � THTTPServer::TConnection
// � ������� ����������� ������ ������� ��� ������� � �������
///////////////////////////////////////////////////////////////////////////////
namespace NWLib
{
namespace Detail
{
///////////////////////////////////////////////////////////////////////////////
// ����� ������ � ����������� TServerTraits � TClientTraits
///////////////////////////////////////////////////////////////////////////////
template< class ConnectionT >
class TTraitsBasic
{
protected:
   //����� �������� ������� TStringParseFunc
   enum TStringParseResult
   {
      SPRContinue,            //���������� ��������� ����� � ��� �� ���������� ������      
      SPRChangeState,         //���������� ��������� ������ ���������� ����� �� ������� TOnReceiveFunc, �� ������ �� ���������      
      SPRStopReading          //���������� �������� ������      
   };

   //��� ������� ��������� ��� ������� ������ ������� ������ ���������
   //����� ���������� � ��������� ������ ��� ��������� � ������� ������� ��������� (��� \r\n)
   //������� ������ ��������� ���������� � � �������� �������� ������� ������� ��������� ����� ���������
   //��� ���� ��� ����� ������.
   typedef TStringParseResult (ConnectionT::*TStringParseFunc)( const char *pBegin, const char *pEnd );

   //��� ������� ��� ������� �� ������� �������� ������
   //pBuf   - ������ �� ��������� �� ������ ��� ��������� �������� ������
   //Length - ������ �� ������ ������ ��� ��������� �������� ������
   //SendSize - ���������� ������������ ���� (0, Length] 
   //�����  - true:  ���������� ��������� ��������� �������� ������ c ����������� pBuf � Length, 
   //                ����� ��������� ������� ����� ����� ������ OnSend
   //         false: ���������� ��������� ������ 
   typedef bool (ConnectionT::*TOnSendFunc)( int SendSize, const char *&pBuf, int &Length );
};


///////////////////////////////////////////////////////////////////////////////
// �������� ������� TraitsT ��� THTTPBasicConnection
// ���������������� �������
///////////////////////////////////////////////////////////////////////////////
template< class ConnectionT >
class TServerTraits: public TTraitsBasic< ConnectionT >
{
protected:
   //������� ����� ��������� ����������� ������ ���������
   enum TStringParseMode
   {
      SPMFirstLine,               //������ ������ ������ �������
      SPMGETHeader,               //������ ��������� ����� ������� GET
      SPMPOSTHeader,              //������ ��������� ����� ������� POST

      SPMCount
   };

   //������� ����� ������ ������
   enum TSendMode
   {
      SMSendFromBufferOnly,              //�������� ������ ��������� � ��������� � ����� ������
      SMSendFromBufferAndDisconnect,     //�������� ������ ��������� � ��������� ����������
      SMSendFromBufferAndThenBody,       //��������� ��������� � ����� ����� ������� � ����� SMSendBody
      SMSendBody,                        //��������� ���� ������ (������ � ���������� ConnectionT) � ����� ��������� � ����� ������

      SMCount
   };

   typedef HTTP::TRequestHeader TReceiveHeader;

protected:
   static const TStringParseFunc StringParseFunc[SPMCount];  //������������ �������� ������ ��������� ����� ��������� � ������� ����� ������
   static const TOnSendFunc OnSendFunc[SMCount];             //������������ �������� ������ �������� ������ � ������� ����� ������

protected:
   void OnCompleateReceive() { static_cast<ConnectionT *>(this)->OnCompleatePostRequest(); }
   void OnSendBufferToSmall() { static_cast<ConnectionT *>(this)->SendBufferToSmall(); }
};


///////////////////////////////////////////////////////////////////////////////
// �������� ������� TraitsT ��� THTTPBasicConnection
// ���������������� �������
///////////////////////////////////////////////////////////////////////////////
template< class ConnectionT >
class TClientTraits: public TTraitsBasic< ConnectionT >
{
protected:
   //������� ����� ��������� ����������� ������ ������
   enum TStringParseMode
   {
      SPMFirstLine,         //������ ������ ������ ������
      SPMOtherLines,        //������ ��������� ����� ������

      SPMCount
   };


   //������� ����� ������ ������
   enum TSendMode
   {
      SMSendGETHeader,          //�������� ������ ��������� ������� GET � ��������� � ����� ������
      SMSendPOSTGeader,         //��������� ��������� ������� POST � ����� ����� ������� � ����� SMSendBody, ��� �������� ����
      SMSendBody,               //��������� ���� ������� (������ � ���������� ConnectionT) � ������� � ����� ������

      SMCount
   };

   typedef HTTP::TResponseHeader TReceiveHeader;

protected:
   static const TStringParseFunc StringParseFunc[SPMCount];  //������������ �������� ������ ��������� ����� ��������� � ������� ����� ������
   static const TOnSendFunc OnSendFunc[SMCount];             //������������ �������� ������ �������� ������ � ������� ����� ������

protected:
   void OnCompleateReceive() { static_cast<ConnectionT *>(this)->OnCompleatePostRequest(); }
   void OnSendBufferToSmall() { static_cast<ConnectionT *>(this)->SendBufferToSmall(); }
};


///////////////////////////////////////////////////////////////////////////////
// ����� ���������� �� �������� ����������� THTTPClient::TConnection � THTTPServer::TConnection
// ��� ����� ��� ���������� �������� ����.
// ConnectionT     - THTTPClient::TConnection ��� THTTPServer::TConnection
// TraitsT         - ���� � ������� ������� ����������� � ������� � �������
// UserConnectionT - ������� ����� ���������� ������� ������� ������������ � ��������� ������� ConnectionT
//                   ������ THTTPClient::TConnection � THTTPServer::TConnection
///////////////////////////////////////////////////////////////////////////////
template<class ConnectionT, template<class> class TraitsT, class UserConnectionT, class FuncImplT>
class THTTPBasicConnection: public TraitsT<ConnectionT>, public UserConnectionT, public AsyncIOConnection::TConnectionBasic<FuncImplT>
{
protected:
   typedef TraitsT<ConnectionT>                                TTraits;
   typedef TReceiveBuf<UserConnectionT::ICReceiveBufferSize>   TReceiveBuf;
   typedef TSendBuf<>                                          TSendBuf;
   typedef TraitsT<ConnectionT>                                TTraits;
   typedef typename TTraits::TStringParseMode                  TStringParseMode;
   typedef typename TTraits::TSendMode                         TSendMode;
   typedef typename TTraits::TReceiveHeader                    TReceiveHeader;
   typedef typename TTraits::TStringParseResult                TStringParseResult;

protected:
   //������� ����� ������ �������
   enum TReceiveMode
   {
      RMReadString,        //��������� ����������� ������ �� ������ � �������� �� ������� ������� ��������� �����
      RMReadBody,          //�������� �������� ������ ������������ ������ ConnectionT.
                           //� ����� ��������� ��������� ����� m_RequestHeader.GetContentLength() ����
      RMCount
   };

   //��� ������� ��� ������� �� ������� ������
   //��� ����� ������ ��� ������������ � m_ReceiveBuf. ������� ������ ���������� ��� ������ � ����������� 
   //����� ��� ����� ����� ������. ������ ��������� ������������ ���������� ������ � ������.
   //�����: ����� �� ���������� ������ ������
   typedef bool (THTTPBasicConnection::*TOnReceiveFunc)();

protected:
   static const TOnReceiveFunc OnReceiveFunc[RMCount];       //������������ �������� ������ ������ � ������� ����� ������

protected:
   TReceiveBuf m_ReceiveBuf;                               //����� ������
   TSendBuf m_SendBuf;                                     //����� ������
   TReceiveMode m_CurReceiveMode;                          //������� ����� ������
   TStringParseMode m_CurStringParseMode;                  //������� ����� ��������� ����� ���������
   TSendMode m_CurSendMode;                                //������� ����� ��������
   TReceiveHeader m_ReceiveHeader;                         //������� ��������� ����������� ������
   TSendMode m_SendMode;                                   //����� ����� ������
   size_t m_NeadToReceive;                                 //���������� ������ ������� ��� ���������� ������� � ���� �������

protected:
   //�������� �� ������ ����������
   static bool IsSpace( char Ch ) { return Ch == ' ' || Ch == '\t'; }   

   //������������ ������ � ��������� ������ (���� �������� ��������) � m_ReceiveHeader
   //�����: true ����� ���������� ��������� ��������� � false � ������ ������������� �������
   bool AddStringToHeader( const char *pBegin, const char *pEnd );

protected:
   //��������� ����������� �� �������� ������
   bool OnSend( int SendSize, const char *&pBuf, int &Length );

   //��������� ����������� � ����� ������
   bool OnReceive( int ReceiveSize, char *&pBuf, int &Length );

protected:
   //������� TOnReceiveFunc, ��������� RMReadString
   //��������� ������� ������ �� ������ � �������  � ������� ������� ��������� �����
   bool OnReceiveReadString();

   //������� TOnReceiveFunc, ��������� RMReadRequestBody
   //������� �������� ������ ������ ���������� �� ConnectionT
   bool OnReceiveBody();

protected:
   template<class InitDataT>
   THTTPBasicConnection( const InitDataT &InitData ): UserConnectionT(InitData) {} 
};
///////////////////////////////////////////////////////////////////////////////

template<class ConnectionT, template<class> class TraitsT, class UserConnectionT, class FuncImplT>
bool THTTPBasicConnection<ConnectionT, TraitsT, UserConnectionT, FuncImplT>::AddStringToHeader( const char *pBegin, const char *pEnd )
{
   const char *pBeginFieldName;     //������ ����� ����
   const char *pEndFieldName;       //����� ����� ����
   const char *pBeginFieldValue;    //������ �������� ����
   const char *pEndFieldValue;      //����� �������� ����

   //������� ��� ����
   //���������� �������
   while( pBegin != pEnd && IsSpace(*pBegin) ) ++pBegin;
   if( pBegin == pEnd ) { return false; }

   pBeginFieldName = pBegin++;

   //���� ':'
   while( pBegin != pEnd && *pBegin != ':' ) ++pBegin;
   if( pBegin == pEnd ) { return false; }

   pEndFieldName = pBegin++;

   //���������� �������
   while( pBegin != pEnd && IsSpace(*pBegin) ) ++pBegin;
   if( pBegin == pEnd ) { return false; }

   //��������� ��� �� ������� � ����� ��������
   while( pBegin != pEnd && IsSpace(*(pEnd - 1)) ) --pEnd;

   pBeginFieldValue = pBegin;
   pEndFieldValue = pEnd;

   std::string FieldName(pBeginFieldName, pEndFieldName);

   ToLower(FieldName, FieldName);

   if( FieldName.compare( "content-length" ) == 0 )
   {
      size_t Val;

      if( pBeginFieldValue == pEndFieldValue || ConvertStringToInteger(pBeginFieldValue, pEndFieldValue, Val) != pEndFieldValue )
         return false;

      m_ReceiveHeader.SetContentLength(Val);
   }

   static_cast<ConnectionT *>(this)->OnHeaderLine( pBeginFieldName, pEndFieldName, pBeginFieldValue, pEndFieldValue );

   return true;
}
///////////////////////////////////////////////////////////////////////////////

template<class ConnectionT, template<class> class TraitsT, class UserConnectionT, class FuncImplT>
bool THTTPBasicConnection<ConnectionT, TraitsT, UserConnectionT, FuncImplT>::OnSend( int SendSize, const char *&pBuf, int &Length )
{    
   APL_ASSERT( SendSize > 0 && SendSize <= Length );

   static_cast<ConnectionT *>(this)->OnIOOperation(); 
   static_cast<ConnectionT *>(this)->OnRawDataSend( pBuf, SendSize );

   if( (static_cast<ConnectionT *>(this)->*OnSendFunc[m_CurSendMode])(SendSize, pBuf, Length) )
   {
      APL_ASSERT_PTR( pBuf );
      APL_ASSERT( Length > 0 );
      return true;
   }

   //�� ��������� �������� �������
   if( !static_cast<ConnectionT *>(this)->IsDisconnecting() )
   {
      //����� �������������� ���� �����
      m_CurReceiveMode = RMReadString;
      m_CurStringParseMode = SPMFirstLine;

      //� ��� � ������� ������ ����� �������� ������
      if( (static_cast<ConnectionT *>(this)->*OnReceiveFunc[m_CurReceiveMode])() )
      {
         char *pReceiveBuf;
         int ReceiveLength;

         m_ReceiveBuf.BeforeRecieveData( pReceiveBuf, ReceiveLength );
         static_cast<ConnectionT *>(this)->TryReceive( pReceiveBuf, ReceiveLength );
      }
   }

   return false;
}
///////////////////////////////////////////////////////////////////////////////

template<class ConnectionT, template<class> class TraitsT, class UserConnectionT, class FuncImplT>
bool THTTPBasicConnection<ConnectionT, TraitsT, UserConnectionT, FuncImplT>::OnReceive( int ReceiveSize, char *&pBuf, int &Length )
{
   APL_ASSERT( ReceiveSize > 0 && ReceiveSize <= Length );

   static_cast<ConnectionT *>(this)->OnIOOperation(); 
   static_cast<ConnectionT *>(this)->OnRawDataReceive( pBuf, ReceiveSize );

   m_ReceiveBuf.AfterRecieveData( pBuf, ReceiveSize );

   if( (static_cast<ConnectionT *>(this)->*OnReceiveFunc[m_CurReceiveMode])() )
   {
      m_ReceiveBuf.BeforeRecieveData( pBuf, Length );

      return true;
   }

   return false;
}
///////////////////////////////////////////////////////////////////////////////

template<class ConnectionT, template<class> class TraitsT, class UserConnectionT, class FuncImplT>
const typename THTTPBasicConnection<ConnectionT, TraitsT, UserConnectionT, FuncImplT>::TOnReceiveFunc 
THTTPBasicConnection<ConnectionT, TraitsT, UserConnectionT, FuncImplT>::OnReceiveFunc[RMCount] =
{
   /* RMReadString */  &THTTPBasicConnection<ConnectionT, TraitsT, UserConnectionT, FuncImplT>::OnReceiveReadString,
   /* RMReadBody   */  &THTTPBasicConnection<ConnectionT, TraitsT, UserConnectionT, FuncImplT>::OnReceiveBody
};
///////////////////////////////////////////////////////////////////////////////

template<class ConnectionT, template<class> class TraitsT, class UserConnectionT, class FuncImplT>
bool THTTPBasicConnection<ConnectionT, TraitsT, UserConnectionT, FuncImplT>::OnReceiveReadString() 
{
   char *pDelim;
   char *pTmp;

   TStringParseResult StringParseResult;

   for(;;)
   {
      //���� �����������
      pDelim = std::search( m_ReceiveBuf.Begin(), m_ReceiveBuf.End(), TSendBuf::szStringDelim, TSendBuf::szStringDelim + TSendBuf::StringDelimSize );

      if( pDelim == m_ReceiveBuf.End() ) 
      {
         //�� �� ����� �����������
         if( !m_ReceiveBuf.PrepareForReceiveOnlyEmpty() )
         {
            TTraits::OnSendBufferToSmall(); //������ ��������� �������� 
            return false; 
         }

         return true;
      }

      //�� ����� �����������
      pTmp = m_ReceiveBuf.Begin();
      m_ReceiveBuf.PopFront( static_cast<int>(pDelim - m_ReceiveBuf.Begin() + TSendBuf::StringDelimSize) );

      StringParseResult = (static_cast<ConnectionT *>(this)->*StringParseFunc[m_CurStringParseMode])(pTmp, pDelim); 

      if( StringParseResult == SPRChangeState )
         return true;

      if( StringParseResult == SPRStopReading )
         return false;

      APL_ASSERT( StringParseResult == SPRContinue );
   }
}
///////////////////////////////////////////////////////////////////////////////

template<class ConnectionT, template<class> class TraitsT, class UserConnectionT, class FuncImplT>
bool THTTPBasicConnection<ConnectionT, TraitsT, UserConnectionT, FuncImplT>::OnReceiveBody()
{
   //GetFromBuffer ����� ���� ����� 0
   size_t GetFromBuffer = std::min( m_NeadToReceive, static_cast<size_t>(m_ReceiveBuf.Size()) );

   static_cast<ConnectionT *>(this)->OnReceiveData( static_cast<const char *>(m_ReceiveBuf.Begin()), static_cast<int>(GetFromBuffer) );
   m_ReceiveBuf.PopFront( static_cast<int>(GetFromBuffer) );

   m_NeadToReceive -= GetFromBuffer;

   if( m_NeadToReceive == 0 )  //�� ��������� ��������� ���� �������
   {
      TTraits::OnCompleateReceive(); 
      return false;
   }

   //������ ������
   m_ReceiveBuf.PrepareForReceiveFull();

   return true;
}
///////////////////////////////////////////////////////////////////////////////


} //namespace Detail
} //namespace NWLib



#endif