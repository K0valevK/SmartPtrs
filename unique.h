#pragma once

#include "compressed_pair.h"

#include <algorithm>
#include <cstddef>  // std::nullptr_t
#include <utility>

template <typename T>
struct DefaultDeleter {
    DefaultDeleter() noexcept = default;
    template <typename S>
    DefaultDeleter(DefaultDeleter<S>&& other) {
    }
    template <typename S>
    DefaultDeleter& operator=(DefaultDeleter<S>&& other) {
        return *this;
    }

    void operator()(T* ptr) const noexcept {
        delete ptr;
    }
};

template <typename T>
struct DefaultDeleter<T[]> {

    DefaultDeleter() noexcept = default;
    template <typename S>
    DefaultDeleter(DefaultDeleter<S>&& other) {
    }
    template <typename S>
    DefaultDeleter& operator=(DefaultDeleter<S>&& other) {
        return *this;
    }

    void operator()(T* ptr) const noexcept {
        delete[] ptr;
    }
};

// Primary template
template <typename T, typename Deleter = DefaultDeleter<T>>
class UniquePtr {
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    explicit UniquePtr(T* ptr = nullptr) {
        pair_.GetFirst() = ptr;
    }
    UniquePtr(T* ptr, Deleter deleter) {
        pair_.GetFirst() = ptr;
        pair_.GetSecond() = std::forward<Deleter>(deleter);
    }

    UniquePtr(const UniquePtr& other) = delete;
    UniquePtr(UniquePtr&& other) noexcept {
        pair_.GetFirst() = other.pair_.GetFirst();
        pair_.GetSecond() = std::forward<Deleter>(other.pair_.GetSecond());
        other.pair_.GetFirst() = nullptr;
    }

    template <typename S, typename SDeleter>
    UniquePtr(UniquePtr<S, SDeleter>&& other) noexcept {
        GetPair().GetFirst() = other.GetPair().GetFirst();
        other.GetPair().GetFirst() = nullptr;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    UniquePtr& operator=(const UniquePtr& other) = delete;
    UniquePtr& operator=(UniquePtr&& other) noexcept {
        if (this == &other) {
            return *this;
        }
        auto tmp = pair_.GetFirst();
        pair_.GetFirst() = other.pair_.GetFirst();
        if (tmp != nullptr) {
            GetDeleter()(tmp);
        }
        other.pair_.GetFirst() = nullptr;
        pair_.GetSecond() = std::move(other.pair_.GetSecond());
        return *this;
    }
    UniquePtr& operator=(std::nullptr_t) {
        GetDeleter()(pair_.GetFirst());
        pair_.GetFirst() = nullptr;
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~UniquePtr() {
        if (pair_.GetFirst() != nullptr) {
            GetDeleter()(pair_.GetFirst());
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    T* Release() {
        auto tmp = pair_.GetFirst();
        pair_.GetFirst() = nullptr;
        return tmp;
    }
    void Reset(T* ptr = nullptr) {
        auto old_ptr = pair_.GetFirst();
        pair_.GetFirst() = ptr;
        if (old_ptr != nullptr) {
            GetDeleter()(old_ptr);
        }
    }
    void Swap(UniquePtr& other) {
        auto tmp = pair_.GetFirst();
        pair_.GetFirst() = other.pair_.GetFirst();
        other.pair_.GetFirst() = tmp;
        auto tmp_del = std::move(pair_.GetSecond());
        pair_.GetSecond() = std::move(other.pair_.GetSecond());
        other.pair_.GetSecond() = std::move(tmp_del);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    CompressedPair<T*, Deleter>& GetPair() {
        return pair_;
    }

    T* Get() const {
        return pair_.GetFirst();
    }
    Deleter& GetDeleter() {
        return pair_.GetSecond();
    }
    const Deleter& GetDeleter() const {
        return pair_.GetSecond();
    }
    explicit operator bool() const {
        if (pair_.GetFirst() == nullptr) {
            return false;
        }
        return true;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Single-object dereference operators

    std::add_lvalue_reference_t<T> operator*() const {
        return *(pair_.GetFirst());
    }
    T* operator->() const {
        return pair_.GetFirst();
    }

private:
    CompressedPair<T*, Deleter> pair_;
};

// Specialization for arrays
template <typename T, typename Deleter>
class UniquePtr<T[], Deleter> {
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    explicit UniquePtr(T* ptr = nullptr) {
        pair_.GetFirst() = ptr;
    }
    UniquePtr(T* ptr, Deleter deleter) {
        pair_.GetFirst() = ptr;
        pair_.GetSecond() = std::forward<Deleter>(deleter);
    }

    UniquePtr(const UniquePtr& other) = delete;
    UniquePtr(UniquePtr&& other) noexcept {
        pair_.GetFirst() = other.pair_.GetFirst();
        pair_.GetSecond() = std::forward<Deleter>(other.pair_.GetSecond());
        other.pair_.GetFirst() = nullptr;
    }

    template <typename S, typename SDeleter>
    UniquePtr(UniquePtr<S, SDeleter>&& other) noexcept {
        pair_.GetFirst() = other.pair_.GetFirst();
        other.pair_.GetFirst() = nullptr;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    UniquePtr& operator=(const UniquePtr& other) = delete;
    UniquePtr& operator=(UniquePtr&& other) noexcept {
        if (this == &other) {
            return *this;
        }
        auto tmp = pair_.GetFirst();
        pair_.GetFirst() = other.pair_.GetFirst();
        if (tmp != nullptr) {
            GetDeleter()(tmp);
        }
        other.pair_.GetFirst() = nullptr;
        pair_.GetSecond() = std::move(other.pair_.GetSecond());
        return *this;
    }
    UniquePtr& operator=(std::nullptr_t) {
        GetDeleter()(pair_.GetFirst());
        pair_.GetFirst() = nullptr;
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~UniquePtr() {
        if (pair_.GetFirst() != nullptr) {
            GetDeleter()(pair_.GetFirst());
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    T* Release() {
        auto tmp = pair_.GetFirst();
        pair_.GetFirst() = nullptr;
        return tmp;
    }
    void Reset(T* ptr = nullptr) {
        auto old_ptr = pair_.GetFirst();
        pair_.GetFirst() = ptr;
        if (old_ptr != nullptr) {
            GetDeleter()(old_ptr);
        }
    }
    void Swap(UniquePtr& other) {
        auto tmp = pair_.GetFirst();
        pair_.GetFirst() = other.pair_.GetFirst();
        other.pair_.GetFirst() = tmp;
        auto tmp_del = std::move(pair_.GetSecond());
        pair_.GetSecond() = std::move(other.pair_.GetSecond());
        other.pair_.GetSecond() = std::move(tmp_del);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    T* Get() const {
        return pair_.GetFirst();
    }
    Deleter& GetDeleter() {
        return pair_.GetSecond();
    }
    const Deleter& GetDeleter() const {
        return pair_.GetSecond();
    }
    explicit operator bool() const {
        if (pair_.GetFirst() == nullptr) {
            return false;
        }
        return true;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Single-object dereference operators

    T& operator[](size_t index) {
        return pair_.GetFirst()[index];
    }
    std::add_lvalue_reference_t<T> operator*() const {
        return *(pair_.GetFirst());
    }
    T* operator->() const {
        return pair_.GetFirst();
    }

private:
    CompressedPair<T*, Deleter> pair_;
};
