#include <cstddef>
#include <memory>

namespace bormon {

template <typename Type, typename Alloc = std::allocator<Type>>
class vector {
public:
    using Alloc_traits = std::allocator_traits<Alloc>;

    vector() noexcept(noexcept(Alloc())) = default;

    // STL vector checks the max possible size via alloc::max_size
    // I will not check
    // ? Constexpr ? 
    explicit vector(size_t count, const Alloc& alloc = Alloc()) 
        : size_{count}, capacity_{count}, allocator_{alloc} {
        initialize(capacity_);
        fill_with_copy(count);
    }

    explicit vector(size_t count, Type& T, const Alloc& alloc = Alloc())
        : size_{count}, capacity_{count}, allocator_{alloc} {
        initialize(capacity_);
        fill_with_copy(count, T);
    }

    template<typename InputIterator>
    vector(InputIterator first, InputIterator second, const Alloc& alloc = Alloc())
        : size_{second - first}, capacity_{size_}, allocator_{alloc} {
        initialize(capacity_);
        fill_in_range(first, second);
    }

    vector(const vector& other) 
        : size_{other.size_}, capacity_{other.capacity_}, allocator_{other.allocator} {
        initialize(capacity_);
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
            initialize(other.capacity_);
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
        try {
            for (int i = 0; i < count; ++i) {
                Alloc_traits::construct(allocator_, ptr_ + i, *first++);
            }
        } catch(...) {
            Alloc_traits::deallocate(allocator_, ptr_, count);
            throw;
        }
    }

    void fill_with_copy(size_t count, const Type& value = Type()) {
        try {
            for (int i = 0; i < count; ++i) {
                Alloc_traits::construct(allocator_, ptr_ + i, value);
            }
        } catch(...) {
            Alloc_traits::deallocate(allocator_, ptr_, count);
            throw;
        }
    }

    void initialize(size_t capacity) {
        ptr_ = Alloc_traits::allocate(allocator_, capacity); 
    }

    void destroy() {
        for (int i = 0; i < size; ++i) {
            Alloc_traits::destroy(allocator_, ptr_ + i);
        }
    }
    
    void deallocate() {
        Alloc_traits::deallocate(allocator_, ptr_, capacity); 
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
