//Автор: Шестеркин Дмитрий(NW) 2007

#ifndef TINYXML_HPP
#define TINYXML_HPP

///////////////////////////////////////////////////////////////////////////////
// Реализация интерфейсов Writer и Reader с использованием парсера 
// TinyXML http://tinyxml.sourceforge.net/
// Дополнительно используется библиотека 
// UTF8-CPP http://utfcpp.sourceforge.net/
// Данная реализация на выходе и на входе принимает кодировку UTF-8 и не 
// требует от исполняемого модуля каких либо дополнительных библиотек и COM 
// объектов
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Покопавшись в исходниках TinyXML, мне так и не стало понятно зачем там 
// повсюду при парсинге учавствует кодировка, т.к. UTF-16 он не понимает, а в 
// UTF-8: "Текст, состоящий только из символов с номером меньше 128, при 
// записи в UTF-8 превращается в обычный текст ASCII. И наоборот, в тексте 
// UTF-8 <b>любой байт со значением меньше 128 изображает символ ASCII с тем 
// же кодом</b> (http://ru.wikipedia.org/wiki/UTF-8). 
// Единственное, зачем разумно используется кодировка в TinyXML, так это пропуск
// целых UTF-8 октетов (2-4 байта) при позиционировании на следующий символ XML
// текста.
// Я всё это к тому, что ошибочно определённая кодировка не сказывается на 
// результате парсинга.
///////////////////////////////////////////////////////////////////////////////
#define TIXML_USE_STL
#include <tinyxml\tinyxml.h>

#pragma warning( push )
#pragma warning( disable: 4244 )
#include <utf8cpp\utf8.h>
#pragma warning( pop )

#include "../LikePointer.hpp"

namespace NWLib
{
namespace Detail
{
///////////////////////////////////////////////////////////////////////////////
// Класс осуществляет конвертацию из UNF-16 в UTF-8
///////////////////////////////////////////////////////////////////////////////
template< class CharT >
class CUTF16toUTF8Converter {};

//Если передаётся строка char конвертировать не нужно
template <>
class CUTF16toUTF8Converter< char >
{
public:
   typedef char TChar;
   typedef std::string TString;

public:
   const char *Convert( const TChar *szStr ) const        { APL_ASSERT_PTR(szStr); return szStr; }
   const std::string &Convert( const TString &Str ) const { return Str; }
};

//Если передаётся строка wchar_t конвертируем при помощи utf8cpp
//Может кидать исключения std::exception
template <>
class CUTF16toUTF8Converter< wchar_t >
{
public:
   typedef wchar_t TChar;
   typedef std::wstring TString;

private:
   mutable std::string m_Buf;

public:
   const char *Convert( const TChar *szStr ) const /*throw (std::exception)*/
   { 
      APL_ASSERT_PTR(szStr);

      try
      {
         m_Buf.clear();
         utf8::utf16to8( szStr, szStr + wcslen(szStr), std::back_inserter(m_Buf) );

         return m_Buf.c_str(); 
      }
      catch (std::exception &e)
      {
         APL_THROW( _T("Ошибка при конвертации UTF-16 в UTF-8: ") << ConvertToTStr(e.what()) );      	
      }
   }

   const std::string &Convert( const TString &Str ) const /*throw (std::exception)*/
   { 
      try
      {
         m_Buf.clear();
         utf8::utf16to8( Str.begin(), Str.end(), std::back_inserter(m_Buf) );

         return m_Buf; 
      }
      catch (std::exception &e)
      {
         APL_THROW( _T("Ошибка при конвертации UTF-16 в UTF-8: ") << ConvertToTStr(e.what()) );      	
      }

   }
};

///////////////////////////////////////////////////////////////////////////////
// Класс осуществляет конвертацию из UNF-8 в UTF-16
///////////////////////////////////////////////////////////////////////////////
template< class CharT >
class CUTF8toUTF16Converter {};

//Если передаётся строка char конвертировать не нужно
template <>
class CUTF8toUTF16Converter<char>
{
public:
   typedef char TChar;
   typedef std::string TString;

public:
   const TChar *Convert( const char *szStr ) const     { APL_ASSERT_PTR(szStr); return szStr; }
   const TString &Convert( const std::string &Str ) const { return Str; }
};

//Если передаётся строка wchar_t конвертируем при помощи utf8cpp
//Может кидать исключения std::exception
template <>
class CUTF8toUTF16Converter< wchar_t >
{
public:
   typedef wchar_t TChar;
   typedef std::wstring TString;

private:
   mutable std::wstring m_Buf;

public:
   const TChar *Convert( const char *szStr ) const /*throw (std::exception)*/
   { 
      APL_ASSERT_PTR(szStr);

      try
      {
         m_Buf.clear();
         utf8::utf8to16( szStr, szStr + strlen(szStr), std::back_inserter(m_Buf) );

         return m_Buf.c_str(); 
      }
      catch (std::exception &e)
      {
         APL_THROW( _T("Ошибка при конвертации UTF-8 в UTF-16: ") << ConvertToTStr(e.what()) );      	
      }
   }

