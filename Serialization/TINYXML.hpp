//�����: ��������� �������(NW) 2007

#ifndef TINYXML_HPP
#define TINYXML_HPP

///////////////////////////////////////////////////////////////////////////////
// ���������� ����������� Writer � Reader � �������������� ������� 
// TinyXML http://tinyxml.sourceforge.net/
// ������������� ������������ ���������� 
// UTF8-CPP http://utfcpp.sourceforge.net/
// ������ ���������� �� ������ � �� ����� ��������� ��������� UTF-8 � �� 
// ������� �� ������������ ������ ����� ���� �������������� ��������� � COM 
// ��������
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// ����������� � ���������� TinyXML, ��� ��� � �� ����� ������� ����� ��� 
// ������� ��� �������� ���������� ���������, �.�. UTF-16 �� �� ��������, � � 
// UTF-8: "�����, ��������� ������ �� �������� � ������� ������ 128, ��� 
// ������ � UTF-8 ������������ � ������� ����� ASCII. � ��������, � ������ 
// UTF-8 <b>����� ���� �� ��������� ������ 128 ���������� ������ ASCII � ��� 
// �� �����</b> (http://ru.wikipedia.org/wiki/UTF-8). 
// ������������, ����� ������� ������������ ��������� � TinyXML, ��� ��� �������
// ����� UTF-8 ������� (2-4 �����) ��� ���������������� �� ��������� ������ XML
// ������.
// � �� ��� � ����, ��� �������� ����������� ��������� �� ����������� �� 
// ���������� ��������.
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
// ����� ������������ ����������� �� UNF-16 � UTF-8
///////////////////////////////////////////////////////////////////////////////
template< class CharT >
class CUTF16toUTF8Converter {};

//���� ��������� ������ char �������������� �� �����
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

//���� ��������� ������ wchar_t ������������ ��� ������ utf8cpp
//����� ������ ���������� std::exception
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
         APL_THROW( _T("������ ��� ����������� UTF-16 � UTF-8: ") << ConvertToTStr(e.what()) );      	
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
         APL_THROW( _T("������ ��� ����������� UTF-16 � UTF-8: ") << ConvertToTStr(e.what()) );      	
      }

   }
};

///////////////////////////////////////////////////////////////////////////////
// ����� ������������ ����������� �� UNF-8 � UTF-16
///////////////////////////////////////////////////////////////////////////////
template< class CharT >
class CUTF8toUTF16Converter {};

//���� ��������� ������ char �������������� �� �����
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

//���� ��������� ������ wchar_t ������������ ��� ������ utf8cpp
//����� ������ ���������� std::exception
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
         APL_THROW( _T("������ ��� ����������� UTF-8 � UTF-16: ") << ConvertToTStr(e.what()) );      	
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
         APL_THROW( _T("������ ��� ����������� UTF-8 � UTF-16: ") << ConvertToTStr(e.what()) );      	
      }
   }
};

///////////////////////////////////////////////////////////////////////////////
// ������ � XML ��� ������ TinyXML
// ������� TiXmlCreateElement � TiXmlCreateText ���� ��������� � TinyXML ����,
// ��������� ��. tinyxml.h
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

   //������/��������� ��������
   void BeginSave() 
   {
      m_pDocument.reset(new TiXmlDocument );
      m_pCurNode = m_pDocument.get();
   }

   //��������� ������ � ���������� � ������������ stream
   void EndSaveToStream() 
   {
      m_Os << *m_pDocument;
   }

   //��������� ������ � TiXmlPrinter
   void EndSaveToPrinter( TiXmlPrinter &Printer ) 
   {
      Printer.SetStreamPrinting();
      m_pDocument->Accept( &Printer );
   }

   //������ ����� ������� ����������� � ������ Name
   void BeginLevel( const TChar *Name )
   {
      APL_ASSERT_PTR(m_pCurNode);
      m_pCurNode = m_pCurNode->LinkEndChild( TiXmlCreateElement(TBase::Convert(Name)) );
   }

   //��������� ������� ����������� � ������ Name
   void EndLevel( const TChar *Name )
   {
      APL_ASSERT_PTR(m_pCurNode);

      m_pCurNode = m_pCurNode->Parent();  
   }

   //��������� ������ Ob � ������ Name
   //����� ���� �������� �������� Ostream << Ob
   template< class T >
   void SaveItem( const TChar *Name, const T &Ob )
   {
      TStringStream StringStream;

      //����������� �����, ������� StringStream.copyfmt( m_Os ); ������, �.�. ���� StringStream � m_Os
      //������
      StringStream.fill(m_Os.fill());
      static_cast<std::ios_base&>(StringStream).copyfmt(m_Os);

      WriteToStream( StringStream, Ob );

      BeginLevel( Name );
      m_pCurNode->LinkEndChild( TiXmlCreateText(TBase::Convert(StringStream.str())) );
      EndLevel( Name );
   }
};

