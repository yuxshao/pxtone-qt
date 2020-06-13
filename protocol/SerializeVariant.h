#ifndef SERIALIZEVARIANT_H
#define SERIALIZEVARIANT_H

#include <QDataStream>
#include <variant>
namespace detail {
template <std::size_t N>
struct variant_switch {
  template <typename Variant>
  inline void operator()(int index, QDataStream &in, Variant &v) const {
    if (index == N) {
      std::variant_alternative_t<N, Variant> a;
      operator>>(in, a);
      v.template emplace<N>(a);
    } else
      variant_switch<N - 1>{}(index, in, v);
  }
};

template <>
struct variant_switch<0> {
  template <typename Variant>
  inline void operator()(int index, QDataStream &in, Variant &v) const {
    if (index == 0) {
      std::variant_alternative_t<0, Variant> a;
      in >> a;
      v.template emplace<0>(a);
    } else
      throw "invalid variant index";
  }
};
}  // namespace detail

template <typename Variant>
QDataStream &deserializeVariant(QDataStream &in, Variant &a) {
  size_t index;
  in >> *(qint64 *)(&index);
  ::detail::variant_switch<size_t(std::variant_size_v<Variant> - 1)>{}(index,
                                                                       in, a);
  return in;
}

template <typename Variant>
QDataStream &serializeVariant(QDataStream &out, const Variant &a) {
  out << qint64(a.index());
  std::visit([&out](auto &&v) { out << v; }, a);
  return out;
}

#endif  // SERIALIZEVARIANT_H
