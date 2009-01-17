#ifndef HTTPServerConnectionStateFuction_HPP
#define HTTPServerConnectionStateFuction_HPP

///////////////////////////////////////////////////////////////////////////////
// ����������� ������� ������ THTTPServer::TConnection ������� ��������� � 
// ��������� ������� ������ ��������, ������������� � ������ ������ �������
///////////////////////////////////////////////////////////////////////////////
namespace NWLib
{
template< class ConnectionT >
const typename Detail::TServerTraits< ConnectionT >::TStringParseFunc 
Detail::TServerTraits< ConnectionT >::StringParseFunc[SPMCount] = 
{
   /* SPMFirstLine  */    &ConnectionT::ParseFirstHeaderString,
   /* SPMGETHeader  */    &ConnectionT::ParseGetHeaderString,  
   /* SPMPOSTHeader */    &ConnectionT::ParsePostHeaderString 
};
///////////////////////////////////////////////////////////////////////////////

template< class ConnectionT >
const typename Detail::TServerTraits< ConnectionT >::TOnSendFunc 
Detail::TServerTraits< ConnectionT >::OnSendFunc[SMCount] =
{
   /* SMSendFromBufferOnly */              &ConnectionT::OnSendFromBufferOnly,
   /* SMSendFromBufferAndDisconnect */     &ConnectionT::OnSendFromBufferAndDisconnect,
   /* SMSendFromBufferAndThenBody */       &ConnectionT::OnSendFromBufferAndThenBody,
   /* SMSendBody */                        &ConnectionT::OnSendBody
};
///////////////////////////////////////////////////////////////////////////////

template< template <class FuncImplT> class ConnectionT, class GlobalDataT >
template<class FuncImplT>
typename THTTPServer<ConnectionT, GlobalDataT>::TConnection<FuncImplT>::TStringParseResult 
THTTPServer<ConnectionT, GlobalDataT>::TConnection<FuncImplT>::ParseFirstHeaderString( const char *pBegin, const char *pEnd )
{
   const char *pBeginMethod; //������ ������ �������
   const char *pEndMethod;   //����� ������ �������
   const char *pBeginURL;    //������ URL �������
   const char *pEndURL;      //����� URL �������

   m_ReceiveHeader.Clear();

   //������� ��� ������
#if 0 //����� ��������� ������ ������� ������ �� �����
   //���������� �������
   while( pBegin != pEnd && IsSpace(*pBegin) ) ++pBegin;
   if( pBegin == pEnd ) { SendErrorInHeader(); return SPRStopReading; }
#endif

   pBeginMethod = pBegin++;

   //���� ������
   while( pBegin != pEnd && !IsSpace(*pBegin) ) ++pBegin;
   if( pBegin == pEnd ) { SendErrorInHeader(); return SPRStopReading; }

   pEndMethod = pBegin++;

   //������� URL
   //���������� �������
   while( pBegin != pEnd && IsSpace(*pBegin) ) ++pBegin;
   if( pBegin == pEnd ) { SendErrorInHeader(); return SPRStopReading; }

   pBeginURL = pBegin++;

   //���� ������
   while( pBegin != pEnd && !IsSpace(*pBegin) ) ++pBegin;
   if( pBegin == pEnd ) { SendErrorInHeader(); return SPRStopReading; }

   pEndURL = pBegin++;

   //������ HTTP ������������ �� �����

   OnStartHeaderLine( pBeginMethod, pEndMethod, pBeginURL, pEndURL );

   //�� ��������� ���������� ������ ������ ���������
   //����� ��� ��� �� �����

   //�������� ������ �� ���������
   struct TMethodName2StringParseMode
   {
      const char * const MethodName;   //��� �������� � RFC 2068 ����� ������������ � ��������
      const TStringParseMode StringParseMode;   
   }; 

   static const TMethodName2StringParseMode MN2SPM[] = {
      { "POST",  SPMPOSTHeader },
      { "GET",  SPMGETHeader }
   };


   size_t i;
   std::pair<bool, const char *> StringMismatchResult;

   for( i = 0; i < APL_ARRSIZE(MN2SPM); ++i ) 
   {  
      StringMismatchResult = StringMismatch(MN2SPM[i].MethodName, pBeginMethod, pEndMethod);

      if( StringMismatchResult.first && StringMismatchResult.second == pEndMethod )
         break;
   }

   if( i == APL_ARRSIZE(MN2SPM) )
   {
      SendMethodNotImplemented();
      return SPRStopReading;
   }

   //�� ������
   m_ReceiveHeader.SetUrl( std::string(pBeginURL, pEndURL) );

   m_CurStringParseMode = MN2SPM[i].StringParseMode; //��������� � ����������� ��������� ����� ��������� 

   return SPRContinue;
}
///////////////////////////////////////////////////////////////////////////////

template< template <class FuncImplT> class ConnectionT, class GlobalDataT >
template<class FuncImplT>
typename THTTPServer<ConnectionT, GlobalDataT>::TConnection<FuncImplT>::TStringParseResult 
THTTPServer<ConnectionT, GlobalDataT>::TConnection<FuncImplT>::ParseGetHeaderString( const char *pBegin, const char *pEnd )
{
   if( pBegin == pEnd )
   {
      //�� ����� ����� ���������   
      TBase::OnCompleateGetRequest(m_ReceiveHeader);

      //����� ������������ ������ ��������� �����-���� ������
      return SPRStopReading;
   }

   if( !AddStringToHeader(pBegin, pEnd) )
   {
      SendErrorInHeader();
      return SPRStopReading;
   }

   return SPRContinue;
}
///////////////////////////////////////////////////////////////////////////////

template< template <class FuncImplT> class ConnectionT, class GlobalDataT >
template<class FuncImplT>
typename THTTPServer<ConnectionT, GlobalDataT>::TConnection<FuncImplT>::TStringParseResult 
THTTPServer<ConnectionT, GlobalDataT>::TConnection<FuncImplT>::ParsePostHeaderString( const char *pBegin, const char *pEnd )
{
   if( pBegin == pEnd )
   {
      //�� ����� ����� ���������     
      if( m_ReceiveHeader.GetContentLength() == HTTP::TRequestHeader::ErrorContentLength() )  //����������� ������ ���� ������� ���� content-length
      {
         SendLengthRequired();
         return SPRStopReading;
      }

      //���������� ������������ ����� �� �� ��������� ���� �������
      HTTP::TErrorCode EC = TBase::OnBeginPostBodyReceive( m_ReceiveHeader );

      if( EC != HTTP::ECOk )
      {
         SendErrorCode( EC );
         return SPRStopReading;
      }

      m_NeadToReceive = m_ReceiveHeader.GetContentLength();

      //�������� ��������� �����
      m_CurReceiveMode = RMReadBody;

      //��������� ����������, � ����� ����� ���� �� ������ (� ����� � ������), 
      //���� �� ���� ��������� ������
      if( (this->*OnReceiveFunc[m_CurReceiveMode])() )
         return SPRChangeState;

      //������ ��������� �������� ������������� ������ �� ������
      //������������ ������ ��� �� ���������

      return SPRStopReading;
   }

   if( !AddStringToHeader(pBegin, pEnd) )
   {
      SendErrorInHeader();
      return SPRStopReading;
   }

   return SPRContinue;
}
///////////////////////////////////////////////////////////////////////////////

template< template <class FuncImplT> class ConnectionT, class GlobalDataT >
template<class FuncImplT>
bool THTTPServer<ConnectionT, GlobalDataT>::TConnection<FuncImplT>::OnSendFromBufferOnly( int SendSize, const char *&pBuf, int &Length )
{
   if( SendSize != Length )
   {
      pBuf += SendSize;
      Length -= SendSize;

      return true;
   }

   return false;
}
///////////////////////////////////////////////////////////////////////////////

template< template <class FuncImplT> class ConnectionT, class GlobalDataT >
template<class FuncImplT>
bool THTTPServer<ConnectionT, GlobalDataT>::TConnection<FuncImplT>::OnSendFromBufferAndDisconnect( int SendSize, const char *&pBuf, int &Length )
{
   if( SendSize != Length )
   {
      pBuf += SendSize;
      Length -= SendSize;

      return true;
   }

   //����������� �� ����������

   //DEBUG_MSG( "��������� ��� ������, ����������� " );
   TServerConnection::EndSend();
   TServerConnection::TryDisconnect();

   return false;
}
///////////////////////////////////////////////////////////////////////////////

template< template <class FuncImplT> class ConnectionT, class GlobalDataT >
template<class FuncImplT>
bool THTTPServer<ConnectionT, GlobalDataT>::TConnection<FuncImplT>::OnSendFromBufferAndThenBody( int SendSize, const char *&pBuf, int &Length )
{
   if( SendSize != Length )
   {
      pBuf += SendSize;
      Length -= SendSize;

      return true;
   }

   m_CurSendMode = SMSendBody;

   //������������ ������ ��� �� ��������� ������ � ������ ���
   APL_CHECK( TBase::OnSendData(pBuf, Length) );

   APL_ASSERT( static_cast<int>(m_NeadToReceive) >= Length );
   APL_DEBUG_OPERATION( m_NeadToReceive -= Length ); 

   return true;
}
///////////////////////////////////////////////////////////////////////////////

template< template <class FuncImplT> class ConnectionT, class GlobalDataT >
template<class FuncImplT>
bool THTTPServer<ConnectionT, GlobalDataT>::TConnection<FuncImplT>::OnSendBody( int SendSize, const char *&pBuf, int &Length )
{
   if( SendSize != Length )
   {
      pBuf += SendSize;
      Length -= SendSize;

      return true;
   }

   if( !TBase::OnSendData(pBuf, Length) )
   {
      //�� ��������� ��� ������ ������������

      //��������� ��� ������������ ������ ����� ������� ������ ������� ������ � ��������� Header.GetContentLength()
      //������� SendHeaderWithBody
      APL_ASSERT( m_NeadToReceive == 0 );

      return false;
   }

   APL_ASSERT( static_cast<int>(m_NeadToReceive) >= Length );
   APL_DEBUG_OPERATION( m_NeadToReceive -= Length ); 

   return true;
}
///////////////////////////////////////////////////////////////////////////////

} //namespace NWLib

#endif