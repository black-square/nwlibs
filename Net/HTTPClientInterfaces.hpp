#ifndef HTTPClientInterfaces_HPP
#define HTTPClientInterfaces_HPP

///////////////////////////////////////////////////////////////////////////////
// Файл содержит описание интерфейсов классов для HTTP клиента
///////////////////////////////////////////////////////////////////////////////
namespace NWLib
{
namespace HTTP
{
///////////////////////////////////////////////////////////////////////////////
// Класс от которого наследуется TCLientConnectionBasic. Служит для перенаправления 
// вызовов внутренним методам класса HTTP Клиента.
// Пользователь в классе производном от TClientConnectionBasic должен вызывать методы
// данного класса для отправки и принятия данных, а также для управления соединением
///////////////////////////////////////////////////////////////////////////////
template< class FuncImplT >
struct TClientCallRedirection
{
   //Тип класса используемого для стратегии разделяемой между соединениями памяти
   typedef typename FuncImplT::TUserGlobalData TUserGlobalData;
   typedef typename FuncImplT::TConnection TConnection;

public:
   typedef TErrorCode TErrorCode;
   typedef TRequestHeader TRequestHeader;
   typedef TResponseHeader TResponseHeader;

public:
   //Поставить в очередь запрос на разрыв соединения.
   //О разрыве соединения класс производный от TServerConnectionBasic будет уведомлён при помощи вызова
   //метода OnDisconnect.
   void TryDisconnect()
   {
      static_cast<TConnection *>(this)->TryDisconnect();
   }

   //Получить ссылку на производный от TClientGlobalDataBasic классс который передан в параметре шаблона 
   //GlobalDataT класса HTTP Клиента
   TUserGlobalData &GetGlobalData() { return static_cast<TConnection *>(this)->GetGlobalData(); }
   const TUserGlobalData &GetGlobalData() const { return static_cast<const TConnection *>(this)->GetGlobalData(); }

   //Установить время ожидания чтения/записи таймера в миллисекундах.
   //Можно воспользоваться значением TimerInfinityVal() для бесконечного ожидания
   void SetWaitableTimer( DWORD dwMilliseconds )
   {
      static_cast<TConnection *>(this)->SetWaitableTimer(dwMilliseconds);
   }

   //Отравить серверу запрс GET (который состоит только из заколовка)
   //При этом Header.GetContentLength() должно быть равно TResponseHeader::ErrorContentLength()
   //Уведомления об ответе сервера будут приходить при помощи методов 
   //OnBeginResponseBodyReceive, OnCompleateResponse, OnReceiveData 
   void SendRequest_GET( const TRequestHeader &Header )
   {
      static_cast<TConnection *>(this)->SendRequest_GET( Header );
   }

   //Отравить серверу запрос POST (который состоит из заголовка и тела)
   //При этом Header.GetContentLength() должно точно соответствовать размеру тела ответа
   //которое планируется передать.
   //Уведомления о том что необходимо передать очередную порцию тела запроса будут приходить
   //при помощи метода OnSendData
   //Уведомления об ответе сервера будут приходить при помощи методов 
   //OnBeginResponseBodyReceive, OnCompleateResponse, OnReceiveData 
   void SendRequest_POST( const TRequestHeader &Header )
   {
      static_cast<TConnection *>(this)->SendRequest_POST( Header );
   }
};


///////////////////////////////////////////////////////////////////////////////
// Базовый класс для стратегии разделяемой области памяти между всеми соединениями.
// Класс HTTP Клиент открыто наследует переданный в параметре шаблона GlobalDataT - класс
// который должен являться наследником TClientGlobalDataBasic и затем передаёт ссылку на 
// этот класс классу соединения. 
// Производный класс может заместить любой метод TClientGlobalDataBasic.
// HTTP Клиент не выполняет какую либо синхронизацию при доступе к GlobalDataT
// пользователь сам должен заботится об этом.
// Можно создать класс, который будет наследоваться от класса HTTP Клиента, а в 
// производном от TClientGlobalDataBasic приводить указатель this на такой класс.
///////////////////////////////////////////////////////////////////////////////
class TClientGlobalDataBasic
{
public:   
   //Конструктор по умолчанию вызывается конструктором класса HTTP Клиента без параметров
   TClientGlobalDataBasic() {}

   //В конструктор можно передать любой необходимой пользователю параметр по средством вызова
   //шаблонного конструктора класса Клиента
   //Переопределяемый конструктор может не быть шаблонным если типы известны.
   template <class InitDataT>
   TClientGlobalDataBasic(const InitDataT &InitData) {}

