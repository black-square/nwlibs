//Автор: Шестеркин Дмитрий(NW) 2005

#ifndef ObjectsSelector_HPP
#define ObjectsSelector_HPP

#include "LikePointer.hpp"

namespace NWLib {

///////////////////////////////////////////////////////////////////////////////
// Классы (контейнеры) связывают указатели на объёкты с некоторым количеством 
// ключей.
// При этом реализуется стратегия владения, т.е. объекты по сохранённым 
// адресам уничтожаются в деструкторе при помощи DeleteStrategy.
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
// Сводное описание:
//    CObjectsSelector -             С одним ключом связан один объект (один к одному)
//    CObjectsSelectorMultiKey  -    С несколькими ключами связан один объект, по одному ключу можно получить только один объект (много к одному)
//    CObjectsSelectorMultiObject -  С несколькими ключами связано несколько объектов, по одному ключу можно получить сразу несколько объектов (много ко многим)
//    CObjectsSelectorIndex       -  Связь объектов с последовательными индаксами в интервале [0; <Кол-во объектов>)
///////////////////////////////////////////////////////////////////////////////
template < class Key, class Object, template <class> class DeleteStrategy = auto_ptr_ex_delete_strategy >
class CObjectsSelector: public DeleteStrategy<Object>, public NonCopyable
{
public:  
   typedef DeleteStrategy<Object> TDeleteStrategy;
   typedef auto_ptr_ex<Object, DeleteStrategy> TAutoPtr;
   typedef auto_ptr_ex<Object, DeleteStrategy> &TAutoPtrRef; 
   typedef Key KeyType;

   typedef auto_ptr_ex<const Object, DeleteStrategy> TConstAutoPtr;
   typedef typename TAutoPtr::value_type ObjectType;
   typedef typename TAutoPtr::pointer ObjectPtrType;
   typedef typename TConstAutoPtr::pointer ObjectConstPtrType;
   typedef typename TAutoPtr::reference ObjectRefType;
   typedef typename TConstAutoPtr::reference ObjectConstRefType;

private:
   typedef AssocVector<Key, ObjectPtrType> TStorage;

   template< class Iterator, class Value >
   struct Selector: public SelectorHelper<Iterator, Value>
   {
      reference operator ()( origin_iterator_reference V ) const { return *V.second; }
   };

private:
   TStorage m_Storage;

public:  
   typedef CSelectIterator<typename TStorage::iterator, Selector<typename TStorage::iterator, Object> > TIterator;
   typedef CSelectIterator<typename TStorage::const_iterator, Selector<typename TStorage::const_iterator, const Object> > TConstIterator;

public:  
   CObjectsSelector() {}
   CObjectsSelector(const TDeleteStrategy &DS): TDeleteStrategy(DS) {}

   TIterator Begin() { return TIterator(m_Storage.begin()); }
   TConstIterator Begin() const { return TConstIterator(m_Storage.begin()); }
   TIterator End() { return TIterator(m_Storage.end()); }
   TConstIterator End() const { return TConstIterator(m_Storage.end()); }

   //Удалить все объекты которыми владеет класс
   void Clear()
   {
      for(TStorage::iterator I = m_Storage.begin(); I != m_Storage.end(); ++I )
         static_cast<TDeleteStrategy&>(*this)( I->second );

      m_Storage.clear();
   }

   bool Empty() const 
   {
      return m_Storage.empty();
   }

   ~CObjectsSelector()
   { 
      Clear();
   }

   //Вернуть указатель на объект по ключу. Если такого объекта нет то возвращается 0
   ObjectConstPtrType Get( const KeyType& K ) const
   {
      TStorage::const_iterator I = m_Storage.find( K );

      if( I != m_Storage.end() )
         return I->second;

      return 0;
   }

   ObjectPtrType Get( const KeyType& K )
   {
      TStorage::iterator I = m_Storage.find( K );

      if( I != m_Storage.end() )
         return I->second;

      return 0;
   }

   //Возвращает ссылку на хранящийся объект по индексу. Если объект по индексу не 
   //зарегистнрирован происходит разыменование нулевого указателя
   ObjectRefType operator[]( const KeyType& K ){ APL_ASSERT_PTR( Get(K) ); return *Get(K); }
   ObjectConstRefType operator[]( const KeyType& K ) const { APL_ASSERT_PTR( Get(K) ); return *Get(K); }

