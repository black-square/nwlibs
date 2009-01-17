#ifndef HTTP_HPP
#define HTTP_HPP

#include "TCPBasicServer.hpp"
#include "HTTPInterfaces.hpp"
#include "Detail/HTTPDetail.hpp"

#include <algorithm>
    
///////////////////////////////////////////////////////////////////////////////
// Модуль реализует HTTP сервер и HTTP клиент которые поддерживают асинхронный 
// ввод/вывод
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Реализация сервера
///////////////////////////////////////////////////////////////////////////////
namespace NWLib
{
template< template <class FuncImplT> class ConnectionT = HTTP::TServerConnectionBasic, class GlobalDataT = HTTP::TServerGlobalDataBasic >
class THTTPServer: public GlobalDataT, public NonCopyable
{
   //Глобальные данные для всех соединений
   //Класс позволяющий получить доступ к GlobalDataT из ThreadsManager
   class TGlobalData: public AsyncIOConnection::TGlobalDataBasic
   {
      THTTPServer *m_pParent;

   public:
      //При таком подходе появляется предупреждение 
      //warning C4355: 'this' : used in base member initializer list
      //explicit TGlobalData( TTCPBasicClientServerBase *pParent ): m_pParent(pParent) {}
      TGlobalData(): m_pParent(0) {}
      void SetParent( THTTPServer *pParent ) { m_pParent = pParent; }
      THTTPServer *GetParent() { APL_ASSERT_PTR(m_pParent); return m_pParent; }
      const THTTPServer *GetParent() const { APL_ASSERT_PTR(m_pParent); return m_pParent; }
   };

   //Класс автоматически освобождает ресурсы для TConnection
   class TConnectionBase: NonCopyable
   {
   private:
      TGlobalData &m_GlobalData;   //Хранится в этом классе для того чтобы 
      //конструктор ConnectionT мог обращаться к GlobalDataT

   public:
      TConnectionBase( TGlobalData &GlobalData ): m_GlobalData(GlobalData) {}
      
      TGlobalData &GetGlobalData() { return m_GlobalData; }
      const TGlobalData &GetGlobalData() const { return m_GlobalData; }
   };

   //Поскольку TConnection не является полностью определённым мы не можем использовать
   //TConnection::TGlobalData в классе TServerCallRedirection. поэтому вводим дополнительную структуру 
   template<class UserConnectionT>
   struct TTypeDefinition
   {
      typedef UserConnectionT TConnection;
      typedef GlobalDataT TUserGlobalData;
   };

   //Класс соединения создаётся при подключении нового пользователя и уничтожается при обрыве соединения
   template<class FuncImplT>
   class TConnection; 
   
   typedef TTCPBasicServer<TConnection, TGlobalData> TServer;  //Реализация сервера

public:
   //Настройки сервера
   typedef typename TServer::TSettings TSettings;

private:
   TServer m_Server;

public:
   THTTPServer() { m_Server.SetParent(this); }

   //Конструктор передающий параметр в GlobalDataT
   template <class InitDataT>
   THTTPServer(const InitDataT &InitData): GlobalDataT(InitData) { m_Server.SetParent(this); }

   //Остановить сервер.
   //Проверка флага оставновки происходит с интервалом указанном в TSettings
   //Сервер/Клиент пытается корректно закрыть все соединения и после этого функция Run
   //возвращает управление
   void Stop() { m_Server.Stop(); }

   //Перестать принимать новые подключения.
   //Проверка флага оставновки происходит с интервалом указанном в TSettings
   //Сервер перестаёт ожжидать новые подключения дожидается когда существующие соединения 
   //завершаться сами и затем функция Run возвращает управление
   void StopWaitConnection(){ m_Server.StopWaitConnection(); }

