#include <cstddef>
#include <memory>

namespace bormon {

template <typename Type, typename Alloc = std::allocator<Type>>
class vector {
public:
    using Alloc_traits = std::allocator_traits<Alloc>;
    using iterator = Type*;

    vector() noexcept(noexcept(Alloc())) = default;

    // STL vector checks the max possible size via alloc::max_size
    // I will not check
    // ? Constexpr ? 

    explicit vector(size_t count, const Alloc& alloc = Alloc()) 
        : size_{count}, capacity_{count}, allocator_{alloc} {
        ptr_ = Alloc_traits::allocate(allocator_, capacity_); 
        fill_with_copy(count);
    }

    explicit vector(size_t count, Type& T, const Alloc& alloc = Alloc())
        : size_{count}, capacity_{count}, allocator_{alloc} {
        ptr_ = Alloc_traits::allocate(allocator_, capacity_); 
        fill_with_copy(count, T);
    }

    template<typename InputIterator>
    vector(InputIterator first, InputIterator second, const Alloc& alloc = Alloc())
        : size_{second - first}, capacity_{size_}, allocator_{alloc} {
        ptr_ = Alloc_traits::allocate(allocator_, capacity_); 
        fill_in_range(first, second);
    }

    vector(const vector& other) 
        : size_{other.size_}, capacity_{other.capacity_}, allocator_{other.allocator} {
        ptr_ = Alloc_traits::allocate(allocator_, capacity_); 
        fill_in_range(other.begin(), other.end());        
    }

    vector(vector<Type>&& other) noexcept   
      : size_{other.size_}, capacity_{other.capacity_}, allocator_{std::move(other.allocator)} {
        other.ptr_ = nullptr;
        other.size_ = 0;
        other.capacity_ = 0;
    }

    vector& operator=(const vector& other) {
        if (this == std::addressof(other)) { return *this; }
        
        if (capacity_ < other.capacity_) {
            destroy();
            deallocate;
            ptr_ = Alloc_traits::allocate(allocator_, other.capacity_); 
        } else {
            destroy();
        }

        fill_in_range(other.begin(), other.end()); 
        return *this; 
    }
    
    vector& operator=(vector&& other) noexcept {
        ptr_ = other.ptr_;
        size_ = other.size_;
        capacity_ = other.capacity_;
        other.ptr_ = nullptr;
        other.size_ = 0;
        other.capacity_ = 0;
    }

    template<typename... Args>
    void emplace_back(Args&&... args) {
        if (size_ == capacity_) {
            reserve(capacity_ * 2);
        }
        Alloc_traits::construct(allocator_, ptr_ + size_, std::forward<Args>(args)...);
        ++size_;
    }

    // Push_back overload for l & r -values
    void push_back(const Type& value) {
        if (size_ == capacity_) {
            reserve(capacity_ * 2);
        }
        Alloc_traits::construct(allocator_, ptr_ + size_, value);
        ++size_;
    }

    void push_back(Type&& value) {
        if (size_ == capacity_) {
            reserve(capacity_ * 2);
        }
        Alloc_traits::construct(allocator_, ptr_ + size_, std::move(value));
        ++size_;
    }


    void pop_back() {
        Alloc_traits::destroy(allocator_, ptr_ + size_);
        --size_;
    }
    
    void reserve(size_t size) {
        if (size <= capacity_) { return; } 
        Type* newptr = Alloc_traits::allocate(allocator_, capacity_); 
        size_t i{0};
        try {
            for(; i < size; ++i) {
                Alloc_traits::construct(allocator_, newptr + i, std::move_if_noexcept(ptr_ + i));
            }
        } catch(...) {
            for(int j = 0; j < i; ++j) {
                Alloc_traits::destroy(allocator_, newptr[j]);
                Alloc_traits::deallocate(allocator_, newptr, i);
            }
            return;
        }
        destroy();
        Alloc_traits::deallocate(allocator_, ptr_, capacity_); 
        ptr_ = newptr;
        capacity_ = size;
    }

