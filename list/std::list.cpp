//Напишите класс List - двусвязный список с правильным использованием аллокатора. Правильное использование аллокатора означает, что ваш лист должен удовлетворять требованиям https://en.cppreference.com/w/cpp/named_req/AllocatorAwareContainer. Должно быть два шаблонных параметра: T - тип элементов в листе, Allocator - тип используемого аллокатора (по умолчанию - std::allocator<T>).
//Должны быть поддержаны:
//Конструкторы: без параметров; от одного числа; от числа и const T&; от одного аллокатора; от числа и аллокатора; от числа, const T& и аллокатора. Если создается пустой лист, то не должно быть никаких выделений динамической памяти, независимо от того, на каком аллокаторе построен этот лист.
//Метод get_allocator(), возвращающий объект аллокатора, используемый в листе на данный момент;
//Конструктор копирования, деструктор, копирующий оператор присваивания;
//Метод size(), работающий за O(1);
//Методы push_back, push_front, pop_back, pop_front;
//Двунаправленные итераторы, удовлетворяющие требованиям https://en.cppreference.com/w/cpp/named_req/BidirectionalIterator. Также поддержите константные и reverse-итераторы;
//Методы begin, end, cbegin, cend, rbegin, rend, crbegin, crend;
//Методы insert(iterator, const T&), а также erase(iterator) - для удаления и добавления одиночных элементов в список.
//Если аллокатор является структурой без каких-либо полей, то такой аллокатор не должен увеличивать sizeof объекта листа. То есть тривиальный аллокатор как поле листа должен занимать 0 байт.:
#include <iostream>
#include <memory>
//в листе bidirectional_iterator
// создадим фейковую вершину, которая и будет endom, тк нам нужно чтобы энд декрементировался и
// после --энд возвращало последний элемент листа
// получается, что будем хранить циклический список (указатель на бегин и энд - одно и то же
// - фейк вершина, когда список пустой)
//можно выделять память под фейкнод просто на стеке, и можно проинициализировать налптр либо зациклить на себя
//сделать указатель на себя

template <size_t N>
class StackStorage {

 private:
  char storage_[N];
  char* ptr_;

 public:
  StackStorage() : ptr_(storage_) {}
  ~StackStorage() {}
  void* allocate(size_t count); //без выравнивания
};

template <size_t N>
//without alignments
void* StackStorage<N>::allocate(size_t size) {
  char* result = ptr_;
  ptr_ += size;
  return reinterpret_cast<void*>(result);
}

template <typename T, size_t N>
class StackAllocator {
 public:
  using value_type = T;
  StackStorage<N>* pool_;
  StackAllocator() = default;
  template <typename U>
  StackAllocator(const StackAllocator<U, N>& other)
      : pool_(other.pool_) {} //по-моему нужен еще конструктор, еще с except
  StackAllocator(const StackStorage<N>&) = default;
  ~StackAllocator() {}
  template <typename U>
  StackAllocator<T, N>& operator=(StackAllocator<U, N> other);
  T* allocate(size_t count);
  void deallocate(T*, size_t) {};
  void swap(StackAllocator& alloc);

  StackAllocator<T, N> select_on_container_copy_construction() { //нужен ли вообще этот метод
    return MyAllocator<T>(*this);
  }

  template <typename U>
  struct rebind {
    using other = StackAllocator<U, N>; // не понимаю все-таки зачем это
  };

};

template <typename T, size_t N>
T* StackAllocator<T, N>::allocate(size_t count) {
  return reinterpret_cast<T*>(pool_->allocate(count * sizeof(T)));
}

template <typename T, size_t N>
void StackAllocator<T, N>::swap(StackAllocator& alloc) {
  std::swap(pool_, alloc.pool_);
}

template <typename T, size_t N>
template <typename U>
StackAllocator<T, N>& StackAllocator<T, N>::operator=(StackAllocator<U, N> other) { //здесь неэффективно, так как копия
  swap(other);
  return *this;
}

template <typename T, typename Alloc = std::allocator<T>>
class List {
 private:
  size_t size_;

  struct BaseNode {
    BaseNode* next;
    BaseNode* prev;
    BaseNode() : next(nullptr), prev(nullptr) {}
  };

  struct Node : BaseNode {
    T value;
    Node() : BaseNode(), value() {}
    Node(const T& value) : BaseNode(), value(value) {}
    Node(const BaseNode &arg, const T &val) : BaseNode(arg), value(val) {}
    Node(BaseNode* arg) : BaseNode(arg), value() {} //конструктор от бейзноды
  };

