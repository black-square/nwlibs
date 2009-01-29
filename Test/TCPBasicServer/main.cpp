// WinSocket.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
    
#include "../../LikePointer.hpp"
#include "../../TestHelper.h"
#include "../../CoutConvert.h"

const int SleepTime = 10;   //Время запаздывания отправки в мс
const int RepeatCount = 3;  //Сколько раз повторять отправку строк

//Класс читает строки из входного файла и отсылает их назад обёрнутыми в скобки
template<class T>
class TConnectionServer: public NWLib::AsyncIOConnection::TConnectionBasic<T>
{
   typedef NWLib::ObjectLevelLockable<TConnectionServer> TMutex;
   static const size_t BufSize = 10;
   static const char szStringDelim[];
   static const size_t StringDelimSize;
   static const int MaxTimeIdle = 10;

   typedef std::string TBuffer;

private:
   char m_ReadBufTmp[BufSize];
   char m_WriteBufTmp[BufSize];
   TBuffer m_ReadBuf;
   TBuffer m_WriteBuf;
   TMutex m_WriteBufMutex; //Защита буфера записи
   bool m_DataSending;     //Ожидается ли отправка данных
   int m_IdleTime;         //Время простоя
   bool m_EndReceive;      //Приём даных закончен

public:
   //Сервер испоользует только одно соединение
   TConnectionServer(bool OnlyOneConnection): m_DataSending(false), m_IdleTime(0), m_EndReceive(false) 
   { 
      GetGlobalData().Check();
      
      std::cout << "TConnectionServer()" << std::endl;
      
      if( OnlyOneConnection) StopWaitConnection();
   }
   ~TConnectionServer(){ std::cout << "~TConnectionServer()" << std::endl; }


   void OnDisconnect()
   {
      std::cout << "OnDisconnect()" << std::endl;
      DEBUG_MSG_LOG1( "Server\t" << "OnDisconnect()" );

   }

   void OnConnect()
   {
      std::cout << "OnConnect()" << std::endl;  
      DEBUG_MSG_LOG1( "Server\t" << "OnConnect()" );
 
      {
         //Ставим в очередь на отправку
         TMutex::Lock Guard(m_WriteBufMutex);
         m_WriteBuf.append("Welcome to simple server");
         m_WriteBuf.append(szStringDelim);
     }

      SendData();
      TryReceive( m_ReadBufTmp, BufSize ); 
   }

   void OnEndReceive( TEndResult ER ) 
   {
      std::cout << "OnEndReceive " << ER << std::endl; 
      DEBUG_MSG_LOG1( "Server\t" << "OnEndReceive " << ER );
 
      {
         //Ставим в очередь на отправку то что осталось
         TMutex::Lock Guard(m_WriteBufMutex);
         if( !m_ReadBuf.empty() )
         {
            m_WriteBuf.append("[[[");
            m_WriteBuf.append( m_ReadBuf.begin(), m_ReadBuf.end() );
            m_WriteBuf.append("]]]");
            m_WriteBuf.append(szStringDelim);
         }
      }

      SendData();

      m_EndReceive = true;
   }

   void OnEndSend( TEndResult ER ) 
   {
      std::cout << "OnEndSend " << ER << std::endl;
      DEBUG_MSG_LOG1( "Server\t" << "OnEndSend " << ER );
   }

   //Уведомление о том что сработал таймер ожидания операции чтения или записи
   //Можно вызвать TryDisconnect, для завершения соединения или выполнить другие действия,
   //например попытаться отправить запрос KEEP ALIVE (если не было ожидания операции записи)
   void OnTimer() 
   {
      std::cout << "OnTimer " << MaxTimeIdle - m_IdleTime - 1 << std::endl;

      if( ++m_IdleTime == MaxTimeIdle )
         TryDisconnect();
   }

   int MoveToWriteBuffer()
   {
      APL_ASSERT( !m_WriteBuf.empty() );
      int SendSize = static_cast<int>(std::min( BufSize, m_WriteBuf.size() ));

      //VC 8.0 ругается на эту строчку, потому что функция не может проверить переполнение буфера
      //необходимо сделать #define _SCL_SECURE_NO_DEPRECATE 
      //http://groups.google.com/group/microsoft.public.vc.language/browse_thread/thread/635dd11faa9651f3/
      std::copy( m_WriteBuf.begin(), m_WriteBuf.begin() + SendSize, m_WriteBufTmp );
      m_WriteBuf.erase( m_WriteBuf.begin(), m_WriteBuf.begin() + SendSize ); 
      return SendSize;
   }

   void SendData()
   {
      TMutex::Lock Guard(m_WriteBufMutex);
      
      if(m_DataSending)
         return;
      
      if( !m_WriteBuf.empty() )
      {
         TrySend( m_WriteBufTmp, MoveToWriteBuffer() );
         m_DataSending = true;
      }
   }


