#include <iterator>
#include <type_traits>
#include <optional>
#include <utility>
#include <cstddef>

#include <iostream>
#include <functional>
		
namespace detail {

// Anything that inherits from View is supposed to be a lightweight object
// that is cheap to copy.
struct View {};

template < typename T >
struct ContainerView : public View {
    using value_type = typename T::value_type;
    using const_iterator = typename T::const_iterator;
    using difference_type = typename T::difference_type;
    using iterator = typename T::const_iterator;

    explicit ContainerView( const T& t ) : _t( t ) {}

    auto begin() const { return _t.begin(); }
    auto end() const { return _t.end(); }

private:
	// reference to container
    const T& _t;
};

template < typename RangeConstructor >
// RangeConstructor f - is just a function that takes auto input and returns something like map(input, functor)
struct RangeBuilder { RangeConstructor f; };

template < typename RangeConstructor >
auto makeRangeBuilder( RangeConstructor c ) {
    return RangeBuilder< RangeConstructor >{ c };
}

template < typename As, typename F >
struct Map : public View {

	using value_type = typename std::result_of_t< F( typename As::value_type ) >;
	using difference_type = typename As::difference_type;

	explicit Map(As inputView, F functor) : _inputView(std::move(inputView)), _functor(std::move(functor)) { }

	struct Iterator {
		using value_type = typename std::result_of_t< F( typename As::value_type ) >;
		using iterator_category = std::forward_iterator_tag;
		using difference_type = ptrdiff_t;
		using pointer = const value_type*;
		using reference = const value_type&;

		// ForwardIterator must be DefaultConstructible
		Iterator() = default;

		Iterator(typename As::iterator it, const F* functor) :  _it(std::move(it)), _functor(functor) { }

		bool operator==(const Map::Iterator& other) const {
			return _it == other._it && _functor == other._functor;
		}	

		bool operator!=(const Map::Iterator& other) const {
			return !(*this == other);
		}

		reference operator*() {
			if (!_value) {
				_value = (*_functor)(*_it);
			}
			return *_value;
		}

		Iterator& operator++() {
			++_it;
			_value.reset();
			return *this;
		}

		Iterator operator++(int) {
			Iterator tmp(*this); // copy
			++*this;
			return tmp;
		}

		pointer operator->() {
			return &(*(*this));
		}

	private:
		typename As::iterator _it;
		const F* _functor;
		std::optional<value_type> _value;
	};

	using iterator = Iterator;
	using const_iterator = Iterator;

	iterator begin() const {
		return Iterator(_inputView.begin(), &_functor);
	}

	iterator end() const {
		return Iterator(_inputView.end(), &_functor);
	}

private:
	const As _inputView;
	const F _functor;
};

template < typename As, typename F >
struct Filter : public View {

	using value_type = typename As::value_type;
	using difference_type = typename As::difference_type;

	explicit Filter(As inputView, F functor) : _inputView(std::move(inputView)), _functor(std::move(functor)) { }

	struct Iterator {
		using value_type = typename As::value_type;
		using iterator_category = std::forward_iterator_tag;
		using difference_type = ptrdiff_t;
		using pointer = const value_type*;
		using reference = const value_type&;

		// ForwardIterator must be DefaultConstructible
		Iterator() = default;

		Iterator(const F* functor, typename As::iterator it, typename As::iterator end)
				: _functor(functor), _it(std::move(it)), _end(std::move(end)) { }

		bool operator==(const Filter::Iterator& other) const {
			return _it == other._it && _functor == other._functor;
		}	

		bool operator!=(const Filter::Iterator& other) const {
			return !(*this == other);
		}

		reference operator*() {
			return *_it;
		}

		Iterator& operator++() {
			do {
				++_it;
			} while (_it != _end &&
					 !(*_functor)(*_it));

			return *this;
		}

		Iterator operator++(int) {
			Iterator tmp(*this); // copy
			++*this;
			return tmp;
		}

		pointer operator->() {
			return &(*(*this));
		}

	private:
		const F* _functor;
		typename As::iterator _it;
		typename As::iterator _end;
	};

	using iterator = Iterator;
	using const_iterator = Iterator;

	iterator begin() const {
		auto it = _inputView.begin();
		while(it != _inputView.end() && !(_functor)(*it)) {
			++it;
		}
		return Iterator(&_functor, it, _inputView.end());
	}

	iterator end() const {
		return Iterator(&_functor, _inputView.end(), _inputView.end());
	}

private:
	const As _inputView;
	const F _functor;
};

template < typename As, typename Bs, typename F >
struct ZipWith : public View {
	using value_type = typename std::result_of_t< F( typename As::value_type, typename Bs::value_type ) >;
	using difference_type = typename As::difference_type;

	explicit ZipWith(As iA, Bs iB, F functor) : _iA(std::move(iA)), _iB(std::move(iB)), _functor(std::move(functor)) { }