   //Добавить объект, при этом при этом происходит смена владельца. Если по ключу объект уже есть то возвращается
   //false и владение не захватывается иначе происходит смена владельца и возвращается true
   bool Add( const KeyType& K, TAutoPtrRef AutoPtrRef )
   {
      std::pair<TStorage::iterator, bool> ResPair = m_Storage.insert( TStorage::value_type( K, 0 ) );

      if( ResPair.second == false )
         return false;

      ResPair.first->second = AutoPtrRef.release();

      return true;
   }

   void SetDeleteStrategy(const TDeleteStrategy &DS) {
      static_cast<TDeleteStrategy&>(*this) = DS;
   }

   const TDeleteStrategy &GetDeleteStrategy() const {
      return static_cast<const TDeleteStrategy&>(*this);
   }

};

///////////////////////////////////////////////////////////////////////////////
// Тоже что и CObjectsSelector, но позволяет связать несколько ключей с одним 
// указателем на объект
///////////////////////////////////////////////////////////////////////////////
template < class Key, class Object, template <class> class DeleteStrategy = auto_ptr_ex_delete_strategy >
class CObjectsSelectorMultiKey: public DeleteStrategy<Object>, public NonCopyable
{
public:  
   typedef DeleteStrategy<Object> TDeleteStrategy;

   typedef auto_ptr_ex<Object, DeleteStrategy> TAutoPtr;
   typedef auto_ptr_ex<Object, DeleteStrategy> &TAutoPtrRef;
   typedef Key KeyType;

   typedef auto_ptr_ex<const Object, DeleteStrategy> TConstAutoPtr;
   typedef typename TAutoPtr::value_type ObjectType;
   typedef typename TAutoPtr::pointer ObjectPtrType;
   typedef typename TConstAutoPtr::pointer ObjectConstPtrType;
   typedef typename TAutoPtr::reference ObjectRefType;
   typedef typename TConstAutoPtr::reference ObjectConstRefType;

private:
   typedef AssocVector<Key, ObjectPtrType> TStorage;
   typedef std::vector<ObjectPtrType> TStorageForDelete;

public:
   typedef typename TStorageForDelete::iterator TIterator;
   typedef typename TStorageForDelete::const_iterator TConstIterator;

private:
   TStorage m_Storage;
   TStorageForDelete m_StorageForDelete; //Объекты для удаления

public:  
   CObjectsSelectorMultiKey() {}
   CObjectsSelectorMultiKey(const TDeleteStrategy &DS): TDeleteStrategy(DS) {}

   //Итераторы возвращаются только для зарегистрированных объектов
   TIterator Begin() { return m_StorageForDelete.begin(); }
   TConstIterator Begin() const { return m_StorageForDelete.begin(); }
   TIterator End() { return m_StorageForDelete.end(); }
   TConstIterator End() const { return m_StorageForDelete.end(); }

   //Удалить все объекты которыми владеет класс
   void Clear()
   {
      for(TStorageForDelete::iterator I = m_StorageForDelete.begin(); I != m_StorageForDelete.end(); ++I )
         static_cast<TDeleteStrategy&>(*this)( *I );

      m_StorageForDelete.clear();
      m_Storage.clear();
   }

   bool Empty() const 
   {
      return m_Storage.empty();
   }

   ~CObjectsSelectorMultiKey()
   { 
      Clear();
   }

   //Вернуть указатель на объект по ключу. Если такого объекта нет то возвращается 0
   ObjectConstPtrType Get( const KeyType& K ) const
   {
      TStorage::const_iterator I = m_Storage.find( K );

      if( I != m_Storage.end() )
         return I->second;

      return 0;
   }

   ObjectPtrType Get( const KeyType& K )
   {
      TStorage::iterator I = m_Storage.find( K );

      if( I != m_Storage.end() )
         return I->second;

      return 0;
   }