   bool OnSend( int SendSize, const char *&pBuf, int &Length )
   {      
      m_IdleTime = 0;
      std::cout << "SEND: " << std::string(pBuf, SendSize) << std::endl;
      
      TMutex::Lock Guard(m_WriteBufMutex);

      APL_ASSERT(m_DataSending);
      
      //Еслди мы не смогли отправить весь буфер повторяем попытку
      if( SendSize != Length )
      {
         pBuf += SendSize;
         Length -= SendSize;

         return true;
      }

      //Всё отправили смотрим есть ли ещё данные для отправки
      if( !m_WriteBuf.empty() )
      {
         Length = MoveToWriteBuffer();
         pBuf = m_WriteBufTmp;
         return true;
      }
      
      m_DataSending = false;

      if( m_EndReceive )
         EndSend();

      return false;
   }

   bool OnReceive( int ReceiveSize, char *&pBuf, int &Length )
   {
      m_IdleTime = 0;
      std::cout << "RECEIVE: " << std::string(pBuf, ReceiveSize) << std::endl;

      APL_ASSERT(ReceiveSize > 0);
      m_ReadBuf.append(pBuf, ReceiveSize);
      pBuf = m_ReadBufTmp;
      Length = BufSize;

      //Ищем перевод строки
      for(;;)
      {
         TBuffer::size_type DelimPos = m_ReadBuf.find( szStringDelim );

         if( DelimPos == TBuffer::npos )
            break;

         {
            //Ставим в очередь на отправку
            TMutex::Lock Guard(m_WriteBufMutex);
            
            for( int i = 0; i < RepeatCount; ++i )
            {
               m_WriteBuf.append("<<<");
               m_WriteBuf.append( m_ReadBuf.begin(), m_ReadBuf.begin() + DelimPos );
               m_WriteBuf.append(">>>");
               m_WriteBuf.append(szStringDelim);
            }
         }
         
         m_ReadBuf.erase( m_ReadBuf.begin(), m_ReadBuf.begin() + DelimPos + StringDelimSize );

         SendData();
      }

      return true;
   }
};

template< class T >
const char TConnectionServer<T>::szStringDelim[] = "\r\n";

template< class T >
const size_t TConnectionServer<T>::StringDelimSize = APL_ARRSIZE(szStringDelim) - 1;

///////////////////////////////////////////////////////////////////////////////


//Клиент читает файл, отправляет его серверу и записывает пришедший ответ в другой файл
template<class T>
class TConnectionClient: public NWLib::AsyncIOConnection::TConnectionBasic<T>
{
   static const size_t BufSize = 10;
   static const int MaxTimeIdle = 10;

private:
   char m_ReadBufTmp[BufSize];
   char m_WriteBufTmp[BufSize];

   std::ifstream m_FlInp;
   std::ofstream m_FlOut;

   int m_IdleTime;        //Время простоя
   int m_ConnectTime;     //Время ожидания попыток подключения

private:
   
   int FillWriteBuffer()
   {
      m_FlInp.read(m_WriteBufTmp, BufSize);
      return m_FlInp.gcount();
   }

public:
   TConnectionClient( NWLib::NullType ): m_ConnectTime(0) 
   { 
      std::cout << "TConnectionClient()" << std::endl;

      GetGlobalData().Check();

      std::string Path(NWLib::GetExeDirPath());
      m_FlInp.open( (Path + "..\\input.txt").c_str(), std::ios::binary );
      m_FlOut.open( (Path + "output.txt").c_str(), std::ios::binary );

      //StopWaitConnection();
   }

   ~TConnectionClient(){ std::cout << "~TConnectionClient()" << std::endl; }


   void OnDisconnect()
   {
      std::cout << "OnDisconnect()" << std::endl;
      DEBUG_MSG_LOG1( "Client\t" << "OnDisconnect()" );
   }

   void OnConnect()
   {
      std::cout << "OnConnect()" << std::endl;
      DEBUG_MSG_LOG1( "Client\t" << "OnConnect()" );

      int Size = FillWriteBuffer();

      if( Size != 0 )
      {
         TrySend( m_WriteBufTmp, Size ); 
         TryReceive( m_ReadBufTmp, BufSize ); 
      }
      else
         EndSend();
   }

   void OnEndReceive( TEndResult ER ) 
   {
      std::cout << "OnEndReceive " << ER << std::endl;
      DEBUG_MSG_LOG1( "Client\t" << "OnEndReceive " << ER );
   }

   void OnEndSend( TEndResult ER ) 
   {
      DEBUG_MSG_LOG1( "Client\t" << "OnEndSend " << ER );
   }

