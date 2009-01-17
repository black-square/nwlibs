//�����: ��������� �������(NW) 2006

#ifndef Serialization_HPP
#define Serialization_HPP

#include <sstream>
#include "..\Auxiliary.h"

/*
   ����� ������� ������� ��������� ��������� XML ������ ��������� � ��������� ������ ������ ���������
   ������� � �����������.
   
   ��� ���� ����� ��������������� �����������/��������� ����������� �������� (����� ��� int, double, 
   std::string ) ������ ��������� [������� 1] 

   [������� 1: ������]
      ��������� ��� ������ ������ ����������� ������� ReadFromStream, WriteToStream. 
      ���������� ���� ������� ��-��������� ������� ���������� >> � << ��������������, 
      ��� ������� ������������ ������.

      template< class ObjectT, class CharT, class CharTraitsT >
      void WriteToStream( std::basic_ostream<CharT, CharTraitsT> &Ostream, const ObjectT &Object );

      template< class ObjectT, class CharT, class CharTraitsT >
      void ReadFromStream( std::basic_istream<CharT, CharTraitsT> &Istream, ObjectT &Object );

      ���� ������ ���������� >> � << �����������������, �� ���������� ������ �� �����������.
   [������� 1: �����]


   ��� ���� ����� ��������������� �����������/��������� ������ ����� (��� ���������) � ������� 
   ��������� ��������� ��������� ���������� ������ ��������� [������� 2] 

   [������� 2: ������]
      ����� ������ ���������� �����:       
      template< class Archive > void Serialize( Archive &A ) 

      ���� ����������� ������� 

      template< class Archive >void NWLib::Serialize ( Archive &A, T& ob )
      ��� T ����� ������� ��������� ������/������.

      Archive - ����� ������� ��������� ���������/���������� ��� ��������� ����������, 
      ��� � ��������� ������ � ����������, ����� ����-�� ���������:*/
#if 0
      //�������� ����� ����������
      //Accessor  - ����� ������� ��������� �������� ������ � ����� ��������� ������ [������� 3]
      //
      //SettingsT - ����� ������/������, ���� ����� ������������ ������ ��������� FindErrorReportT [������� 4]
      //            ������� ��� ����� ��������� ���������� ��������� �� ��������� CThrowExceptionOnFindError 
      //            (������� ����������)


      //���������/��������� ��������� ���� ob ��������� ������ (class ��� struct) ��� ������ Name
      //��� ���� T ������ ��������� [������� 1]
      template< class T >
      void Element( const TChar *Name, /*const*/ T &ob )
      {
         ElementEx( Name, ob, CDefaultSettings() );
      }
      
      template< class T, class SettingsT >
      void ElementEx( const TChar *Name, /*const*/ T &ob, const SettingsT &Settings );

      //���������/��������� ��������� ���� ��������� ������ (class ��� struct) ob ��� ������ Name
      //��� ���� T ������ ��������� [������� 1] 
      template< class T, class AccessorT >
      void Element( const TChar *Name, /*const*/ T &ob, const AccessorT &Accessor )
      {
         ElementEx( Name, ob, Accessor, CDefaultSettings() );
      }
      
      template< class T, class AccessorT, class SettingsT  >
      void Element( const TChar *Name, /*const*/ T &ob, const AccessorT &Accessor, const SettingsT &Settings );

      //���������/��������� ��������� ��������� ob ��������� ������ (class ��� struct) ��� ������ Name
      //��� ���� T ������ ��������� [������� 2]
      template< class T >
      void Group( const TChar *Name, /*const*/ T &ob )
      {
         GroupEx( Name, ob, CDefaultSettings() );
      }

      template< class T, class SettingsT >
      void GroupEx( const TChar *Name, /*const*/ T &ob, const SettingsT &Settings  );

      //���������/��������� ��������� ��������� ob ��������� ������ (class ��� struct) ��� ������ Name
      //��� ���� T ������ ��������� [������� 2] 
      template< class T, class AccessorT >
      void Group( const TChar *Name, /*const*/ T &ob, const AccessorT &Accessor );
      {
         GroupEx( Name, ob, Accessor, CDefaultSettings() );
      }

      template< class T, class AccessorT, class SettingsT >
      void Group( const TChar *Name, /*const*/ T &ob, const AccessorT &Accessor, const SettingsT &Settings  );

      //���������/��������� STL ��������� ob ��������� ������ (class ��� struct) ��� ������ Name.
      //ob �������� ������� �������� ��� ������� ������ ����������� [������� 1].
      //������ ��������� ������� ����������� ��� ������ ItemName
      template< class T >
      void STLContainerElement( const TChar *Name, const TChar *ItemName, /*const*/ T &ob )
      {
         STLContainerElementEx( Name, ItemName, ob, CDefaultSettings() );
      }
      
      template< class T, class SettingsT >
      void STLContainerElement( const TChar *Name, const TChar *ItemName, /*const*/ T &ob, const SettingsT &Settings  );

      //���������/��������� STL ��������� ob ��������� ������ (class ��� struct) ��� ������ Name.
      //ob �������� ������� �������� ��� ������� ������ ����������� [������� 1].
      //������ ��������� ������� ����������� ��� ������ ItemName
      template< class T, class AccessorT >
      void STLContainerElement( const TChar *Name, const TChar *ItemName, /*const*/ T &ob, const AccessorT &Accessor )
      {
         STLContainerElementEx( Name, ItemName, ob, Accessor, CDefaultSettings() );
      }

      template< class T, class AccessorT, class SettingsT >
      void STLContainerElement( const TChar *Name, const TChar *ItemName, /*const*/ T &ob, const AccessorT &Accessor, const SettingsT &Settings );

      //���������/��������� STL ��������� ob ��������� ������ (class ��� struct) ��� ������ Name.
      //ob �������� �������� ������ ��� ������� ������ ����������� [������� 2].
      //������ ��������� ������� ����������� ��� ������ ItemName
      template< class T >
      void STLContainerGroup( const TChar *Name, const TChar *ItemName, /*const*/ T &ob )
      {
         STLContainerGroupEx( Name, ItemName, ob, CDefaultSettings() );
      }
      
      template< class T, class SettingsT >
      void STLContainerGroup( const TChar *Name, const TChar *ItemName, /*const*/ T &ob, const SettingsT &Settings  );

      //���������/��������� STL ��������� ob ��������� ������ (class ��� struct) ��� ������ Name.
      //ob �������� �������� ������ ��� ������� ������ ����������� [������� 2].
      //������ ��������� ������� ����������� ��� ������ ItemName
      template< class T, class AccessorT >
      void STLContainerGroup( const TChar *Name, const TChar *ItemName, /*const*/ T &ob, const AccessorT &Accessor )
      {
         STLContainerGroupEx( Name, ItemName, ob, Accessor, CDefaultSettings() );
      }
      template< class T, class AccessorT, class SettingsT >
      void STLContainerGroup( const TChar *Name, const TChar *ItemName, /*const*/ T &ob, const AccessorT &Accessor, const SettingsT &Settings  );
