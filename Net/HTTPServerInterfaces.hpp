#ifndef HTTPServerInterfaces_HPP
#define HTTPServerInterfaces_HPP

///////////////////////////////////////////////////////////////////////////////
// Файл содержит описание интерфейсов классов для HTTP сервера
///////////////////////////////////////////////////////////////////////////////
namespace NWLib
{
namespace HTTP
{
///////////////////////////////////////////////////////////////////////////////
// Класс от которого наследуется TServerConnectionBasic. Служит для перенаправления 
// вызовов внутренним методам класса HTTP Сервера.
// Пользователь в классе производном от TServerConnectionBasic должен вызывать методы
// данного класса для отправки и принятия данных, а также для управления соединением
///////////////////////////////////////////////////////////////////////////////
template< class FuncImplT >
struct TServerCallRedirection
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

   //Получить ссылку на производный от TServerGlobalDataBasic классс который передан в параметре шаблона 
   //GlobalDataT класса HTTP Сервера
   TUserGlobalData &GetGlobalData() { return static_cast<TConnection *>(this)->GetGlobalData(); }
   const TUserGlobalData &GetGlobalData() const { return static_cast<const TConnection *>(this)->GetGlobalData(); }

   //Установить время ожидания чтения/записи таймера в миллисекундах.
   //Можно воспользоваться значением TimerInfinityVal() для бесконечного ожидания
   void SetWaitableTimer( DWORD dwMilliseconds )
   {
      static_cast<TConnection *>(this)->SetWaitableTimer(dwMilliseconds);
   }

   //Перестать принимать новые соединения. После этого сервер ждёт завершения всех
   //созданных к данному моменту соединений, при этом не передаётся флаг досрочного закрытия соединений.
   //GlobalData можно использовать как счётчик соединенний
   //Для того чтобы текущее соединение стало последним надо в конструкторе класса наследника от
   //TServerConnectionBasic вызвать данную функцию (Если вызывать в методе OnConnection или ещё где либо, 
   //возможно создание нескольких новых соединений из-за многопоточности)
   void StopWaitConnection()
   {
      static_cast<TConnection *>(this)->StopWaitConnection();
   }

   //Отправить код ошибки клиенту в качестве ответа и закрыть соединение
   //При этом EC не может быть равен ECOk
   //Функция может вызываться только в обработчике событий OnCompleate...Request
   //и не в каких либо других функцииях
   void SendErrorCode( TErrorCode EC )
   {
      static_cast<TConnection *>(this)->SendErrorCode( EC );
   }

   //Отправить ответ клиенту состоящий только из заголовка (ответ не содержит тела).
   //При этом Header.GetContentLength() должно быть равно TResponseHeader::ErrorContentLength()
   //Функция может вызываться только в обработчике событий OnCompleate...Request
   //и не в каких либо других функцииях
   void SendHeader( TErrorCode EC, const TResponseHeader &Header )
   {
      static_cast<TConnection *>(this)->SendHeader( EC, Header );
   }

   //Отправить ответ клиенту который состоит из заголовка и тела.
   //При этом Header.GetContentLength() должно точно соответствовать размеру тела ответа
   //которое планируется передать.
   //Уведомления о том что необходимо передать очередную порцию данных будут приходить
   //при помощи метода OnSendData
   //Функция может вызываться только в обработчике событий OnCompleate...Request
   //и не в каких либо других функцииях
   void SendHeaderWithBody( TErrorCode EC, const TResponseHeader &Header )
   {
      static_cast<TConnection *>(this)->SendHeaderWithBody( EC, Header );
   }
};


///////////////////////////////////////////////////////////////////////////////
// Базовый класс для стратегии разделяемой области памяти между всеми соединениями.
// Класс HTTP Сервер открыто наследует переданный в параметре шаблона GlobalDataT - класс
// который должен являться наследником TServerGlobalDataBasic и затем передаёт ссылку на 
// этот класс классу соединения. 
// Производный класс может заместить любой метод TServerGlobalDataBasic.
// HTTP Сервер не выполняет какую либо синхронизацию при доступе к GlobalDataT
// пользователь сам должен заботится об этом.
// Можно создать класс, который будет наследоваться от класса HTTP Сервера, а в 
// производном от TServerGlobalDataBasic приводить указатель this на такой класс.
///////////////////////////////////////////////////////////////////////////////
class TServerGlobalDataBasic
{
public:   
   //Конструктор по умолчанию вызывается конструктором класса HTTP Сервера без параметров
   TServerGlobalDataBasic() {}

   //В конструктор можно передать любой необходимой пользователю параметр по средством вызова
   //шаблонного конструктора класса Сервера
   //Переопределяемый конструктор может не быть шаблонным если типы известны.
   template <class InitDataT>
   TServerGlobalDataBasic(const InitDataT &InitData) {}

