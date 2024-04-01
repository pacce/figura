#include <figura/figura.hpp>

figura::Width width     = 800;
figura::Height height   = 640;

template <typename T>
struct Fraction {
    figura::Height height;

    T operator()(const figura::Height& h) const {
        return static_cast<T>(h) / static_cast<T>(height);
    }
};

int
main(int, char**) {
    std::vector<figura::Color<float>> colors;
    colors.reserve(width * height);

    Fraction<float> fraction{height};

    for (figura::Height h = 0; h < height; h++) {
    for (figura::Width w = 0; w < width; w++) {
        colors.push_back(figura::color::RGB<float>(fraction(h), 1.0f - fraction(h), 0.0f));
    }
    }

    figura::write("main.png", colors, width, height);
    return 0;
}