///////////////////////////////////////////////////////////////////////////////
// ������ �� XML ��� ������ TinyXML
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
   //�������� ������ ����� ������ ������
   bool CheckStream( const TStringStream & Stream ) const
   {
      std::ios::iostate State( Stream.rdstate() );
       
      //�� ������ ������� �� ����� ����� � �� ������ ���� ������
      return (State & std::ios::eofbit) && !(State & std::ios::failbit);
   }
     
   //����� � ���� pParentNode ������� � ������ Name � ��������� pChildNode ��������� �� ����
   //�����: ������� �� �����
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

   //��������� ����� ���� 
   //T ������ ��������� [������� 1]
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
      
      //����������� �����, ������� StringStream.copyfmt( m_Os ); ������, �.�. ���� StringStream � m_Is
      //������
      Stream.fill(m_Is.fill());
      static_cast<std::ios_base&>(Stream).copyfmt(m_Is);

      if( pCurNode != 0 )
         WriteToStream( Stream, TUTF8toUTF16::Convert(pCurNode->Value()) );

      //��� ������ ������ ������ ��������� ������� �� ��������
      //������� �� ����� ��� �� �����
      //APL_THROW( _T("� �������� �� ������ �� ���� ��������� �������") );
      
      ReadFromStream( Stream, Ob );
      
      if( !CheckStream( Stream ) )
      {
         APL_THROW( 
            _T("�������� '") << ConvertToTStr(Stream.str()) << _T("' ���� '") << 
            GetNodePath(pNode) << _T("' �� ������������� �������") 
         );
      }
   }

   //�������� ������ ���� ���� ��� ����������� ���������
   std::basic_string<TCHAR> GetNodePath( TiXmlNode *pNode ) const
   {
      APL_ASSERT_PTR(pNode);

      typedef std::basic_string<TCHAR> TDstStr;
      typedef std::vector<TiXmlNode *> TNodeVec;

      TNodeVec vecPath; 
      Detail::CUTF8toUTF16Converter<TCHAR> Converter;

      //��������� ���� ��������� �� ���� ��� ���� ����� ��������� ���������� ������� ������ �����
      //���� ����� �� ������ ��������� ��������������� ����
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
   //������, ������������� ���������� ���������� ������
   struct TFindCursor
   {
   private:
      std::string ElementName;     //��� �������� ��������
      TiXmlNode *pCurNode;         //���������� ���� � ������
      TiXmlNode *pRootNode;        //����, ������� ��� ������� ������ �� ������ ������

      friend class CTinyXMLReaderBase;   
   };

   explicit CTinyXMLReaderBase( TIstream &Is ): m_Is(Is), m_pRootNode(0), m_ForceUTF8(false) {}

   //��������� ������ �� ����������� � ������������ stream
   void BeginLoadFromStream()
   {
      m_pDocument.reset( new TiXmlDocument );
      m_pRootNode = m_pDocument.get();

      //� TinyXML ���� ������������� �������� >>, �� �� �������� �������� ��-�� 
      //�������������� ��������, ������� � ������ �������� �� �����
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
         APL_THROW( _T("������ ��� �������� XML: ") << ConvertToTStr(m_pDocument->ErrorDesc()) );
   }

   //��������� ������ �� std::string
   void BeginLoadFromString( const std::string &String )
   {
      m_pDocument.reset( new TiXmlDocument );
      m_pRootNode = m_pDocument.get();

      m_pDocument->Parse( String.c_str(), 0, m_ForceUTF8 ? TIXML_ENCODING_UTF8 : TIXML_DEFAULT_ENCODING );

      if( m_pDocument->Error() )
         APL_THROW( _T("������ ��� �������� XML: ") << ConvertToTStr(m_pDocument->ErrorDesc()) );
   }

   void EndLoad(){}

   //������� �� ������� Name
   bool BeginLevel( const TChar *Name )
   {
      APL_ASSERT_PTR(m_pRootNode);
      return FindChild(m_pRootNode, TUTF16toUTF8::Convert(Name), m_pRootNode);
   }

   //����� � ������ Name
   void EndLevel( const TChar *Name )
   {
      APL_ASSERT_PTR(m_pRootNode);
      m_pRootNode = m_pRootNode->Parent();
      APL_ASSERT_PTR(m_pRootNode);
   }

   //��������� Ob � ������ Name
   //T ������ ��������� [������� 1]
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

   //���������������� ����� ���� ��������� � ������ Name
   void FindInit( const TChar *Name, TFindCursor &FindCursor )
   {
      APL_ASSERT_PTR( m_pRootNode );
      
      FindCursor.ElementName = TUTF16toUTF8::Convert(Name);
      FindCursor.pRootNode = m_pRootNode;
      FindCursor.pCurNode = 0;
   }

   //����� ��������� (� �.�. ������) �������.
   //FindCursor - ������ ���� ������� ��������������� �������� FindInit
   //����� - true ������� ������� �������, false - ������� ����� �� �������
   //T ������ ��������� [������� 1]
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
   //�������� ���� � �������� ����
   std::basic_string<TCHAR> GetCurNodePath() const
   {
      APL_ASSERT_PTR( m_pRootNode );
      return GetNodePath( m_pRootNode );
   }

   //��������� ������� �� ��� �����, ������� ����� ��������, ������� � ��������� UTF8
   //���� ��������� ���������� �� ��������� �� �� ��� ����� ���������� ���������� ������������������ ��� 
   //�� ��������, ������� ����� ������ ������������ ������ �������� ������ ��� �����������
   void SetForceUTF8( bool Val ){ m_ForceUTF8 = Val; }
};
} //namespace Detail