#endif
/*
   [������� 2: �����]

   [������� 3: ������]
      ��������� AccessorT
      ���������� ����������� ���������������� �� ����������� ���� ��������� 
      ���������� ��� ������ ��������� Accessor � �������� ������ CWriteArchive. ��� ���� Accessor
      ������ ������������ ������ ���������.

      interface AccessorT
      {
         typedef ... ValueType;   //��������� ��� � �������� ����������� ������
         typedef ... ParamValue;  //��� ������������� ������������� ���� ���� ValueType, ���� const ValueType &

         //����� � ��������� ������ �������� ���������� �����
         ParamValue Get( Object &Obj ) const;  
         
         //����� � ��������� ������ ������������� ������ �� �������� ���������� �����
         //��� ������������ ������ Serialize. ����� ����� �������� � �������������
         //const_cast
         ValueType &GetRefForSerialize( Object &Obj ) const;  

         //���������� � ��������� ������ �������� ���������� �����
         void Set( Object &Obj, ParamValue Val ) const;
      };

   [������� 3: �����]

   [������� 4: ������]
      ��������� FindErrorReportT
      ������������ ������ ��� ������ ������, ���������� ������ �������������� �� ������ ��� ������ 
      ������� ��������. 
      � �������� ������������ ��� ���� ����� ���� ������� ���������� ���� ������ �� ������ � ����� 
      ������� ������� ��� ������� �� ������������ ��� ������

      interface FindErrorReportT
      {
         //���������� � ��� ������ ����� �� ������ ������� � ������ Name
         //� ������ ������ Reader
         template<class CharT, class TReader>
         void FindErrorReport(const CharT *Name, const TReader &Reader) const;

      }
   [������� 4: �����]


   ��� ������ ���������� �������������� ����� CWriteArchive. ����� ������� �����: */
#if 0  
   //��������� ��� �������� ��������� � ��� ��������� � �� ��������� � �����
   //Name - �������� ����� ��������� ������
   //ob - ������ �� ��������� ������ (class ��� struct)
   //��� ���� T ������ ��������� [������� 2]
   template< class T > void Save( const TChar *Name,  T &ob );
#endif
/*
   ��� ������ ���������� �������������� ����� CReadArchive. ����� ������� �����: */
#if 0  
   //��������� ��� �������� ��������� � ��� ��������� � �� ��������� � �����
   //Name - �������� ����� ��������� ������
   //ob - ������ �� ��������� ������ (class ��� struct)
   //��� ���� T ������ ��������� [������� 2]
   template< class T > void Load( const TChar *Name,  T &ob );
#endif


#if 0
//������� ������ (����� ������� ��. Test\Serialization\Main.cpp)
struct Foo
{
   int m_A;
   double m_B;
   std::basic_string<TCHAR> m_C;

