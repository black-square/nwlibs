#include "..\..\FirstHeader.h"
#include "..\..\Serialization\Serialization.hpp"
#include "..\..\Serialization\MSXML.hpp"
#include "..\..\Serialization\TinyXML.hpp"
#include "..\..\TestHelper.h"
#include "..\..\CoutConvert.h"
#include "..\..\FileSystem.h"

#include <list>
#include <fstream>

using namespace std;
using namespace NWLib;

class Coord
{
   int m_X, m_Y;

public:
   Coord(): m_X(0), m_Y(0) {}
   Coord(int X, int Y): m_X(X), m_Y(Y) {}

   template< class Archive >
      void Serialize( Archive &A ) 
   {
      A.Element( _T("X"), m_X );
      A.Element( _T("Y"), m_Y );
   }
};
///////////////////////////////////////////////////////////////////////////////

class CA
{
   unsigned int m_A;
   double m_B;
   basic_string<TCHAR> m_S;
   std::vector<long> m_Container;
   std::list<Coord> m_CordContainer;

public:
   CA(unsigned int A = 0, double B = 0, basic_string<TCHAR> S = basic_string<TCHAR>() ): m_A(A), m_B(B), m_S(S) 
   {
      while( A-- )
      {
         m_Container.push_back((long)(B *= 2));
         m_CordContainer.push_back(Coord( A, 3 * A ));
      }
   }

   template< class Archive >
   void Serialize( Archive &A ) 
   {
      A.Element( _T("speed"), m_A );
      A.Element( _T("weight"), m_B );
      A.Element( _T("name"), m_S );
      A.STLContainerElement( _T("Container"), _T("item"), m_Container );
      A.STLContainerGroup( _T("CordContainer"), _T("Coord"), m_CordContainer );
  }
};
///////////////////////////////////////////////////////////////////////////////

class CB
{
   CA m_Ob1;
   CA m_Ob2;
   basic_string<TCHAR> m_Name;

   std::list<Coord> m_Coords;
   std::vector<double> m_DblVector;

public:
   CB( const CA &A1 = CA(), const CA &A2 = CA(), const basic_string<TCHAR> &Name = basic_string<TCHAR>(), int n = 0): m_Ob1( A1), m_Ob2( A2), m_Name( Name )
   { 
      while( n-- )
      {
         m_Coords.push_back( Coord( n*100, n *10 ) );
         m_DblVector.push_back( n * 1.1 );
      }
   }

   const CA &GetOb1() const { return m_Ob1; }
   void SetOb1(const CA &Val) { m_Ob1 = Val; }
   
   const CA &GetOb2() const { return m_Ob2; }
   void SetOb2(const CA &Val) { m_Ob2 = Val; }
   
   const basic_string<TCHAR> &GetName() const { return m_Name; }
   void SetName( const basic_string<TCHAR> &Value ) { m_Name = Value; }
   
   const std::list<Coord> &GetCoords() const { return m_Coords; }
   void SetCoords( const std::list<Coord> &Value ) { m_Coords = Value; }

   const std::vector<double> &GetDblVector() const { return m_DblVector; }
   void SetDblVector( const std::vector<double> &Value ) { m_DblVector = Value; }
};
///////////////////////////////////////////////////////////////////////////////

namespace NWLib {
template< class Archive >
void Serialize( Archive &A, CB& ob )
{
   A.Element( _T("name"), ob, SetGetBind<CB, basic_string<TCHAR> >( &CB::SetName, &CB::GetName ) );
   A.Group( _T("truck"), ob, SetGetBind<CB, CA>( &CB::SetOb1, &CB::GetOb1 ) );
   A.Group( _T("anothercar"), ob, SetGetBind<CB, CA>( &CB::SetOb2, &CB::GetOb2 ) );
   A.STLContainerGroup( _T("coords"), _T("coord"), ob, SetGetBind<CB, std::list<Coord> >( &CB::SetCoords, &CB::GetCoords ) );
   A.STLContainerElement( _T("dblvector"), _T("double"), ob, SetGetBind<CB, std::vector<double> >( &CB::SetDblVector, &CB::GetDblVector ) );
}
}

//Простой тест
struct Foo
{
   int m_A;
   double m_B;
   std::basic_string<TCHAR> m_C;

   Foo( int A = 0, double B = 0, const std::basic_string<TCHAR> &C = _T("<NONAME>")): m_A(A), m_B(B), m_C(C) {}

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
   Foo m_F1;
   Foo m_F2;
   std::basic_string<TCHAR> m_Name;
   
   Bar(const Foo &F1 = Foo(), const Foo &F2 = Foo(), const std::basic_string<TCHAR> &Name = _T("<ANYNAME>")): m_F1(F1), m_F2(F2), m_Name(Name) {}