	struct Iterator {
		using value_type = typename std::result_of_t< F( typename As::value_type, typename Bs::value_type ) >;
		using iterator_category = std::forward_iterator_tag;
		using difference_type = ptrdiff_t;
		using pointer = const value_type*;
		using reference = const value_type&;

		// ForwardIterator must be DefaultConstructible
		Iterator() = default;

		Iterator(typename As::iterator itA, typename Bs::iterator itB, const F* functor, typename As::iterator endA, typename Bs::iterator endB)
			: _itA(std::move(itA)), _itB(std::move(itB)), _functor(functor), _endA(std::move(endA)), _endB(std::move(endB)) { }

		bool operator==(const ZipWith::Iterator& other) const {
			return _itA == other._itA && _itB == other._itB && _functor == other._functor;
		}

		bool operator!=(const ZipWith::Iterator& other) const {
			return !(*this == other);
		}

		reference operator*() { 
			if(!_value) {
				_value = (*_functor)(*_itA, *_itB);
			}
			return *_value;
		}

		Iterator& operator++() {
			++_itA;
			++_itB;

			if (_itA == _endA || _itB == _endB) {
				_itA = _endA;
				_itB = _endB;
			}

			_value.reset();
			return *this;
		}

		Iterator operator++(int) {
			Iterator tmp(*this); // copy
			++*this;
			return tmp;
		}
		
		pointer operator->() {
			return &(*(*this));
		}

	private:
		typename As::iterator _itA;
		typename Bs::iterator _itB;
		const F* _functor;
		typename As::iterator _endA;
		typename Bs::iterator _endB;
		std::optional<value_type> _value;
	};

	using iterator = Iterator;
	using const_iterator = Iterator;

	iterator begin() const {
		if (_iA.begin() == _iA.end() || _iB.begin() == _iB.end()) {
			return Iterator(_iA.end(), _iB.end(), &_functor, _iA.end(), _iB.end());
		}
		return Iterator(_iA.begin(), _iB.begin(), &_functor, _iA.end(), _iB.end());
	}

	iterator end() const {
		return Iterator(_iA.end(), _iB.end(), &_functor, _iA.end(), _iB.end());
	}

private:
	const As _iA;
	const Bs _iB;
	const F _functor;
};

template < typename Integer >
struct Range : public View {
	using value_type = Integer;
	using difference_type = std::ptrdiff_t;

	explicit Range(Integer from, Integer to, Integer step) : _from(std::move(from)), _to(std::move(to)), _step(std::move(step)) { }

	struct Iterator {
		using value_type = Integer;
		using iterator_category = std::forward_iterator_tag;
		using difference_type = ptrdiff_t;
		using pointer = const value_type*;
		using reference = const value_type&;

		// ForwardIterator must be DefaultConstructible
		Iterator() = default;

		Iterator(Integer from, Integer to, Integer step) : _from(std::move(from)), _to(std::move(to)), _step(std::move(step)) { }

		bool operator==(const Range::Iterator& other) const {
			return _from == other._from && _to == other._to && _step == other._step;
		}	

		bool operator!=(const Range::Iterator& other) const {
			return !(*this == other);
		}

		reference operator*() {
			return _from;
		}

		Iterator& operator++() {
			if (_step > 0) {
				if (_to - _step > _from) {
					_from += _step;
				} else {
					_from = _to;
				}
			} else {
				if (_to - _step < _from) {
					_from += _step;
				} else {
					_from = _to;
				}
			}

			return *this;
		}

		Iterator operator++(int) {
			Iterator tmp(*this); // copy
			++*this;
			return tmp;
		}

		pointer operator->() {
			return &(*(*this));
		}

	private:
		Integer _from;
		Integer _to;
		Integer _step;
	};

	using iterator = Iterator;
	using const_iterator = Iterator;

	iterator begin() const {
		if ((_step < 0 && _from < _to) || (_step > 0 && _from > _to)) {
			return Iterator(_to, _to, _step);
		}
		return Iterator(_from, _to, _step);
	}

	iterator end() const {
		return Iterator(_to, _to, _step);
	}

private:
	Integer _from;
	Integer _to;
	Integer _step;
};

template < typename Integer >
struct InfiniteSequence : public View {
	using value_type = Integer;
	using difference_type = std::ptrdiff_t;

	explicit InfiniteSequence(Integer from, Integer step) : _from(std::move(from)), _step(std::move(step)) { }

	struct Iterator {
		using value_type = Integer;
		using iterator_category = std::forward_iterator_tag;
		using difference_type = ptrdiff_t;
		using pointer = const value_type*;
		using reference = const value_type&;

		// ForwardIterator must be DefaultConstructible
		Iterator() = default;

		Iterator(Integer from, Integer step, bool inf) : _from(std::move(from)), _step(std::move(step)), _inf(inf) { }

		bool operator==(const InfiniteSequence< value_type >::Iterator& other) const {
			return _from == other._from && _step == other._step && _inf == other._inf;
		}

		bool operator!=(const InfiniteSequence::Iterator& other) const {
			return !(*this == other);
		}