   Foo(): m_A(1), m_B(-12.3e55), m_C(_T("Any String")) {}

   template< class Archive >
   void Serialize( Archive &A ) 
   {
      A.Element( _T("a"), m_A );
      A.Element( _T("b"), m_B );
      A.Element( _T("c"), m_C );
   }
};

struct Bar
{
   Foo F1;
   Foo F2;
   std::basic_string<TCHAR> m_Name;
   
   Bar(): m_Name(_T("Any Name")) {}

   template< class Archive >
   void Serialize( Archive &A ) 
   {
      A.Element( _T("name"), m_Name );
      A.Group( _T("foo1"), F1 );
      A.Group( _T("foo2"), F2 );
   }

   basic_string<TCHAR> GetXML()
   {
      basic_stringstream<TCHAR> Stream;
      Bar B;
      CWriteArchive<basic_stringstream<TCHAR> > Archive(Stream);
      Archive.Save( _T("bar"), B );
      return Stream.str();
   }
};

/* GetXML() ����������:
<bar>
	<name>Any Name</name>
	<foo1>
		<a>1</a>
		<b>-1.23e+056</b>
		<c>Any String</c>
	</foo1>
	<foo2>
		<a>1</a>
		<b>-1.23e+056</b>
		<c>Any String</c>
	</foo2>
</bar>
*/

#endif

namespace NWLib {

///////////////////////////////////////////////////////////////////////////////
// ������� �������������� �������� ��-���������, �� ����������� �������� ��� � 
// ������� ����������
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// ��������� �� ��������� ��� ������� ������� ����������� ������-������.
// ����� ������ Serialize. ����� ����������� ��� ��������� �������, � �� ������
// �������-����.
///////////////////////////////////////////////////////////////////////////////
template< class Archive, class T >
inline void Serialize( Archive &A, T& ob )
{
   ob.Serialize( A );
}

///////////////////////////////////////////////////////////////////////////////
// ������ � ����� ��-���������. ����� ��������� <<
///////////////////////////////////////////////////////////////////////////////
template< class ObjectT, class CharT, class CharTraitsT >
inline void WriteToStream( std::basic_ostream<CharT, CharTraitsT> &Ostream, const ObjectT &Object )
{
   Ostream << Object;
}

///////////////////////////////////////////////////////////////////////////////
// ������ �� ������ ��-���������. ����� ��������� >>
///////////////////////////////////////////////////////////////////////////////
template< class ObjectT, class CharT, class CharTraitsT >
inline void ReadFromStream( std::basic_istream<CharT, CharTraitsT> &Istream, ObjectT &Object )
{
   Istream >> Object;
}

template< class CharT, class CharTraitsT >
inline void ReadFromStream( std::basic_istream<CharT, CharTraitsT> &Istream, std::basic_string<CharT, CharTraitsT> &Object )
{
#if 0
   std::getline( Istream, Object );

   //���� �� ��������� ������ ������, �� ����� ���������� ���� failbit, 
   //� �� ������ ��������� ������ ������ �����, ������� ���������� ��� ����
   Istream.clear( Istream.rdstate() & ~std::ios::failbit );
#else  
   CharT Buf[100];
   
   Object.clear();

   do 
   {
      Istream.read( Buf, APL_ARRSIZE(Buf) );    
      Object.append( Buf, Buf + Istream.gcount() );
   } 
   while( Istream.gcount() == APL_ARRSIZE(Buf) );

   //�� ��������� �� ����� ���� � ������� ��� ���������� ���� failbit, ���������� ���
   Istream.clear( Istream.rdstate() & ~std::ios::failbit );
#endif
}


///////////////////////////////////////////////////////////////////////////////
// ������ ����������� ���������������� �� ����������� ������� �����. 
// ���������� ������ ����� ������ ������ ����������� ������ ����������� ���������� � 
// �������� ������� ����� �� ������ ������.
// ������� ����������� ����� ������� ���� ����������� ����, ��� ���� ��������� ������� 
// � ������. ��� ���� ����� ������� ��, ��� ������ �� ������������, �.�. ������� ����������� 
// ������ ���������� ����� �������� �������, � ����� ����� �������������.
// ������ ����� ��������� ���� � �� �� ����������. ����� �� ��� ����������� 
// ������ ��� ���������������� ������ � ������.
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
/*
//�������� ��� ���� ������� �������� ������ ��� ������
//TOstream - ����� ������� ���������� �������� << ��� ������ ����������, ����
//           ��� �������� ����������� ������� WriteToStream
template < class CharT, class CharTraitsT = std::char_traits<CharT> >
interface IWriter
{
protected:
   typedef std::basic_ostream< CharT, CharTraitsT > TOstream;
   typedef typename CharT TChar; 
   
   explicit IWriter( TOstream &Os );

   //������ ����� ������� ����������� � ������ Name
   void BeginLevel( const TChar *Name );
   
   //��������� ������� ����������� � ������ Name
   void EndLevel( const TChar *Name );
   
   //��������� Ob � ������ Name
   //T ������ ��������� [������� 1]
   template< class T >
   void SaveItem( const TChar *Name, const T &Ob );

   //������/��������� ����������
   void BeginSave();
   void EndSave();
};
*/
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
/*
//�������� ��� ���� ������� �������� ������ ��� ������
//TIstream - ����� ������� ���������� �������� >> ��� ����� ����������, ����
//           ��� �������� ����������� ������� ReadFromStream
template < class CharT, class CharTraitsT = std::char_traits<CharT> >
interface IReader
{
protected:
   typedef std::basic_istream< CharT, CharTraitsT > TIstream;  
   typedef typename CharT TChar;  
   
   //������, ������������� ���������� ���������� ������
   class TFindCursor{};
 
   explicit IReader( TIstream &Is );

   //������� �� ������� Name
   //����: ������ �� ������� � ������ Name
   bool BeginLevel( const TChar *Name );
   
   //����� � ������ Name
   void EndLevel( const TChar *Name );
   
   //��������� Ob � ������ Name
   //T ������ ��������� [������� 1]
   //����: ������ �� ������� � ������ Name   
   template< class T >
   bool LoadItem( const TChar *Name, T &Ob );

   //���������������� ����� ���� ��������� � ������ Name
   void FindInit( const TChar *Name, TFindCursor &FindCursor );
   
   //����� ��������� (� �.�. ������) ������� � �������� ��� �������� � ob.
   //FindCursor - ������ ���� ������� ��������������� �������� FindInit
   //����� - true ������� ������� �������, false - ������� ����� �� �������
   //T ������ ��������� [������� 1]
   template< class T >
   bool FindNext( TFindCursor &FindCursor, T &Ob);

   //����� ��������� (� �.�. ������) �������.
   //FindCursor - ������ ���� ������� ��������������� �������� FindInit
   //����� - true ������� ������� �������, false - ������� ����� �� �������
   template< class T >
   bool FindNext( TFindCursor &FindCursor);

   //������/��������� ��������
   void BeginLoad();
   void EndLoad();

public:
   //�������� ���� � �������� ����
   std::basic_string<TCHAR> GetCurNodePath() const;
};
*/
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// ����� ��� ������ ����������. 
// WriterT - ����� ����������� ������ ������
///////////////////////////////////////////////////////////////////////////////
template<class CharT, template<class, template<class> class> class WriterT, template<class> class CharTraits = std::char_traits >
class CWriteArchive: public WriterT<CharT, CharTraits>
{
public:
   typedef WriterT<CharT, CharTraits> TWriter;
   typedef typename TWriter::TChar TChar;
   typedef typename TWriter::TOstream TOstream;

public:
   //����������� ������ ����������� ��������� ������ �� �����
   explicit CWriteArchive( TOstream &Os ): TWriter( Os ) {}
   