   //Возвращает ссылку на хранящийся объект по индексу. Если объект по индексу не 
   //зарегистнрирован происходит разыменование нулевого указателя
   ObjectRefType operator[]( const KeyType& K ){ APL_ASSERT_PTR( Get(K) ); return *Get(K); }
   ObjectConstRefType operator[]( const KeyType& K ) const { APL_ASSERT_PTR( Get(K) ); return *Get(K); }

   //Добавить объект и ассоциировать его с групой ключей при этом при этом происходит 
   //смена владельца этого объекта. Если хотя бы по одному ключу объект уже есть возвращается
   //false и владение не захватывается иначе происходит смена владельца и возвращается true
   template< class InputIterator >
   bool Add( InputIterator FirstKey, InputIterator LastKey, TAutoPtrRef AutoPtrRef )
   {
      std::pair<TStorage::iterator, bool> ResPair;

      APL_ASSERT( std::find(m_StorageForDelete.begin(),  m_StorageForDelete.end(), AutoPtrRef.get()) == m_StorageForDelete.end() );

      for( InputIterator CurKey = FirstKey; CurKey != LastKey; ++CurKey )
      {
         ResPair = m_Storage.insert( TStorage::value_type( *CurKey, AutoPtrRef.get() ) );

         if( ResPair.second == false )
         {
            //Необходимо удалить уже добавленные объекты
            for( ; FirstKey != CurKey; ++FirstKey )
               m_Storage.erase( *FirstKey );

            return false;
         }
      }

      //Меняем владельца
      m_StorageForDelete.push_back(0);
      m_StorageForDelete.back() = AutoPtrRef.release();

      return true;
   }

   //Эквиваленто вызову Add( FirstKey, FirstKey + 1, AutoPtrRef )
   bool Add( const KeyType& K, TAutoPtrRef AutoPtrRef )
   {
      return Add( &K, &K + 1, AutoPtrRef );
   }

   void SetDeleteStrategy(const TDeleteStrategy &DS) {
      static_cast<TDeleteStrategy&>(*this) = DS;
   }

   const TDeleteStrategy &GetDeleteStrategy() const {
      return static_cast<const TDeleteStrategy&>(*this);
   }
};

///////////////////////////////////////////////////////////////////////////////
// Тоже что и CObjectsSelector, но позволяет связать несколько ключей с одним 
// указателем на объект и допускает что по одному ключу может быть несколько 
// объектов. Но всё вавно в CObjectsSelectorMultiObject должны хранится только 
// уникальные объекты
///////////////////////////////////////////////////////////////////////////////
template < class Key, class Object, template <class> class DeleteStrategy = auto_ptr_ex_delete_strategy >
class CObjectsSelectorMultiObject: public DeleteStrategy<Object>, public NonCopyable
{
public:  
   typedef DeleteStrategy<Object> TDeleteStrategy;

   typedef auto_ptr_ex<Object, DeleteStrategy> TAutoPtr;
   typedef auto_ptr_ex<Object, DeleteStrategy> &TAutoPtrRef;
   typedef Key KeyType;

   typedef auto_ptr_ex<const Object, DeleteStrategy> TConstAutoPtr;
   typedef typename TAutoPtr::value_type ObjectType;
   typedef typename TAutoPtr::pointer ObjectPtrType;
   typedef typename TConstAutoPtr::pointer ObjectConstPtrType;
   typedef typename TAutoPtr::reference ObjectRefType;
   typedef typename TConstAutoPtr::reference ObjectConstRefType;

private:
   typedef MultiAssocVector<Key, ObjectPtrType> TStorage;
   typedef std::vector<ObjectPtrType> TStorageForDelete;

private:
   template<class Iterator, class Value>
   struct Selector: SelectorHelper<Iterator, Value> 
   {
      reference operator ()( origin_iterator_reference V ) const { return *V.second; }
   };

public:
   typedef typename TStorageForDelete::iterator TIterator;
   typedef typename TStorageForDelete::const_iterator TConstIterator;

   typedef CSelectIterator<typename TStorage::iterator, Selector<typename TStorage::iterator, Object> > TResultIterator;
   typedef CSelectIterator<typename TStorage::const_iterator, Selector<typename TStorage::const_iterator, const Object> > TConstResultIterator;

