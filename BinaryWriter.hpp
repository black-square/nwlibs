#ifndef BinaryWriter_HPP
#define BinaryWriter_HPP

#include <hash_map>

///////////////////////////////////////////////////////////////////////////////
// Класс реализует базовые операции для записи бинарных файлов под 
// некоторую, не совпадающую с текущей, платформу:
//    - Запись произвольного типа
//    - Выравнивание позиции по опр. границе
//    - Отложенные указатели (удобный способ записывать структуры заранее 
//      неизвестного размера)
//
// Параметр PtrDiffWriterT должен наследовать TDefaultPtrDiffWriter и должен 
// заметстить необх. функции
///////////////////////////////////////////////////////////////////////////////
template< class PtrDiffWriterT >
class TBinaryWriter;

struct TDefaultPtrDiffWriter
{
   //Записать смещение указателя diff, в сконвертированном под целевую
   //платформу, формате
   void writePtrDiff( long diff );
};
///////////////////////////////////////////////////////////////////////////////

template<class PtrDiffWriterT = TDefaultPtrDiffWriter>
class TBinaryWriter: private PtrDiffWriterT
{
public:     
   TBinaryWriter(): m_pFile(0) {}
   template< class T > explicit TBinaryWriter( const T &v ): m_pFile(0), PtrDiffWriterT(v) {}
   ~TBinaryWriter() { close(); }

   //Открыть файл szFileName
   void open( const TCHAR *szFileName )
   {
      m_pFile = _tfopen( szFileName, _T("wb") );

      if( m_pFile == 0 )
         APL_THROW( _T("Ошибка при открытии файла ") << NWLib::ConvertToTStr(szFileName) );
   }

   //Закрыть файл
   void close()
   {
      APL_ASSERT( m_defPtrs.empty() );

      if( m_pFile ) 
         fclose( m_pFile );

      m_pFile = 0;
   }

   //Записать отложенный указатель.
   //Функция резервирует место в файле под целочисленное смещениев в байтах, относительно текущей позиции,
   //данных, которые сейчас находятся по адресу realPointer, но при этом сами данные не записываются.
   //Вместо этого, realPointer запоминается и в будущем при вызове функции write, если её параметр 
   //realPointer совпадёт с запомненном значением, то в зарезервированное место записывается 
   //действиетльное значение смещения, которое теперь можно оперделить.
   //Параметр realPointer должен совпадать со значением которое в последствии будет 
   //передано в функцию write
   //
   //Например, если нужно записать структуру с двумя строками переменной длины код будет 
   //следующий:
   /*
      static void m( TBinaryWriter &bw, const std::string &s1, const std::string &s2 )
      {
         //Записываем сначала указатели
         bw.writeDeferredPtr(&s1);
         bw.writeDeferredPtr(&s2);

         //Затем сами строки
         bw.write( s1.c_str(), &s1, s1.size() );
         bw.write( s2.c_str(), &s2, s2.size() );
      }
   */
   void writeDeferredPtr( const void *realPointer )
   {
      APL_ASSERT( m_pFile != 0 );
      APL_ASSERT( realPointer != 0 );
      m_defPtrs.insert( TDeferredPointers::value_type(realPointer, ftell(m_pFile)) );
      PtrDiffWriterT::writePtrDiff( -1 ); //Резервируем место
   }

   //Записать базовый тип данных data.
   //Тип T может быть одим из базовых типов char, int, float, если на целевой платформы типы 
   //пердставляются таким же образом как и на текущей. Если типы представляются иначе, нужно 
   //сконвертировать типы должным образом и передать родное значение для целевой платформы 
   //(вплоть до того, что все типы можно записывать побайтно).
   //
   //Не важно, сконвертирован тип или нет, значение realPointer должно указывать на родной
   //для данной платформы тип (если его запись происходит по частям, то значения для каждой 
   //части будут совпадать), это нужно для правильной работы функции writeDeferredPtr.
   //Если на целевой платформе представление типов совпадает, то realPointer == pData
   //
   //sizeofData - количество байт, которое нужно записать
   template< class T >
   void write( const T *pData, const void *realPointer, size_t sizeofData = sizeof(T) )
   {
      APL_ASSERT( m_pFile != 0 );
      APL_ASSERT( realPointer != 0 );
      correctDeferredPtrs( realPointer );
      
      if( fwrite(pData, sizeofData, 1, m_pFile) != 1 )
         APL_THROW( _T("Ошибка при записи в файл") );
   }

   //Преобразовать Big-endian в Little-endian и наоборот
   template< class T >
   static T inverse( T val )
   {
      static const T byteMask = ~( ~T(0) << CHAR_BIT );
      T rez = 0;
      size_t n = sizeof( T );

      while( n-- )
      {
         rez <<= CHAR_BIT;
         rez |= val & byteMask;
         val >>= CHAR_BIT;
      }

      return rez;
   }
   
   //Выровнить следующую операцию записи по границе n байт
   void align( int n )
   {
      APL_ASSERT( m_pFile != 0 );
      APL_ASSERT( ((n - 1) & n) == 0 ); //Степень двойки
      
      const long curPlace = ftell( m_pFile );
      const long tmp = n - 1;
      const long alignedPlace = (curPlace + tmp) & (~tmp);

      long bytesToWrite = alignedPlace - curPlace;
      const unsigned char data = 0;

      while( bytesToWrite-- )
         if( fwrite(&data, sizeof(data), 1, m_pFile) != 1 )
            APL_THROW( _T("Ошибка при записи в файл") );
   }

private:
   //Скорректировать, если нужно, отложенные указатели на realPointer, т.к. realPointer
   //будет записан в текущую позицию
   void correctDeferredPtrs( const void *realPointer )
   {
      APL_ASSERT( m_pFile != 0 );
      APL_ASSERT( realPointer != 0 );

      const std::pair<TDeferredPointers::iterator, TDeferredPointers::iterator> defPtrs = 
         m_defPtrs.equal_range( realPointer ); 

      if( defPtrs.first == defPtrs.second )
         return;

      APL_ASSERT( m_pFile != 0 );

      //Сохраняем текущую позицию
      const long curPlace = ftell( m_pFile );

      for( TDeferredPointers::iterator it = defPtrs.first; it != defPtrs.second; ++it )
      {
         APL_ASSERT( curPlace > it->second );

         //Возвращаемся на зарезервированное место и записываем смещение
         fseek( m_pFile, it->second, SEEK_SET );
         PtrDiffWriterT::writePtrDiff( curPlace - it->second );
      }

      m_defPtrs.erase( defPtrs.first, defPtrs.second );
      
      //Востанавливаем текущую позицию
      fseek( m_pFile, curPlace, SEEK_SET );
   }

private:
   //Контейнер для хранения отложенных указателей
   typedef stdext::hash_multimap<const void *, long> TDeferredPointers;
   
private:
   TDeferredPointers m_defPtrs;
   FILE *m_pFile;
};

#endif