   const TString &Convert( const std::string &Str ) const /*throw (std::exception)*/
   { 
      try
      {
         m_Buf.clear();
         utf8::utf8to16( Str.begin(), Str.end(), std::back_inserter(m_Buf) );

         return m_Buf; 
      }
      catch (std::exception &e)
      {
         APL_THROW( _T("Ошибка при конвертации UTF-8 в UTF-16: ") << ConvertToTStr(e.what()) );      	
      }
   }
};

///////////////////////////////////////////////////////////////////////////////
// Запись в XML при помощи TinyXML
// Функции TiXmlCreateElement и TiXmlCreateText были добавлены в TinyXML мною,
// подробнее см. tinyxml.h
///////////////////////////////////////////////////////////////////////////////
template < class CharT, template <class> class CharTraitsT >
class CTinyXMLWriterBase: private NonCopyable, private Detail::CUTF16toUTF8Converter<CharT>
{
   typedef auto_ptr_ex<TiXmlDocument> TDocumentPtr;
   typedef Detail::CUTF16toUTF8Converter<CharT> TBase;
   typedef std::basic_ostringstream< CharT, CharTraitsT<CharT> > TStringStream;

protected:
   typedef std::basic_ostream< char, CharTraitsT<char> > TOstream;
   typedef typename CharT TChar;

private:
   TOstream &m_Os;
   TDocumentPtr m_pDocument;
   TiXmlNode *m_pCurNode;
   
protected:
   explicit CTinyXMLWriterBase( TOstream &Os ): m_Os(Os), m_pCurNode(0) {}

   //Начать/Закончить загрузку
   void BeginSave() 
   {
      m_pDocument.reset(new TiXmlDocument );
      m_pCurNode = m_pDocument.get();
   }

   //Сохранить данные в переданный в конструкторе stream
   void EndSaveToStream() 
   {
      m_Os << *m_pDocument;
   }

   //Сохранить данные в TiXmlPrinter
   void EndSaveToPrinter( TiXmlPrinter &Printer ) 
   {
      Printer.SetStreamPrinting();
      m_pDocument->Accept( &Printer );
   }

   //Начать новый уровень вложенности с именем Name
   void BeginLevel( const TChar *Name )
   {
      APL_ASSERT_PTR(m_pCurNode);
      m_pCurNode = m_pCurNode->LinkEndChild( TiXmlCreateElement(TBase::Convert(Name)) );
   }

   //Закончить уровень вложенности с именем Name
   void EndLevel( const TChar *Name )
   {
      APL_ASSERT_PTR(m_pCurNode);

      m_pCurNode = m_pCurNode->Parent();  
   }

   //Сохранить данные Ob с именем Name
   //Дожен быть определён оператор Ostream << Ob
   template< class T >
   void SaveItem( const TChar *Name, const T &Ob )
   {
      TStringStream StringStream;

      //Настраиваем флаги, вызвать StringStream.copyfmt( m_Os ); нельзя, т.к. типы StringStream и m_Os
      //разные
      StringStream.fill(m_Os.fill());
      static_cast<std::ios_base&>(StringStream).copyfmt(m_Os);

      WriteToStream( StringStream, Ob );

      BeginLevel( Name );
      m_pCurNode->LinkEndChild( TiXmlCreateText(TBase::Convert(StringStream.str())) );
      EndLevel( Name );
   }
};

///////////////////////////////////////////////////////////////////////////////
// Чтение из XML при помощи TinyXML
///////////////////////////////////////////////////////////////////////////////
template < class CharT, template <class> class CharTraitsT >
class CTinyXMLReaderBase: private NonCopyable, private Detail::CUTF8toUTF16Converter<CharT>, private Detail::CUTF16toUTF8Converter<CharT>
{
   typedef auto_ptr_ex<TiXmlDocument> TDocumentPtr;
   typedef Detail::CUTF8toUTF16Converter<CharT> TUTF8toUTF16;
   typedef Detail::CUTF16toUTF8Converter<CharT> TUTF16toUTF8;
   typedef std::basic_stringstream< CharT, CharTraitsT<CharT> > TStringStream;

protected:
   typedef std::basic_istream< char, CharTraitsT<char> > TIstream;
   typedef typename CharT TChar;

private:
   TIstream &m_Is;
   TDocumentPtr m_pDocument;
   TiXmlNode *m_pRootNode;
   bool m_ForceUTF8;

private:
   //Проверка потока после чтения данных
   bool CheckStream( const TStringStream & Stream ) const
   {
      std::ios::iostate State( Stream.rdstate() );
       
      //Мы должны считать до конца файла и не должно быть ошибок
      return (State & std::ios::eofbit) && !(State & std::ios::failbit);
   }
     
