#ifndef HTTPDetail_HPP
#define HTTPDetail_HPP

///////////////////////////////////////////////////////////////////////////////
// Детали реализации модуля HTTP.hpp
///////////////////////////////////////////////////////////////////////////////
namespace NWLib
{
namespace Detail
{
   //Буфер чтения
   //Инкапсулирует в себе данные о текущем состояние буфера чтения, управляет обработанными и свободными данными
   //и подготавливает свободное место для следующей операции чтения
   template< size_t ReadBufferSize >
   class TReceiveBuf
   {
      char *m_pReceiveBegin;                  //Начало буфера для следующей операции чтения                       
      int m_ReceiveLength;                    //Количество данных, которые необходимо прочитать в следующей операции чтения          
      char *m_pCurBegin;                      //Начало интервала необработанных символов в буфере чтения                       
      char *m_pCurEnd;                        //Конец интервала необработанных символов в буфере чтения 
      char m_ReadBuffer[ReadBufferSize];      //Буфер чтения
 
   public:
      TReceiveBuf()
      {
         m_pCurBegin = m_pCurEnd = m_ReadBuffer;
         m_pReceiveBegin = 0;
         m_ReceiveLength = 0;
      }

      //Начало интервала необработанных символов в буфере чтения
      char *Begin() const { return m_pCurBegin; }

      //Конец интервала необработанных символов в буфере чтения 
      char *End() const { return m_pCurEnd; }

      //Размер интервала необработанных символов в буфере чтения
      int Size() const { APL_ASSERT(m_pCurBegin <= m_pCurEnd); return static_cast<int>(m_pCurEnd - m_pCurBegin); }

      //Функция должна вызываться после приёма очередной порции данных.
      //Увеличивает интервал необработанных символов с конца
      void AfterRecieveData( char *pBuf, int Length )
      {
         APL_ASSERT( Length > 0 );
         APL_ASSERT_PTR( pBuf ); 
         APL_ASSERT( pBuf == m_pCurEnd && pBuf + Length <= m_ReadBuffer + ReadBufferSize );

         m_pCurEnd += Length;

         APL_DEBUG_OPERATION( m_pReceiveBegin = 0 ); 
         APL_DEBUG_OPERATION( m_ReceiveLength = 0 );
      }

      //Функция передаёт указатель на начало буфера чтения и количество данных которые необходимо прочитать
      //внешней функции.
      //Всегда должна вызываться перед передачей буфера в функцию чтения.
      //После вызова AfterRecieveData и перед BeforeRecieveData необходимо вызвать одну из функций PrepareForReceive...
      void BeforeRecieveData( char *&pBuf, int &Length ) const
      {
         APL_ASSERT( m_ReceiveLength > 0 );
         APL_ASSERT_PTR( m_pReceiveBegin ); 
         APL_ASSERT( m_pReceiveBegin == m_pCurEnd && m_pReceiveBegin + m_ReceiveLength <= m_ReadBuffer + ReadBufferSize );

         pBuf = m_pReceiveBegin;
         Length = m_ReceiveLength;
      }

      //Подготвить пустой буфер (Begin() == End()) для приёма новых данных.
      void PrepareForReceiveFull()
      {
         APL_ASSERT( m_pCurBegin == m_pCurEnd );

         m_pReceiveBegin = m_ReadBuffer;
         m_ReceiveLength = ReadBufferSize;
         m_pCurBegin = m_pCurEnd = m_ReadBuffer;
      }


      //Подготвить не пустой буфер (Begin() != End()) для приёма новых данных
      //Итераторы Begin() и End() становятся не действительными.
      //Возвр: Если место в буфере для чтения данных
      bool PrepareForReceiveOnlyEmpty()
      {
         //Если в конце буфера есть место то попытаемся прочитать туда данные
         if( m_pCurEnd != m_ReadBuffer + ReadBufferSize )
         {
            m_pReceiveBegin = m_pCurEnd;
            m_ReceiveLength = static_cast<int>(m_ReadBuffer + ReadBufferSize - m_pCurEnd);

            APL_ASSERT( m_pReceiveBegin + m_ReceiveLength == m_ReadBuffer + ReadBufferSize );

            return true;
         }

         //Если вначале буфера есть место, то переместим данные в начало и попытаемся прочитать данные в освободившееся
         //место
         if( m_pCurBegin != m_ReadBuffer )
         {
            APL_ASSERT( m_pCurEnd == m_ReadBuffer + ReadBufferSize );

            std::copy( m_pCurBegin, m_pCurEnd, m_ReadBuffer );

            //Перемещаем указатели на освободившеяся место
            int CurSize = Size();

            m_ReceiveLength = ReadBufferSize - CurSize;
            m_pReceiveBegin = m_ReadBuffer + CurSize;
            m_pCurBegin = m_ReadBuffer;
            m_pCurEnd = m_pReceiveBegin;

            APL_ASSERT( m_pReceiveBegin + m_ReceiveLength == m_ReadBuffer + ReadBufferSize );

            return true;
         }

         //Буфер кончился а мы так и не получили разделитель сообщаем об этом клиенту
         APL_ASSERT( m_pCurBegin == m_ReadBuffer && m_pCurEnd == m_ReadBuffer + ReadBufferSize );

         return false;
      }