   //��������� ��� �������� ��������� � ��� ��������� � �� ��������� � �����
   //Name - �������� ����� ��������� ������
   //ob - ������ �� ��������� ������ (class ��� struct)
   //��� ���� T ������ ��������� [������� 2]
   template< class T >
   void Save( const TChar *Name, /*const*/ T &ob )
   {
      TWriter::BeginSave();
      TWriter::BeginLevel(Name);
      Serialize( *this, ob );
      TWriter::EndLevel( Name );
      TWriter::EndSave();
   }

   //��������� ��������� ���� ob ��������� ������ (class ��� struct) ��� ������ Name
   //��� ���� T ������ ��������� [������� 1]
   template< class T, class SettingsT >
   void ElementEx( const TChar *Name, /*const*/ T &ob, const SettingsT &Settings  )
   {
      TWriter::SaveItem( Name, ob );
   }

   template< class T >
   void Element( const TChar *Name, /*const*/ T &ob )
   {
      ElementEx( Name, ob, CDefaultSettings() );
   }


   //��������� ��������� ���� ��������� ������ (class ��� struct) ob ��� ������ Name
   //��� ���� T ������ ��������� [������� 1] 
   template< class T, class AccessorT, class SettingsT >
   void ElementEx( const TChar *Name, /*const*/ T &ob, const AccessorT &Accessor, const SettingsT &Settings  )
   {
      TWriter::SaveItem( Name, Accessor.Get(ob) );
   }

   template< class T, class AccessorT >
   void Element( const TChar *Name, /*const*/ T &ob, const AccessorT &Accessor )
   {
      ElementEx( Name, ob, Accessor, CDefaultSettings() );
   }

   //��������� ��������� ��������� ob ��������� ������ (class ��� struct) ��� ������ Name
   //��� ���� T ������ ��������� [������� 2]
   template< class T, class SettingsT >
   void GroupEx( const TChar *Name, /*const*/ T &ob, const SettingsT &Settings  )
   {
      TWriter::BeginLevel( Name );
      Serialize( *this, ob );
      TWriter::EndLevel( Name );
   }