   typedef std::pair< TResultIterator, TResultIterator > TResultPair;
   typedef std::pair< TConstResultIterator, TConstResultIterator > TConstResultPair;

private:
   TStorage m_Storage;
   TStorageForDelete m_StorageForDelete; //Объекты для удаления

public:  
   CObjectsSelectorMultiObject() {}
   CObjectsSelectorMultiObject(const TDeleteStrategy &DS): TDeleteStrategy(DS) {}

   //Итераторы возвращаются только для зарегистрированных объектов
   TIterator Begin() { return m_StorageForDelete.begin(); }
   TConstIterator Begin() const { return m_StorageForDelete.begin(); }
   TIterator End() { return m_StorageForDelete.end(); }
   TConstIterator End() const { return m_StorageForDelete.end(); }

   //Удалить все объекты которыми владеет класс
   void Clear()
   {
      for(TStorageForDelete::iterator I = m_StorageForDelete.begin(); I != m_StorageForDelete.end(); ++I )
         static_cast<TDeleteStrategy&>(*this)( *I );

      m_StorageForDelete.clear();
      m_Storage.clear();
   }

   bool Empty() const 
   {
      return m_Storage.empty();
   }

   ~CObjectsSelectorMultiObject()
   { 
      Clear();
   }

   //Вернуть итераторы объектов по ключу K. Если таких объектов нет возвращается пустой интервал
   TResultPair Get( const KeyType& K )
   {
      return(m_Storage.equal_range(K) );
   }

   TConstResultPair Get( const KeyType& K ) const
   {
      return(m_Storage.equal_range(K) );
   }

   //То же что и Get
   TResultPair operator[]( const KeyType& K ){ return Get(K); }
   TConstResultPair operator[]( const KeyType& K ) const { return Get(K); }

   //Добавить объект и ассоциировать его с групой ключей при этом при этом происходит 
   //смена владельца этого объекта. 
   template< class InputIterator >
   void Add( InputIterator FirstKey, InputIterator LastKey, TAutoPtrRef AutoPtrRef )
   {
      APL_ASSERT( std::find(m_StorageForDelete.begin(),  m_StorageForDelete.end(), AutoPtrRef.get()) == m_StorageForDelete.end() );

      for( InputIterator CurKey = FirstKey; CurKey != LastKey; ++CurKey )
         m_Storage.insert( TStorage::value_type( *CurKey, AutoPtrRef.get() ) );

      //Меняем владельца
      m_StorageForDelete.push_back(0);
      m_StorageForDelete.back() = AutoPtrRef.release();
   }

   //Эквиваленто вызову Add( FirstKey, FirstKey + 1, AutoPtrRef )
   void Add( const KeyType& K, TAutoPtrRef AutoPtrRef )
   {
      Add( &K, &K + 1, AutoPtrRef );
   }

   void SetDeleteStrategy(const TDeleteStrategy &DS) {
      static_cast<TDeleteStrategy&>(*this) = DS;
   }

   const TDeleteStrategy &GetDeleteStrategy() const {
      return static_cast<const TDeleteStrategy&>(*this);
   }
};

///////////////////////////////////////////////////////////////////////////////
// Класс связывает указатели на объёкты с последовательными индексами, 
// начинающимися с нуля (как в обычных массивах)
// При этом реализуется стратегия владения, т.е. объекты по сохранённым 
// адресам уничтожаются в деструкторе при помощи DeleteStrategy.
// В качестве хранилища объектов используется std::vector, поэтому итераторы 
// произвольного доступа
///////////////////////////////////////////////////////////////////////////////
template < class Object, template <class> class DeleteStrategy = auto_ptr_ex_delete_strategy >
class CObjectsSelectorIndex: public DeleteStrategy<Object>, public NonCopyable
{
public:  
   typedef DeleteStrategy<Object> TDeleteStrategy;
   typedef auto_ptr_ex<Object, DeleteStrategy> TAutoPtr;
   typedef auto_ptr_ex<Object, DeleteStrategy> &TAutoPtrRef; 

   typedef auto_ptr_ex<const Object, DeleteStrategy> TConstAutoPtr;
   typedef typename TAutoPtr::value_type ObjectType;
   typedef typename TAutoPtr::pointer ObjectPtrType;
   typedef typename TConstAutoPtr::pointer ObjectConstPtrType;
   typedef typename TAutoPtr::reference ObjectRefType;
   typedef typename TConstAutoPtr::reference ObjectConstRefType;

private:
   typedef std::vector<ObjectPtrType> TStorage;

