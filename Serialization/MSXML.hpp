//Автор: Шестеркин Дмитрий(NW) 2006

#ifndef MSXML_HPP
#define MSXML_HPP

///////////////////////////////////////////////////////////////////////////////
// Реализация интерфейсов Writer и Reader с использованием парсера MSXML
///////////////////////////////////////////////////////////////////////////////

#include "../stlauxiliary.hpp"
#include <Atlbase.h>
#include <objbase.h>
#include <msxml.h>

namespace NWLib
{
struct CCOMBase
{
#ifdef NWLIB_STOCONA_FRAMEWORK
   
   CCOMBase()
   {
      //Мне очень не нравится идея вызывать CoInitialize в конструкторе
      //и CoUninitialize в деструкторе, но это приходится делать т.к.
      //"это работает уже полгода без проблем"
      ::CoInitialize(NULL);
   }
   
   ~CCOMBase()
   {
      ::CoUninitialize();
   }

#endif
};
///////////////////////////////////////////////////////////////////////////////

//Запись в поток строки BSTR
template < class CharT, class CharTraitsT >
inline void WriteBSTRToStream( std::basic_ostream< CharT, CharTraitsT > &Ostream, BSTR Str )
{
   std::basic_string<CharT, CharTraitsT> StrTmp;
   Convert( std::wstring(Str), StrTmp );
   WriteToStream( Ostream, StrTmp );
}

template < class CharTraitsT >
inline void WriteBSTRToStream( std::basic_ostream< WCHAR, CharTraitsT > &Ostream, BSTR Str )
{
   //В этом случае никакие преобразования не нужны
   WriteToStream( Ostream, static_cast<const WCHAR *>(Str) );
}
///////////////////////////////////////////////////////////////////////////////

//Прочитать из потока строки BSTR
template < class CharT, class CharTraitsT >
inline void ReadBSTRFromStream( std::basic_istream< CharT, CharTraitsT > &Istream, ATL::CComBSTR &Str )
{ 
   Str.Empty();

   CharT Buff[256];

   do 
   {
      Istream.read( Buff, APL_ARRSIZE(Buff) );
      ATL::CComBSTR TmpStr(Istream.gcount(), Buff);
      Str.Append(TmpStr);   

   } while(Istream.gcount() == APL_ARRSIZE(Buff));
}

template < class CharTraitsT >
inline void ReadBSTRFromStream( std::basic_istream< WCHAR, CharTraitsT > &Istream, ATL::CComBSTR &Str )
{ 
   //Оптимизация исключающая временные объекты
   Str.Empty();

   WCHAR Buff[256];

   do 
   {
      Istream.read( Buff, APL_ARRSIZE(Buff) );
      Str.Append(Buff, Istream.gcount());   

   }
   while(Istream.gcount() == APL_ARRSIZE(Buff));

}

///////////////////////////////////////////////////////////////////////////////
// Запись в XML при помощи MSXML
///////////////////////////////////////////////////////////////////////////////
template < class CharT, template <class> class CharTraitsT >
class CMSXMLWriter: public CCOMBase, private NonCopyable
{
   typedef ATL::CComPtr<IXMLDOMDocument> TDOMDocument;
   typedef ATL::CComPtr<IXMLDOMElement> TDOMElement;
   typedef ATL::CComPtr<IXMLDOMNode> TDOMNode;
   typedef ATL::CComPtr<IXMLDOMText> TDOMText;
   
   typedef std::basic_ostringstream< CharT, CharTraitsT<CharT> > TStringStream;

protected:
   typedef std::basic_ostream< CharT, CharTraitsT<CharT> > TOstream;
   typedef typename CharT TChar;

private:
   TOstream &m_Os;
   TDOMDocument m_DOMDocument;
   TDOMNode m_CurRootNode;
    
protected:
   explicit CMSXMLWriter( TOstream &Os ): m_Os(Os) {}