   //Найти у узла pParentNode потомка с именем Name и присвоить pChildNode указатель на него
   //Возвр: Удалось ли найти
   bool FindChild( TiXmlNode *pParentNode, const char *Name, TiXmlNode *&pChildNode )
   {
      APL_ASSERT_PTR(pParentNode);
      APL_ASSERT_PTR(Name);
      
      TiXmlNode *pCurNode;

      for( pCurNode = pParentNode->FirstChild(); pCurNode; pCurNode = pCurNode->NextSibling() )
      {
         if( pCurNode->Type() == TiXmlNode::ELEMENT && std::strcmp(pCurNode->Value(), Name) == 0)
         {
            pChildNode = pCurNode;
            return true;
         }
      }

      return false;
   }

   //Прочитать текст узла 
   //T должен выполнять [Правило 1]
   template< class T >
   void ReadNode( TiXmlNode *pNode, T &Ob )
   {
      APL_ASSERT_PTR( pNode );

      TiXmlNode *pCurNode;

      for( pCurNode = pNode->FirstChild(); pCurNode; pCurNode = pCurNode->NextSibling() )
      {
         if( pCurNode->Type() == TiXmlNode::TEXT )
            break;
      }

      TStringStream Stream;
      
      //Настраиваем флаги, вызвать StringStream.copyfmt( m_Os ); нельзя, т.к. типы StringStream и m_Is
      //разные
      Stream.fill(m_Is.fill());
      static_cast<std::ios_base&>(Stream).copyfmt(m_Is);

      if( pCurNode != 0 )
         WriteToStream( Stream, TUTF8toUTF16::Convert(pCurNode->Value()) );

      //При записи пустой строки текстовый потомок не создаётся
      //Поэтому мы можем его не найти
      //APL_THROW( _T("У элемента не найден ни один текстовый потомок") );
      
      ReadFromStream( Stream, Ob );
      
      if( !CheckStream( Stream ) )
      {
         APL_THROW( 
            _T("Значение '") << ConvertToTStr(Stream.str()) << _T("' узла '") << 
            GetNodePath(pNode) << _T("' не соответствует формату") 
         );
      }
   }

   //Получить полный путь узла для отладочного сообщения
   std::basic_string<TCHAR> GetNodePath( TiXmlNode *pNode ) const
   {
      APL_ASSERT_PTR(pNode);

      typedef std::basic_string<TCHAR> TDstStr;
      typedef std::vector<TiXmlNode *> TNodeVec;

      TNodeVec vecPath; 
      Detail::CUTF8toUTF16Converter<TCHAR> Converter;

      //Сохраняем всех родителей на пути для того чтобы сохранить правильный порядок вывода узлов
      //Если этого не делать получится инвертированный путь
      while( pNode->Type() != TiXmlNode::DOCUMENT )
      {
         vecPath.push_back( pNode );
         pNode = vecPath.back()->Parent();
         APL_ASSERT_PTR( pNode );        
      }

      TDstStr RetVal;

      for( TNodeVec::reverse_iterator I = vecPath.rbegin(); I != vecPath.rend(); ++I )
      {
         RetVal += _T("/");
         RetVal += Converter.Convert( (*I)->Value() );
      }

      return RetVal;
   }

protected:
   //Курсор, инкапсулирует внутреннюю реализацию поиска
   struct TFindCursor
   {
   private:
      std::string ElementName;     //Имя искомого элемента
      TiXmlNode *pCurNode;         //Предыдущий узел в поиске
      TiXmlNode *pRootNode;        //Узел, который был текущем корнем до начала поиска

      friend class CTinyXMLReaderBase;   
   };

   explicit CTinyXMLReaderBase( TIstream &Is ): m_Is(Is), m_pRootNode(0), m_ForceUTF8(false) {}