   template< class T >
   void Group( const TChar *Name, /*const*/ T &ob )
   {
      GroupEx( Name, ob, CDefaultSettings() );
   }

   //��������� ��������� ��������� ob ��������� ������ (class ��� struct) ��� ������ Name
   //��� ���� T ������ ��������� [������� 2] 
   template< class T, class AccessorT, class SettingsT >
   void GroupEx( const TChar *Name, /*const*/ T &ob, const AccessorT &Accessor, const SettingsT &Settings  )
   {
      TWriter::BeginLevel( Name );
      Serialize( *this, Accessor.GetRefForSerialize(ob) );
      TWriter::EndLevel( Name );
   }

   template< class T, class AccessorT >
   void Group( const TChar *Name, /*const*/ T &ob, const AccessorT &Accessor )
   {
      GroupEx( Name, ob, Accessor, CDefaultSettings() );
   }

   //��������� STL ��������� ob ��������� ������ (class ��� struct) ��� ������ Name.
   //ob �������� ������� �������� ��� ������� ������ ����������� [������� 1].
   //������ ��������� ������� ����������� ��� ������ ItemName
   template< class T, class SettingsT >
   void STLContainerElementEx( const TChar *Name, const TChar *ItemName, /*const*/ T &ob, const SettingsT &Settings  )
   {
      TWriter::BeginLevel( Name );
      
      for( typename T::const_iterator I = ob.begin(); I != ob.end(); ++I )
         TWriter::SaveItem( ItemName, *I );

      TWriter::EndLevel( Name );
   }

   template< class T >
   void STLContainerElement( const TChar *Name, const TChar *ItemName, /*const*/ T &ob )
   {
      STLContainerElementEx( Name, ItemName, ob, CDefaultSettings() );
   }

   
   //��������� STL ��������� ob ��������� ������ (class ��� struct) ��� ������ Name.
   //ob �������� ������� �������� ��� ������� ������ ����������� [������� 1].
   //������ ��������� ������� ����������� ��� ������ ItemName
   template< class T, class AccessorT, class SettingsT >
   void STLContainerElementEx( const TChar *Name, const TChar *ItemName, /*const*/ T &ob, const AccessorT &Accessor, const SettingsT &Settings  )
   {
      TWriter::BeginLevel( Name );
      
      for( typename AccessorT::ValueType::const_iterator I = Accessor.Get(ob).begin(); I != Accessor.Get(ob).end(); ++I )
         TWriter::SaveItem( ItemName, *I );

      TWriter::EndLevel( Name );
   }

   template< class T, class AccessorT >
   void STLContainerElement( const TChar *Name, const TChar *ItemName, /*const*/ T &ob, const AccessorT &Accessor )
   {
      STLContainerElementEx( Name, ItemName, ob, Accessor, CDefaultSettings() );
   }

   //��������� STL ��������� ob ��������� ������ (class ��� struct) ��� ������ Name.
   //ob �������� �������� ������ ��� ������� ������ ����������� [������� 2].
   //������ ��������� ������� ����������� ��� ������ ItemName
   template< class T, class SettingsT >
   void STLContainerGroupEx( const TChar *Name, const TChar *ItemName, /*const*/ T &ob, const SettingsT &Settings  )
   {
      TWriter::BeginLevel( Name );

      for( typename T::iterator I = ob.begin(); I != ob.end(); ++I )
      {
         TWriter::BeginLevel( ItemName );
         Serialize( *this, *I );
         TWriter::EndLevel( ItemName );
      }

      TWriter::EndLevel( Name );
   }

   template< class T >
   void STLContainerGroup( const TChar *Name, const TChar *ItemName, /*const*/ T &ob )
   {
      STLContainerGroupEx( Name, ItemName, ob, CDefaultSettings() );
   }
   
   //��������� STL ��������� ob ��������� ������ (class ��� struct) ��� ������ Name.
   //ob �������� �������� ������ ��� ������� ������ ����������� [������� 2].
   //������ ��������� ������� ����������� ��� ������ ItemName
   template< class T, class AccessorT, class SettingsT >
   void STLContainerGroupEx( const TChar *Name, const TChar *ItemName, /*const*/ T &ob, const AccessorT &Accessor, const SettingsT &Settings  )
   {
      TWriter::BeginLevel( Name );

      for( typename AccessorT::ValueType::iterator I = Accessor.GetRefForSerialize(ob).begin(); I != Accessor.GetRefForSerialize(ob).end(); ++I )
      {
         TWriter::BeginLevel( ItemName );
         Serialize( *this, *I );
         TWriter::EndLevel( ItemName );
      }

      TWriter::EndLevel( Name );
   }

