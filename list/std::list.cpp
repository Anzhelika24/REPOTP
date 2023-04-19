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
  explicit StackAllocator(StackStorage<N>& pool) : pool_(&pool) {};
  ~StackAllocator() {}
  template <typename U>
  StackAllocator<T, N>& operator=(StackAllocator<U, N> other);
  T* allocate(size_t count);
  void deallocate(T*, size_t) {};
  void swap(StackAllocator& alloc);

//  StackAllocator<T, N> select_on_container_copy_construction() { //нужен ли вообще этот метод
//    return MyAllocator<T>(*this);
//  }

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
    Node(BaseNode*) : BaseNode(), value() {} //конструктор от бейзноды
  };

  BaseNode* fakeNode; // for function link two_nodes

 public:
  List();
  List(size_t size);
  List(size_t size, const T&);
  List(const Alloc&);
  List(size_t, const Alloc&);
  List(size_t, const T&, const Alloc&);
  List(const List&);
  ~List();
  List& operator=(const List&);
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
 public:

  template <bool IsConst>
  class Iterator {

   public:
    BaseNode* node;

    using iterator_category = std::bidirectional_iterator_tag;
    using pointer = std::conditional_t<IsConst, const T* , T* >;
    using reference = std::conditional_t<IsConst, const T &, T &>;
    using difference_type = std::ptrdiff_t;
    using value_type = std::conditional_t<IsConst, const T, T>;

    std::conditional_t<IsConst, const T&, T&> operator*() const;
    std::conditional_t<IsConst, const T*, T*> operator->() const;
    Iterator(BaseNode* ptr) : node(ptr) {}
    Iterator(const Iterator<false>& other) : node(other.node) {}

    Iterator& operator++() {
      node = node->next;
      return *this;
    }

    Iterator operator++(int) {
      Iterator copy(*this);
      operator++();
      return copy;
    }

    Iterator& operator--() {
      node = node->prev;
      return *this;
    }

    Iterator operator--(int) {
      Iterator copy(*this);
      operator--();
      return copy;
    }

    Iterator& operator=(const Iterator& other) {
      node = other.node;
      return *this;
    }

    Iterator& operator+=(size_t n) {
      for (int i = 0; i < n; ++i) {
        node = node->next;
      }
    }

    Iterator& operator-=(size_t n) {
      for (int i = 0; i < n; ++i) {
        node = node->prev;
      }
    }

    bool operator==(const Iterator& other) const {
      return node == other.node;
    }

    bool operator!=(const Iterator& other) const {
      return node != other.node;
    }
  };

 public:

  using iterator = Iterator<false>;
  using const_iterator = Iterator<true>;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  iterator begin() { return iterator(fakeNode->next); }
  const_iterator begin() const  { return begin(); }
  iterator end() { return iterator(fakeNode); }
  const_iterator end() const { return end(); }
  const_iterator cbegin() const { return begin(); }
  const_iterator cend() const { return end(); }
  reverse_iterator rbegin() { return reverse_iterator(fakeNode); }
  const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); }
  reverse_iterator rend() { return reverse_iterator(begin()); }
  const_reverse_iterator rend() const { return const_reverse_iterator(begin()); }
  const_reverse_iterator crbegin() const { return const_reverse_iterator(end()); }
  const_reverse_iterator crend() const { return const_reverse_iterator(begin()); }

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

//  void link_with_basenode(BaseNode* prev, BaseNode* next) {
//    prev->next = next;
//    next->prev = prev;
//    next->next = prev;
//    prev->prev = next;
//  }

  void link_with_node(BaseNode* prev, Node* next) {
    prev->next = next;
    next->prev = prev;
    next->next = prev;
    prev->prev = next;
  }

