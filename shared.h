#pragma once

#include "sw_fwd.h"  // Forward declaration

#include <cstddef>  // std::nullptr_t
#include <type_traits>


template <typename T>
class SharedPtr {
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    SharedPtr() : block_(nullptr), ptr_(nullptr) {
        ESFT();
    }
    SharedPtr(std::nullptr_t) : block_(nullptr), ptr_(nullptr) {
        ESFT();
    }
    explicit SharedPtr(T* ptr) {
        block_ = new ControlBlockPointer(ptr);
        ptr_ = ptr;
        ESFT();
    }

    SharedPtr(const SharedPtr& other) {
        block_ = other.block_;
        ptr_ = other.ptr_;
        if (block_) {
            block_->strong_cnt += 1;
        }
        ESFT();
    }
    SharedPtr(SharedPtr&& other) {
        block_ = other.block_;
        ptr_ = other.ptr_;
        other.block_ = nullptr;
        other.ptr_ = nullptr;
        ESFT();
    }

    template <typename S>
    explicit SharedPtr(S* ptr) {
        block_ = new ControlBlockPointer(ptr);
        ptr_ = ptr;
        ESFT();
    }
    template <typename S>
    SharedPtr(const SharedPtr<S>& other) {
        block_ = other.GetBlock();
        ptr_ = other.Get();
        if (block_) {
            block_->strong_cnt += 1;
        }
        ESFT();
    }
    template <typename S>
    SharedPtr(SharedPtr<S>&& other) {
        block_ = other.GetBlock();
        ptr_ = other.Get();
        if (block_) {
            block_->strong_cnt += 1;
        }
        other.Reset();
        ESFT();
    }

    SharedPtr(T* ptr, ControlBlockEmplace<T>* block) {
        block_ = block;
        ptr_ = ptr;
        ESFT();
    }

    // Aliasing constructor
    template <typename Y>
    SharedPtr(const SharedPtr<Y>& other, T* ptr) {
        ptr_ = ptr;
        block_ = other.GetBlock();
        if (block_) {
            block_->strong_cnt += 1;
        }
        ESFT();
    }

    // Promote `WeakPtr`
    explicit SharedPtr(const WeakPtr<T>& other) {
        if (other.Expired()) {
            throw BadWeakPtr();
        }
        block_ = other.GetBlock();
        ptr_ = other.Get();
        if (block_) {
            block_->strong_cnt += 1;
        }
        ESFT();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    SharedPtr& operator=(const SharedPtr& other) {
        if (this == &other) {
            return *this;
        }
        auto tmp = block_;
        block_ = other.block_;
        ptr_ = other.ptr_;
        if (block_) {
            block_->strong_cnt += 1;
        }
        if (tmp) {
            tmp->weak_cnt += 1;
            tmp->strong_cnt -= 1;
            if (tmp->strong_cnt == 0) {
                tmp->Destroy();
            }
            tmp->weak_cnt -= 1;
            if (tmp->strong_cnt == 0 && tmp->weak_cnt == 0) {
                delete tmp;
            }
        }
        return *this;
    }
    SharedPtr& operator=(SharedPtr&& other) {
        if (this == &other) {
            return *this;
        }
        auto tmp = block_;
        block_ = other.block_;
        ptr_ = other.ptr_;
        other.block_ = nullptr;
        other.ptr_ = nullptr;
        if (tmp) {
            tmp->weak_cnt += 1;
            tmp->strong_cnt -= 1;
            if (tmp->strong_cnt == 0) {
                tmp->Destroy();
            }
            tmp->weak_cnt -= 1;
            if (tmp->strong_cnt == 0 && tmp->weak_cnt == 0) {
                delete tmp;
            }
        }
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~SharedPtr() {
        if (block_) {
            block_->weak_cnt += 1;
            block_->strong_cnt -= 1;
            if (block_->strong_cnt == 0) {
                block_->Destroy();
            }
            block_->weak_cnt -= 1;
            if (block_->strong_cnt == 0 && block_->weak_cnt == 0) {
                delete block_;
            }
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    void Reset() {
        auto old_block = block_;
        block_ = nullptr;
        ptr_ = nullptr;
        if (old_block) {
            old_block->weak_cnt += 1;
            old_block->strong_cnt -= 1;
            if (old_block->strong_cnt == 0) {
                old_block->Destroy();
            }
            old_block->weak_cnt -= 1;
            if (old_block->strong_cnt == 0 && old_block->weak_cnt == 0) {
                delete old_block;
            }
        }
    }
    void Reset(T* ptr) {
        if (ptr_ == ptr) {
            return;
        }
        auto old_block = block_;
        block_ = new ControlBlockPointer(ptr);
        ptr_ = ptr;
        if (old_block) {
            old_block->weak_cnt += 1;
            old_block->strong_cnt -= 1;
            if (old_block->strong_cnt == 0) {
                old_block->Destroy();
            }
            old_block->weak_cnt -= 1;
            if (old_block->strong_cnt == 0 && old_block->weak_cnt == 0) {
                delete old_block;
            }
        }
    }
    template <typename S>
    void Reset(S* ptr) {
        if (ptr_ == ptr) {
            return;
        }
        auto old_block = block_;
        block_ = new ControlBlockPointer(ptr);
        ptr_ = ptr;
        if (old_block) {
            old_block->weak_cnt += 1;
            old_block->strong_cnt -= 1;
            if (old_block->strong_cnt == 0) {
                old_block->Destroy();
            }
            old_block->weak_cnt -= 1;
            if (old_block->strong_cnt == 0 && old_block->weak_cnt == 0) {
                delete old_block;
            }
        }
    }
    void Swap(SharedPtr& other) {
        auto tmp = block_;
        auto tmp_ptr = ptr_;
        block_ = other.block_;
        ptr_ = other.ptr_;
        other.block_ = tmp;
        other.ptr_ = tmp_ptr;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    ControlBlockBase* GetBlock() const {
        return block_;
    }
    T* Get() const {
        if (ptr_) {
            return ptr_;
        }
        return nullptr;
    }
    T& operator*() const {
        return *ptr_;
    }
    T* operator->() const {
        return ptr_;
    }
    size_t UseCount() const {
        if (block_) {
            return block_->strong_cnt;
        }
        return 0;
    }
    explicit operator bool() const {
        if (ptr_) {
            return true;
        }
        return false;
    }

    void ESFT() {
        if constexpr (std::is_convertible_v<T*, EnableSharedFromThisBase*>) {
            InitWeakThis(ptr_);
        }
    }
    template <typename Y>
    void InitWeakThis(EnableSharedFromThis<Y>* e) {
        e->weak_this_ = *this;
    }

private:
    ControlBlockBase* block_;
    T* ptr_;
};

template <typename T, typename U>
inline bool operator==(const SharedPtr<T>& left, const SharedPtr<U>& right) {
    return left.Get() == right.Get();
}


template <typename T, typename... Args>
SharedPtr<T> MakeShared(Args&&... args) {
    auto block = new ControlBlockEmplace<T>(std::forward<Args>(args)...);
    return SharedPtr<T>(block->GetRawPtr(), block);
}


template <typename T>
class EnableSharedFromThis : public EnableSharedFromThisBase {
public:
    SharedPtr<T> SharedFromThis() {
        return weak_this_.Lock();
    }
    SharedPtr<const T> SharedFromThis() const {
        return weak_this_.Lock();
    }

    WeakPtr<T> WeakFromThis() noexcept {
        return weak_this_;
    }
    WeakPtr<const T> WeakFromThis() const noexcept {
        return weak_this_;
    }

    WeakPtr<T> weak_this_;
};