   //Начать/Закончить загрузку
   void BeginSave() 
   {
      m_DOMDocument.Release();
      m_CurRootNode.Release();

      HRESULT Result;

      if( (Result = m_DOMDocument.CoCreateInstance(__uuidof(DOMDocument))) != S_OK  )
         APL_THROW( _T("Ошибка при создании DOMDocument: ") << GetHRErrorInfo(Result) );

      APL_CHECK_S_OK( m_DOMDocument.QueryInterface(&m_CurRootNode) );
   }

   void EndSave() 
   {
      ATL::CComBSTR Xml;
      BSTR TmpStr = 0;
      
      //Если использовать &Xml как параметр то BoundsChecker 7.2, необоснованно ругается на утечку памяти
      m_DOMDocument->get_xml( &TmpStr ); 
      Xml.Attach(TmpStr);
      
      WriteBSTRToStream( m_Os, Xml );
   }


   //Начать новый уровень вложенности с именем Name
   void BeginLevel( const TChar *Name )
   {
      TDOMElement Element;
      ATL::CComBSTR tagName( Name );
      TDOMNode NewChild;

      APL_CHECK_S_OK( m_DOMDocument->createElement(tagName, &Element) );
      APL_CHECK_S_OK( m_CurRootNode->appendChild(Element, &NewChild) );

      m_CurRootNode = NewChild;
   }

   //Закончить уровень вложенности с именем Name
   void EndLevel( const TChar *Name )
   {
      TDOMNode NewChild;
      
      APL_CHECK_S_OK( m_CurRootNode->get_parentNode(&NewChild) );
      m_CurRootNode = NewChild;
   }

   //Сохранить данные Ob с именем Name
   //Дожен быть определён оператор Ostream << Ob
   template< class T >
   void SaveItem( const TChar *Name, const T &Ob )
   {
      TStringStream StringStream;
      StringStream.copyfmt( m_Os );    //Настраиваем флаги

      WriteToStream( StringStream, Ob );
      
      ATL::CComBSTR Value( StringStream.str().c_str() );
      TDOMText TextNode;
      ATL::CComBSTR tagName( Name );
      TDOMNode NewChild;

      BeginLevel( Name );
      APL_CHECK_S_OK( m_DOMDocument->createTextNode(Value, &TextNode) );
      APL_CHECK_S_OK( m_CurRootNode->appendChild(TextNode, &NewChild) );
      EndLevel( Name );
   }
};


///////////////////////////////////////////////////////////////////////////////
// Чтение из XML при помощи MSXMl
///////////////////////////////////////////////////////////////////////////////
template < class CharT, template <class> class CharTraitsT >
class CMSXMLReader: public CCOMBase, private NonCopyable
{
   typedef ATL::CComPtr<IXMLDOMDocument> TDOMDocument;
   typedef ATL::CComPtr<IXMLDOMElement> TDOMElement;
   typedef ATL::CComPtr<IXMLDOMNode> TDOMNode;
   typedef ATL::CComPtr<IXMLDOMText> TDOMText; 
   typedef ATL::CComPtr<IXMLDOMNodeList> TDOMNodeList; 

   typedef std::basic_stringstream< CharT, CharTraitsT<CharT> > TStringStream;

protected:
   typedef std::basic_istream< CharT, CharTraitsT<CharT> > TIstream;
   typedef typename CharT TChar;

private:
   TIstream &m_Is;
   TDOMDocument m_DOMDocument;
   TDOMNode m_CurRootNode;

private:
   //Проверка потока после чтения данных
   bool CheckStream( const TStringStream & Stream ) const
   {
      std::ios::iostate State( Stream.rdstate() );
       
      //Мы должны считать до конца файла и не должно быть ошибок
      return (State & std::ios::eofbit) && !(State & std::ios::failbit);
   }
   
   
   //Найти у узла ParentNode потомка с именем Name и присвоить ChildNode указатель на него
   //Возвр: Удалось ли найти
   bool FindChild( TDOMNode ParentNode, const ATL::CComBSTR &Name, TDOMNode &ChildNode )
   {
      TDOMNodeList NodeList;
      TDOMNode CurNode;
      ATL::CComBSTR NodeName;

      APL_CHECK_S_OK( ParentNode->get_childNodes(&NodeList) );

      long Length, i;

      APL_CHECK_S_OK( NodeList->get_length(&Length) );

      for( i = 0; i < Length; ++i )
      {
         CurNode.Release();
         NodeName.Empty();
         APL_CHECK_S_OK( NodeList->get_item(i, &CurNode) );
         APL_CHECK_S_OK( CurNode->get_nodeName(&NodeName) );

         if( Name == NodeName )
         {
            ChildNode = CurNode;
            return true;
         }
      }

      return false;
   }