   //Деструктор вызовется в деструкторе класса Сервера
   ~TServerGlobalDataBasic() {}
};


///////////////////////////////////////////////////////////////////////////////
// Базовый класс соединения.
// Класс HTTP Сервера создаёт при поступлении нового соединения класс производный 
// от TServerConnectionBasic и переданный в параметре ConnectionT.
// Производный класс должен быть шаблоном перенаправляющем свой параметр классу
// TServerConnectionBasic (никаким другим образом параметр шаблона не используется в 
// производных классах)
// При закрытии соединения класс уничтожатся.
///////////////////////////////////////////////////////////////////////////////
template< class FuncImplT >
class TServerConnectionBasic: public TServerCallRedirection<FuncImplT>
{
public:
   //Константы инициализации которые могут быть переопределены в производных классах
   enum TInitConsts
   {
      //Размер буфера используемого при приёме данных. Если строка заголовка будет длиннее
      //размера буфера, то клинету будет выдан код ошибки 400 (Испорченный Запрос, Bad Request)
      //и соединение закроется.
      ICReceiveBufferSize = 1024
   };

protected:
   //Конструктор по умолчанию никогда не будет вызван, переопредилять его не имеет смысл
   TServerConnectionBasic() {}

public:
   //Конструктор вызывается при создании подключения
   //Пользоваться методом GetGlobalData уже можно
   //При создании соединения в конструктор можно передать любой необходимой пользователю 
   //параметр по средством вызова Run у сервера с параметром InitData. 
   //Переопределяемый конструктор может не быть шаблонным если тип InitDataT извесен.
   //Если нет необходимости передавать параметр в конструкторе то можно в качестве параметра 
   //InitDataT метода Run у сервера и передать например NullType
   template <class InitDataT>
   TServerConnectionBasic(const InitDataT &InitData) {}

   //Уведомление в том что соединение установлено
   //Сервер начинает принимать запросы клиента
   void OnConnect() {}

   //Уведомление в том что соединение разорвано.
   void OnDisconnect() {}

   //Уведомление в том что сработал таймер ожидания операции чтения или записи
   //Можно вызвать TryDisconnect, для завершения соединения или выполнить другие действия,
   //например попытаться отправить запрос KEEP ALIVE (если не было ожидания операции записи)
   void OnTimer() {}

   //Уведомление в том что были оправлены или приняты данные.
   //Можно использовать эту функцию для сброса таймаута ожидания операции ввода/вывода
   void OnIOOperation() {}

   //Уведомление в том что был принят заголовок RequestHeader запроса POST
   //Пользователь должен вернуть ECOk, если он планирует принимать тело запроса.
   //При приёме данных он будет уведомлён вызовом функции OnReceiveData;
   //Любое другое возвр. значение приведёт к тому что оно будет отправлено клиенту как
   //ошибка и соединение закроется
   TErrorCode OnBeginPostBodyReceive( const TRequestHeader &RequestHeader ) { return ECInternalServerError; }

   //Уведомление в том что был полностью принят запрос POST.
   //Заголовок запроса был передан в вызове OnBeginPostBodyReceive, а тело в OnReceiveData
   //Пользователь ДОЛЖЕН вызвать одну из Send... функций для уведомления клиента
   void OnCompleatePostRequest() { SendErrorCode(ECInternalServerError); }

   //Уведомление в том что был полностью принят запрос GET
   //m_RequestHeader - Заголовок запроса
   //Пользователь ДОЛЖЕН вызвать одну из Send... функций для уведомления клиента
   void OnCompleateGetRequest( const TRequestHeader &RequestHeader ) { SendErrorCode(ECInternalServerError); }

   //Уведомление в том что от киента была принята очередная порция данных тела запроса POST
   //После того как все данные будут приняты вызовется метод OnCompleatePostRequest
   //Length может быть = 0
   void OnReceiveData( const char *pBegin, int Length ) {}

   //Уведомление в том что от пользователя требуется очередная порция данных для тела ответа
   //Возвр: true -  указатель pBegin указывает на данные которые необходимо передать, а Length на длину этих
   //данных(должен быть > 0), false - передавать ничего не надо (в предыдущем вызове функции была передана последняя порция данных)
   //Клиент ДОЛЖЕН передать ровно столько данных сколько было указано в параметре Header.GetContentLength()
   //функции SendHeaderWithBody
   bool OnSendData( const char *&pBegin, int &Length ) { return false; }

   //Деструктор объекта вызывается в сразу после того как соединение разорвано
   ~TServerConnectionBasic() {}

   ///////////////////////////////////////////////////////////////////////////////
   // Функции уведомляющие о поступлении новых необработанных данных, имеет смысл 
   // использовать в основном для отладки
   ///////////////////////////////////////////////////////////////////////////////

   //Уведомление в том что клиенту была отправлена некоторая порция данных.
   //Функция позволяет следить за полным потоком "сырых" данных от сервера к клиенту
   void OnRawDataSend( const char *pBegin, int Length ) {}

   //Уведомление в том что от клиента была принята некоторая порция данных.
   //Функция позволяет следить за полным потоком "сырых" данных от клиента к серверу
   void OnRawDataReceive( const char *pBegin, int Length ) {}

   //Уведомление в том что от клиента была получена первая строка заголовка
   //Все строки заголовка будут обработаны внутри реализации и необходимая информация 
   //будет занесена в TRequestHeader
   //    [pBeginMethod, pEndMethod) - Название HTTP метода
   //    [pBeginURL, pEndURL) - URL на который поступил запрос
   void OnStartHeaderLine( const char *pBeginMethod, const char *pEndMethod, const char *pBeginURL, const char *pEndURL ) {}

   //Уведомление в том что от клиента была получена вторая и далее строка заголовка
   //Все строки заголовка будут обработаны внутри реализации и необходимая информация будет занесена
   //в TRequestHeader
   //    [pBeginFieldName, pEndFieldName) - Имя поля заголовка
   //    [pBeginFieldValue, pEndFieldValue) - Значение поля заголовка
   //Функция в паре с OnStartHeaderLine может быть использована для ограничения количества строк которое клиент может прислать в
   //заголовке
   void OnHeaderLine( const char *pBeginFieldName, const char *pEndFieldName, const char *pBeginFieldValue, const char *pEndFieldValue ) {}
};
} //namespace HTTP
} //namespace NWLib





#endif