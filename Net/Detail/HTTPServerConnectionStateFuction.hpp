#ifndef HTTPServerConnectionStateFuction_HPP
#define HTTPServerConnectionStateFuction_HPP

///////////////////////////////////////////////////////////////////////////////
// Определение методов класса THTTPServer::TConnection которые относятся к 
// различным режимам работы автомата, используемого в логике работы сервера
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
   const char *pBeginMethod; //Начало метода запроса
   const char *pEndMethod;   //Конец метода запроса
   const char *pBeginURL;    //Начало URL запроса
   const char *pEndURL;      //Конец URL запроса

   m_ReceiveHeader.Clear();

   //Находим имя метода
#if 0 //Перед названием метода пробелы стоять не могут
   //Пропускаем пробелы
   while( pBegin != pEnd && IsSpace(*pBegin) ) ++pBegin;
   if( pBegin == pEnd ) { SendErrorInHeader(); return SPRStopReading; }
#endif

   pBeginMethod = pBegin++;

   //Ищем пробел
   while( pBegin != pEnd && !IsSpace(*pBegin) ) ++pBegin;
   if( pBegin == pEnd ) { SendErrorInHeader(); return SPRStopReading; }

   pEndMethod = pBegin++;

   //Находим URL
   //Пропускаем пробелы
   while( pBegin != pEnd && IsSpace(*pBegin) ) ++pBegin;
   if( pBegin == pEnd ) { SendErrorInHeader(); return SPRStopReading; }

   pBeginURL = pBegin++;

   //Ищем пробел
   while( pBegin != pEnd && !IsSpace(*pBegin) ) ++pBegin;
   if( pBegin == pEnd ) { SendErrorInHeader(); return SPRStopReading; }

   pEndURL = pBegin++;

   //Версию HTTP обрабатывать не будем

   OnStartHeaderLine( pBeginMethod, pEndMethod, pBeginURL, pEndURL );

   //Мы корректно распознали первую строку заголовка
   //Узнаём что это за метод

   //Название метода на состояние
   struct TMethodName2StringParseMode
   {
      const char * const MethodName;   //Как написано в RFC 2068 метод чувствителен к регистру
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

   //Всё удачно
   m_ReceiveHeader.SetUrl( std::string(pBeginURL, pEndURL) );

   m_CurStringParseMode = MN2SPM[i].StringParseMode; //Переходим к распознанию остальных строк заголовка 

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
      //Мы нашли конец заголовка   
      TBase::OnCompleateGetRequest(m_ReceiveHeader);

      //Здесь пользователь обязан отправить какие-либо данные
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
      //Мы нашли конец заголовка     
      if( m_ReceiveHeader.GetContentLength() == HTTP::TRequestHeader::ErrorContentLength() )  //Обязательно должно быть указано поле content-length
      {
         SendLengthRequired();
         return SPRStopReading;
      }

      //Спрашиваем пользователя будет ли он принимать тело запроса
      HTTP::TErrorCode EC = TBase::OnBeginPostBodyReceive( m_ReceiveHeader );

      if( EC != HTTP::ECOk )
      {
         SendErrorCode( EC );
         return SPRStopReading;
      }

      m_NeadToReceive = m_ReceiveHeader.GetContentLength();

      //Изменяем состояние приёма
      m_CurReceiveMode = RMReadBody;

      //Состояние изменилось, а буфер может быть не пустым (а может и пустым), 
      //пока не надо принимать данные
      if( (this->*OnReceiveFunc[m_CurReceiveMode])() )
         return SPRChangeState;

      //Смогли полностью получить запрашевыемые данные из буфера
      //Пользователь должен что то отправить

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

   //Отключаемся от соединения

   //DEBUG_MSG( "Отправлен код ошибки, отключаемся " );
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

   //Пользователь должен что то отправить хотябы в первый раз
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
      //Мы отправили все данные пользователя

      //Проверяем что пользователь послал ровно столько данных сколько указал в параметре Header.GetContentLength()
      //функции SendHeaderWithBody
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