   template< class T, class AccessorT >
   void STLContainerGroup( const TChar *Name, const TChar *ItemName, /*const*/ T &ob, const AccessorT &Accessor )
   {
      STLContainerGroupEx( Name, ItemName, ob, Accessor, CDefaultSettings() );
   }
};

///////////////////////////////////////////////////////////////////////////////
// ����� ��� ������ ����������. 
// ReaderT - ����� ����������� ������ ������
///////////////////////////////////////////////////////////////////////////////
template<class CharT, template<class, template<class> class> class ReaderT, template<class> class CharTraits = std::char_traits >
class CReadArchive: public ReaderT<CharT, CharTraits>
{
public:
   typedef ReaderT<CharT, CharTraits> TReader;
   typedef typename TReader::TChar TChar;
   typedef typename TReader::TIstream TIstream;
   typedef typename TReader::TFindCursor TFindCursor;

public:
   //����������� ������ ����������� ��������� ������ �� �����
   CReadArchive( TIstream &Is ): TReader( Is ) {}
   
   //��������� ��� �������� ��������� � ��� ��������� � �� ��������� � �����
   //Name - �������� ����� ��������� ������
   //ob - ������ �� ��������� ������ (class ��� struct)
   //��� ���� T ������ ��������� [������� 2]
   template< class T >
   void Load( const TChar *Name, T &ob )
   {
      TReader::BeginLoad();
      
      if( !TReader::BeginLevel( Name ) )
      {
         CDefaultSettings().FindErrorReport( Name, static_cast<const TReader &>(*this) );
         return;
      }
      
      Serialize( *this, ob );
      TReader::EndLevel( Name );
      TReader::EndLoad();
   }

   //��������� ��������� ���� ob ��������� ������ (class ��� struct) ��� ������ Name
   //��� ���� T ������ ��������� [������� 1]
   template< class T, class SettingsT >
   void ElementEx( const TChar *Name, T &ob, const SettingsT &Settings  )
   {
      if( !TReader::LoadItem(Name, ob) )
         Settings.FindErrorReport( Name, static_cast<const TReader &>(*this) );
   }

   template< class T >
   void Element( const TChar *Name, T &ob )
   {
      ElementEx( Name, ob, CDefaultSettings() );
   }

   //��������� ��������� ���� ��������� ������ (class ��� struct) ob ��� ������ Name
   //��� ���� T ������ ��������� [������� 1] 
   template< class T, class AccessorT, class SettingsT >
   void ElementEx( const TChar *Name, T &ob, const AccessorT &Accessor, const SettingsT &Settings )
   {
      typename AccessorT::ValueType TmpOb;
      
      if( !TReader::LoadItem(Name, TmpOb) )
      {
         Settings.FindErrorReport( Name, static_cast<const TReader &>(*this) );
         return;
      }

      Accessor.Set(ob, TmpOb);
   }

   template< class T, class AccessorT >
   void Element( const TChar *Name, T &ob, const AccessorT &Accessor )
   {
      ElementEx( Name, ob, Accessor, CDefaultSettings() );
   }

   //��������� ��������� ��������� ob ��������� ������ (class ��� struct) ��� ������ Name
   //��� ���� T ������ ��������� [������� 2]
   template< class T, class SettingsT >
   void GroupEx( const TChar *Name, T &ob, const SettingsT &Settings  )
   {
      if( !TReader::BeginLevel(Name) )
      {
         Settings.FindErrorReport( Name, static_cast<const TReader &>(*this) );
         return;
      }

      Serialize( *this, ob );
      TReader::EndLevel( Name );
   }

   template< class T >
   void Group( const TChar *Name, T &ob )
   {
      GroupEx( Name, ob, CDefaultSettings() );
   }

   //��������� ��������� ��������� ob ��������� ������ (class ��� struct) ��� ������ Name
   //��� ���� T ������ ��������� [������� 2] 
   template< class T, class AccessorT, class SettingsT >
   void GroupEx( const TChar *Name, T &ob, const AccessorT &Accessor, const SettingsT &Settings )
   {
      if( !TReader::BeginLevel(Name) )
      {
         Settings.FindErrorReport( Name, static_cast<const TReader &>(*this) );
         return;
      }

      Serialize( *this, Accessor.GetRefForSerialize(ob) );
      TReader::EndLevel( Name );
   }

   template< class T, class AccessorT >
   void Group( const TChar *Name, T &ob, const AccessorT &Accessor )
   {
      GroupEx( Name, ob, Accessor, CDefaultSettings() );
   }

   //��������� STL ��������� ob ��������� ������ (class ��� struct) ��� ������ Name.
   //ob �������� ������� �������� ��� ������� ������ ����������� [������� 1].
   //������ ��������� ������� ����������� ��� ������ ItemName
   template< class T, class SettingsT >
   void STLContainerElementEx( const TChar *Name, const TChar *ItemName, T &ob, const SettingsT &Settings )
   {
      typename T::value_type TmpOb;
      TFindCursor FindCursor;

      ob.clear();

      if( !TReader::BeginLevel(Name) )
      {
         Settings.FindErrorReport( Name, static_cast<const TReader &>(*this) );
         return;
      }

      TReader::FindInit( ItemName, FindCursor );

      while( TReader::FindNext(FindCursor, TmpOb) )
         ob.push_back( TmpOb );

      TReader::EndLevel( Name );
   }

