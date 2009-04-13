#ifndef BinaryWriter_HPP
#define BinaryWriter_HPP

#include <hash_map>

///////////////////////////////////////////////////////////////////////////////
// ����� ��������� ������� �������� ��� ������ �������� ������ ��� 
// ���������, �� ����������� � �������, ���������:
//    - ������ ������������� ����
//    - ������������ ������� �� ���. �������
//    - ���������� ��������� (������� ������ ���������� ��������� ������� 
//      ������������ �������)
//
// �������� PtrDiffWriterT ������ ����������� TDefaultPtrDiffWriter � ������ 
// ���������� �����. �������
///////////////////////////////////////////////////////////////////////////////
template< class PtrDiffWriterT >
class TBinaryWriter;

struct TDefaultPtrDiffWriter
{
   //�������� �������� ��������� diff, � ����������������� ��� �������
   //���������, �������
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

   //������� ���� szFileName
   void open( const TCHAR *szFileName )
   {
      m_pFile = _tfopen( szFileName, _T("wb") );

      if( m_pFile == 0 )
         APL_THROW( _T("������ ��� �������� ����� ") << NWLib::ConvertToTStr(szFileName) );
   }

   //������� ����
   void close()
   {
      APL_ASSERT( m_defPtrs.empty() );

      if( m_pFile ) 
         fclose( m_pFile );

      m_pFile = 0;
   }

   //�������� ���������� ���������.
   //������� ����������� ����� � ����� ��� ������������� ��������� � ������, ������������ ������� �������,
   //������, ������� ������ ��������� �� ������ realPointer, �� ��� ���� ���� ������ �� ������������.
   //������ �����, realPointer ������������ � � ������� ��� ������ ������� write, ���� � �������� 
   //realPointer ������� � ����������� ���������, �� � ����������������� ����� ������������ 
   //�������������� �������� ��������, ������� ������ ����� ����������.
   //�������� realPointer ������ ��������� �� ��������� ������� � ����������� ����� 
   //�������� � ������� write
   //
   //��������, ���� ����� �������� ��������� � ����� �������� ���������� ����� ��� ����� 
   //���������:
   /*
      static void m( TBinaryWriter &bw, const std::string &s1, const std::string &s2 )
      {
         //���������� ������� ���������
         bw.writeDeferredPtr(&s1);
         bw.writeDeferredPtr(&s2);

         //����� ���� ������
         bw.write( s1.c_str(), &s1, s1.size() );
         bw.write( s2.c_str(), &s2, s2.size() );
      }
   */
   void writeDeferredPtr( const void *realPointer )
   {
      APL_ASSERT( m_pFile != 0 );
      APL_ASSERT( realPointer != 0 );
      m_defPtrs.insert( TDeferredPointers::value_type(realPointer, ftell(m_pFile)) );
      PtrDiffWriterT::writePtrDiff( -1 ); //����������� �����
   }

   //�������� ������� ��� ������ data.
   //��� T ����� ���� ���� �� ������� ����� char, int, float, ���� �� ������� ��������� ���� 
   //�������������� ����� �� ������� ��� � �� �������. ���� ���� �������������� �����, ����� 
   //��������������� ���� ������� ������� � �������� ������ �������� ��� ������� ��������� 
   //(������ �� ����, ��� ��� ���� ����� ���������� ��������).
   //
   //�� �����, �������������� ��� ��� ���, �������� realPointer ������ ��������� �� ������
   //��� ������ ��������� ��� (���� ��� ������ ���������� �� ������, �� �������� ��� ������ 
   //����� ����� ���������), ��� ����� ��� ���������� ������ ������� writeDeferredPtr.
   //���� �� ������� ��������� ������������� ����� ���������, �� realPointer == pData
   //
   //sizeofData - ���������� ����, ������� ����� ��������
   template< class T >
   void write( const T *pData, const void *realPointer, size_t sizeofData = sizeof(T) )
   {
      APL_ASSERT( m_pFile != 0 );
      APL_ASSERT( realPointer != 0 );
      correctDeferredPtrs( realPointer );
      
      if( fwrite(pData, sizeofData, 1, m_pFile) != 1 )
         APL_THROW( _T("������ ��� ������ � ����") );
   }

   //������������� Big-endian � Little-endian � ��������
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
   
   //��������� ��������� �������� ������ �� ������� n ����
   void align( int n )
   {
      APL_ASSERT( m_pFile != 0 );
      APL_ASSERT( ((n - 1) & n) == 0 ); //������� ������
      
      const long curPlace = ftell( m_pFile );
      const long tmp = n - 1;
      const long alignedPlace = (curPlace + tmp) & (~tmp);

      long bytesToWrite = alignedPlace - curPlace;
      const unsigned char data = 0;

      while( bytesToWrite-- )
         if( fwrite(&data, sizeof(data), 1, m_pFile) != 1 )
            APL_THROW( _T("������ ��� ������ � ����") );
   }

private:
   //���������������, ���� �����, ���������� ��������� �� realPointer, �.�. realPointer
   //����� ������� � ������� �������
   void correctDeferredPtrs( const void *realPointer )
   {
      APL_ASSERT( m_pFile != 0 );
      APL_ASSERT( realPointer != 0 );

      const std::pair<TDeferredPointers::iterator, TDeferredPointers::iterator> defPtrs = 
         m_defPtrs.equal_range( realPointer ); 

      if( defPtrs.first == defPtrs.second )
         return;

      APL_ASSERT( m_pFile != 0 );

      //��������� ������� �������
      const long curPlace = ftell( m_pFile );

      for( TDeferredPointers::iterator it = defPtrs.first; it != defPtrs.second; ++it )
      {
         APL_ASSERT( curPlace > it->second );

         //������������ �� ����������������� ����� � ���������� ��������
         fseek( m_pFile, it->second, SEEK_SET );
         PtrDiffWriterT::writePtrDiff( curPlace - it->second );
      }

      m_defPtrs.erase( defPtrs.first, defPtrs.second );
      
      //�������������� ������� �������
      fseek( m_pFile, curPlace, SEEK_SET );
   }

private:
   //��������� ��� �������� ���������� ����������
   typedef stdext::hash_multimap<const void *, long> TDeferredPointers;
   
private:
   TDeferredPointers m_defPtrs;
   FILE *m_pFile;
};

#endif
