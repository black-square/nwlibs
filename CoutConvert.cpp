﻿//Автор: Шестеркин Дмитрий(NW) 2005

#include "stdafx.h"
#include <iostream>
#include <vector>
#include <limits>

namespace std
{
std::ostream &operator <<( std::ostream &stream, const char * const obj )
{  
   APL_ASSERT_PTR(obj);
   
   int Rez = static_cast<int>(strlen(obj));

   if( &stream == &std::cout )
   {
	   std::vector<CHAR> Vec(Rez + 1); //+1 Для избежания проблем с пустой строкой

	   CharToOemBuffA(obj, &Vec[0], Rez);

	   stream.write(&Vec[0], static_cast<std::streamsize>(Rez) );
   }
   else
   {
      stream.write(obj, Rez );
   }

	return stream;
}
///////////////////////////////////////////////////////////////////////////////

std::ostream &operator<< ( std::ostream &stream, const std::string &obj )
{   
   if( &stream == &std::cout )
   {
      int Rez = static_cast<int>(obj.length());

      std::vector<CHAR> Vec(Rez + 1); //+1 Для избежания проблем с пустой строкой

	   CharToOemBuffA(obj.c_str(), &Vec[0], Rez);

	   stream.write(&Vec[0], static_cast<std::streamsize>(Rez) );
   }
   else
   {
      stream.write( obj.c_str(), static_cast<std::streamsize>(obj.length()) );
   }

	return stream;
}
///////////////////////////////////////////////////////////////////////////////

std::ostream &operator<<( std::ostream &stream, const wchar_t * const obj )
{
   APL_ASSERT_PTR(obj);
   
   int Rez = static_cast<int>(wcslen(obj));
	
	std::vector<CHAR> Vec(Rez + 1); //+1 Для избежания проблем с пустой строкой

	APL_CHECK( WideCharToMultiByte(CP_ACP, 0, obj, Rez, &Vec[0], static_cast<int>(Vec.size()), NULL, NULL) == Rez);
   
   if( &stream == &std::cout )
      APL_CHECK( CharToOemBuffA(&Vec[0], &Vec[0], Rez) );

	stream.write(&Vec[0], static_cast<std::streamsize>(Rez) );
	return stream;
}
///////////////////////////////////////////////////////////////////////////////

std::ostream &operator<< ( std::ostream &stream, const std::wstring &obj )
{
	int Rez = static_cast<int>(obj.length()); 
	
	std::vector<CHAR> Vec(Rez + 1); //+1 Для избежания проблем с пустой строкой

	APL_CHECK( WideCharToMultiByte(CP_ACP, 0, obj.c_str(), Rez, &Vec[0], static_cast<int>(Vec.size()), NULL, NULL) == Rez );
   
   if( &stream == &std::cout )
      APL_CHECK( CharToOemBuffA(&Vec[0], &Vec[0], Rez ) );

	stream.write(&Vec[0], static_cast<std::streamsize>(Rez) );
	return stream;
}
///////////////////////////////////////////////////////////////////////////////

//std::ostream &operator<<( std::ostream &stream, allocator<int>::size_type obj )
//{
//   typedef unsigned long long TConvertType;
//   APL_ASSERT( obj <= std::numeric_limits<TConvertType>::max() );
//   return stream << (TConvertType)(obj);
//}

} //namespace std

namespace NWLib {

} //namespace NWLib 