   //Загрузить данные из переданного в конструкторе stream
   void BeginLoadFromStream()
   {
      m_pDocument.reset( new TiXmlDocument );
      m_pRootNode = m_pDocument.get();

      //В TinyXML есть перегруженный оператор >>, но он работает медленно из-за 
      //дополнительных проверок, которые в данной ситуации не нужны
      std::string InputStr;

      InputStr.reserve( 8 * 1000 );

      char Buff[256];

      do 
      {
         m_Is.read( Buff, APL_ARRSIZE(Buff) );
         InputStr.append( Buff, m_Is.gcount() );    

      } 
      while(m_Is.gcount() == APL_ARRSIZE(Buff));

      m_pDocument->Parse( InputStr.c_str(), 0, m_ForceUTF8 ? TIXML_ENCODING_UTF8 : TIXML_DEFAULT_ENCODING );

      if( m_pDocument->Error() )
         APL_THROW( _T("Ошибка при загрузке XML: ") << ConvertToTStr(m_pDocument->ErrorDesc()) );
   }

   //Загрузить данные из std::string
   void BeginLoadFromString( const std::string &String )
   {
      m_pDocument.reset( new TiXmlDocument );
      m_pRootNode = m_pDocument.get();

      m_pDocument->Parse( String.c_str(), 0, m_ForceUTF8 ? TIXML_ENCODING_UTF8 : TIXML_DEFAULT_ENCODING );

      if( m_pDocument->Error() )
         APL_THROW( _T("Ошибка при загрузке XML: ") << ConvertToTStr(m_pDocument->ErrorDesc()) );
   }

   void EndLoad(){}

   //Перейти на уровень Name
   bool BeginLevel( const TChar *Name )
   {
      APL_ASSERT_PTR(m_pRootNode);
      return FindChild(m_pRootNode, TUTF16toUTF8::Convert(Name), m_pRootNode);
   }

   //Выйти с уровня Name
   void EndLevel( const TChar *Name )
   {
      APL_ASSERT_PTR(m_pRootNode);
      m_pRootNode = m_pRootNode->Parent();
      APL_ASSERT_PTR(m_pRootNode);
   }

   //Загрузить Ob с именем Name
   //T должен выполнять [Правило 1]
   template< class T >
   bool LoadItem( const TChar *Name, T &Ob )
   {
      TiXmlNode *pCurNode;

      if( !FindChild(m_pRootNode, TUTF16toUTF8::Convert(Name), pCurNode) )
         return false;

      APL_ASSERT_PTR(pCurNode);

      ReadNode( pCurNode, Ob );
      return true;
   }

   //Инициализировать поиск всех элементов с именем Name
   void FindInit( const TChar *Name, TFindCursor &FindCursor )
   {
      APL_ASSERT_PTR( m_pRootNode );
      
      FindCursor.ElementName = TUTF16toUTF8::Convert(Name);
      FindCursor.pRootNode = m_pRootNode;
      FindCursor.pCurNode = 0;
   }

   //Найти следующий (в т.ч. первый) элемент.
   //FindCursor - должен быть заранее инициализирован функцией FindInit
   //Возвр - true элемент успешно записан, false - элемент найти не удалось
   //T должен выполнять [Правило 1]
   template< class T >
   bool FindNext( TFindCursor &FindCursor, T &Ob)
   {
      APL_ASSERT_PTR( FindCursor.pRootNode );
   
      for(;;) 
      {
         FindCursor.pCurNode = FindCursor.pRootNode->IterateChildren(FindCursor.pCurNode);
         
         if( FindCursor.pCurNode == 0 )
         {
            m_pRootNode = FindCursor.pRootNode;
            return false;
         }

         if( FindCursor.pCurNode->Type() == TiXmlNode::ELEMENT && FindCursor.pCurNode->Value() == FindCursor.ElementName )
         {
            ReadNode(FindCursor.pCurNode, Ob);
            return true;
         }
      } 
   }

   bool FindNext( TFindCursor &FindCursor )
   {
      APL_ASSERT_PTR( FindCursor.pRootNode );

      for(;;) 
      {
         FindCursor.pCurNode = FindCursor.pRootNode->IterateChildren(FindCursor.pCurNode);

         if( FindCursor.pCurNode == 0 )
         {
            m_pRootNode = FindCursor.pRootNode;
            return false;
         }

         if( FindCursor.pCurNode->Type() == TiXmlNode::ELEMENT && FindCursor.pCurNode->Value() == FindCursor.ElementName )
         {
            m_pRootNode = FindCursor.pCurNode;
            return true;
         }
      } 
   }

public:
   //Получить путь к текущему узлу
   std::basic_string<TCHAR> GetCurNodePath() const
   {
      APL_ASSERT_PTR( m_pRootNode );
      return GetNodePath( m_pRootNode );
   }

