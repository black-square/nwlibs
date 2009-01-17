//�����: ��������� �������(NW) 2006

#ifndef ReferenceCounters_HPP
#define ReferenceCounters_HPP

///////////////////////////////////////////////////////////////////////////////
// ��������� ���������� ��������� ������
///////////////////////////////////////////////////////////////////////////////

namespace NWLib {

///////////////////////////////////////////////////////////////////////////////
// ����� ��������� ��������� ������� ������ ������ ������ �� �������� ����� 
// ������������� � ������� ����� ����������� ��-���������.
// ��� ������ ������������ ������ Create ����� �������� �����-�� ��������� �� 
// �� ��� ���������� ��� �������� ������� � CAutoRefCount, ������� ����� 
// ���������� ���� � �����. ������� � STL ����������� � �.�. ����� ���� ��� 
// ��������� ����� ��������� ����� ��������� ������������� ��������� ����� 
// delete
///////////////////////////////////////////////////////////////////////////////
namespace Private
{
   //���������� ����� ������� ������������� ������ �� �������� ������
   template< class ObjectT >
   class CImpl: public ObjectT
   {
      size_t m_RefCount;

      CImpl(): m_RefCount(1) {}
   public:
      static CImpl *Create() { return new CImpl; }

      void AddRef(){ ++m_RefCount; }
      void Release(){ APL_ASSERT( m_RefCount > 0 ); if( --m_RefCount == 0 ) delete this; }
   };
   
   //���������� ����������������� ���������
   template<class ResultT, class ObjectT>
   class CPtrBase
   {
      CImpl<ObjectT> *m_pT;

   public:         
      template < class T1, class T2 > friend class CPtrBase;

      CPtrBase( CImpl<ObjectT> *pT = 0 ): m_pT(pT) {}
      //CPtrBase( TUndefinedBoolType pT = 0 ): m_pT(0) {} //��������� �������������� ������ �� 0

      CPtrBase( const CPtrBase &Other )
      { 
         m_pT = Other.m_pT;

         if( m_pT != 0 )
            m_pT->AddRef();
      }

      ~CPtrBase()
      {
         if( m_pT != 0 )
            m_pT->Release();
      }

      void Swap( CPtrBase &Other )
      {
         std::swap(m_pT, Other.m_pT);
      }

      void Release()
      {
         if( m_pT != 0 )
         {
            m_pT->Release();
            m_pT = 0;
         }
      }

      CPtrBase &operator=( const CPtrBase &Other )
      {
         CPtrBase(Other).Swap(*this);
         return *this;
      }

      //�������� ��������� ��������� �� ���������/����������� ���� � ��������� � ���� bool
   private:  
      struct OperatorHelper{ int i; }; //��. ��������������(loki)
      typedef int OperatorHelper::*TUndefinedBoolType;
   public:
      //�������� ��������� ��������� �� ���������/����������� ���� � ��������� � ���� bool
      operator TUndefinedBoolType() const { return m_pT != 0? &OperatorHelper::i : 0; }

      ResultT *Get() const { return m_pT; }
      ResultT &operator*() const { APL_ASSERT( Get() != 0 ); return *Get(); }
      ResultT *operator->() const { return &**this; }

      //���� �� ���������� �� ����� �������������� �������������� �������������� � TUndefinedBoolType
      //� ���������� ���������
      friend bool operator==( const CPtrBase &P1, const CPtrBase &P2 ){ return P1.m_pT == P2.m_pT; }
      friend bool operator!=( const CPtrBase &P1, const CPtrBase &P2 ){ return P1.m_pT != P2.m_pT; }

      //���� �� ���������� �� ��� �������� P1 == 0, 0 == P1, P1 != 0, 0 != P1 ��������� 
      //��������������� �.�. ���������� �� explicit
      friend bool operator==( const CPtrBase &P1, TUndefinedBoolType P2 ){ return P1.m_pT == 0; }
      friend bool operator==( TUndefinedBoolType P2, const CPtrBase &P1 ){ return P1.m_pT == 0; }
      friend bool operator!=( const CPtrBase &P1, TUndefinedBoolType P2 ){ return P1.m_pT != 0; }
      friend bool operator!=( TUndefinedBoolType P2, const CPtrBase &P1 ){ return P1.m_pT != 0; }


   protected:
      template< class T1, class T2 >
      CPtrBase( const CPtrBase<T1, T2> &Other ) //�������� ����������� �� �������� ����������� �� ���������
      { 
         m_pT = Other.m_pT;

         if( m_pT != 0 )
            m_pT->AddRef();
      }

      template< class T1, class T2 >
      CPtrBase &operator=( const CPtrBase<T1, T2> &Other ) //�������� �������� ������������ �� �������� �������� ������������ �� ���������
      {
         CPtrBase(Other).Swap(*this);
         return *this;
      }
   };
} //namespace Private

template< class ObjectT >
class CAutoRefCount
{
public:

   //���������������� ��������� ����������� �������� Create
   class SmartPtr: public Private::CPtrBase<ObjectT, ObjectT>
   {
   public:
      SmartPtr( Private::CImpl<ObjectT> * pImpl = 0 ): Private::CPtrBase<ObjectT, ObjectT>(pImpl){}
   };

   //����������� ����-�� ��������� (��������� �������� ������ ������ � const ObjectT)
   //����� ���� ������������� �� SmartPtr, �� �� ��������
   class ConstSmartPtr: public Private::CPtrBase<const ObjectT, ObjectT>
   {
   public:
      ConstSmartPtr( Private::CImpl<ObjectT> * pImpl = 0 ): Private::CPtrBase<const ObjectT, ObjectT>(pImpl){}
      ConstSmartPtr( const SmartPtr &SM ): Private::CPtrBase<const ObjectT, ObjectT>(SM) {}
      ConstSmartPtr &operator=( const SmartPtr &SM ){ Private::CPtrBase<const ObjectT, ObjectT>::operator=(SM); return *this; }
   };

   //������ ����� ������ ���� ObjectT
   static SmartPtr Create(){ return SmartPtr( Private::CImpl<ObjectT>::Create() ); }
};

} //namespace NWLib 

#endif
