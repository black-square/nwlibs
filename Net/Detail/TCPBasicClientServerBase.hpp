#ifndef TCPBasicClientServerBase_HPP
#define TCPBasicClientServerBase_HPP

#include "../../ThreadsManager.hpp"


///////////////////////////////////////////////////////////////////////////////
// Модуль содержит базовый класс для реализации TCP сервера и клиента работающего 
// при помощи функций Berkly-socket и использующего функцию select для синхронизации 
// ввода/вывода
// Описание интерфейсов клиента см AsyncIOConnection.hpp
///////////////////////////////////////////////////////////////////////////////

namespace NWLib
{
namespace Detail
{
   ///////////////////////////////////////////////////////////////////////////////
   // Класс инициализирует в конструкторе структуру WSADATA и удаляет её в деструкторе
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
   // Преобразовывает из времени в миллисекундах во время в TIMEVAL
   // Параметр pTimeVal необходимо непосредственно передавать в select,
   // после вызова функции он может указывать либо на TimeVal либо на NULL
   // (если параметр dwMilliseconds == TimerInfinityVal())
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
// Классы используются для того чтобы организовать специализацию функций класса 
// TTCPBasicClientServerBase
///////////////////////////////////////////////////////////////////////////////
class TCPBasicClientTag {};
class TCPBasicServerTag {};

///////////////////////////////////////////////////////////////////////////////
// Реализация той части класса TTCPBasicClientServerBase, которая отличается у
// клиента и сервера
///////////////////////////////////////////////////////////////////////////////
template< class TagT > class TServerClientSideImpl;

template<>
class TServerClientSideImpl<TCPBasicServerTag>
{
protected:
   //Дополнительные параметры для TTCPBasicClientServerBase::TConnectionInitData, которые отличаются от того
   //имеем ли мы дело с сервером или клиентом
   struct TBaseInitData
   {  
      /* У сервера пока нет доп данных */ 
   };

   //Тип остановки соединений
   enum TStopConnection 
   {
      SCContinue,        //Продолжать выполнять существующие соединения
      SCStopListen,      //Перестать принимать новые соединения но продолжать выполнять существующие
      SCFullStop         //Перестать принимать новые сединения и уведомить старые о том что необходимо их закрыть
   };

private:
   volatile TStopConnection m_StopConnection;   //Необходимо остановить сервер

protected:   
   //Необходимо ли останавливаться
   TStopConnection NeadStop() const
   {
      return m_StopConnection;
   }

   //Функция Run Сервера должна вызывать эту функцию перед выполнением своей работы
   void PrepareRun()
   {
      m_StopConnection = SCContinue;
   }


public:
   TServerClientSideImpl(): m_StopConnection(SCContinue) {}

public:
   //Остановить сервер
   //Проверка флага оставновки происходит с интервалом указанном в TSettings
   //Сервер пытается корректно закрыть все соединения и после этого функция Run
   //возвращает управление
   void Stop()
   {
      m_StopConnection = SCFullStop;
   }

   //Перестать серверу принимать новые подключения
   //Проверка флага оставновки происходит с интервалом указанном в TSettings
   //Сервер перестаёт ожжидать новые подключения дожидается когда существующие соединения 
   //завершаться сами и затем функция Run возвращает управление
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
   //Дополнительные параметры для TTCPBasicClientServerBase::TConnectionInitData, которые отличаются от того
   //имеем ли мы дело с сервером или клиентом
   struct TBaseInitData
   {  
      DWORD ConnectionWaitTime; //Время в миллисекундах ожидания подключения, аналогично TSettings::ConnectionWaitTime
      sockaddr_in SockAddr;     //Информация необходимая для подключения
   };
};

///////////////////////////////////////////////////////////////////////////////
// Базовый класс для TCP сервера и клиента работающего при помощи функций 
// Berkly-socket и использующего функцию select для синхронизации ввода/вывода.
// Для каждого нового соединения создаётся отдельный поток.
// TagT - Может быть либо TCPBasicClientTag либо TCPBasicServerTag
///////////////////////////////////////////////////////////////////////////////
template<class TagT, template <class FuncImplT> class ConnectionT = AsyncIOConnection::TConnectionBasic, class GlobalDataT = AsyncIOConnection::TGlobalDataBasic>
class TTCPBasicClientServerBase: public TServerClientSideImpl<TagT>, public GlobalDataT, public NonCopyable
{
public:
   //Настройки сервера/клиента
   struct TSettings;

protected:
   typedef typename TServerClientSideImpl<TagT>::TBaseInitData TBaseInitData;

   template<class InitDataT>
   struct TConnectionInitData: TBaseInitData
   {
      //Сокет клиента только что создан, ещё необходимо установить соединение
      //Сокет сервера уже полностью готов к тому чтобы писать и читать данные
      SOCKET Socket; 
      