   //Позволяет указать то что поток, который будет прочитан, записан в кодировке UTF8
   //Если кодировка определена не правильно ни на чём кроме некоторого уменьшения производительности это 
   //не скажется, поэтому имеет смысел пользоваться данной функцией только для оптимизации
   void SetForceUTF8( bool Val ){ m_ForceUTF8 = Val; }
};
} //namespace Detail

///////////////////////////////////////////////////////////////////////////////
// Запись в XML при помощи TinyXML
// Производится запись в поток Os, переданный в конструкторе
///////////////////////////////////////////////////////////////////////////////
template < class CharT, template <class> class CharTraitsT >
class CTinyXMLWriter: public Detail::CTinyXMLWriterBase<CharT, CharTraitsT>
{
   typedef Detail::CTinyXMLWriterBase<CharT, CharTraitsT> TBase;

protected:
   typedef typename Detail::CTinyXMLWriterBase<CharT, CharTraitsT>::TOstream TOstream;

protected:
   explicit CTinyXMLWriter( TOstream &Os ): TBase(Os) {}
   void EndSave() { TBase::EndSaveToStream(); }
};

///////////////////////////////////////////////////////////////////////////////
// Запись в XML при помощи TinyXML
// Специальная оптимизация: Запись производится во внутрений буфер std::string,
// а поток Os переданный в конструкторе используется только для копирования флагов 
// формата
// Работает быстрее, чем CTinyXMLWriter
///////////////////////////////////////////////////////////////////////////////
template < class CharT, template <class> class CharTraitsT >
class CTinyXMLWriterString: public Detail::CTinyXMLWriterBase<CharT, CharTraitsT>
{
   typedef Detail::CTinyXMLWriterBase<CharT, CharTraitsT> TBase;
   typedef auto_ptr_ex<TiXmlPrinter> TPrinterPtr;

protected:
   typedef typename Detail::CTinyXMLWriterBase<CharT, CharTraitsT>::TOstream TOstream;

private:
   TPrinterPtr m_pPrinter;

protected:
   explicit CTinyXMLWriterString( TOstream &Os ): TBase(Os) {}
   void EndSave() 
   { 
      m_pPrinter.reset( new TiXmlPrinter() );
      TBase::EndSaveToPrinter( *m_pPrinter ); 
   }

public:
   //В данную строку будет записана результирующая XML, во время вызова функции CWriteArchive::Save()
   const std::string &Str() const { return m_pPrinter->Str(); }
};                                  

///////////////////////////////////////////////////////////////////////////////
// Чтение из XML при помощи TinyXML
// Производится запись в поток Is, переданный в конструкторе
///////////////////////////////////////////////////////////////////////////////
template < class CharT, template <class> class CharTraitsT >
class CTinyXMLReader: public Detail::CTinyXMLReaderBase<CharT, CharTraitsT>
{
   typedef Detail::CTinyXMLReaderBase<CharT, CharTraitsT> TBase;

protected:
   typedef typename Detail::CTinyXMLReaderBase<CharT, CharTraitsT>::TIstream TIstream;

protected:
   explicit CTinyXMLReader( TIstream &Is ): TBase(Is) {}
   void BeginLoad() { TBase::BeginLoadFromStream(); }
};

///////////////////////////////////////////////////////////////////////////////
// Чтение из XML при помощи TinyXML
// Специальная оптимизация: Чтение производится из внутренего буфера std::string,
// а поток Is переданный в конструкторе используется только для копирования флагов 
// формата
// Работает быстрее, чем CTinyXMLReader
///////////////////////////////////////////////////////////////////////////////
template < class CharT, template <class> class CharTraitsT >
class CTinyXMLReaderString: public Detail::CTinyXMLReaderBase<CharT, CharTraitsT>
{
   typedef Detail::CTinyXMLReaderBase<CharT, CharTraitsT> TBase;

protected:
   typedef typename Detail::CTinyXMLReaderBase<CharT, CharTraitsT>::TIstream TIstream;

private:
   std::string m_InputBuff;

protected:
   explicit CTinyXMLReaderString( TIstream &Is ): TBase(Is) {}
   void BeginLoad() { TBase::BeginLoadFromString(m_InputBuff); }

public:
   //Из данной строки будет производится чтение XML, во время вызова CReadArchive::Load()
   std::string &Str() { return m_InputBuff; }
   const std::string &Str() const { return m_InputBuff; }
};

} //namespace NWLib 

#endif