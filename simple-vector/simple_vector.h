#pragma once

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <initializer_list>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include "array_ptr.h"

using namespace std::string_literals;

struct ReserveProxyObj {
    ReserveProxyObj(size_t capacity) 
    : capacity_to_reserve(capacity){}
    size_t capacity_to_reserve;
};

ReserveProxyObj Reserve(size_t capacity_to_reserve) {
        return ReserveProxyObj(capacity_to_reserve);
}

template <typename Type>
class SimpleVector {
public:
    using Iterator = Type*;
    using ConstIterator = const Type*;

    SimpleVector() noexcept = default;

    // Создаёт вектор из size элементов, инициализированных значением по умолчанию
    explicit SimpleVector(size_t size) 
    : array_(size) {
        capacity_ = size;
        std::fill_n(&array_[0], size, Type());
        size_ = size;

    }

    // Создаёт вектор из size элементов, инициализированных значением value
    SimpleVector(size_t size, const Type& value) 
    : array_(size) {
        capacity_ = size;
        std::fill_n(&array_[0], size, value);
        size_ = size;
    }

    // Создаёт вектор из std::initializer_list
    SimpleVector(std::initializer_list<Type> init) 
    : array_(init.size()) {
        capacity_ = static_cast<int>(init.size());
        std::copy(init.begin(), init.end(), &array_[0]);
        size_ = static_cast<int>(init.size());
    }

    ~SimpleVector() {
    }

    // Возвращает количество элементов в массиве
    size_t GetSize() const noexcept {
        return size_;
    }

    // Возвращает вместимость массива
    size_t GetCapacity() const noexcept {
        return capacity_;
    }

    // Сообщает, пустой ли массив
    bool IsEmpty() const noexcept {
        return size_ == 0;
    }

