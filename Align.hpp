//Автор: Шестеркин Дмитрий(NW) 2009

#ifndef Align_HPP
#define Align_HPP

namespace NWLib {
namespace Detail {
    ///////////////////////////////////////////////////////////////////////////////
    // Целочисленный беззнаковый тип к которому можно безопасно, при помощи 
    // reinterpret_cast, привести любой указатель, выполнить некоторые 
    // арифметические действия над ним и снова привести к указателю
    ///////////////////////////////////////////////////////////////////////////////
    typedef UINT_PTR TUIntPtr;
    //STATIC_ASSERT( sizeof(TUIntPtr) == sizeof(void *) );
    //STATIC_ASSERT( ALIGN_OF(TUIntPtr) == ALIGN_OF(void *) );
} //namespace Detail

///////////////////////////////////////////////////////////////////////////////
// Является ли значение val степенью двойки
///////////////////////////////////////////////////////////////////////////////
template<class T>
inline bool numberIsPower2( T val )
{
    return ( val & (val - 1) ) == 0;
}

///////////////////////////////////////////////////////////////////////////////
// Выравнивание переменной x или указателя px по границе base вверх или вниз
///////////////////////////////////////////////////////////////////////////////
template <typename ValueT>
inline ValueT alignUp(ValueT x, size_t base)
{
    APL_ASSERT( numberIsPower2(base) );

    const size_t tmp = base - 1;
    return static_cast<ValueT>( (x + tmp) & ~tmp );
}
///////////////////////////////////////////////////////////////////////////////

template <typename ValueT>
inline ValueT *alignUp(ValueT *px, size_t base)
{
    return reinterpret_cast<ValueT *>(alignUp(reinterpret_cast<Detail::TUIntPtr>(px), base));
}
///////////////////////////////////////////////////////////////////////////////

template <typename ValueT>
inline ValueT alignDown(ValueT x, size_t base)
{
    APL_ASSERT( numberIsPower2(base) );

    return static_cast<ValueT>( x & ~(base - 1) );
}
///////////////////////////////////////////////////////////////////////////////

template <typename ValueT>
inline ValueT *alignDown(ValueT *px, size_t base)
{
    return reinterpret_cast<ValueT *>(alignDown(reinterpret_cast<Detail::TUIntPtr>(px), base));
}

///////////////////////////////////////////////////////////////////////////////

} //namespace NWLib

#endif