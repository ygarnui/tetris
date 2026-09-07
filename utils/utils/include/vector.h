#pragma once

#include <vector>

template <typename T, typename I>
class Vector
{
public:
    using reference = typename std::vector<T>::reference;
    using const_reference = typename std::vector<T>::const_reference;
    using iterator = typename std::vector<T>::iterator;
    using const_iterator = typename std::vector<T>::const_iterator;

    Vector() = default;

    explicit Vector(size_t size) : vec_(size) { }
    explicit Vector(size_t size, const T& val) : vec_(size, val) { }

    Vector(std::vector<T>&& vec) : vec_(std::move(vec)) { }

    template<class InputIt>
    Vector(InputIt first, InputIt last) : vec_(first, last) { }

    Vector(std::initializer_list<T> init) : vec_(init) { }

    [[nodiscard]] bool operator == (const Vector& b) const { return vec_ == b.vec_; }

    [[nodiscard]] bool operator != (const Vector& b) const { return vec_ != b.vec_; }

    [[nodiscard]] bool empty() const { return vec_.empty(); }

    [[nodiscard]] auto size() const { return vec_.size(); }

    [[nodiscard]] auto capacity() const { return vec_.capacity(); }

    void clear() { vec_.clear(); }

    void resize(size_t newSize) { vec_.resize(newSize); }
    void resize(size_t newSize, const T& t) { vec_.resize(newSize, t); }

    void reserve(size_t capacity) { vec_.reserve(capacity); }

    [[nodiscard]] const_reference operator[](I i) const { return vec_[size_t(i)]; }
    [[nodiscard]] reference operator[](I i) { return vec_[size_t(i)]; }

    void push_back(const T& t) { vec_.push_back(t); }
    void push_back(T&& t) { vec_.push_back(std::move(t)); }

    void pop_back() { vec_.pop_back(); }

    template<typename... Args>
    decltype(auto) emplace_back(Args&&... args) { return vec_.emplace_back(std::forward<Args>(args)...); }

    [[nodiscard]] const_reference front() const { return vec_.front(); }
    [[nodiscard]] reference front() { return vec_.front(); }
    [[nodiscard]] const_reference back() const { return vec_.back(); }
    [[nodiscard]] reference back() { return vec_.back(); }

    [[nodiscard]] auto data() { return vec_.data(); }
    [[nodiscard]] auto data() const { return vec_.data(); }

    void swap(Vector& b) { vec_.swap(b.vec_); }

    [[nodiscard]] inline auto begin()
    {
        return vec_.begin();
    }

    [[nodiscard]] inline auto end()
    {
        return vec_.end();
    }

    iterator erase(iterator pos)
    {
        return vec_.erase(pos);
    }

    iterator erase(const_iterator pos)
    {
        return vec_.erase(pos);
    }

    std::vector<T> vec_;
private:
};
