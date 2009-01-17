#ifndef HTTPDetail_HPP
#define HTTPDetail_HPP

///////////////////////////////////////////////////////////////////////////////
// ������ ���������� ������ HTTP.hpp
///////////////////////////////////////////////////////////////////////////////
namespace NWLib
{
namespace Detail
{
   //����� ������
   //������������� � ���� ������ � ������� ��������� ������ ������, ��������� ������������� � ���������� �������
   //� �������������� ��������� ����� ��� ��������� �������� ������
   template< size_t ReadBufferSize >
   class TReceiveBuf
   {
      char *m_pReceiveBegin;                  //������ ������ ��� ��������� �������� ������                       
      int m_ReceiveLength;                    //���������� ������, ������� ���������� ��������� � ��������� �������� ������          
      char *m_pCurBegin;                      //������ ��������� �������������� �������� � ������ ������                       
      char *m_pCurEnd;                        //����� ��������� �������������� �������� � ������ ������ 
      char m_ReadBuffer[ReadBufferSize];      //����� ������
 
   public:
      TReceiveBuf()
      {
         m_pCurBegin = m_pCurEnd = m_ReadBuffer;
         m_pReceiveBegin = 0;
         m_ReceiveLength = 0;
      }

      //������ ��������� �������������� �������� � ������ ������
      char *Begin() const { return m_pCurBegin; }

      //����� ��������� �������������� �������� � ������ ������ 
      char *End() const { return m_pCurEnd; }

      //������ ��������� �������������� �������� � ������ ������
      int Size() const { APL_ASSERT(m_pCurBegin <= m_pCurEnd); return static_cast<int>(m_pCurEnd - m_pCurBegin); }

      //������� ������ ���������� ����� ����� ��������� ������ ������.
      //����������� �������� �������������� �������� � �����
      void AfterRecieveData( char *pBuf, int Length )
      {
         APL_ASSERT( Length > 0 );
         APL_ASSERT_PTR( pBuf ); 
         APL_ASSERT( pBuf == m_pCurEnd && pBuf + Length <= m_ReadBuffer + ReadBufferSize );

         m_pCurEnd += Length;

         APL_DEBUG_OPERATION( m_pReceiveBegin = 0 ); 
         APL_DEBUG_OPERATION( m_ReceiveLength = 0 );
      }

      //������� ������� ��������� �� ������ ������ ������ � ���������� ������ ������� ���������� ���������
      //������� �������.
      //������ ������ ���������� ����� ��������� ������ � ������� ������.
      //����� ������ AfterRecieveData � ����� BeforeRecieveData ���������� ������� ���� �� ������� PrepareForReceive...
      void BeforeRecieveData( char *&pBuf, int &Length ) const
      {
         APL_ASSERT( m_ReceiveLength > 0 );
         APL_ASSERT_PTR( m_pReceiveBegin ); 
         APL_ASSERT( m_pReceiveBegin == m_pCurEnd && m_pReceiveBegin + m_ReceiveLength <= m_ReadBuffer + ReadBufferSize );

         pBuf = m_pReceiveBegin;
         Length = m_ReceiveLength;
      }

      //���������� ������ ����� (Begin() == End()) ��� ����� ����� ������.
      void PrepareForReceiveFull()
      {
         APL_ASSERT( m_pCurBegin == m_pCurEnd );

         m_pReceiveBegin = m_ReadBuffer;
         m_ReceiveLength = ReadBufferSize;
         m_pCurBegin = m_pCurEnd = m_ReadBuffer;
      }


