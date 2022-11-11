#pragma once

#include <exception>

struct ControlBlockBase {
    size_t strong_cnt = 1;
    size_t weak_cnt = 0;

    virtual void Destroy(){};

    virtual ~ControlBlockBase() = default;
};

template <typename T>
struct ControlBlockPointer : public ControlBlockBase {
    explicit ControlBlockPointer(T* ptr) : ptr(ptr) {
    }

    void Destroy() override {
        delete ptr;
    }

    ~ControlBlockPointer() override {
    }

    T* ptr;
};

template <typename T>
struct ControlBlockEmplace : public ControlBlockBase {
    template <typename... Args>
    ControlBlockEmplace(Args&&... args) {
        new (&storage) T{std::forward<Args>(args)...};
    }

    T* GetRawPtr() {
        return reinterpret_cast<T*>(&storage);
    }

    void Destroy() override {
        GetRawPtr()->~T();
    }

    ~ControlBlockEmplace() override {
    }

    std::aligned_storage_t<sizeof(T), alignof(T)> storage;
};

class EnableSharedFromThisBase {};

template <typename T>
class EnableSharedFromThis;

class BadWeakPtr : public std::exception {};

template <typename T>
class SharedPtr;

template <typename T>
class WeakPtr;
