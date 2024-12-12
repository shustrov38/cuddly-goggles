#pragma once

namespace geometry {
class Point {
public:
    double x;
    double y;

    Point()
    {
        Init();
    }

    Point(double x, double y)
        : x(x)
        , y(y)
    {
        Init();
    }
    
    size_t GetId() const
    {
        return mId;
    }

    template <size_t I>
    friend std::tuple_element_t<I, Point> &get(Point &p)
    {
        if constexpr (I == 0) {
            return p.x;
        }
        if constexpr (I == 1) {
            return p.y;
        }
    }

private:
    void Init()
    {
        static size_t globalId = 0;
        mId = globalId++;
    }

    size_t mId;
};
} // namespace geometry

namespace std {
    template <>
    struct tuple_size<geometry::Point> : std::integral_constant<std::size_t, 2> {};

    template <size_t I>
    struct tuple_element<I, geometry::Point> : tuple_element<I, tuple<double, double>> {
        static_assert(I < 2, "Index out of bounds for Person");
    };
} // namespace std