   template< class T >
   void STLContainerElement( const TChar *Name, const TChar *ItemName, T &ob )
   {
      STLContainerElementEx( Name, ItemName, ob, CDefaultSettings() );
   }
   
   //��������� STL ��������� ob ��������� ������ (class ��� struct) ��� ������ Name.
   //ob �������� ������� �������� ��� ������� ������ ����������� [������� 1].
   //������ ��������� ������� ����������� ��� ������ ItemName
   template< class T, class AccessorT, class SettingsT >
   void STLContainerElementEx( const TChar *Name, const TChar *ItemName, T &ob, const AccessorT &Accessor, const SettingsT &Settings  )
   {
      typename AccessorT::ValueType TmpContainer;
      typename AccessorT::ValueType::value_type TmpValue;

      TFindCursor FindCursor;

      if( !TReader::BeginLevel(Name) )
      {
         Settings.FindErrorReport( Name, static_cast<const TReader &>(*this) );
         return;
      }

      TReader::FindInit( ItemName, FindCursor );

      while( TReader::FindNext(FindCursor, TmpValue) )
      {
         TmpContainer.push_back(TmpValue);
      }

      TReader::EndLevel( Name );

      Accessor.Set(ob, TmpContainer);
   }

   template< class T, class AccessorT >
   void STLContainerElement( const TChar *Name, const TChar *ItemName, T &ob, const AccessorT &Accessor )
   {
      STLContainerElementEx( Name, ItemName, ob, Accessor, CDefaultSettings() );
   }

   //��������� STL ��������� ob ��������� ������ (class ��� struct) ��� ������ Name.
   //ob �������� �������� ������ ��� ������� ������ ����������� [������� 2].
   //������ ��������� ������� ����������� ��� ������ ItemName
   template< class T, class SettingsT >
   void STLContainerGroupEx( const TChar *Name, const TChar *ItemName, T &ob, const SettingsT &Settings )
   {
      typename T::value_type TmpOb;
      TFindCursor FindCursor;

      ob.clear();

      if( !TReader::BeginLevel(Name) )
      {
         Settings.FindErrorReport( Name, static_cast<const TReader &>(*this) );
         return;
      }

      TReader::FindInit( ItemName, FindCursor );

      while( TReader::FindNext(FindCursor) )
      {
         Serialize( *this, TmpOb );
         ob.push_back( TmpOb );
      }

      TReader::EndLevel( Name );
   }

   template< class T >
   void STLContainerGroup( const TChar *Name, const TChar *ItemName, T &ob )
   {
      STLContainerGroupEx( Name, ItemName, ob, CDefaultSettings() );
   }
  
   //��������� STL ��������� ob ��������� ������ (class ��� struct) ��� ������ Name.
   //ob �������� �������� ������ ��� ������� ������ ����������� [������� 2].
   //������ ��������� ������� ����������� ��� ������ ItemName
   template< class T, class AccessorT, class SettingsT >
   void STLContainerGroupEx( const TChar *Name, const TChar *ItemName,  T &ob, const AccessorT &Accessor, const SettingsT &Settings  )
   {
      typename AccessorT::ValueType TmpContainer;
      typename AccessorT::ValueType::value_type TmpValue;

      TFindCursor FindCursor;

      if( !TReader::BeginLevel(Name) )
      {
         Settings.FindErrorReport( Name, static_cast<const TReader &>(*this) );
         return;
      }

      TReader::FindInit( ItemName, FindCursor );

      while( TReader::FindNext(FindCursor) )
      {
         Serialize( *this, TmpValue );
         TmpContainer.push_back(TmpValue);
      }

      TReader::EndLevel( Name );

      Accessor.Set(ob, TmpContainer);
   }

   template< class T, class AccessorT >
   void STLContainerGroup( const TChar *Name, const TChar *ItemName, T &ob, const AccessorT &Accessor )
   {
      STLContainerGroupEx( Name, ItemName, ob, Accessor, CDefaultSettings() );
   }
};

///////////////////////////////////////////////////////////////////////////////
// ���������� ���������� AccessorT [������� 3] ��� ������ ���������� �� ������� Set/Get
// ��� ������������� ���������� ������� ������� 
//    SetGetBind</*��� ��������� ������*/, /*��� ����������� ������������� �����*/>(/*��������� �� ������� ���� Set*/, /*��������� �� ������� ���� Get*/)
//    ��� ��������� �� ������� ����� ����� ���� 0;
///////////////////////////////////////////////////////////////////////////////
template< class Object, class T >
struct CSetGetBindImpl
{
   typedef T ValueType;
   typedef const ValueType &ParamValue;
   typedef void (Object::*SetFunc)(ParamValue);
   typedef ParamValue (Object::*GetFunc)() const;
   