//  void link_three_basenodes(BaseNode* first, BaseNode* second, BaseNode* third) {
//    third->prev = second;
//    second->next = third;
//    first->prev = third;
//    third->next = first;
//  }

  void link_three_nodes_from_left(BaseNode* first, Node* second, Node* third) {
    third->prev = second;
    second->next = third;
    first->prev = third;
    third->next = first;
  }

  void fill_empty() {
    Node* Node_before = static_cast<Node*>(create_basenode()); //как применить наследование от ноды, чтобы
    link_with_node(fakeNode, Node_before);
    for (size_t i = 1; i < size_; ++i) {
      Node* newNode = static_cast<Node*>(create_basenode());
      link_three_nodes_from_left(fakeNode, Node_before, newNode);
      Node_before = newNode;
    }
  }

  void fill_with_value(const T& value) {
    Node* Node_before = create_node(value);
    link_two_nodes(fakeNode, Node_before);
    for (size_t i = 1; i < size_; ++i) {
      Node* newNode = create_node(value);
      link_three_nodes_from_left(fakeNode, Node_before, newNode);
      Node_before = newNode;
    }
  }
  void deallocate_node(Node* node) {
    AllocTraits::destroy(alloc_, node);
    AllocTraits::deallocate(alloc_, node, 1);
  }

  size_t size() const { return size_; }
  NodeAlloc get_allocator() const { return alloc_;}
  bool empty() const { return size_ == 0;}
  void clear() {}

  void push_back(const T& value) {
    Node* node = create_node(value);
    if(empty()) {
      link_with_node(fakeNode, node);
    } else {
      link_three_nodes_from_left(fakeNode, static_cast<Node*>(fakeNode->prev), node);
    }
    ++size_;
  }

  void push_front(const T& value) {
    Node* node = create_node(value);
    if(empty()) {
      link_with_node(fakeNode, node);
    } else {
      node->next = fakeNode->next;
      node->prev = fakeNode;
      fakeNode->next = node;
      fakeNode->next->prev = node;
    }
    ++size_;
  }

  template <bool IsConst>
  void insert(Iterator<IsConst> iter, const T& value) { //problems with const iter
    Node* next_node = static_cast<Node*>(iter.node);
    Node* prev_node = static_cast<Node*>(next_node->prev);
    Node* new_node = create_node(value);
    prev_node->next = new_node;
    new_node->prev = prev_node;
    new_node->next = next_node;
    next_node->prev = new_node;
    ++size_;
    //вставка по константному итератору, значение вставляется ПЕРЕД итератором
    // чтобы вставить в начало листа, делаем инсерт по итератору бегин
    // чтобы вставить в конец листа, делаем инсерт по итератору энд
  }
  template <bool IsConst>
  void erase(Iterator<IsConst> iter) {
    BaseNode* pop_node = iter.node;
    BaseNode* first = pop_node->prev;
    BaseNode* second = pop_node->next;
    first->next = second;
    second->prev = first;
    deallocate_node(static_cast<Node*>(pop_node));
    --size_;
  }
  void pop_back() { erase(--end()); }
  void pop_front() { erase(begin()); }
};

template <typename T, typename Alloc>
template <bool IsConst>
std::conditional_t<IsConst, const T&, T&> List<T, Alloc>::Iterator<IsConst>::operator*() const {
  return (static_cast<Node*>(node))->value;
}

template <typename T, typename Alloc>
template <bool IsConst>
std::conditional_t<IsConst, const T*, T*> List<T, Alloc>::Iterator<IsConst>::operator->() const {
  return *Iterator(node);
}


template <typename T, typename Alloc>
List<T, Alloc>::List() : size_(0), alloc_(NodeAlloc()) {
  create_fakenode();
}

template <typename T, typename Alloc>
List<T, Alloc>::List(size_t size) : size_(size), alloc_(NodeAlloc()) {
  create_fakenode();
  fill_empty();
}

template <typename T, typename Alloc>
List<T, Alloc>::List(size_t size, const T& value) : size_(size), alloc_(NodeAlloc()) {
  create_fakenode();
  fill_with_value(value);
}

template <typename T, typename Alloc>
List<T, Alloc>::List(const Alloc& alloc) : size_(0), alloc_(AllocTraits::select_on_container_copy_construction(alloc)) {
  create_fakenode();
}
template <typename T, typename Alloc>
List<T, Alloc>::List(size_t size, const Alloc& alloc) : List(alloc) {
  size_ = size;
  fill_empty();
}

template <typename T, typename Alloc>
List<T, Alloc>::List(size_t size, const T& value, const Alloc& alloc) : List(alloc) {
  size_ = size;
  fill_with_value(value);
}

template<typename T, typename Alloc>
List<T, Alloc>::List(const List& other) : List(other.alloc_) {
  size_ = other.size_;
  for(auto it = other.begin(); it != other.end(); ++it) {
    push_back(*it);
  }
}

template<typename T, typename Alloc>
List<T, Alloc>& List<T, Alloc>:: operator=(const List& other) {
  auto copy = &other;
  while (size_ != 0) pop_back();
  if(AllocTraits::propagate_on_container_copy_assignment::value) {
    AllocTraits::deallocate(alloc_, static_cast<Node*>(fakeNode), 1);
    alloc_ = other.alloc_;
    fakeNode = AllocTraits::allocate(alloc_, 1);
    fakeNode->next = fakeNode;
    fakeNode->prev = fakeNode;
  }
  BaseNode* other_begin = other.fakeNode->next;
  for (auto& elem: other) {
    push_back(elem);
    other_begin = other_begin->next;
  }
  return *this;
}

template <typename T, class Alloc>
List<T, Alloc>::~List() {
  while (size_ != 0) {
    pop_back();
  }
  AllocTraits::deallocate(alloc_, static_cast<Node*>(fakeNode), 1);
//  AllocTraits::deallocate(alloc_, fakeNode, 1);
}

