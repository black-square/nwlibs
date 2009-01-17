//�����: ��������� �������(NW) 2006

#ifndef MSXML_HPP
#define MSXML_HPP

///////////////////////////////////////////////////////////////////////////////
// ���������� ����������� Writer � Reader � �������������� ������� MSXML
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
      //��� ����� �� �������� ���� �������� CoInitialize � ������������
      //� CoUninitialize � �����������, �� ��� ���������� ������ �.�.
      //"��� �������� ��� ������� ��� �������"
      ::CoInitialize(NULL);
   }
   
   ~CCOMBase()
   {
      ::CoUninitialize();
   }

#endif
};
///////////////////////////////////////////////////////////////////////////////

//������ � ����� ������ BSTR
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
   //� ���� ������ ������� �������������� �� �����
   WriteToStream( Ostream, static_cast<const WCHAR *>(Str) );
}
///////////////////////////////////////////////////////////////////////////////

//��������� �� ������ ������ BSTR
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
   //����������� ����������� ��������� �������
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
// ������ � XML ��� ������ MSXML
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

   //������/��������� ��������
   void BeginSave() 
   {
      m_DOMDocument.Release();
      m_CurRootNode.Release();

      HRESULT Result;

      if( (Result = m_DOMDocument.CoCreateInstance(__uuidof(DOMDocument))) != S_OK  )
         APL_THROW( _T("������ ��� �������� DOMDocument: ") << GetHRErrorInfo(Result) );

      APL_CHECK_S_OK( m_DOMDocument.QueryInterface(&m_CurRootNode) );
   }

   void EndSave() 
   {
      ATL::CComBSTR Xml;
      BSTR TmpStr = 0;
      
      //���� ������������ &Xml ��� �������� �� BoundsChecker 7.2, ������������� �������� �� ������ ������
      m_DOMDocument->get_xml( &TmpStr ); 
      Xml.Attach(TmpStr);
      
      WriteBSTRToStream( m_Os, Xml );
   }


   //������ ����� ������� ����������� � ������ Name
   void BeginLevel( const TChar *Name )
   {
      TDOMElement Element;
      ATL::CComBSTR tagName( Name );
      TDOMNode NewChild;

      APL_CHECK_S_OK( m_DOMDocument->createElement(tagName, &Element) );
      APL_CHECK_S_OK( m_CurRootNode->appendChild(Element, &NewChild) );

      m_CurRootNode = NewChild;
   }

   //��������� ������� ����������� � ������ Name
   void EndLevel( const TChar *Name )
   {
      TDOMNode NewChild;
      
      APL_CHECK_S_OK( m_CurRootNode->get_parentNode(&NewChild) );
      m_CurRootNode = NewChild;
   }

   //��������� ������ Ob � ������ Name
   //����� ���� �������� �������� Ostream << Ob
   template< class T >
   void SaveItem( const TChar *Name, const T &Ob )
   {
      TStringStream StringStream;
      StringStream.copyfmt( m_Os );    //����������� �����

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
// ������ �� XML ��� ������ MSXMl
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
   //�������� ������ ����� ������ ������
   bool CheckStream( const TStringStream & Stream ) const
   {
      std::ios::iostate State( Stream.rdstate() );
       
      //�� ������ ������� �� ����� ����� � �� ������ ���� ������
      return (State & std::ios::eofbit) && !(State & std::ios::failbit);
   }
   
   
   //����� � ���� ParentNode ������� � ������ Name � ��������� ChildNode ��������� �� ����
   //�����: ������� �� �����
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

   //��������� ����� ���� 
   //T ������ ��������� [������� 1]
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
      Stream.copyfmt(m_Is);   //����������� �����

      if( i < Length )
      {
         APL_CHECK_S_OK( TextNode->get_nodeValue(&Value) );
         APL_ASSERT( Value.vt == VT_BSTR );

         WriteBSTRToStream( Stream, Value.bstrVal );
      }

      //��� ������ ������ ������ ��������� ������� �� ��������
      //APL_THROW( _T("� �������� �� ������ �� ���� ��������� �������") );
       
      ReadFromStream( Stream, Ob );
      
      if( !CheckStream( Stream ) )
      {
         APL_THROW( 
            _T("�������� '") << ConvertToTStr(Stream.str()) << _T("' ���� '") << 
            GetNodePath(Node) << _T("' �� ������������� �������") 
         );
      }
   }

   //�������� ������ ���� ���� ��� ����������� ���������
   std::basic_string<TCHAR> GetNodePath( TDOMNode Node ) const
   {
      typedef std::basic_string<TCHAR> TDstStr;
      typedef std::vector<TDOMNode> TNodeVec;

      TNodeVec vecPath; 
      DOMNodeType NodeType;

      //��������� ���� ��������� �� ���� ��� ���� ����� ��������� ���������� ������� ������ �����
      //���� ����� �� ������ ��������� ��������������� ����
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
   //������, ������������� ���������� ���������� ������
   struct TFindCursor
   {
   private:
      ATL::CComBSTR ElementName;   //��� �������� ��������
      TDOMNodeList NodeList;  //������� ������ �����
      TDOMNode RootNode;      //����, ������� ��� ������� ������ �� ������ ������

      friend class CMSXMLReader;   
   };

   explicit CMSXMLReader( TIstream &Is ): m_Is(Is) {}

   //������/��������� ��������
   void BeginLoad()
   {
      m_DOMDocument.Release();
      m_CurRootNode.Release();

      HRESULT Result;

      if( (Result = m_DOMDocument.CoCreateInstance(__uuidof(DOMDocument))) != S_OK  )
         APL_THROW( _T("������ ��� �������� DOMDocument: ") << GetHRErrorInfo(Result) );
      
      APL_CHECK_S_OK( m_DOMDocument->put_async( FALSE ) );
      APL_CHECK_S_OK( m_DOMDocument->put_resolveExternals( FALSE ) );
      APL_CHECK_S_OK( m_DOMDocument.QueryInterface(&m_CurRootNode) );
      
      ATL::CComBSTR Str;
      VARIANT_BOOL isSuccessful;
      ReadBSTRFromStream( m_Is, Str );

      if( m_DOMDocument->loadXML( Str, &isSuccessful ) != S_OK || !isSuccessful/*isSuccessful != TRUE */) // isSuccessful ������-�� ����� -1
         APL_THROW( _T("������ ��� �������� XML") );
   }

   void EndLoad(){}

   //������� �� ������� Name
   bool BeginLevel( const TChar *Name )
   {
      return FindChild(m_CurRootNode, Name, m_CurRootNode);
   }

   //����� � ������ Name
   void EndLevel( const TChar *Name )
   {
      TDOMNode NewChild;

      APL_CHECK_S_OK( m_CurRootNode->get_parentNode(&NewChild) );
      m_CurRootNode = NewChild;
   }

   //��������� Ob � ������ Name
   //T ������ ��������� [������� 1]
   template< class T >
   bool LoadItem( const TChar *Name, T &Ob )
   {
      TDOMNode CurNode;

      if( !FindChild(m_CurRootNode, Name, CurNode) )
         return false;

      ReadNode( CurNode, Ob );
      return true;
   }

   //���������������� ����� ���� ��������� � ������ Name
   void FindInit( const TChar *Name, TFindCursor &FindCursor )
   {
      FindCursor.ElementName = Name;
      FindCursor.NodeList.Release();
      FindCursor.RootNode = m_CurRootNode;
      APL_CHECK_S_OK( m_CurRootNode->get_childNodes(&FindCursor.NodeList) );
   }

   //����� ��������� (� �.�. ������) �������.
   //FindCursor - ������ ���� ������� ��������������� �������� FindInit
   //����� - true ������� ������� �������, false - ������� ����� �� �������
   //T ������ ��������� [������� 1]
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
   //�������� ���� � �������� ����
   std::basic_string<TCHAR> GetCurNodePath() const
   {
      return GetNodePath( m_CurRootNode );
   }
};

} //namespace NWLib 

#endif