   //Запустить сервер с настройками Settings.
   //Сервер начинает слушать порт и как только поступает входящие соединение создаёт ConnectionT
   //который обслуживает соединение
   //Поток вызвавший Run блокируется пока не будет вызван из другого потока метод Stop()
   //ConnectionInitData - Параметр который передаётся конструктору производного от TConnectionBasic классу
   //                     если нет необходимости передавать параметр можно, например, передать NullType
   template<class InitDataT>
   void Run( const TSettings &Settings, const InitDataT &InitData ){ m_Server.Run(Settings, InitData); }
};
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
// Реализация клиента
// Реализация была прервана, решил использовать в проекте libcurl 
// http://curl.haxx.se/libcurl/
///////////////////////////////////////////////////////////////////////////////
template< template <class FuncImplT> class ConnectionT = HTTP::TClientConnectionBasic, class GlobalDataT = HTTP::TClientGlobalDataBasic >
class THTTPClient: public GlobalDataT, public NonCopyable
{
   //Глобальные данные для всех соединений
   //Класс позволяющий получить доступ к GlobalDataT из ThreadsManager
   class TGlobalData: public AsyncIOConnection::TGlobalDataBasic
   {
      THTTPClient *m_pParent;

   public:
      //При таком подходе появляется предупреждение 
      //warning C4355: 'this' : used in base member initializer list
      //explicit TGlobalData( TTCPBasicClientServerBase *pParent ): m_pParent(pParent) {}
      TGlobalData(): m_pParent(0) {}
      void SetParent( THTTPClient *pParent ) { m_pParent = pParent; }
      THTTPClient *GetParent() { APL_ASSERT_PTR(m_pParent); return m_pParent; }
      const THTTPClient *GetParent() const { APL_ASSERT_PTR(m_pParent); return m_pParent; }
   };

   //Класс автоматически освобождает ресурсы для TConnection
   class TConnectionBase: NonCopyable
   {
   private:
      TGlobalData &m_GlobalData;   //Хранится в этом классе для того чтобы 
      //конструктор ConnectionT мог обращаться к GlobalDataT

   public:
      TConnectionBase( TGlobalData &GlobalData ): m_GlobalData(GlobalData) {}
      
      TGlobalData &GetGlobalData() { return m_GlobalData; }
      const TGlobalData &GetGlobalData() const { return m_GlobalData; }
   };

   //Поскольку TConnection не является полностью определённым мы не можем использовать
   //TConnection::TGlobalData в классе TServerCallRedirection. поэтому вводим дополнительную структуру 
   template<class UserConnectionT>
   struct TTypeDefinition
   {
      typedef UserConnectionT TConnection;
      typedef GlobalDataT TUserGlobalData;
   };

   //Класс соединения создаётся при создании соединения и уничтожается при обрыве соединения
   template<class FuncImplT>
   class TConnection; 
   
   typedef TTCPBasicClient<TConnection, TGlobalData> TClient;  //Реализация клиента

public:
   //Настройки сервера
   typedef typename TClient::TSettings TSettings;

private:
   TClient m_Client;

public:
   THTTPClient() { m_Client.SetParent(this); }

   //Конструктор передающий параметр в GlobalDataT
   template <class InitDataT>
   THTTPClient(const InitDataT &InitData): GlobalDataT(InitData) { m_Server.SetParent(this); }

   //Ждать пока все соединения закроются
   void Wait() { m_Client.Wait(); }

   //Уведомить соединения о том что им необходимо закрыться
   void Stop() { m_Client.Stop(); }

   //Попытаться соединится с сервером используя наcтройки Settings.
   //Клиент пытается соединится c сервером в отдельном потоке и вызывающий поток возвращает управление
   //Уведомления о неудачных попытках соединения приходят в уведомлении HTTP::TClientConnectionBasic::OnConnectTimer  
   //ConnectionInitData - Параметр который передаётся конструктору производного от TConnectionBasic классу
   //                     если нет необходимости передавать параметр можно, например, передать NullType
   template<class InitDataT>
   void Connect( const TSettings &Settings, const InitDataT &ConnectionInitData ) { m_Client.Connect(Settings, ConnectionInitData); }
};
///////////////////////////////////////////////////////////////////////////////

} //namespace NWLib


#include "Detail/HTTPServerConnection.hpp"

#endif