    // Type must be default constructable or have copy constructor
    void resize(size_t size, const Type& value = Type()) {
        if (size == capacity_) { return; }
        Type* newptr = Alloc_traits::allocate(allocator_, size); 
        size_t i{0};
        size_t max{0};
        try {
            (size < size_) ? (max = size) : (max = size_);
            for(; i < max; ++i) {
                Alloc_traits::construct(allocator_, newptr + i, std::move_if_noexcept(ptr_ + i));
            }
        } catch(...) {
            for(int j = 0; j < max; ++j) {
                Alloc_traits::destroy(allocator_, newptr[j]);
                Alloc_traits::deallocate(allocator_, newptr, i);
            }
            return;
        }
        if (size < size_) {
            destroy();
            Alloc_traits::deallocate(allocator_, ptr_, capacity_); 
            ptr_ = newptr;
            capacity_ = size;
            size_ = size;   
        } else {
            for (; i < size; ++i) {
                Alloc_traits::construct(allocator_, newptr + i, value);
            }
            destroy();
            Alloc_traits::deallocate(allocator_, ptr_, capacity_); 
            ptr_ = newptr;
            capacity_ = size;
            size_ = size;
        }
    }

    void shrink_to_fit() {
        Type* newptr = Alloc_traits::allocate(allocator_, size_); 
        size_t i{0};
        try {
            for(; i < size; ++i) {
                Alloc_traits::construct(allocator_, newptr + i, std::move_if_noexcept(ptr_ + i));
            }
        } catch(...) {
            for(int j = 0; j < i; ++j) {
                Alloc_traits::destroy(allocator_, newptr[j]);
                Alloc_traits::deallocate(allocator_, newptr, i);
            }
            return;
        }
        destroy();
        Alloc_traits::deallocate(allocator_, ptr_, capacity_); 
        ptr_ = newptr;
        capacity_ = size_;
    }

    Type& at(size_t position) {
        if (size_ >= position) {
            throw std::out_of_range("");
        }
        return *(ptr_ + position);
    }

    const Type& at(size_t position) const {
        if (size_ >= position) {
            throw std::out_of_range("");
        }
        return *(ptr_ + position);
    }

    Type& operator[](size_t position) {
        return *(ptr_ + position);
    }

    const Type& operator[](size_t position) const {
        return *(ptr_ + position);
    }

    size_t capacity() const noexcept {
        return capacity_;
    }

    size_t size() const noexcept {
        return size_;
    }

private:
    template<typename InputIterator>
    void fill_in_range(InputIterator first, InputIterator second) {
        size_t count = second - first;
        size_t i{0};
        try {
            for (; i < count; ++i) {
                Alloc_traits::construct(allocator_, ptr_ + i, *first++);
            }
        } catch(...) {
            for (int j = 0; j < i; ++j) {
                Alloc_traits::destroy(allocator_, ptr_ + j); 
            }
            Alloc_traits::deallocate(allocator_, ptr_, count);
            throw;
        }
    }

    void fill_with_copy(size_t count, const Type& value = Type()) {
        size_t i{0};
        try {
            for (; i < count; ++i) {
                Alloc_traits::construct(allocator_, ptr_ + i, value);
            }
        } catch(...) {
            for (int j = 0; j < i; ++j) {
                Alloc_traits::destroy(allocator_, ptr_ + j); 
            }
            Alloc_traits::deallocate(allocator_, ptr_, count);
            throw;
        }
    }

    void destroy() {
        for (int i = 0; i < size_; ++i) {
            Alloc_traits::destroy(allocator_, ptr_ + i);
        }
    }
    
    void deallocate() {
        Alloc_traits::deallocate(allocator_, ptr_, capacity_); 
        size_ = 0;
        capacity_ = 0;
        ptr_ = nullptr;
    }

    // Vector contain pointers but I will use size_t instead
    Type* ptr_;
    size_t size_;
    size_t capacity_;
    Alloc allocator_;
};
}  // namespace bormon
