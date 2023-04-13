#include <cstddef>
#include <memory>

namespace bormon {

template <typename Type, typename Alloc = std::allocator<Type>>
class vector {
public:
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
    // Vector contain pointers but I will use size_t instead
    Type* ptr_;
    size_t size_;
    size_t capacity_;
    Alloc allocator_;
};
}  // namespace bormon