   //Прочитать текст узла 
   //T должен выполнять [Правило 1]
   template< class T >
   void ReadNode( TDOMNode Node, T &Ob )
   {
      TDOMNode TextNode;
      DOMNodeType NodeType;
      CComVariant Value;
      TDOMNodeList NodeList;
      long Length, i;

      APL_CHECK_S_OK( Node->get_childNodes(&NodeList) );
      APL_CHECK_S_OK( NodeList->get_length(&Length) );

      for( i = 0; i < Length; ++i )
      {
         TextNode.Release();
         APL_CHECK_S_OK( NodeList->get_item(i, &TextNode) );
         APL_CHECK_S_OK( TextNode->get_nodeType(&NodeType) );

         if( NodeType == NODE_TEXT )
            break;
      }

      TStringStream Stream;
      Stream.copyfmt(m_Is);   //Настраиваем флаги

      if( i < Length )
      {
         APL_CHECK_S_OK( TextNode->get_nodeValue(&Value) );
         APL_ASSERT( Value.vt == VT_BSTR );

         WriteBSTRToStream( Stream, Value.bstrVal );
      }

      //При записи пустой строки текстовый потомок не создаётся
      //APL_THROW( _T("У элемента не найден ни один текстовый потомок") );
       
      ReadFromStream( Stream, Ob );
      
      if( !CheckStream( Stream ) )
      {
         APL_THROW( 
            _T("Значение '") << ConvertToTStr(Stream.str()) << _T("' узла '") << 
            GetNodePath(Node) << _T("' не соответствует формату") 
         );
      }
   }

   //Получить полный путь узла для отладочного сообщения
   std::basic_string<TCHAR> GetNodePath( TDOMNode Node ) const
   {
      typedef std::basic_string<TCHAR> TDstStr;
      typedef std::vector<TDOMNode> TNodeVec;

      TNodeVec vecPath; 
      DOMNodeType NodeType;

      //Сохраняем всех родителей на пути для того чтобы сохранить правильный порядок вывода узлов
      //Если этого не делать получится инвертированный путь
      for(;;)
      {
         APL_CHECK_S_OK( Node->get_nodeType(&NodeType) );

         if( NodeType == NODE_DOCUMENT )
            break;

         vecPath.push_back( Node );
         Node.Release();
         APL_CHECK_S_OK( vecPath.back()->get_parentNode(&Node) );        
      }

      TDstStr RetVal;
      ATL::CComBSTR NodeName;

      for( TNodeVec::reverse_iterator I = vecPath.rbegin(); I != vecPath.rend(); ++I )
      {
         NodeName.Empty();
         (*I)->get_nodeName( &NodeName );

         RetVal += _T("/");
         RetVal += ConvertToBuf<TDstStr>( static_cast<const WCHAR *>(NodeName) );
      }

      return RetVal;
   }

protected:
   //Курсор, инкапсулирует внутреннюю реализацию поиска
   struct TFindCursor
   {
   private:
      ATL::CComBSTR ElementName;   //Имя искомого элемента
      TDOMNodeList NodeList;  //Текущий список Узлов
      TDOMNode RootNode;      //Узел, который был текущем корнем до начала поиска

      friend class CMSXMLReader;   
   };

   explicit CMSXMLReader( TIstream &Is ): m_Is(Is) {}