   CSetGetBindImpl( SetFunc SF, GetFunc GF ): m_SetFunc(SF), m_GetFunc(GF) {}
   ParamValue Get( Object &Obj ) const { APL_ASSERT_PTR(m_GetFunc); return (Obj.*m_GetFunc)(); }
   T &GetRefForSerialize( Object &Obj ) const { return const_cast<T &>( Get(Obj) ); }
   void Set( Object &Obj, ParamValue Val ) const { APL_ASSERT_PTR(m_SetFunc); (Obj.*m_SetFunc)(Val); }

private:
   SetFunc m_SetFunc;
   GetFunc m_GetFunc;
};

template< class Object, class T >
struct CSetGetBindImplNoRef
{
   typedef T ValueType;
   typedef ValueType ParamValue;
   typedef void (Object::*SetFunc)(ParamValue);
   typedef ValueType (Object::*GetFunc)() const;

   CSetGetBindImplNoRef( SetFunc SF, GetFunc GF ): m_SetFunc(SF), m_GetFunc(GF) {}
   ValueType Get( Object &Obj ) const { APL_ASSERT_PTR(m_GetFunc); return (Obj.*m_GetFunc)(); }
   T &GetRefForSerialize( Object &Obj ) const 
   { 
      //!!!�� �� ���������!!! 
      //������ ����� ������������� ������ �� rvalue
      return const_cast<T &>( Get(Obj) ); 
   }
   void Set( Object &Obj, ValueType Val ) const { APL_ASSERT_PTR(m_SetFunc); (Obj.*m_SetFunc)(Val); }

private:
   SetFunc m_SetFunc;
   GetFunc m_GetFunc;
};


template< class Object, class T >
CSetGetBindImpl<Object, T> SetGetBind(void (Object::*SetFunc)(const T &), const T &(Object::*GetFunc)() const )
{
   return CSetGetBindImpl<Object, T>( SetFunc, GetFunc);
}

template< class Object, class T >
CSetGetBindImplNoRef<Object, T> SetGetBind(void (Object::*SetFunc)(T), T (Object::*GetFunc)() const )
{
   return CSetGetBindImplNoRef<Object, T>(SetFunc, GetFunc);
}


///////////////////////////////////////////////////////////////////////////////
// ���������� ���������� FindErrorReportT [������� 4]
///////////////////////////////////////////////////////////////////////////////
//������� ���������� ��� ��������� ������� ������
//������������ �� ���������
struct CThrowExceptionOnFindError
{
   //���������� � ��� ������ ����� �� ������ ������� � ������ Name
   template<class CharT, class TReader>
   void FindErrorReport(const CharT *Name, const TReader &Reader) const
   {
      APL_THROW( _T("�� ������ ���� '") << Reader.GetCurNodePath() << _T("/") << ConvertToTStr(Name) << _T("'") );
   }
};

//������ �� ������ ���� ������� �� ������. �.�. ������� �� ������������
struct CNotMandatory
{
   //���������� � ��� ������ ����� �� ������ ������� � ������ Name
   template<class CharT, class TReader>
   void FindErrorReport(const CharT *Name, const TReader &Reader) const
   {
      // ������ �� ������
   }
};

///////////////////////////////////////////////////////////////////////////////
// ����� ��� �������� �� ���������
///////////////////////////////////////////////////////////////////////////////
struct CDefaultSettings: public CThrowExceptionOnFindError {};

///////////////////////////////////////////////////////////////////////////////
// �������������� ������� ����������� ������
///////////////////////////////////////////////////////////////////////////////

//�������� �� ������ Object � ������� ��������� ����� Serialize XML ������ OutXML � 
//��������� ��������� ���� RootName
template< template<class, template<class> class > class WriterT, class Object, class CharInT, class CharOutT >
void GetSerializeXMLString( Object &Object, const CharInT *RootName, std::basic_string<CharOutT> &OutXML, std::ios_base::fmtflags FmtFlags = 0 )
{
   std::basic_stringstream<CharOutT> Stream;

   Stream.setf( FmtFlags );

   CWriteArchive<CharInT, WriterT> Archive(Stream);
   Archive.Save( RootName, Object );
   OutXML = Stream.str();
}

//��������� �� XML ������, � �������� ����� RootName, ������ Object � ������� ��������� ����� Serialize
template< template<class, template<class> class> class ReaderT, class Object, class CharT >
void SetSerializeXMLString( Object &Object, const CharT *RootName, const std::basic_string<CharT> &InXML, std::ios_base::fmtflags FmtFlags = 0 )
{
   std::basic_stringstream<CharT> Stream(InXML);

   Stream.setf( FmtFlags );

   CReadArchive<CharT, ReaderT> Archive(Stream);
   Archive.Load( RootName, Object );
}

} //namespace NWLib 

#endif