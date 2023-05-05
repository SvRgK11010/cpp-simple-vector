#pragma once
#include "array_ptr.h"
#include <cassert>
#include <stdexcept>
#include <initializer_list>
#include <algorithm>

class ReserveProxyObj {
public:
    ReserveProxyObj(const size_t value) {
        value_ = value;
    }
    size_t Get() {
        return value_;
    }
private:
    size_t value_ = 0;
};

template <typename Type>
class SimpleVector {
public:
    using Iterator = Type*;
    using ConstIterator = const Type*;

    SimpleVector() noexcept = default;
    
    explicit SimpleVector(size_t size)
        :items_(size) {
        size_ = size;
        capacity_ = size;
        std::fill(begin(), end(), Type());
    }

    SimpleVector(ReserveProxyObj to_res) {
        capacity_ = to_res.Get();
        size_ = 0;
    }

    SimpleVector(SimpleVector&& other) {
        items_ = std::move(other.items_);
        std::exchange(size_, other.size_);
        std::exchange(capacity_, other.capacity_);
        other.size_ = 0;
        other.capacity_ = 0;
    }

    void Reserve(size_t new_capacity) {
        if (new_capacity > capacity_) {
            ArrayPtr<Type> new_vec(new_capacity);
            std::copy(begin(), end(), new_vec.Get());
            items_.swap(new_vec);
            capacity_ = new_capacity;
        }
    }
    
    SimpleVector(size_t size, const Type& value)
        :items_(size)
    {
        capacity_ = size;
        size_ = size;
        std::fill(begin(), end(), value);
    }

    SimpleVector(std::initializer_list<Type> init)
        :items_(init.size()) {
        size_ = init.size();
        capacity_ = init.size();
        std::copy(init.begin(), init.end(), begin());
    }

    SimpleVector(const SimpleVector& other)
        :items_(other.GetSize()), size_(other.GetSize()), capacity_(other.GetCapacity()) {
        std::copy(other.begin(), other.end(), this->begin());
    }

    size_t GetSize() const noexcept {
        return size_;
    }

    size_t GetCapacity() const noexcept {
        return capacity_;
    }

    bool IsEmpty() const noexcept {
        return size_ == 0;
    }

    SimpleVector& operator=(SimpleVector&& rhs) {
        if (*this!= rhs) {
            SimpleVector tmp_list(std::move(rhs));
            swap(tmp_list);
        }
        return *this;
    }

    SimpleVector& operator=(const SimpleVector& rhs) {
        if (this != &rhs) {
            SimpleVector tmp_list(rhs);
            swap(tmp_list);
        }
        return *this;
    }

    Type& operator[](size_t index) noexcept {
        return items_[index];
    }

    const Type& operator[](size_t index) const noexcept {
        return items_[index];
    }
        
    Type& At(size_t index) {
        if (index >= size_)
        {
            throw std::out_of_range("");
        }
        return items_[index];
    }

    const Type& At(size_t index) const {
        if (index >= size_)
        {
            throw std::out_of_range("");
        }
        return items_[index];
    }
       
    void Clear() noexcept {
        size_ = 0u;
    }

    void Resize(size_t new_size) {
        if (new_size > capacity_) {
            ArrayPtr<Type> new_vec(new_size);
            std::move(items_.Get(), items_.Get() + size_, new_vec.Get());
            items_.swap(new_vec);
            Iterator it_begin = items_.Get() + size_;
            size_t steps = new_size - size_;
            Iterator it_end = it_begin + steps;
            std::generate(it_begin, it_end, [] { return Type{}; });
            capacity_ = std::max(new_size, capacity_ * 2);
            size_ = new_size;
        }
        else if (new_size > size_ && new_size < capacity_) {
            Iterator it_begin = items_.Get() + size_;
            size_t steps = new_size - size_;
            Iterator it_end = it_begin + steps;
            std::generate(it_begin, it_end, [] { return Type{}; });
            size_ = new_size;
        }
        else if (new_size < size_) { size_ = new_size; }
    }