   //Начать/Закончить загрузку
   void BeginLoad()
   {
      m_DOMDocument.Release();
      m_CurRootNode.Release();

      HRESULT Result;

      if( (Result = m_DOMDocument.CoCreateInstance(__uuidof(DOMDocument))) != S_OK  )
         APL_THROW( _T("Ошибка при создании DOMDocument: ") << GetHRErrorInfo(Result) );
      
      APL_CHECK_S_OK( m_DOMDocument->put_async( FALSE ) );
      APL_CHECK_S_OK( m_DOMDocument->put_resolveExternals( FALSE ) );
      APL_CHECK_S_OK( m_DOMDocument.QueryInterface(&m_CurRootNode) );
      
      ATL::CComBSTR Str;
      VARIANT_BOOL isSuccessful;
      ReadBSTRFromStream( m_Is, Str );

      if( m_DOMDocument->loadXML( Str, &isSuccessful ) != S_OK || !isSuccessful/*isSuccessful != TRUE */) // isSuccessful почему-то равен -1
         APL_THROW( _T("Ошибка при загрузке XML") );
   }

   void EndLoad(){}

   //Перейти на уровень Name
   bool BeginLevel( const TChar *Name )
   {
      return FindChild(m_CurRootNode, Name, m_CurRootNode);
   }

   //Выйти с уровня Name
   void EndLevel( const TChar *Name )
   {
      TDOMNode NewChild;

      APL_CHECK_S_OK( m_CurRootNode->get_parentNode(&NewChild) );
      m_CurRootNode = NewChild;
   }

   //Загрузить Ob с именем Name
   //T должен выполнять [Правило 1]
   template< class T >
   bool LoadItem( const TChar *Name, T &Ob )
   {
      TDOMNode CurNode;

      if( !FindChild(m_CurRootNode, Name, CurNode) )
         return false;

      ReadNode( CurNode, Ob );
      return true;
   }

   //Инициализировать поиск всех элементов с именем Name
   void FindInit( const TChar *Name, TFindCursor &FindCursor )
   {
      FindCursor.ElementName = Name;
      FindCursor.NodeList.Release();
      FindCursor.RootNode = m_CurRootNode;
      APL_CHECK_S_OK( m_CurRootNode->get_childNodes(&FindCursor.NodeList) );
   }

   //Найти следующий (в т.ч. первый) элемент.
   //FindCursor - должен быть заранее инициализирован функцией FindInit
   //Возвр - true элемент успешно записан, false - элемент найти не удалось
   //T должен выполнять [Правило 1]
   template< class T >
   bool FindNext( TFindCursor &FindCursor, T &Ob)
   {
      TDOMNode CurNode;
      ATL::CComBSTR NodeName;

      for( ;; )
      {
         CurNode.Release();
         if( FindCursor.NodeList->nextNode(&CurNode) != S_OK )
         {
            m_CurRootNode = FindCursor.RootNode;
            return false;
         }

         NodeName.Empty();
         APL_CHECK_S_OK( CurNode->get_nodeName(&NodeName) );

         if( NodeName == FindCursor.ElementName )
         {
            ReadNode(CurNode, Ob);
            return true;
         }
      }
   }

   bool FindNext( TFindCursor &FindCursor )
   {
      TDOMNode CurNode;
      ATL::CComBSTR NodeName;

      for( ;; )
      {
         CurNode.Release();
         if( FindCursor.NodeList->nextNode(&CurNode) != S_OK )
         {
            m_CurRootNode = FindCursor.RootNode;
            return false;
         }

         NodeName.Empty();
         APL_CHECK_S_OK( CurNode->get_nodeName(&NodeName) );

         if( NodeName == FindCursor.ElementName )
         {
            m_CurRootNode = CurNode;
            return true;
         }
      }
   }

public:
   //Получить путь к текущему узлу
   std::basic_string<TCHAR> GetCurNodePath() const
   {
      return GetNodePath( m_CurRootNode );
   }
};

} //namespace NWLib 

#endif