      //Известить буфер о том что интервал [Begin(), Begin() + Count) был успешно обработан. 
      //Т.е. просто переместить указатель Begin() вперёд на Count.
      //Count может быть = 0
      void PopFront( int Count )
      {
         APL_ASSERT( Count >= 0 && Count <= Size() );

         m_pCurBegin += Count;
      }
   };

   //Буффер записи данных
   //Класс формирует ответ на запрос пользователя в зависимости от переданных параметров
   template<int Dummy = 0> //Переменная шаблона нужна только для того чтобы не заводить CPP-файл
   class TSendBuf
   {
   public:
      //Типы кодов ответа 
      struct TStatusCode
      {
         const char *StatusCode;
         const char *ReasonPhrase;
      };

     static const TStatusCode Error2Status[HTTP::ECCount];
     static const char szHttpVersion[];                        //Версия HTTP
     static const char szStringDelim[];                        //Разделитель строк заголовка
     static const int  StringDelimSize;                        //Длина разделителя строк заголовка

   private:
     typedef std::string TBuf;

   private:
      TBuf m_Buf;

   private:

      //Добавить провизвольное число в буффер
      template<class T>
      void AddIntegerToBuf( T Val )
      {
         char TmpBuf[20];
         char *pRezult = ConvertIntegerToString( Val, TmpBuf, TmpBuf + APL_ARRSIZE(TmpBuf) - 1 );

         APL_ASSERT( pRezult != TmpBuf );
         *pRezult = '\0';

         m_Buf.append(TmpBuf);
      }

      //Добавляем строку с кодом сообщения
      void AppendErrorCodeToBuffer( HTTP::TErrorCode EC )
      {
         APL_ASSERT(EC >= 0 && EC < HTTP::ECCount);

         m_Buf.append(szHttpVersion);
         m_Buf.append(" ");
         m_Buf.append(Error2Status[EC].StatusCode);
         m_Buf.append(" ");
         m_Buf.append(Error2Status[EC].ReasonPhrase);
         m_Buf.append(szStringDelim);
      }

   private:
      //Проверка на корректность перед отправкой
      bool Check() const
      {
         //Проверяем размере и наличие двух разделителей в конце
         if( m_Buf.size() <= StringDelimSize * 2 )
            return false;

         if( !StringMismatch(szStringDelim, m_Buf.end() - StringDelimSize, m_Buf.end()).first )
            return false;

         if( !StringMismatch(szStringDelim, m_Buf.end() - StringDelimSize * 2, m_Buf.end()).first )
            return false;

         return true;
      }

   public:
      void Clear() { m_Buf.clear(); }

      //Начало буфера для записи
      const char *Begin() const
      {
         APL_ASSERT( Check() );
         return m_Buf.c_str();
      }

      //Конец буфера для записи
      int Length() const
      {
         return static_cast<int>(m_Buf.size());
      }

      //Создать буфер для возвращения кода ошибки
      void MakeErrorCode( HTTP::TErrorCode EC )
      {
         Clear();

         AppendErrorCodeToBuffer(EC);

         m_Buf.append("Connection: close");
         m_Buf.append(szStringDelim);
         
         m_Buf.append(szStringDelim);
      }

      //Создать буфер для заголовка
      void MakeHeader( HTTP::TErrorCode EC, const HTTP::TResponseHeader &Header )
      {
         Clear();
         AppendErrorCodeToBuffer(EC);

         if( Header.GetContentLength() != HTTP::TRequestHeader::ErrorContentLength() )
         {
            m_Buf.append("Content-Length: ");
            AddIntegerToBuf( Header.GetContentLength() );
            m_Buf.append(szStringDelim);
         }
         
#if 0
         m_Buf.append("Content-Type: text/xml");
         m_Buf.append(szStringDelim);
#endif

         m_Buf.append(szStringDelim);
      }
   };

   //Отбражение внутреннего кода ошибки на стандартный HTTP статус
   //Автомантическое преобразование с помощью RegularExpression в VisualStudio
   //Find:     {:i}.+\"{:d+}\".+\,:b*{.+}$
   //Replace:  /*\1*/ { "\2", "\3" },
   template<int Dummy>
   const typename TSendBuf<Dummy>::TStatusCode TSendBuf<Dummy>::Error2Status[HTTP::ECCount] =
   {
      /*ECOk*/                     { "200", "OK" },
      /*ECBadRequest*/             { "400", "Bad Request" },
      /*ECLengthRequired*/         { "411", "Length Required" },
      /*ECForbidden*/              { "403", "Forbidden" },
      /*ECNotFound*/               { "404", "Not Found" },
      /*ECMethodNotAllowed*/       { "405", "Method Not Allowed" },
      /*ECRequestEntityTooLarge*/  { "413", "Request Entity Too Large" },
      /*ECUnsupportedMediaType*/   { "415", "Unsupported Media Type" },
      /*ECInternalServerError*/    { "500", "Internal Server Error" },
      /*ECNotImplemented*/         { "501", "Not Implemented" }
   };

   //Версия HTTP
   template<int Dummy>
   const char TSendBuf<Dummy>::szHttpVersion[] = "HTTP/1.1"; 

   //Разделитель строк заголовка
   template<int Dummy>
   const char TSendBuf<Dummy>::szStringDelim[] = "\r\n";     

   //Длина разделителя строк заголовка
   template<int Dummy>
   const int TSendBuf<Dummy>::StringDelimSize = APL_ARRSIZE(szStringDelim) - 1;    


} //namespace Detail
} //namespace NWLib

#endif