   template< class Iterator, class Value >
   struct Selector: public SelectorHelper<Iterator, Value>
   {
      reference operator ()( origin_iterator_reference V ) const { return *V; }
   };

public:
   typedef typename TStorage::size_type KeyType;

private:
   TStorage m_Storage;

public:  
   typedef CSelectIterator<typename TStorage::iterator, Selector<typename TStorage::iterator, Object> > TIterator;
   typedef CSelectIterator<typename TStorage::const_iterator, Selector<typename TStorage::const_iterator, const Object> > TConstIterator;

public:  
   CObjectsSelectorIndex() {}
   CObjectsSelectorIndex(const TDeleteStrategy &DS): TDeleteStrategy(DS) {}

   TIterator Begin() { return TIterator(m_Storage.begin()); }
   TConstIterator Begin() const { return TConstIterator(m_Storage.begin()); }
   TIterator End() { return TIterator(m_Storage.end()); }
   TConstIterator End() const { return TConstIterator(m_Storage.end()); }

   //Удалить все объекты которыми владеет класс
   void Clear()
   {
      for(TStorage::iterator I = m_Storage.begin(); I != m_Storage.end(); ++I )
         static_cast<TDeleteStrategy&>(*this)( *I );

      m_Storage.clear();
   }

   bool Empty() const { return m_Storage.empty(); }

   ~CObjectsSelectorIndex() { Clear(); }

   //Выделить память под ожидаемое количество объктов 
   //Только, выделяет память под объекты, количество объектов остаётся неизменным
   void Reserve( KeyType N ){ m_Storage.reserve( N ); }

   //Ссылка на первый/последний элемент
   ObjectRefType Front(){ APL_ASSERT( !Empty() ); return *m_Storage.front(); }
   ObjectConstRefType Front() const { APL_ASSERT( !Empty() ); return *m_Storage.front(); }
   ObjectRefType Back(){ APL_ASSERT( !Empty() ); return *m_Storage.back(); }
   ObjectConstRefType Back() const { APL_ASSERT( !Empty() ); return *m_Storage.back(); }

   //Количество хранящихся объектов
   KeyType Size() const { return m_Storage.size(); }

   //Удаление объекта по ключу
   void Erase( KeyType K )
   {
      APL_ASSERT( K >= 0 && K < m_Storage.size() );

      static_cast<TDeleteStrategy&>(*this)( m_Storage[K] );
      m_Storage.erase(m_Storage.begin() + K);
   }

   //Вернуть указатель на объект по индексу.
   ObjectConstPtrType Get( KeyType K ) const
   {
      APL_ASSERT( K >= 0 && K < m_Storage.size() );

      return m_Storage[K];
   }

   ObjectPtrType Get( KeyType K )
   {
      APL_ASSERT( K >= 0 && K < m_Storage.size() );

      return m_Storage[K];
   }

   //Возвращает ссылку на хранящийся объект по индексу. Если объект по индексу не 
   //зарегистнрирован происходит разыменование нулевого указателя
   ObjectRefType operator[]( KeyType K ){ APL_ASSERT_PTR( Get(K) ); return *Get(K); }
   ObjectConstRefType operator[]( KeyType K ) const { APL_ASSERT_PTR( Get(K) ); return *Get(K); }

   //Добавить объект, при этом при этом происходит смена владельца. Индекс добавляемого объекта
   //будет равен количеству объектов до добавления. (Происходит добавление в конец)
   //Возвращает индекс добавленного элемента
   KeyType Append( TAutoPtrRef AutoPtrRef )
   {
      m_Storage.push_back( AutoPtrRef.get() );
      AutoPtrRef.release();

      return m_Storage.size() - 1;
   }

   void SetDeleteStrategy(const TDeleteStrategy &DS) {
      static_cast<TDeleteStrategy&>(*this) = DS;
   }

   const TDeleteStrategy &GetDeleteStrategy() const {
      return static_cast<const TDeleteStrategy&>(*this);
   }
};

} //namespace NWLib 

#endif