   //Деструктор вызовется в деструкторе класса Клиента
   ~TClientGlobalDataBasic() {}
};


///////////////////////////////////////////////////////////////////////////////
// Базовый класс соединения.
// Класс HTTP Клиента создаёт вначале соединения класс производный 
// от TClientConnectionBasic и переданный в параметре ConnectionT.
// Производный класс должен быть шаблоном перенаправляющем свой параметр классу
// TClientConnectionBasic (никаким другим образом параметр шаблона не используется в 
// производных классах)
// При закрытии соединения класс уничтожатся.
///////////////////////////////////////////////////////////////////////////////
template< class FuncImplT >
class TClientConnectionBasic: public TClientCallRedirection<FuncImplT>
{
public:
   //Константы инициализации которые могут быть переопределены в производных классах
   enum TInitConsts
   {
      //Размер буфера используемого при приёме данных. Если строка заголовка будет длиннее
      //размера буфера, то соединение закроется.
      ICReceiveBufferSize = 1024
   };

protected:
   //Конструктор по умолчанию никогда не будет вызван, переопредилять его не имеет смысл
   TClientConnectionBasic() {}

public:
   //Конструктор вызывается при создании подключения
   //Пользоваться методом GetGlobalData уже можно
   //При создании соединения в конструктор можно передать любой необходимой пользователю 
   //параметр по средством вызова Connect у клиента с параметром InitData. 
   //Переопределяемый конструктор может не быть шаблонным если тип InitDataT извесен.
   //Если нет необходимости передавать параметр в конструкторе то можно в качестве параметра 
   //InitDataT метода Run у сервера и передать например NullType
   template <class InitDataT>
   TClientConnectionBasic(const InitDataT &InitData) {}

   //Уведомление в том что соединение установлено
   //Необходимо выполнить запрос к серверу, при помощи методов SendRequest_*
   void OnConnect() {}

   //Уведомление в том что соединение разорвано.
   void OnDisconnect() {}

   //Уведомление в том что сработал таймер ожидания операции чтения или записи
   //Можно вызвать TryDisconnect, для завершения соединения или выполнить другие действия,
   //например попытаться отправить запрос KEEP ALIVE (если не было ожидания операции записи)
   void OnTimer() {}

   //Уведомление в том что сработал таймер ожидания подключения к серверу
   //Можно вызвать TryDisconnect, для завершения попыток подключения или выполнить другие действия
   void OnConnectTimer() {}

   //Уведомление в том что были оправлены или приняты данные.
   //Можно использовать эту функцию для сброса таймаута ожидания операции ввода/вывода
   void OnIOOperation() {}
   
   //Уведомление в том что был принят заголовок ответа сервера
   //Пользователь может закрыть соедиенение при помощи вызова TryDisconnect
   //О приёме данных он будет уведомлён вызовом функции OnReceiveData
   void OnBeginResponseBodyReceive( const TResponseHeader &ResponseHeader ) { }

   //Уведомление в том что был полностью принят ответ сервера.
   //Заголовок ответа был передан в вызове OnBeginResponseBodyReceive, а тело в OnReceiveData
   void OnCompleateResponse() { }

   //Уведомление в том что от киента была принята очередная порция данных тела ответа сервера
   //После того как все данные будут приняты вызовется метод OnCompleateResponse
   //Length может быть = 0
   void OnReceiveData( const char *pBegin, int Length ) {}

   //Уведомление в том что от пользователя требуется очередная порция данных для тела ответа
   //Возвр: true -  указатель pBegin указывает на данные которые необходимо передать, 
   //               а Length на длину этих данных (должен быть > 0) 
   //       false - передавать ничего не надо (в предыдущем вызове функции была передана 
   //               последняя порция данных)
   //Клиент ДОЛЖЕН передать ровно столько данных сколько было указано в параметре Header.GetContentLength()
   //функции SendRequest_POST
   bool OnSendData( const char *&pBegin, int &Length ) { return false; }

   //Деструктор объекта вызывается в сразу после того как соединение разорвано
   ~TClientConnectionBasic() {}

   ///////////////////////////////////////////////////////////////////////////////
   // Функции уведомляющие о поступлении новых необработанных данных, имеет смысл 
   // использовать в основном для отладки
   ///////////////////////////////////////////////////////////////////////////////

   //Уведомление в том что серверу была отправлена некоторая порция данных.
   //Функция позволяет следить за полным потоком "сырых" данных от клиента к серверу
   void OnRawDataSend( const char *pBegin, int Length ) {}

   //Уведомление в том что от сервера была принята некоторая порция данных.
   //Функция позволяет следить за полным потоком "сырых" данных от сервера к клиенту
   void OnRawDataReceive( const char *pBegin, int Length ) {}

   //Уведомление в том что от сервера была получена первая строка заголовка
   //Все строки заголовка будут обработаны классом внутри реализации и необходимая 
   //информация будет занесена в TResponseHeader
   //    [pBeginMethod, pEndMethod) - Название HTTP метода
   //    [pBeginURL, pEndURL) - URL на который поступил запрос
   void OnStartHeaderLine( const char *pBeginMethod, const char *pEndMethod, const char *pBeginURL, const char *pEndURL ) {}

   //Уведомление в том что от сервера была получена вторая и далее строка заголовка
   //Все строки заголовка будут обработаны внутри реализации и необходимая информация будет занесена
   //в TResponseHeader
   //    [pBeginFieldName, pEndFieldName) - Имя поля заголовка
   //    [pBeginFieldValue, pEndFieldValue) - Значение поля заголовка
   //Функция в паре с OnStartHeaderLine может быть использована для ограничения количества строк которое сервер может прислать в
   //заголовке
   void OnHeaderLine( const char *pBeginFieldName, const char *pEndFieldName, const char *pBeginFieldValue, const char *pEndFieldValue ) {}
};
} //namespace HTTP
} //namespace NWLib





#endif