    void PushBack(const Type& item) {
        if (size_ < capacity_) {

            items_[size_++] = item;

        }
        else {
            size_t new_size = std::max(1, static_cast<int>(size_) * 2);
            ArrayPtr<Type> new_vec(new_size);
            std::copy(items_.Get(), items_.Get() + size_, new_vec.Get());
            items_.swap(new_vec);
            new_vec[size_] = item;            
            capacity_ = new_size;
            size_ += 1;
        }
    }

    void PushBack(Type&& item) {
        if (size_ < capacity_) {

            items_[size_++] = std::move(item);

        }
        else {
            size_t new_size = std::max(1, static_cast<int>(size_) * 2);
            ArrayPtr<Type> new_vec(new_size);
            std::move(items_.Get(), items_.Get() + size_, new_vec.Get());
            new_vec[size_] = std::move(item);
            items_.swap(new_vec);
            capacity_ = new_size;
            size_ += 1;
        }
    }

    void PopBack() noexcept {
        if (!IsEmpty()) {
            --size_;
        }
    }

    Iterator Insert(ConstIterator pos, const Type& value) {
        size_t index = pos - begin();
        if (size_ < capacity_) {
            if (pos == end()) {
                items_[size_] = value;
            }
            else {
                std::copy_backward(items_.Get() + index, items_.Get() + size_, items_.Get() + size_ + 1);
                items_[index] = value;
            }
            ++size_;
        }
        else {
            size_t new_capacity = capacity_ == 0 ? 1 : 2 * capacity_;
            ArrayPtr<Type> new_vec(new_capacity);
            std::copy(items_.Get(), items_.Get() + index, new_vec.Get());
            new_vec[index] = value;
            std::copy(items_.Get() + index, items_.Get() + size_, new_vec.Get() + index + 1);
            items_.swap(new_vec);
            capacity_ = new_capacity;
            ++size_;
        }
        return begin() + index;
    }

    Iterator Insert(Iterator pos, Type&& value) {
        assert(pos >= begin() && pos <= end());
        size_t index = pos - begin();
        if (size_ < capacity_) {
            if (pos == end()) {
                items_[size_] = std::move(value);
            }
            else {
                
                std::move_backward(items_.Get() + index, items_.Get() + size_, items_.Get() + size_ + 1);
                items_[index] = std::move(value);
            }
            ++size_;
        }
        else {
            size_t new_capacity = capacity_ == 0 ? 1 : 2 * capacity_;
            ArrayPtr<Type> new_vec(new_capacity);
            std::move(items_.Get(), items_.Get() + index, new_vec.Get());
            new_vec[index] = std::move(value);
            std::move(items_.Get() + index, items_.Get() + size_, new_vec.Get() + index + 1);
            items_.swap(new_vec);
            capacity_ = new_capacity;
            ++size_;
        }
        return begin() + index;
    }

    Iterator Erase(ConstIterator pos) {
        assert(pos >= begin() && pos < end());
        auto index = std::distance(cbegin(), pos);
        auto it = begin() + index;
        std::move((it + 1), end(), it);
        --size_;
        return Iterator(pos);
    }

    void swap(SimpleVector& other) noexcept {
        items_.swap(other.items_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }

    Iterator begin() noexcept {
        return items_.Get();
    }

    Iterator end() noexcept {        
        Iterator it_end = items_.Get() + size_;
        return it_end;
    }

    ConstIterator begin() const noexcept {
        return items_.Get();
    }

    ConstIterator end() const noexcept {
        ConstIterator it_end = items_.Get() + size_;
        return it_end;
    }

    ConstIterator cbegin() const noexcept {       
        return items_.Get();
    }

    ConstIterator cend() const noexcept {
        ConstIterator it_end = items_.Get() + size_;
        return it_end;
    }

private:
    ArrayPtr<Type> items_;
    size_t size_ = 0;
    size_t capacity_ = 0;
};

template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    if (lhs.GetSize() == rhs.GetSize()) {
        return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
    }
    else { return false; }
}

template <typename Type>
inline bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs == rhs);
}

template <typename Type>
inline bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template <typename Type>
inline bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return (lhs == rhs) || (lhs < rhs);
}

template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return rhs < lhs;
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return (lhs == rhs) || (rhs < lhs);
}

ReserveProxyObj Reserve(size_t capacity_to_reserve) {
    return ReserveProxyObj(capacity_to_reserve);
}