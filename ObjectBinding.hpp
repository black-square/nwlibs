//�����: ��������� �������(NW) 2007

#ifndef ObjectBinding_HPP
#define ObjectBinding_HPP

///////////////////////////////////////////////////////////////////////////////
// ����� �������� ���������� ����������� ������� ������� ������� ���� � ������ 
///////////////////////////////////////////////////////////////////////////////

#include "StlAuxiliary.hpp"

namespace NWLib {


///////////////////////////////////////////////////////////////////////////////
// ����� ������������ ��� ���������� �������� �� ������ ���� �� ������, ������ 
// ������ ������ (��� �������� ��� � ����) ����� ����������� � ����� ������ 
// ������� � ��� ���� ����������� ����� ������������� ������� 
// ��� ������������� ������ �������� ������� ������ ������� ������������� �� ���� 
// TRelationshipOneToMany::THolder
// �� ������ �� ������������� ������������ ����������. �������������� � 
// ����������� ��������������� ��� ������ ��������� ConteinerStrategyT 
// (������� ����������� ������� TRelationshipOneToMany)
// ���������� ConteinerStrategyT ������ ������������� ��
// TRelationshipOneToManyBaseStrategy � �������� �������
///////////////////////////////////////////////////////////////////////////////
template < 
   //��������� �� �������� ������
   //������ ���� ��� ������ ��������� � ���������� (�������� � ��������� 
   //�������������� �����������)
   class PointerT = NullType *, 
   
   //�������� �� ����������
   class IteratorT = PointerT, 
   
   //��� �������� ������� Append, ���������� �� TPointer � ����� � ��� ��� �������� ����� ����������
   //���������� �������������� ����������
   class AppendParamT = PointerT, 
   
   //��������� ������������ ������, ��������� �� ������� ���������� ������� GetParent(). 
   //GetParent() ���������� �������� ������, ���� ����������� �������� 
   //TRelationshipOneToMany::THolder::GetParent()
   class ParentT = NullType
>
struct TRelationshipOneToManyBaseStrategy
{
   typedef PointerT TPointer;
   typedef IteratorT TIterator;
   typedef AppendParamT TAppendParam;
   typedef ParentT TParent;

   //������/����� ���������
   TIterator Begin();
   TIterator End();

   //�������� ������� � ��������� � ������� �������� �� ����
   TIterator Append( TAppendParam Param );

   //������� ������� �� ����������
   void Erase( TIterator iterChild );

   //������� ��� �������� �� ����������
   void Clear();

   //�������� �� ��������������� ��������� ��������� �� �������� ������
   TPointer GetPtr( typename std::iterator_traits<TIterator>::reference Ref );

   //�������� �� ���� TAppendParam ��������� �� �������� ������
   TPointer GetPtr( TAppendParam Param );

   //����� ���������� ����������� ����������� ���� ��������
   template<class T>
   TRelationshipOneToManyBaseStrategy( const T &t ) {}
   TRelationshipOneToManyBaseStrategy() {}

   //��������� �� ��������� ������������ ������ ���������� �������� ������, ���� ����������� 
   //�������� TRelationshipOneToMany::THolder::GetParent()
   TParent *GetParent();
   const TParent *GetParent() const;

   //������� ���������� ����� ��� ��� ����������� ��������� ������� iterChild ����� �������
   //� ����� ��������� ����� �����
   //(������ Erase � Clear ���������� ��� � ���������� ������ �������� ������ -> ��������)
   void BeforeUnregister( TIterator iterChild ) const {}
};
///////////////////////////////////////////////////////////////////////////////

template< class ConteinerStrategyT >
class TRelationshipOneToMany: public ConteinerStrategyT
{
   typedef typename ConteinerStrategyT::TIterator    TIterator;
   typedef typename ConteinerStrategyT::TPointer     TPointer;
   typedef typename ConteinerStrategyT::TAppendParam TAppendParam;

public:
   // ����� �� �������� ������ ������������� ��� �������� �������
   class THolder;

private:
   //���� ������������ ������ THolder �� ��������� ��������������� ����
   //���� ������� � ����������� TRelationshipOneToMany
   typedef typename TRelationshipOneToMany::THolder TThisHolder;

public:  
   TRelationshipOneToMany() {}

   //VC ����� �������� �� ��� ������� ���� ��������� ������� ����������� 
   //TRelationshipOneToMany. ��� ������ �����������, ��������� �����
   //http://rsdn.ru/Forum/Message.aspx?mid=2493952
   //��� ������� �������� ����� � ���� ������������� � ConteinerStrategyT
   //���������� ��������� �����������
   template<class T>
   explicit TRelationshipOneToMany( const T &t ): ConteinerStrategyT(t) {}

   //������� ������������ this � �������� Param ������
   void Register( TAppendParam Param );

   //������� ����� ������������� this � ��������� iterChild �������
   void Unregister( TIterator iterChild );

   //������� ��� �����
   void UnregisterAll();

   //������� ������������ ����� � ��������� ������� Param � ������� ��� � this
   void MoveFrom( TAppendParam Param );

