#ifndef HTTPBasicConnection_HPP
#define HTTPBasicConnection_HPP

///////////////////////////////////////////////////////////////////////////////
// Реализация базового класса для THTTPClient::TConnection и THTTPServer::TConnection
// в котором реализуются общшие функции для сервера и клиента
///////////////////////////////////////////////////////////////////////////////
namespace NWLib
{
namespace Detail
{
///////////////////////////////////////////////////////////////////////////////
// Общие детали в реализациях TServerTraits и TClientTraits
///////////////////////////////////////////////////////////////////////////////
template< class ConnectionT >
class TTraitsBasic
{
protected:
   //Возвр значение функции TStringParseFunc
   enum TStringParseResult
   {
      SPRContinue,            //Продолжаем обработку строк в том же состояниии чтения      
      SPRChangeState,         //Изменилось состояние чтения необходимо выйти из функции TOnReceiveFunc, но чтение не прерывать      
      SPRStopReading          //Необходимо прервать чтение      
   };

   //Тип функции обработки для каждого режима разбора строки заголовка
   //После нахождении в заголовке строки она передаётся в текущую функцию обработки (без \r\n)
   //которая должна корректно обработать её и возможно поменять текущую функцию обработки строк заголовка
   //или даже сам режим чтения.
   typedef TStringParseResult (ConnectionT::*TStringParseFunc)( const char *pBegin, const char *pEnd );

   //Тип функции для каждого из режимов отправки данных
   //pBuf   - Ссылка на указатель на буффер для следующей операции записи
   //Length - Ссылка на размер данных для следующей операции записи
   //SendSize - Количество отправленных байт (0, Length] 
   //Возвр  - true:  Необходимо выполнить очередную операцию записи c параметрами pBuf и Length, 
   //                после окончания которой будет вновь вызван OnSend
   //         false: Необходимо закончить запись 
   typedef bool (ConnectionT::*TOnSendFunc)( int SendSize, const char *&pBuf, int &Length );
};


///////////////////////////////////////////////////////////////////////////////
// Параметр шаблона TraitsT для THTTPBasicConnection
// Функциональность сервера
///////////////////////////////////////////////////////////////////////////////
template< class ConnectionT >
class TServerTraits: public TTraitsBasic< ConnectionT >
{
protected:
   //Текущий режим обработки прочитанной строки заголовка
   enum TStringParseMode
   {
      SPMFirstLine,               //Чтение первой строки запроса
      SPMGETHeader,               //Чтение остальных строк запроса GET
      SPMPOSTHeader,              //Чтение остальных строк запроса POST

      SPMCount
   };

   //Текущий режим записи данных
   enum TSendMode
   {
      SMSendFromBufferOnly,              //Оправить только заголовок и вернуться в режим чтения
      SMSendFromBufferAndDisconnect,     //Оправить только заголовок и Разорвать соединение
      SMSendFromBufferAndThenBody,       //Отправить заголовок и после этого перейти в режим SMSendBody
      SMSendBody,                        //Отправить тело ответа (берётся у наследника ConnectionT) и затем вернуться в режим чтения

      SMCount
   };

   typedef HTTP::TRequestHeader TReceiveHeader;

protected:
   static const TStringParseFunc StringParseFunc[SPMCount];  //Соответствие текущего режима обработки строк заголовка и функции этого режима
   static const TOnSendFunc OnSendFunc[SMCount];             //Соответствие текущего режима передачи данных и функции этого режима

protected:
   void OnCompleateReceive() { static_cast<ConnectionT *>(this)->OnCompleatePostRequest(); }
   void OnSendBufferToSmall() { static_cast<ConnectionT *>(this)->SendBufferToSmall(); }
};


///////////////////////////////////////////////////////////////////////////////
// Параметр шаблона TraitsT для THTTPBasicConnection
// Функциональность клиента
///////////////////////////////////////////////////////////////////////////////
template< class ConnectionT >
class TClientTraits: public TTraitsBasic< ConnectionT >
{
protected:
   //Текущий режим обработки прочитанной строки ответа
   enum TStringParseMode
   {
      SPMFirstLine,         //Чтение первой строки ответа
      SPMOtherLines,        //Чтение остальных строк ответа

      SPMCount
   };


   //Текущий режим записи данных
   enum TSendMode
   {
      SMSendGETHeader,          //Оправить только заголовок запроса GET и вернуться в режим чтения
      SMSendPOSTGeader,         //Отправить заголовок запроса POST и после этого перейти в режим SMSendBody, для отправки тела
      SMSendBody,               //Отправить тело запроса (берётся у наследника ConnectionT) и перейти в режим чтения

      SMCount
   };

   typedef HTTP::TResponseHeader TReceiveHeader;

protected:
   static const TStringParseFunc StringParseFunc[SPMCount];  //Соответствие текущего режима обработки строк заголовка и функции этого режима
   static const TOnSendFunc OnSendFunc[SMCount];             //Соответствие текущего режима передачи данных и функции этого режима

protected:
   void OnCompleateReceive() { static_cast<ConnectionT *>(this)->OnCompleatePostRequest(); }
   void OnSendBufferToSmall() { static_cast<ConnectionT *>(this)->SendBufferToSmall(); }
};


///////////////////////////////////////////////////////////////////////////////
// Класс соединения от которого наследуются THTTPClient::TConnection и THTTPServer::TConnection
// Был введён для исключения повторов кода.
// ConnectionT     - THTTPClient::TConnection или THTTPServer::TConnection
// TraitsT         - Типы и функции которые различаются в клиенте и сервере
// UserConnectionT - Базовый класс соединения который передал пользователь в параметре шаблона ConnectionT
//                   класса THTTPClient::TConnection и THTTPServer::TConnection
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
   //Текущий режим чтения сервера
   enum TReceiveMode
   {
      RMReadString,        //Разбиение поступивших данных на строки и передача их текущей функции обработки строк
      RMReadBody,          //Передача принятых данных пользователю классу ConnectionT.
                           //В общей сложности передаётся ровно m_RequestHeader.GetContentLength() байт
      RMCount
   };