   template< class Archive >
   void Serialize( Archive &A ) 
   {
      A.Element( _T("name"), m_Name );
      A.Group( _T("foo1"), m_F1 );
      A.Group( _T("foo2"), m_F2 );
   }
};

///////////////////////////////////////////////////////////////////////////////

template< class T >
void SaveToFile( const char *FileName, const TCHAR *RootTagName, T &Object )
{
   #ifndef UNICODE
      basic_ofstream<TCHAR> Stream( FileName, ios::binary );
      Stream.setf(ios_base::boolalpha);
      CWriteArchive<TCHAR, CMSXMLWriter> Archive(Stream);
      Archive.Save( RootTagName, Object );
   #else
      //В Юникоде стандартные потоки не работают, совсем!
      basic_stringstream<TCHAR> Stream;
      Stream.setf(ios_base::boolalpha);
      CWriteArchive<TCHAR, CMSXMLWriter> Archive(Stream);
      Archive.Save( RootTagName, Object );
      
      basic_ofstream<CHAR> FileStream ( FileName, ios::binary );  
      FileStream << CHAR( 0xFF ) << CHAR(0xFE);
      WriteUnicode( FileStream, Stream.str() );
   #endif
}
///////////////////////////////////////////////////////////////////////////////

template< class T >
void SaveToFileUTF8( const char *FileName, const TCHAR *RootTagName, T &Object )
{
   ofstream Stream( FileName, ios::binary );
   Stream.setf(ios_base::boolalpha);
   CWriteArchive<TCHAR, CTinyXMLWriter> Archive(Stream);
   Archive.Save( RootTagName, Object );
}
///////////////////////////////////////////////////////////////////////////////

template< class T >
void LoadFromFile( const char *FileName, const TCHAR *RootTagName, T &Object )
{
   #ifndef UNICODE
      basic_ifstream<TCHAR> Stream( FileName, ios::binary );
      Stream.setf(ios_base::boolalpha);
      CReadArchive<TCHAR, CMSXMLReader> Archive(Stream);
      Archive.Load( RootTagName, Object );
   #else
      //В Юникоде стандартные потоки не работают, совсем!
      wstring FileNameW, XML;
      Convert( FileName, FileNameW );
      
      CUnicodeFileReader FR(FileNameW.c_str());
      FR.Read( XML, CUnicodeFileReader::SNoDelimCharacter() );

      basic_stringstream<TCHAR> Stream(XML);
      Stream.setf(ios_base::boolalpha);
      CReadArchive<TCHAR, CMSXMLReader> Archive(Stream);
      Archive.Load( RootTagName, Object );
   #endif
}
///////////////////////////////////////////////////////////////////////////////

template< class T >
void LoadFromFileUTF8( const char *FileName, const TCHAR *RootTagName, T &Object )
{
   ifstream Stream( FileName, ios::binary );
   Stream.setf(ios_base::boolalpha);
   CReadArchive<TCHAR, CTinyXMLReader> Archive(Stream);
   Archive.Load( RootTagName, Object );
}
///////////////////////////////////////////////////////////////////////////////

template< class T >
basic_string<TCHAR> GetXML( const TCHAR *RootTagName, T &Object )
{
   basic_stringstream<TCHAR> Stream;
   CWriteArchive<TCHAR, CMSXMLWriter> Archive(Stream);
   Archive.Save( RootTagName, Object );
   return Stream.str();
}
///////////////////////////////////////////////////////////////////////////////

typedef std::list<Coord> TCoordList;

namespace NWLib {
template< class Archive >
void Serialize( Archive &A, TCoordList& ob )
{
   A.STLContainerGroup( _T("coords"), _T("coord"), ob );
}
}


///////////////////////////////////////////////////////////////////////////////
// Потестим необязательные элементы
///////////////////////////////////////////////////////////////////////////////
struct Bar2
{
   Foo m_F1;
   Foo m_F2;
   bool m_Flag;

   Bar2( const Foo &F1 = Foo(), const Foo &F2 = Foo(), bool Flag = false ): m_F1(F1), m_F2(F2), m_Flag(Flag) {}

   template< class Archive >
   void Serialize( Archive &A ) 
   {
      A.ElementEx( _T("flag"), m_Flag, NWLib::CNotMandatory() );
      A.Group( _T("foo1"), m_F1 );
      A.GroupEx( _T("foo3"), m_F2, NWLib::CNotMandatory() );
   }
};