   ~TRelationshipOneToMany() { UnregisterAll(); }

public: //������� ������������ ��� �������, � assert'��
   //��������� �� ���������� ��������
   //const ���������������� ��� ���� ����� �� ��������� �� ������������ ����������� Begin() � End()
   bool Check( TIterator iterChild ) /*const*/;  

private:
   //��������������� �� ��� ������
   bool IsContain( TAppendParam Param );

private:                                                                  
   //������ �� ������������ ������ � ���������
   struct TRegData;
};
///////////////////////////////////////////////////////////////////////////////

template< class ConteinerStrategyT >
class TRelationshipOneToMany<ConteinerStrategyT>::THolder: public NWLib::NonCopyable
{
public:
   ~THolder() { Unregister(); }
   
   //������������� ������� �����������
   void Unregister() { if(m_RegData.m_pParent) m_RegData.m_pParent->Unregister(m_RegData.m_iterChild); }

   //������� ������ ��, ����������� �������������, ����� �������� ��� 0 ���� ��� �����������
   typename ConteinerStrategyT::TParent *GetParent() 
   { return m_RegData.m_pParent ? m_RegData.m_pParent->ConteinerStrategyT::GetParent() : 0; }

   typename const ConteinerStrategyT::TParent *GetParent() const 
   { return m_RegData.m_pParent ? m_RegData.m_pParent->ConteinerStrategyT::GetParent() : 0;  }

private:
   friend TRelationshipOneToMany;

   void SetRegData( const TRegData &RegData ) { m_RegData = RegData; }
   const TRegData &GetRegData() const { return m_RegData; }
private:
   TRegData m_RegData;
};
///////////////////////////////////////////////////////////////////////////////

template< class ConteinerStrategyT >
struct TRelationshipOneToMany<ConteinerStrategyT>::TRegData
{      
private:   
   friend TRelationshipOneToMany;
   friend TRelationshipOneToMany::THolder;

   TRegData(): m_pParent(0) {}
   TRegData( TRelationshipOneToMany *pParent, TIterator iterChild ): m_pParent(pParent), m_iterChild(iterChild) {}

   TRelationshipOneToMany *m_pParent;
   TIterator m_iterChild;
};
///////////////////////////////////////////////////////////////////////////////

template< class ConteinerStrategyT >
inline bool TRelationshipOneToMany<ConteinerStrategyT>::IsContain( TAppendParam Param )
{
   for( TIterator I = ConteinerStrategyT::Begin(); I != ConteinerStrategyT::End(); ++I )
      if( ConteinerStrategyT::GetPtr(*I) == ConteinerStrategyT::GetPtr(Param) )
         return true;

   return false;
}
///////////////////////////////////////////////////////////////////////////////

template< class ConteinerStrategyT >
inline bool TRelationshipOneToMany<ConteinerStrategyT>::Check( TIterator iterChild )
{
   return 
      NWLib::CheckInRange(ConteinerStrategyT::Begin(), ConteinerStrategyT::End(), iterChild) &&
      ConteinerStrategyT::GetPtr(*iterChild) != 0 &&
      ConteinerStrategyT::GetPtr(*iterChild)->TThisHolder::GetRegData().m_pParent == this &&
      ConteinerStrategyT::GetPtr(*iterChild)->TThisHolder::GetRegData().m_iterChild == iterChild;
}
///////////////////////////////////////////////////////////////////////////////

template< class ConteinerStrategyT >
inline void TRelationshipOneToMany<ConteinerStrategyT>::Register( TAppendParam Param )
{
   APL_ASSERT_PTR( ConteinerStrategyT::GetPtr(Param) );
   APL_ASSERT( !IsContain(Param) );
   APL_ASSERT( ConteinerStrategyT::GetPtr(Param)->TThisHolder::GetRegData().m_pParent == 0 );

   ConteinerStrategyT::GetPtr(Param)->TThisHolder::SetRegData( TRegData(this, ConteinerStrategyT::Append(Param)) ); 
}
///////////////////////////////////////////////////////////////////////////////

template< class ConteinerStrategyT >
inline void TRelationshipOneToMany<ConteinerStrategyT>::Unregister( TIterator iterChild )
{
   APL_ASSERT( Check(iterChild) );

   ConteinerStrategyT::BeforeUnregister( iterChild );
   ConteinerStrategyT::GetPtr( *iterChild )->TThisHolder::SetRegData( TRegData() );
   ConteinerStrategyT::Erase( iterChild );
}
///////////////////////////////////////////////////////////////////////////////

template< class ConteinerStrategyT >
inline void TRelationshipOneToMany<ConteinerStrategyT>::UnregisterAll()
{
   for( TIterator I = ConteinerStrategyT::Begin(); I != ConteinerStrategyT::End(); ++I )
   {
      APL_ASSERT( Check(I) );
      ConteinerStrategyT::BeforeUnregister( I );
      ConteinerStrategyT::GetPtr(*I)->TThisHolder::SetRegData( TRegData() );
   }

   ConteinerStrategyT::Clear();
}
///////////////////////////////////////////////////////////////////////////////

template< class ConteinerStrategyT >
inline void TRelationshipOneToMany<ConteinerStrategyT>::MoveFrom( TAppendParam Param )
{
   APL_ASSERT_PTR( ConteinerStrategyT::GetPtr(Param) );
   APL_ASSERT_PTR( ConteinerStrategyT::GetPtr(Param)->TThisHolder::GetRegData().m_pParent );  
   APL_ASSERT( ConteinerStrategyT::GetPtr(Param)->TThisHolder::GetRegData().m_pParent != this );  
   
   ConteinerStrategyT::GetPtr(Param)->TThisHolder::Unregister();
   Register(Param);
}
///////////////////////////////////////////////////////////////////////////////

} //namespace NWLib 

#endif