    // Возвращает ссылку на элемент с индексом index
    Type& operator[](size_t index) noexcept {
        assert(index < size_);
        return array_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    const Type& operator[](size_t index) const noexcept {
        assert(index < size_);
        return array_[index];
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    Type& At(size_t index) {
        if (index >= size_) {
            throw std::out_of_range("index >= size"s);
        } else {
            return array_[index];
        }
    }

    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    const Type& At(size_t index) const {
        if (index >= size_) {
            throw std::out_of_range("index >= size"s);
        } else {
            return array_[index];
        }
    }

    // Обнуляет размер массива, не изменяя его вместимость
    void Clear() noexcept {
        size_ = 0;
    }

    // Изменяет размер массива.
    // При увеличении размера новые элементы получают значение по умолчанию для типа Type
    void Resize(size_t new_size) {
        if (new_size <= size_) {
            size_ = new_size;
        } else {
            if (new_size <= capacity_) {
                for (size_t i = size_; i < new_size; ++i) {
                    array_[i] = Type();
                }
                size_ = new_size;
            } else {
                ArrayPtr<Type> new_array(new_size);
                for (size_t i = 0; i < size_; ++i) {
                    new_array[i] = std::move(array_[i]);
                }
                for (size_t i = size_; i < new_size; ++i) {
                    new_array[i] = Type();
                }
                capacity_ = new_size;
                size_ = new_size;
                array_.swap(new_array);
            }            
        }
    }

    // Возвращает итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator begin() noexcept {
        if (size_ == 0) {
            return nullptr;
        } else {
            return &array_[0];
        }
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator end() noexcept {
        if (size_ == 0) {
            return nullptr;
        } else {
            return &array_[size_];
        }
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator begin() const noexcept {
        if (size_ == 0) {
            return nullptr;
        } else {
            return const_cast<Type*>(&array_[0]);
        }
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator end() const noexcept {
        if (size_ == 0) {
            return nullptr;
        } else {
            return const_cast<Type*>(&array_[size_]);
        }
    }

    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cbegin() const noexcept {
        return begin();
    }

    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cend() const noexcept {
        return end();
    }

    // Обменивает значение с другим вектором
    void swap(SimpleVector& other) noexcept {
        array_.swap(other.array_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }

    void Reserve(size_t new_capacity) {
        if (new_capacity <= capacity_) {
            return;
        } else {
            ArrayPtr<Type> new_array(new_capacity);
            for (size_t i = 0; i < size_; ++i) {
                new_array[i] = std::move(array_[i]);
            }
            capacity_ = new_capacity;
            array_.swap(new_array);
        }
    }

    SimpleVector(ReserveProxyObj reserve) {
        SimpleVector copy;
        copy.Reserve(reserve.capacity_to_reserve);
        swap(copy);
    }

    SimpleVector(const SimpleVector& other) {
        SimpleVector copy;
        copy.Reserve(other.GetSize());
        for (Type element : other) {
            copy.PushBack(element);
        }
        swap(copy);
    }

    SimpleVector(SimpleVector&& other) {
        SimpleVector copy;
        copy.Reserve(other.GetSize());
        for (Type& element : other) {
            copy.PushBack(std::move(element));
        }
        other.Clear();
        swap(copy);
    }

    SimpleVector& operator=(const SimpleVector& rhs) {
        SimpleVector copy;
        copy.Reserve(rhs.GetSize());
        for (Type element : rhs) {
            copy.PushBack(element);
        }
        swap(copy);
        return *this;
    }

    // Добавляет элемент в конец вектора
    // При нехватке места увеличивает вдвое вместимость вектора
    void PushBack(const Type& item) {
        if (capacity_ == 0) {
            Reserve(1);
        }
        if (size_ == capacity_) {
            Reserve(capacity_ * 2);
        }
        array_[size_] = item;
        ++size_;
    }

    void PushBack(Type&& item) {
        if (capacity_ == 0) {
            Reserve(1);
        }
        if (size_ == capacity_) {
            Reserve(capacity_ * 2);
        }
        array_[size_] = std::move(item);
        ++size_;
    }

    // Вставляет значение value в позицию pos.
    // Возвращает итератор на вставленное значение
    // Если перед вставкой значения вектор был заполнен полностью,
    // вместимость вектора должна увеличиться вдвое, а для вектора вместимостью 0 стать равной 1
    Iterator Insert(ConstIterator pos, const Type& value) {
        assert(pos >= begin() && pos <= end());
        int pos_index = pos - begin();
        if (capacity_ == 0) {
            Reserve(1);
        }
        if (size_ == capacity_) {
            Reserve(capacity_ * 2);
        }
        std::copy_backward(&array_[pos_index], &array_[size_], &array_[size_ + 1]);
        array_[pos_index] = value;
        ++size_;
        return &array_[pos_index];
    }

    Iterator Insert(ConstIterator pos, Type&& value) {
        assert(pos >= begin() && pos <= end());
        int pos_index = pos - begin();
        if (capacity_ == 0) {
            Reserve(1);
        }
        if (size_ == capacity_) {
            Reserve(capacity_ * 2);
        }
        std::move_backward(&array_[pos_index], &array_[size_], &array_[size_ + 1]);
        array_[pos_index] = std::move(value);
        ++size_;
        return &array_[pos_index];
    }

    // "Удаляет" последний элемент вектора. Вектор не должен быть пустым
    void PopBack() noexcept {
        if (size_ > 0) {
            --size_;
        }
    }

    // Удаляет элемент вектора в указанной позиции
    Iterator Erase(ConstIterator pos) {
        assert(pos >= begin() && pos < end());
        int pos_index = pos - begin();
        std::move(&array_[pos_index + 1], &array_[size_], &array_[pos_index]);
        --size_;
        Iterator result = size_ > 0 ? &array_[pos_index] : nullptr;
        return result;
    }


private:
    ArrayPtr<Type> array_;
    size_t size_ = 0;
    size_t capacity_ = 0;

};

template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    if (lhs.GetSize() != rhs.GetSize()) {
        return false;
    }
    return std::equal(lhs.begin(), lhs.end(), rhs.begin());
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
    return !(lhs > rhs);
}

template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return rhs < lhs;
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs < rhs);
}