   //Тип функции для каждого из режимов чтения
   //При приёме данных они записываются в m_ReceiveBuf. Функция должна обработать эти данные и подготовить 
   //буфер для приёма новых данных. Должна корректно обрабатывать отсутствие данных в буфере.
   //Возвр: Нужно ли продолжать чтение данных
   typedef bool (THTTPBasicConnection::*TOnReceiveFunc)();

protected:
   static const TOnReceiveFunc OnReceiveFunc[RMCount];       //Соответствие текущего режима чтения и функции этого режима

protected:
   TReceiveBuf m_ReceiveBuf;                               //Буфер чтения
   TSendBuf m_SendBuf;                                     //Буфер записи
   TReceiveMode m_CurReceiveMode;                          //Текущее режим чтения
   TStringParseMode m_CurStringParseMode;                  //Текущее режим обработки строк заголовка
   TSendMode m_CurSendMode;                                //Текущий режим передачи
   TReceiveHeader m_ReceiveHeader;                         //Текущий заголовок принимаемых данных
   TSendMode m_SendMode;                                   //Режим приёма данных
   size_t m_NeadToReceive;                                 //Количество данных которое ещё необходимо принять в теле запроса

protected:
   //Является ли символ пробельным
   static bool IsSpace( char Ch ) { return Ch == ' ' || Ch == '\t'; }   

   //Расшифровать строку и дабавиить данные (если параметр известен) к m_ReceiveHeader
   //Возвр: true можно продолжать обработку заголовка и false в случае неправильного формата
   bool AddStringToHeader( const char *pBegin, const char *pEnd );

protected:
   //Обработка уведомления об отправке данных
   bool OnSend( int SendSize, const char *&pBuf, int &Length );

   //Обработка уведомления о приёме данных
   bool OnReceive( int ReceiveSize, char *&pBuf, int &Length );

protected:
   //Функция TOnReceiveFunc, состояние RMReadString
   //Разбивает входные данные на строки и передаёт  в текущую функцию обработки строк
   bool OnReceiveReadString();

   //Функция TOnReceiveFunc, состояние RMReadRequestBody
   //Передаёт принятые данные классу наследнику от ConnectionT
   bool OnReceiveBody();

protected:
   template<class InitDataT>
   THTTPBasicConnection( const InitDataT &InitData ): UserConnectionT(InitData) {} 
};
///////////////////////////////////////////////////////////////////////////////

template<class ConnectionT, template<class> class TraitsT, class UserConnectionT, class FuncImplT>
bool THTTPBasicConnection<ConnectionT, TraitsT, UserConnectionT, FuncImplT>::AddStringToHeader( const char *pBegin, const char *pEnd )
{
   const char *pBeginFieldName;     //Начало имени поля
   const char *pEndFieldName;       //Конец имени поля
   const char *pBeginFieldValue;    //Начало значения поля
   const char *pEndFieldValue;      //Конец значения поля

   //Находим имя поля
   //Пропускаем пробелы
   while( pBegin != pEnd && IsSpace(*pBegin) ) ++pBegin;
   if( pBegin == pEnd ) { return false; }

   pBeginFieldName = pBegin++;

   //Ищем ':'
   while( pBegin != pEnd && *pBegin != ':' ) ++pBegin;
   if( pBegin == pEnd ) { return false; }

   pEndFieldName = pBegin++;

   //Пропускаем пробелы
   while( pBegin != pEnd && IsSpace(*pBegin) ) ++pBegin;
   if( pBegin == pEnd ) { return false; }

   //Пропустим так же пробелы с конца значения
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

   //Мы полностью ответили клиенту
   if( !static_cast<ConnectionT *>(this)->IsDisconnecting() )
   {
      //Вновь инициализируем приём даных
      m_CurReceiveMode = RMReadString;
      m_CurStringParseMode = SPMFirstLine;

      //У нас в буффере чтения могли остаться данные
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
      //Ищем разделитель
      pDelim = std::search( m_ReceiveBuf.Begin(), m_ReceiveBuf.End(), TSendBuf::szStringDelim, TSendBuf::szStringDelim + TSendBuf::StringDelimSize );

      if( pDelim == m_ReceiveBuf.End() ) 
      {
         //Мы не нашли разделитель
         if( !m_ReceiveBuf.PrepareForReceiveOnlyEmpty() )
         {
            TTraits::OnSendBufferToSmall(); //Буффер полностью заполнен 
            return false; 
         }

         return true;
      }

      //Мы нашли разделитель
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
   //GetFromBuffer может быть равен 0
   size_t GetFromBuffer = std::min( m_NeadToReceive, static_cast<size_t>(m_ReceiveBuf.Size()) );

   static_cast<ConnectionT *>(this)->OnReceiveData( static_cast<const char *>(m_ReceiveBuf.Begin()), static_cast<int>(GetFromBuffer) );
   m_ReceiveBuf.PopFront( static_cast<int>(GetFromBuffer) );

   m_NeadToReceive -= GetFromBuffer;

   if( m_NeadToReceive == 0 )  //Мы полностью прочитали тело запроса
   {
      TTraits::OnCompleateReceive(); 
      return false;
   }

   //Буффер пустой
   m_ReceiveBuf.PrepareForReceiveFull();

   return true;
}
///////////////////////////////////////////////////////////////////////////////


} //namespace Detail
} //namespace NWLib



#endif