  BaseNode* fakeNode; // for function link two_nodes

 public:
  List();
  List(size_t size);
  List(size_t size, const T&);
  List(const Alloc&);
  List(size_t, const Alloc&);
  List(size_t, const T&, const Alloc&);
  // переписать
//cделать указатель на налптр или указатель на самого себя
  using value_type = T;

//  using NodeAlloc = typename Alloc::template rebind<Node>::other; //эта строка под вопросом, вроде надо
  using NodeAlloc = typename std::allocator_traits<Alloc>::template rebind_alloc<Node>;
  using AllocTraits = std::allocator_traits<NodeAlloc>;//эта строка под вопросом, вроде надо
  using size_type = std::size_t;
  using difference_type = std::ptrdiff_t;
  using reference = T&;
  using const_reference = const T&;
  using pointer = typename std::allocator_traits<Alloc>::pointer;
  using const_pointer = typename std::allocator_traits<Alloc>::const_pointer;

 private:
  NodeAlloc alloc_;

  struct Iterator {
    BaseNode* node;

    Iterator& operator++() {
      node = node->next;
      return *this;
    }

    Iterator operator++(int) {

    }

    Iterator& operator--() {
      node = node->prev;
      return *this;
    }

    Iterator operator--(int) {

    }

  };

 public:
//  BaseNode* get_fakeNode() {
//    return fakeNode;
//  }

  BaseNode* create_basenode() {
    BaseNode* node = AllocTraits::allocate(alloc_, 1);
    AllocTraits::construct(alloc_, node);
    return node;
  }
  Node* create_node(const T& value) {
    Node* node = AllocTraits::allocate(alloc_, 1);
    AllocTraits::construct(alloc_, node, value);
    return node;
  }

  void create_fakenode() {
    fakeNode = create_basenode();
    fakeNode->next = fakeNode->prev = fakeNode;
  }

  void link_with_basenode(BaseNode* prev, BaseNode* next) {
    prev->next = next;
    next->prev = prev;
    next->next = prev;
    prev->prev = next;
  }
  void link_with_node(BaseNode* prev, Node* next) {
    link_two_basenodes(prev, next);
}
  void link_three_basenodes(BaseNode* first, BaseNode* second, BaseNode* third) {
    second->next = third;
    third->prev = second;
    second->next = first;
    first->prev = third;
  }
  void link_three_nodes(BaseNode* first, Node* second, Node* third) {
    link_three_basenodes(first, second, third);
  }
  size_t size() const { return size_; }
  bool empty() const {return size_ == 0;}
  void clear() {}

  void insert() {//вставка по константному итератору, значение вставляется ПЕРЕД итератором
    // чтобы вставить в начало листа, делаем инсерт по итератору бегин
    // чтобы вставить в конец листа, делаем инсерт по итератору энд
  }

};

//сделать конструкторы
template <typename T, typename Alloc>
List<T, Alloc>::List() : size_(0), alloc_(NodeAlloc()) {
  create_fakenode();
}

template <typename T, typename Alloc>
List<T, Alloc>::List(size_t size) : size_(size), alloc_(NodeAlloc()) {
  create_fakenode();
  BaseNode* Node_before = create_basenode(); //как применить наследование от ноды, чтобы
  link_with_basenode(fakeNode, Node_before);
  for (size_t i = 1; i < size_; ++i) {
    BaseNode* newNode = create_basenode();
    link_three_basenodes(fakeNode, Node_before, newNode);
    Node_before = newNode;
  }
}

template <typename T, typename Alloc>
List<T, Alloc>::List(size_t size, const T& value) : size_(size), alloc_(NodeAlloc()) {
  create_fakenode();
  Node* Node_before = create_node();
  link_two_nodes(fakeNode, Node_before);
  for (size_t i = 1; i < size_; ++i) {
    Node* newNode = create_node();
    link_three_nodes(fakeNode, Node_before, newNode);
    Node_before = newNode;
  }
}

template <typename T, typename Alloc>
List<T, Alloc>::List(const Alloc& alloc) : size_(0), alloc_(AllocTraits::select_on_container_copy_construction()) {
  create_fakenode();
}
template <typename T, typename Alloc>
List<T, Alloc>::List(size_t, const T&, const Alloc&);


int main() {
  List<int> l(5);
}