   //Уведомление о том что сработал таймер ожидания операции чтения или записи
   //Можно вызвать TryDisconnect, для завершения соединения или выполнить другие действия,
   //например попытаться отправить запрос KEEP ALIVE (если не было ожидания операции записи)
   void OnTimer() 
   {
      std::cout << "OnTimer " << MaxTimeIdle - m_IdleTime - 1 << std::endl;

      if( ++m_IdleTime == MaxTimeIdle )
         TryDisconnect();
   }

   //Уведомление в том что сработал таймер ожидания подключения к серверу
   //Можно вызвать TryDisconnect, для завершения попыток подключения или выполнить другие действия
   void OnConnectTimer()
   {
      std::cout << "OnConnectTimer " << MaxTimeIdle - m_ConnectTime - 1 << std::endl;

      if( ++m_ConnectTime == MaxTimeIdle )
         TryDisconnect();
   }

   bool OnSend( int SendSize, const char *&pBuf, int &Length )
   {      
      m_IdleTime = 0;
      std::cout << "SEND: " << std::string(pBuf, SendSize) << std::endl;

      //Еслди мы не смогли отправить весь буфер повторяем попытку
      if( SendSize != Length )
      {
         pBuf += SendSize;
         Length -= SendSize;

         return true;
      }

      int Size = FillWriteBuffer();

      if( Size != 0 )
      {
         pBuf = m_WriteBufTmp;
         Length = Size;

         return true;
      }

      EndSend();
      return false;
   }

   bool OnReceive( int ReceiveSize, char *&pBuf, int &Length )
   {
      m_IdleTime = 0;
      std::cout << "RECEIVE: " << std::string(pBuf, ReceiveSize) << std::endl;
      Sleep(SleepTime);

      m_FlOut.write( pBuf, ReceiveSize );

      pBuf = m_ReadBufTmp;
      Length = BufSize;

      return true;
   }
};

template NWLib::TTCPBasicServer<>;
template NWLib::TTCPBasicClient<>;
template NWLib::THTTPServer<>;

struct TGlobalData
{
   TGlobalData()
   {
      std::cout << "TGlobalData()" << std::endl;
   }

   ~TGlobalData()
   {
      std::cout << "~TGlobalData()" << std::endl;
   }

   void Check()
   {
      std::cout << "TGlobalData::Check()" << std::endl;
   }
};

typedef NWLib::TTCPBasicServer<TConnectionServer, TGlobalData> TServer;
typedef NWLib::TTCPBasicClient<TConnectionClient, TGlobalData> TClient;

NWLib::auto_ptr_ex<TServer> g_pServer;
NWLib::auto_ptr_ex<TClient> g_pClient;

//Обрабатываем прерывания с консоли
BOOL WINAPI ConsoleCtrlHandler( DWORD dwCtrlType )
{
   switch (dwCtrlType)
   {
   case CTRL_C_EVENT:
   case CTRL_BREAK_EVENT:
   case CTRL_CLOSE_EVENT:
   case CTRL_SHUTDOWN_EVENT:
      if( g_pServer ) g_pServer->Stop();
      if( g_pClient ) g_pClient->Stop();

      return TRUE;
   default:
      return FALSE;
   }

}

int _tmain(int argc, _TCHAR* argv[])
{ 
   NWLib::TConsoleAutoStop ConsoleAutoStop;
   
   APL_TRY()
   {      
      if( argc < 2 )
         APL_THROW( "Вызывайте с параметром '-S' для сервера и с параметром '-C' для клиента" );

      SetConsoleCtrlHandler(ConsoleCtrlHandler, TRUE);
      std::cout << "Нажмите CTRL + C для остановки..." << std::endl;

      if( _stricmp( argv[1], "-S_Multi") == 0 )
      {
         std::cout << "Run Server Multi Connection" << std::endl;
         TServer::TSettings Settings;

         g_pServer.reset(new TServer);
         g_pServer->Run(Settings, false );
      }
      if( _stricmp( argv[1], "-S_Single") == 0 )
      {
         std::cout << "Run Server Single Connection" << std::endl;
         TServer::TSettings Settings;

         g_pServer.reset(new TServer);
         g_pServer->Run(Settings, true );
      }else if( _stricmp( argv[1], "-C") == 0 )
      {
         std::cout << "Run Client" << std::endl;
         TClient::TSettings Settings;
         
         if(argc < 3)
            APL_THROW( "Необходимо указать IP Адресс" );

         g_pClient.reset( new TClient );

         Settings.Adress = argv[2];
         g_pClient->Connect(Settings, NWLib::NullType());
         g_pClient->Wait();
      }
   }
   APL_CATCH()

   g_pServer.reset();
   g_pClient.reset();
  
   return 0;
}