		reference operator*() {
			return _from;
		}

		Iterator& operator++() {
			_from += _step;
			return *this;
		}

		Iterator operator++(int) {
			Iterator tmp(*this); // copy
			++*this;
			return tmp;
		}

		pointer operator->() {
			return &(*(*this));
		}

	private:
		Integer _from;
		Integer _step;
		bool _inf;
	};

	using iterator = Iterator;
	using const_iterator = Iterator;

	iterator begin() const {
		return Iterator(_from, _step, false);
	}

	iterator end() const {
		return Iterator(_from, _step, true);
	}

private:
	Integer _from;
	Integer _step;
};

template < typename As  >
struct Take : public View {
	using value_type = typename As::value_type;
	using difference_type = typename As::difference_type;

	explicit Take(As inputView, size_t n) : _inputView(std::move(inputView)), _n(n) { }

	struct Iterator {
		using value_type = typename As::value_type;
		using iterator_category = std::forward_iterator_tag;
		using difference_type = ptrdiff_t;
		using pointer = const value_type*;
		using reference = const value_type&;

		// ForwardIterator must be DefaultConstructible
		Iterator() = default;

		Iterator(typename As::iterator it, size_t n, typename As::iterator end) : _it(std::move(it)), _n(n), _end(std::move(end)) { }

		bool operator==(const Take::Iterator& other) const {
			return _it == other._it && _n == other._n;
		}	

		bool operator!=(const Take::Iterator& other) const {
			return !(*this == other);
		}

		reference operator*() {
			return *_it;
		}

		Iterator& operator++() {
			++_it;
			--_n;
			if (_n == 0) {
				_it = _end;
			}
			if (_it == _end) {
				_n = 0;
			}
			return *this;
		}

		Iterator operator++(int) {
			Iterator tmp(*this); // copy
			++*this;
			return tmp;
		}

		pointer operator->() {
			return &(*(*this));
		}

	private:
		typename As::iterator _it;
		size_t _n;
		typename As::iterator _end;
	};

	using iterator = Iterator;
	using const_iterator = Iterator;

	iterator begin() const {
		return Iterator(_inputView.begin(), _n, _inputView.end());
	}

	iterator end() const {
		if (_n == 0) {
			return Iterator(_inputView.begin(), 0, _inputView.end());
		}

		if (_inputView.begin() == _inputView.end()) {
			return Iterator(_inputView.end(), _n, _inputView.end());
		}

		return Iterator(_inputView.end(), 0, _inputView.end());
	}

private:
	const As _inputView;
	const size_t _n;
};

} // namespace detail

template < typename T, typename = std::enable_if_t< std::is_base_of_v< detail::View, T > > >
T view( T t ) { return t; }

template < typename T, typename = std::enable_if_t< !std::is_base_of_v< detail::View, T > > >
auto view( const T& t ) {
    return detail::ContainerView< T >{ t };
}

template < typename V, typename Constructor >
auto operator|( const V& left, detail::RangeBuilder< Constructor > builder ) {
    return builder.f( view( left ) );
}

template < typename As, typename F >
auto map( const As& input, F f ) {
    return detail::Map{ view( input ), f };
}

template < typename F >
auto map( F f ) {
    return detail::makeRangeBuilder( [=]( auto input ){
        return map( input, f );
    } );
}

template < typename As, typename F >
auto filter( const As& input, F f ) {
    return detail::Filter{ view( input ), f };
}

template < typename F >
auto filter( F f ) {
    return detail::makeRangeBuilder( [=]( auto input ){
        return filter( input, f );
    } );
}

template < typename As, typename Bs, typename F >
auto zipWith( const As& iA, const Bs& iB, F f ) {
    return detail::ZipWith{ view( iA ), view( iB ),  f };
}

template < typename As, typename Bs >
auto zip( const As& iA, const Bs& iB ) {
    return detail::ZipWith{ view( iA ), view( iB ), 
		[]( typename As::value_type vA, typename Bs::value_type vB){ return std::pair {vA, vB}; } };
}

template < typename Integer >
auto range( Integer from, Integer to, Integer step = 1 ) {
    return detail::Range{ from, to, step };
}

template < typename Integer >
auto range( Integer to ) {
    return detail::Range{static_cast<Integer>(0), to, static_cast<Integer>(1) };
}

template < typename Integer >
auto infiniteSequence( Integer from, Integer step = 1 ) {
	return detail::InfiniteSequence{ from, step };
}

template < typename As >
auto enumerate( const As& a ) {
	return zip(infiniteSequence(static_cast<size_t >(0)), a);
}

inline auto enumerate() {
	return detail::makeRangeBuilder( []( auto a ) {
		return zip(infiniteSequence(static_cast<size_t >(0)), a);
	} );
}

template < typename As >
auto take( const As& in, size_t n ) {
	return detail::Take{ view( in ), n };
}

inline auto take( size_t n ) {
	return detail::makeRangeBuilder( [=]( auto input ){ 
		return detail::Take{ input, n };
	} );
}
