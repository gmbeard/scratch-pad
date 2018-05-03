#include <iostream>
#include <type_traits>
#include <iterator>
#include <vector>
#include <cassert>
#include <algorithm>

#define Ensure(condition) assert((condition))

template<typename C>
struct ByteBufferView;

template<typename C, bool IsConst>
struct ByteBufferViewIterator {

    friend struct ByteBufferViewIterator<C, true>;
    friend struct ByteBufferViewIterator<C, false>;

    template<typename _C>
    friend struct ByteBufferView;

    using Parent = C;
    using value_type = typename Parent::Item;
    using reference = value_type&;
    using pointer = value_type*;
    using difference_type = std::ptrdiff_t;
    using iterator_category = std::random_access_iterator_tag;
#ifdef _MSC_VER
    using _Unchecked_type = value_type;
#endif

private:
    ByteBufferViewIterator(Parent const* parent, size_t pos) noexcept :
        parent_{parent}
    ,   pos_{pos}
    ,   len_{parent->size()}
    { }

public:
    template<typename U,
             typename = std::enable_if_t<IsConst>>
    ByteBufferViewIterator(ByteBufferViewIterator<U, false> const& other)
        noexcept :
            parent_{other.parent_}
        ,   pos_{other.pos_}
        ,   len_{other.len_}
    { }

    ByteBufferViewIterator& operator++() noexcept {
        Ensure(pos_ < len_);
        pos_ += 1;
        return *this;
    }

    ByteBufferViewIterator operator++(int) noexcept {
        auto tmp = *this;
        ++(*this);
        return tmp;
    }

    auto operator*() const noexcept -> reference {
        Ensure(pos_ < len_);
        return *(data() + pos_);
    }

    auto operator->() const noexcept -> pointer {
        return &*(*this);
    }

    auto operator[](size_t n) const noexcept -> reference {
        return *(*this + n);
    }

    auto operator-(ByteBufferViewIterator const& other) const noexcept -> size_t {
        return (*this - other.pos_).pos_;
    }

    auto operator-(size_t n) const noexcept -> ByteBufferViewIterator {
        auto tmp = *this;
        tmp -= n;
        return tmp;
    }

    auto operator+(size_t n) const noexcept -> ByteBufferViewIterator {
        auto tmp = *this;
        tmp += n;
        return tmp;
    }

    friend auto operator+(size_t n, ByteBufferViewIterator const& it)
        noexcept -> ByteBufferViewIterator
    {
        return it + n;
    }

    auto operator+=(size_t n) noexcept -> ByteBufferViewIterator& {
        Ensure((len_ - pos_) >= n);
        pos_ += n;
        return *this;
    }

    auto operator-=(size_t n) noexcept -> ByteBufferViewIterator& {
        Ensure(static_cast<size_t>((data() + pos_) - data()) >= n);
        pos_ -= n;
        return *this;
    }

    friend auto operator<(ByteBufferViewIterator const& l,
                          ByteBufferViewIterator const& r)
        noexcept -> bool
    {
        Ensure(l.data() == r.data());
        Ensure(l.len_ == r.len_);
        return l.pos_ < r.pos_;
    }

    friend auto operator<=(ByteBufferViewIterator const& l,
                           ByteBufferViewIterator const& r)
        noexcept -> bool
    {
        return l < r || l == r;
    }

    friend auto operator>(ByteBufferViewIterator const& l,
                          ByteBufferViewIterator const& r)
        noexcept -> bool
    {
        return !(l <= r);
    }

    friend auto operator>=(ByteBufferViewIterator const& l,
                           ByteBufferViewIterator const& r)
        noexcept -> bool
    {
        return !(l < r);
    }

    friend auto operator==(ByteBufferViewIterator const& l,
                           ByteBufferViewIterator const& r)
        noexcept -> bool
    {
        Ensure(l.data() == r.data());
        Ensure(l.len_ == r.len_);
        return l.pos_ == r.pos_;
    }

    friend auto operator!=(ByteBufferViewIterator const& l,
                           ByteBufferViewIterator const& r)
        noexcept -> bool
    {
        return !(l == r);
    }

private:
    auto data() const noexcept -> pointer {
        return parent_->data();
    }

    C const* parent_;
    size_t pos_;
    size_t len_;
};

template<typename C>
struct ByteBufferView {

    template<typename _C>
    friend struct ByteBufferView;

    using Item = C;
    using Pointer = Item*;
    using iterator =
        std::conditional_t<std::is_const_v<Item>, 
                           ByteBufferViewIterator<ByteBufferView, true>,
                           ByteBufferViewIterator<ByteBufferView, false>>;
    using const_iterator =
        ByteBufferViewIterator<ByteBufferView, true>;
    using value_type = Item;

    static constexpr auto _no_narrow(uint8_t) -> void;

    template<
        typename Container,
        typename _IsContiguousBytes =
            std::enable_if_t<
                std::is_convertible_v<
                    typename std::iterator_traits<decltype(std::declval<Container>().begin())>::iterator_category*,
                    std::random_access_iterator_tag*>,
            decltype(_no_narrow({std::declval<std::remove_const_t<Item>>()}))>>
    ByteBufferView(Container&& c) :
        data_{&*c.begin()}
    { }

    template<typename _C,
        typename = std::enable_if_t<
            std::is_const_v<Item> &&
            !std::is_const_v<_C>>>
    ByteBufferView(ByteBufferView<_C> const& c) :
        ByteBufferView{c.container_}
    { }

    auto data() const -> Pointer {
        return data_;
    }

    auto size() const -> size_t {
        return len_;
    }

    auto begin() const -> iterator {
        return { this, 0 };
    }

    auto end() const -> iterator {
        return { this, size() };
    }

    auto cbegin() const -> const_iterator {
        return { this, 0 };
    }

    auto cend() const -> const_iterator {
        return { this, size() };
    }

private:
    Pointer data_;
    size_t len_;
};

auto const_conversion(ByteBufferView<uint8_t const>) {
    (void)0;
}

template<typename C>
auto const_conversion_iter(ByteBufferViewIterator<C, true>) {
    (void)0;
}

int main(int, char const**) {
    std::vector<unsigned char> v(10);
    ByteBufferView<uint8_t> view{v};
    std::cerr << "Size: " << view.size() << "\n";

    for (auto& i : view) {
        i = 42;
    }

    std::copy(std::begin(view), std::end(view),
              std::ostream_iterator<uint8_t>{std::cerr, "\n"});

    auto it = view.begin();
    ByteBufferViewIterator<decltype(view), true> it2 = it;
    const_conversion(view);
//    const_conversion_iter(it);

    return 0;
}
