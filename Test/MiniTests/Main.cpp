//#define NWLIB_STOCONA_FRAMEWORK
#include "..\..\FirstHeader.h"
#include "..\..\FileSystem.h"
#include "..\..\TestHelper.h"
#include "..\..\CoutConvert.h"
#include "..\..\MemoryManager.hpp"

#include <shlwapi.h>

#include<map>

using namespace std;

#ifdef NWLIB_STOCONA_FRAMEWORK
using namespace StoconaSearch::Lib;
SET_LOAD_MANAGER_IMPLEMENTATION;
#else
using namespace NWLib;
#endif

///////////////////////////////////////////////////////////////////////////////
namespace SelectOperatorTest
{
typedef std::map<int, double> TStorage;

template<class Iterator, class Value>
struct Selector: SelectorHelper<Iterator, Value> 
{
   reference operator ()( origin_iterator_reference V ) const { return V.second; }
};

typedef CSelectIterator<typename TStorage::iterator, Selector<typename TStorage::iterator, double> > TResultIterator;
typedef CSelectIterator<typename TStorage::const_iterator, Selector<typename TStorage::const_iterator, const double> > TConstResultIterator;


void Run()
{
   TStorage Storage;

   Storage[17] = 8.5;
   
   TResultIterator Iter(Storage.begin());
   TConstResultIterator ConstIter(Storage.begin());
   TConstResultIterator ConstIter2(Iter);
   //TResultIterator Iter2(ConstIter); //Не должно компилится
   
   ConstIter = Iter;
   //Iter = ConstIter; //Не должно компилится

   Iter == Iter;
   Iter == ConstIter;
   ConstIter == Iter;
   ConstIter == ConstIter;

   //*ConstIter = 3; //Не должно компилится
   cout << *ConstIter << endl;
   
   *Iter = 25.5;
   cout << *Iter << endl;


}
} //namespace SelectOperatorTest
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
namespace TryConvertOrCreateTest
{
   struct A 
   {
      std::string S_;
      A( std::string S = ""): S_(S) {}
   };

   struct B
   {
      std::string S_;
      B( const A &a ): S_(a.S_){}
   };

   struct C 
   {
      std::string S_;
   };
   
   
   void Run()
   {
      A a("TEST");

      B b( TryConvertOrCreate<A, B>::Get(a));
      C c( TryConvertOrCreate<A, C>::Get(a));
      B b2(a);
      //C c2(a); //Не должен компилится

      cout << b.S_ << endl << c.S_ << endl;
   }

} //namespace TryConvertOrCreateTest




///////////////////////////////////////////////////////////////////////////////
void TestIntegerToStringImpl( int N, int BufSize )
{
   std::vector<char> Buf( BufSize );
   std::vector<char>::iterator I = ConvertIntegerToString( N, Buf.begin(), Buf.end() - 1 );

   cout << "Число: " << N  << " Размер буфера: " << BufSize << " Результат: '";


   if( I == Buf.begin() )
      cout << "ОШИБКА";
   else
   {
      *I = '\0';
      cout << &Buf[0];
   }

   cout << "'" << endl;
}

void TestIntegerToString()
{
   TestIntegerToStringImpl( 123456, 10 );
   TestIntegerToStringImpl( -456123, 10 );
   TestIntegerToStringImpl( -12, 4 );
   TestIntegerToStringImpl( -12, 3 );
   TestIntegerToStringImpl( 1, 2 );
   TestIntegerToStringImpl( 0, 2 );
   TestIntegerToStringImpl( 0, 1 );
}



namespace ClonePtrTest
{
struct A
{
   A *Clone() const { return new A; }
};


struct B
{
   B *Clone() const { return new B; }
};


void Run()
{
   clone_ptr_ex<A> APtr(new A);
   clone_ptr_ex<const A> ConstAPtr(new A);
   clone_ptr_ex<B> BPtr(new B);

   auto_ptr_ex<A> APtr2(new A);
   auto_ptr<A> APtr3(new A);

   APtr = APtr;

   //APtr = ConstAPtr;
   ConstAPtr = APtr;
   ConstAPtr = APtr2;
   ConstAPtr = APtr3;
}
} //namespace ClonePtrTest

namespace ObjectsPool
{
   template TObjectsPool<int>;

   void Run()
   {

   }

} //namespace ClonePtrTest


void AssertPtr()
{
   int Int;
   const char **rgChar[20];
   auto_ptr_ex<int> AutoPtr( new int() );

   APL_ASSERT_PTR( &Int );
   APL_ASSERT_PTR( rgChar );
   APL_ASSERT_PTR( AutoPtr );
   APL_ASSERT_PTR( "Test" );
}

int _tmain()
{
   TConsoleAutoStop ConsoleAutoStop;

   APL_TRY()
   {  
      SetCurrentDirectory( GetExeDirPath().c_str() );
      wstring Str;
     
      BOOL Rez = PathIsDirectory("..\\\\tst\\");
      cout << "PathIsDirectory " << Rez << " " << GetDWErrorInfo( GetLastError() ) << endl;
      
      
      //cout << "ReadUnicodeFile " << GetDWErrorInfo( ReadUnicodeFile("test.txt", Str) ) << endl;
      //cout << Str << endl;
      //cout << "WriteUnicodeFile " << GetDWErrorInfo( WriteUnicodeFile("test3.txt", Str) ) << endl;

      //BOOL Rez = CreateDirectory("TST", NULL);
      //cout << "CreateDirectory " << Rez << " " << GetDWErrorInfo( GetLastError() ) << endl;
      //cout << "CopyDirectory " << GetDWErrorInfo( CopyDirectory( "c:\\SS\\StoconaSearch\\Database\\", "c:\\SS\\StoconaSearch\\TST\\" ) ) << endl;
      //cout << "DeleteDirectory " << GetDWErrorInfo( DeleteDirectory( "c:\\SS\\StoconaSearch\\TST\\" ) ) << endl;

      //cout << "CreateFullPath " << GetDWErrorInfo( CreateFullPath( "TST\\You represent and warrant that you have the adequate legal capacity to enter into this Agreement  You further represent and warrant that you will use the Software only for lawful purposes and in accordan1111111111111111111111111111111111111111111ce with this Agreement, an" ) ) << endl;


      //SelectOperatorTest::Run();
      //TryConvertOrCreateTest::Run();

      AssertPtr();
      TestIntegerToString();
   }
   APL_CATCH()

   return 0;
}