int _tmain()
{ 
   TConsoleAutoStop ConsoleAutoStop;
   
   ::CoInitialize(NULL);

   APL_TRY()
   {  
      SetCurrentDirectory( GetExeDirPath().c_str() );
      
      for( int i = 0; i < 1; ++i )
      {
         CB cb(CA(4, 2.6, _T("honda")), CA(3, -4.7, _T("\"ОКА\"<капсула смерти>")), _T("Два слова"), 8 );
         cout << GetXML(_T("cars"), cb) << endl;
         CB cb2, cb2_UTF8;
         
         Bar B( Foo(1, 2.0, _T("Первый")), Foo(-1, -2.0e-11, _T("Второй")), _T("<ROOT>") );
         Bar B2, B2_UTF8;

         cout << GetXML(_T("bar"), B2) << endl;

         SaveToFile( "OUTLite.xml", _T("bar"), B );
         LoadFromFile("OUTLite.xml", _T("bar"), B2 );
         SaveToFile(  "OUTLiteResult.xml", _T("bar"), B2 );

         SaveToFileUTF8( "UTF8_OUTLite.xml", _T("bar"), B );
         LoadFromFileUTF8( "UTF8_OUTLite.xml", _T("bar"), B2_UTF8 );
         SaveToFileUTF8( "UTF8_OUTLiteResult.xml", _T("bar"), B2_UTF8 );

         SaveToFile( "OUTTmp.xml", _T("cars"), cb2 );
         SaveToFile( "OUT.xml",  _T("cars"), cb );
         LoadFromFile( "OUT.xml",  _T("cars"), cb2 );
         SaveToFile( "OUTResult.xml",  _T("cars"), cb2 );

         SaveToFileUTF8( "UTF8_OUTTmp.xml", _T("cars"), cb2_UTF8 );
         SaveToFileUTF8( "UTF8_OUT.xml",  _T("cars"), cb );
         LoadFromFileUTF8( "UTF8_OUT.xml",  _T("cars"), cb2_UTF8 );
         SaveToFileUTF8( "UTF8_OUTResult.xml",  _T("cars"), cb2_UTF8 );

         TCoordList CordList, CordList2, CordList2_UTF8;

         for( int i = 0; i < 20; i += 2 )
            CordList.push_back( Coord(i, i + 1) );

         SaveToFile( "OUTListTCoordList.xml",  _T("cars"), CordList );
         LoadFromFile( "OUTListTCoordList.xml",  _T("cars"), CordList2 );
         SaveToFile( "OUTListTCoordListOUT.xml",  _T("cars"), CordList2 );

         SaveToFileUTF8( "UTF8_OUTListTCoordList.xml",  _T("cars"), CordList );
         LoadFromFileUTF8( "UTF8_OUTListTCoordList.xml",  _T("cars"), CordList2_UTF8);
         SaveToFileUTF8( "UTF8_OUTListTCoordListOUT.xml",  _T("cars"), CordList2_UTF8 );


         Bar2 Br; 
         #ifdef UNICODE
            LoadFromFile( "..\\Bar2Test_UNICODE.xml",  _T("bar"), Br );
         #else
            LoadFromFile( "..\\Bar2Test_ANSI.xml",  _T("bar"), Br );
         #endif

         SaveToFile( "Bar2TestOut.xml",  _T("bar"), Br );
         SaveToFileUTF8( "UTF8_Bar2TestOut.xml",  _T("bar"), Br );

         Bar2 Br_UTF8;
         #ifdef UNICODE
            LoadFromFileUTF8( "..\\Bar2Test_UTF8.xml",  _T("bar"), Br_UTF8 );
         #else
            LoadFromFileUTF8( "..\\Bar2Test_ANSI.xml",  _T("bar"), Br_UTF8 );
         #endif

         SaveToFileUTF8( "UTF8_Bar2TestOut_UTF8.xml",  _T("bar"), Br_UTF8 );
     }

      extern void TestStreamFlags();

      TestStreamFlags();
   }
   APL_CATCH()

   ::CoUninitialize();

   return 0;
}


void TestStreamFlags()
{
   typedef std::basic_stringstream< TCHAR > TStringStream;

   TStringStream Stream( _T("1234z 234.4 test") );

   int i;
   float f;
   TCHAR str[100];

   if( !((Stream >> i).rdstate( ) & ios::failbit) )
      std::cout << "NO FAILBIT: " << i << std::endl;
   else 
   {
      std::cout << "FAILBIT" << std::endl;
      Stream.clear();

   }

   if( !((Stream >> f).rdstate( ) & ios::failbit) )
      std::cout << "NO FAILBIT: " << f << std::endl;
   else
   {
      std::cout << "FAILBIT" << std::endl;
      Stream.clear();
   }

   if( !((Stream >> str).rdstate( ) & ios::failbit) )
      std::cout << "NO FAILBIT: " << str << std::endl;
   else
   {
      std::cout << "FAILBIT" << std::endl;
      Stream.clear();
   }

   std::cout << "FLAGS_AFTER: " << std::hex << Stream.rdstate() << std::endl;
}