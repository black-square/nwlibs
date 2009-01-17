#ifndef HTTPInterfaces_HPP
#define HTTPInterfaces_HPP

///////////////////////////////////////////////////////////////////////////////
// ���� �������� �������� ����������� ������� ��� HTTP �������/�������
///////////////////////////////////////////////////////////////////////////////

namespace NWLib
{
namespace HTTP
{
//���� ������ ������������ ������� ��������
enum TErrorCode
{
   ECOk,                          //"200"   ; �� ������, OK
   ECBadRequest,                  //"400"   ; ����������� ������, Bad Request
   ECLengthRequired,              //"411"   ; ��������� �����, Length Required
   ECForbidden,                   //"403"   ; ���������, Forbidden
   ECNotFound,                    //"404"   ; �� ������, Not Found
   ECMethodNotAllowed,            //"405"   ; ����� �� ��������, Method Not Allowed
   ECRequestEntityTooLarge,       //"413"   ; ������ ������� ������� �������, Request Entity Too Large
   ECUnsupportedMediaType,        //"415"   ; ���������������� ����� ���, Unsupported Media Type
   ECInternalServerError,         //"500"   ; ���������� ������ �������, Internal Server Error
   ECNotImplemented,              //"501"   ; �� �����������, Not Implemented

   ECCount
};    

//������� ����� ��� ���������� HTTP �������/������
class THeaderBase
{
   size_t m_ContentLength; 

public:
   THeaderBase() { Clear(); }

   //��������� ����� ���� ���������
   static size_t ErrorContentLength() { return std::numeric_limits<size_t>::max(); }
   
   //����� ���� ���������
   //���� ��� ����� ErrorContentLength() �� ��������� ��� ������ �� �������� ����
   size_t GetContentLength() const { return m_ContentLength; }
   void SetContentLength( size_t Val ) { APL_ASSERT( Val != ErrorContentLength() ); m_ContentLength = Val; }

   //�������� ���������
   void Clear()
   {
      m_ContentLength = ErrorContentLength();
   }


#if 0
   //���������� ������ ��������� � ���������� ����������� ��������� ������
   // FieldName - ��� ���� ��������� �����Ĩ���� � ������� ��������
   // [pBeginFieldValue, pEndFieldValue) - �������� ���� ���������
   bool PraseHeaderString( const std::string &FieldName, const char *pBeginFieldValue, const char *pEndFieldValue )
   {
      APL_ASSERT_PTR(pBeginFieldValue);
      APL_ASSERT_PTR(pEndFieldValue);

      if( FieldName.compare( "content-length" ) == 0 )
      {
         size_t Val;

         if( pBeginFieldValue == pEndFieldValue || ConvertStringToInteger(pBeginFieldValue, pEndFieldValue, Val) != pEndFieldValue )
            return false;

         SetContentLength(Val);
      }

      return false;
   }
#endif 
};

//������ ��������� ��������� ������� �������
class TRequestHeader: public THeaderBase
{
   std::string m_Url;

public:
   TRequestHeader() { Clear(); }

   //������������� URL
   const std::string &GetUrl() const { return m_Url; }
   void SetUrl(const std::string &Val) { m_Url = Val; }

   void Clear()
   {
      THeaderBase::Clear();
      m_Url.clear();
   }
};


//������ ��������� ��������� ������ �������
class TResponseHeader: public THeaderBase { /*���� ��� ������������������ �����*/ };
} //namespace HTTP
} //namespace NWLib

#include "HTTPServerInterfaces.hpp"
#include "HTTPClientInterfaces.hpp"

#endif