///////////////////////////////////////////////////////////////////////////////
// ������ � XML ��� ������ TinyXML
// ������������ ������ � ����� Os, ���������� � ������������
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
// ������ � XML ��� ������ TinyXML
// ����������� �����������: ������ ������������ �� ��������� ����� std::string,
// � ����� Os ���������� � ������������ ������������ ������ ��� ����������� ������ 
// �������
// �������� �������, ��� CTinyXMLWriter
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
   //� ������ ������ ����� �������� �������������� XML, �� ����� ������ ������� CWriteArchive::Save()
   const std::string &Str() const { return m_pPrinter->Str(); }
};                                  

///////////////////////////////////////////////////////////////////////////////
// ������ �� XML ��� ������ TinyXML
// ������������ ������ � ����� Is, ���������� � ������������
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
// ������ �� XML ��� ������ TinyXML
// ����������� �����������: ������ ������������ �� ���������� ������ std::string,
// � ����� Is ���������� � ������������ ������������ ������ ��� ����������� ������ 
// �������
// �������� �������, ��� CTinyXMLReader
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
   //�� ������ ������ ����� ������������ ������ XML, �� ����� ������ CReadArchive::Load()
   std::string &Str() { return m_InputBuff; }
   const std::string &Str() const { return m_InputBuff; }
};

} //namespace NWLib 

#endif