      //���������� �� ������ ����� (Begin() != End()) ��� ����� ����� ������
      //��������� Begin() � End() ���������� �� ���������������.
      //�����: ���� ����� � ������ ��� ������ ������
      bool PrepareForReceiveOnlyEmpty()
      {
         //���� � ����� ������ ���� ����� �� ���������� ��������� ���� ������
         if( m_pCurEnd != m_ReadBuffer + ReadBufferSize )
         {
            m_pReceiveBegin = m_pCurEnd;
            m_ReceiveLength = static_cast<int>(m_ReadBuffer + ReadBufferSize - m_pCurEnd);

            APL_ASSERT( m_pReceiveBegin + m_ReceiveLength == m_ReadBuffer + ReadBufferSize );

            return true;
         }

         //���� ������� ������ ���� �����, �� ���������� ������ � ������ � ���������� ��������� ������ � ��������������
         //�����
         if( m_pCurBegin != m_ReadBuffer )
         {
            APL_ASSERT( m_pCurEnd == m_ReadBuffer + ReadBufferSize );

            std::copy( m_pCurBegin, m_pCurEnd, m_ReadBuffer );

            //���������� ��������� �� �������������� �����
            int CurSize = Size();

            m_ReceiveLength = ReadBufferSize - CurSize;
            m_pReceiveBegin = m_ReadBuffer + CurSize;
            m_pCurBegin = m_ReadBuffer;
            m_pCurEnd = m_pReceiveBegin;

            APL_ASSERT( m_pReceiveBegin + m_ReceiveLength == m_ReadBuffer + ReadBufferSize );

            return true;
         }

         //����� �������� � �� ��� � �� �������� ����������� �������� �� ���� �������
         APL_ASSERT( m_pCurBegin == m_ReadBuffer && m_pCurEnd == m_ReadBuffer + ReadBufferSize );

         return false;
      }

      //��������� ����� � ��� ��� �������� [Begin(), Begin() + Count) ��� ������� ���������. 
      //�.�. ������ ����������� ��������� Begin() ����� �� Count.
      //Count ����� ���� = 0
      void PopFront( int Count )
      {
         APL_ASSERT( Count >= 0 && Count <= Size() );

         m_pCurBegin += Count;
      }
   };

   //������ ������ ������
   //����� ��������� ����� �� ������ ������������ � ����������� �� ���������� ����������
   template<int Dummy = 0> //���������� ������� ����� ������ ��� ���� ����� �� �������� CPP-����
   class TSendBuf
   {
   public:
      //���� ����� ������ 
      struct TStatusCode
      {
         const char *StatusCode;
         const char *ReasonPhrase;
      };

     static const TStatusCode Error2Status[HTTP::ECCount];
     static const char szHttpVersion[];                        //������ HTTP
     static const char szStringDelim[];                        //����������� ����� ���������
     static const int  StringDelimSize;                        //����� ����������� ����� ���������

   private:
     typedef std::string TBuf;

   private:
      TBuf m_Buf;

   private:

      //�������� ������������� ����� � ������
      template<class T>
      void AddIntegerToBuf( T Val )
      {
         char TmpBuf[20];
         char *pRezult = ConvertIntegerToString( Val, TmpBuf, TmpBuf + APL_ARRSIZE(TmpBuf) - 1 );

         APL_ASSERT( pRezult != TmpBuf );
         *pRezult = '\0';

         m_Buf.append(TmpBuf);
      }

      //��������� ������ � ����� ���������
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
      //�������� �� ������������ ����� ���������
      bool Check() const
      {
         //��������� ������� � ������� ���� ������������ � �����
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

      //������ ������ ��� ������
      const char *Begin() const
      {
         APL_ASSERT( Check() );
         return m_Buf.c_str();
      }

      //����� ������ ��� ������
      int Length() const
      {
         return static_cast<int>(m_Buf.size());
      }

      //������� ����� ��� ����������� ���� ������
      void MakeErrorCode( HTTP::TErrorCode EC )
      {
         Clear();

         AppendErrorCodeToBuffer(EC);

         m_Buf.append("Connection: close");
         m_Buf.append(szStringDelim);
         
         m_Buf.append(szStringDelim);
      }

      //������� ����� ��� ���������
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

   //���������� ����������� ���� ������ �� ����������� HTTP ������
   //��������������� �������������� � ������� RegularExpression � VisualStudio
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

   //������ HTTP
   template<int Dummy>
   const char TSendBuf<Dummy>::szHttpVersion[] = "HTTP/1.1"; 

   //����������� ����� ���������
   template<int Dummy>
   const char TSendBuf<Dummy>::szStringDelim[] = "\r\n";     

   //����� ����������� ����� ���������
   template<int Dummy>
   const int TSendBuf<Dummy>::StringDelimSize = APL_ARRSIZE(szStringDelim) - 1;    


} //namespace Detail
} //namespace NWLib

#endif
