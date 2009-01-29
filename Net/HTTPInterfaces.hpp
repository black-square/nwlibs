#ifndef HTTPInterfaces_HPP
#define HTTPInterfaces_HPP

///////////////////////////////////////////////////////////////////////////////
// Файл содержит описание интерфейсов классов для HTTP сервера/клиента
///////////////////////////////////////////////////////////////////////////////

namespace NWLib
{
namespace HTTP
{
//Коды ошибок возвращаемые клиенту сервером
enum TErrorCode
{
   ECOk,                          //"200"   ; Всё хорошо, OK
   ECBadRequest,                  //"400"   ; Испорченный Запрос, Bad Request
   ECLengthRequired,              //"411"   ; Требуется длина, Length Required
   ECForbidden,                   //"403"   ; Запрещено, Forbidden
   ECNotFound,                    //"404"   ; Не найден, Not Found
   ECMethodNotAllowed,            //"405"   ; Метод не дозволен, Method Not Allowed
   ECRequestEntityTooLarge,       //"413"   ; Объект запроса слишком большой, Request Entity Too Large
   ECUnsupportedMediaType,        //"415"   ; Неподдерживаемый медиа тип, Unsupported Media Type
   ECInternalServerError,         //"500"   ; Внутренняя ошибка сервера, Internal Server Error
   ECNotImplemented,              //"501"   ; Не реализовано, Not Implemented

   ECCount
};    

//Базовый класс для заголовков HTTP запроса/ответа
class THeaderBase
{
   size_t m_ContentLength; 

public:
   THeaderBase() { Clear(); }

   //Ошибочная длина тела сообщения
   static size_t ErrorContentLength() { return std::numeric_limits<size_t>::max(); }
   
   //Длина тела сообщения
   //Если она равна ErrorContentLength() то считается что запрос не содержит тела
   size_t GetContentLength() const { return m_ContentLength; }
   void SetContentLength( size_t Val ) { APL_ASSERT( Val != ErrorContentLength() ); m_ContentLength = Val; }

   //Очистить параметры
   void Clear()
   {
      m_ContentLength = ErrorContentLength();
   }


#if 0
   //Обработать строку заголовка и установить необходимые параметры класса
   // FieldName - Имя поля заголовка ПРИВЕДЁННОЕ К НИЖНЕМУ РЕГИСТРУ
   // [pBeginFieldValue, pEndFieldValue) - Значение поля заголовка
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

//Хранит параметры заголовка запроса клиента
class TRequestHeader: public THeaderBase
{
   std::string m_Url;

public:
   TRequestHeader() { Clear(); }

   //Запрашиваемый URL
   const std::string &GetUrl() const { return m_Url; }
   void SetUrl(const std::string &Val) { m_Url = Val; }

   void Clear()
   {
      THeaderBase::Clear();
      m_Url.clear();
   }
};


//Хранит параметры заголовка ответа сервера
class TResponseHeader: public THeaderBase { /*Пока нет специализированных полей*/ };
} //namespace HTTP
} //namespace NWLib

#include "HTTPServerInterfaces.hpp"
#include "HTTPClientInterfaces.hpp"

#endif