      //Параметр служит для передачи значений для иницализации в класс производный от TConnectionBasic
      const InitDataT &m_InitData; 

      //Указатель на настройки 
      const TSettings &SettingsRef;

      TConnectionInitData(const InitDataT &InitData, const TSettings &Settings): 
         m_InitData( InitData ), SettingsRef(Settings) {}
   };

private:   
   //Глобальные данные для всех соединений
   //Класс позволяющий получить доступ к GlobalDataT из ThreadsManager
   class TGlobalData: public ThreadsManagerStrategy::TGlobalDataBasic
   {
      TTCPBasicClientServerBase *m_pParent;

   public:
      //При таком подходе появляется предупреждение 
      //warning C4355: 'this' : used in base member initializer list
      //explicit TGlobalData( TTCPBasicClientServerBase *pParent ): m_pParent(pParent) {}
      TGlobalData(): m_pParent(0) {}
      void SetParent( TTCPBasicClientServerBase *pParent ) { m_pParent = pParent; }
      TTCPBasicClientServerBase *GetParent() { APL_ASSERT_PTR(m_pParent); return m_pParent; }
      const TTCPBasicClientServerBase *GetParent() const { APL_ASSERT_PTR(m_pParent); return m_pParent; }
   };

   //Класс автоматически освобождает ресурсы для TConnection
   class TConnectionBase: NonCopyable, private TBaseInitData
   {
      SOCKET m_Socket;             //Сокет для записи/чтения
      TGlobalData &m_GlobalData;   //Хранится в этом классе для того чтобы 
                                   //конструктор ConnectionT мог обращаться к GlobalDataT

   public:
      TConnectionBase( SOCKET Socket, TGlobalData &GlobalData, const TBaseInitData &BaseConnectionInitData ): 
         m_Socket(Socket), m_GlobalData(GlobalData), TBaseInitData(BaseConnectionInitData) {}

      ~TConnectionBase() { APL_CHECK( closesocket(m_Socket) == 0 ); }

      TGlobalData &GetGlobalData() { return m_GlobalData; }
      const TGlobalData &GetGlobalData() const { return m_GlobalData; }
      SOCKET &GetSocket() { return m_Socket; }
      const TBaseInitData &GetBaseInitData() const { return *this; }
   };

   //Поскольку TConnection не является полностью определённым мы не можем использовать
   //TConnection::TGlobalData в конструкторе ConnectionT. поэтому вводим дополнительную структуру
   class TConnection;
   struct TTypeDefinition
   {
      typedef TConnection TConnection;
      typedef GlobalDataT TUserGlobalData;
   };

   //Класс инкапсулирует в себе работу с конкретным соединением
   //После того как удачно сработал метод accept создаётся данный класс который полностью управляет соединением
   class TConnection;

protected:
   typedef TThreadsManager<TConnection, TGlobalData> TThreadsManager;

private:
   Detail::TWSAInitManager m_WSAInitManager;

protected:
   TTCPBasicClientServerBase() {}

   //Конструктор передающий параметр в GlobalDataT
   template <class InitDataT>
   TTCPBasicClientServerBase(const InitDataT &InitData): GlobalDataT(InitData) {}
};

///////////////////////////////////////////////////////////////////////////////

template < class TagT, template <class FuncImplT> class ConnectionT, class GlobalDataT >
struct TTCPBasicClientServerBase<TagT, ConnectionT, GlobalDataT>::TSettings
{
   //Время в миллисекундах ожидания подключения. 
   //По истечении этого времени, если подключение не осуществилось проверяется не попытался ли пользователь
   //прекратить попытки подколючения, и если так то прекращаем попытки подключения.
   //Можно использовать TimerInfinityVal()
   DWORD ConnectionWaitTime; 

   //Время в миллисекундах ожидания Приёма/Передачи данных
   //Если в течении этого времени не удалось оправить/принять данные, то вызывается функция OnTimer()
   //Можно использовать TimerInfinityVal()
   DWORD DefaultIOWaitTime;   

   //Время в миллисекундах ожидания Передачи данных перед закрытием соединения. 
   //Если в момент закрытия соединения есть неотправленные данные то ожидается 
   //их передача в течении данного времени, и только после этого соединение закрывается.
   DWORD DefaultCloseWaitTime; 

   //IP Адрес соединения
   //Пустая строка означает любой адресс
   std::string Adress; 

   //Порт соединения 
   //автоматически вызывается htons
   u_short Port;              

   TSettings():
   ConnectionWaitTime(5000),
      DefaultIOWaitTime(5000),
      DefaultCloseWaitTime(0),
      Adress(""),             
      Port(23)                //23 - порт telnet
   {}
};

} //namespace Detail
} //namespace NWLib

#endif

