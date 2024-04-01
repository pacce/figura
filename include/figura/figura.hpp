#ifndef FIGURA_HPP__
#define FIGURA_HPP__

#include <png.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <ostream>
#include <variant>
#include <vector>

namespace figura {
    using Pixel     = uint8_t;
    using Image     = std::vector<Pixel>;
    using Height    = std::size_t;
    using Width     = std::size_t;

namespace color {
    template <typename T>
    class RGB {
        static_assert(std::is_floating_point<T>::value);
        public:
             RGB() : RGB(0.0, 0.0, 0.0) {}
             RGB(T r, T g, T b) 
                : r_(std::clamp<T>(r, 0.0, 1.0))
                , g_(std::clamp<T>(g, 0.0, 1.0))
                , b_(std::clamp<T>(b, 0.0, 1.0))
            {}

            friend std::ostream&
            operator<<(std::ostream& os, const RGB& color) {
                os << color.r_ << ", " << color.g_ << ", " << color.b_;
                return os;
            }

            std::uint8_t r() const { return static_cast<std::uint8_t>(r_ * 255.0f); }
            std::uint8_t g() const { return static_cast<std::uint8_t>(g_ * 255.0f); }
            std::uint8_t b() const { return static_cast<std::uint8_t>(b_ * 255.0f); }
            std::uint8_t a() const { return 255; }

            RGB<T>&
            operator*=(const T& scale) {
                r_ = std::clamp<T>(r_ * scale, 0.0, 1.0);
                g_ = std::clamp<T>(g_ * scale, 0.0, 1.0);
                b_ = std::clamp<T>(b_ * scale, 0.0, 1.0);

                return *this;
            }

            friend RGB<T>
            operator*(const RGB<T>& c, const T& scale) {
                RGB<T> xs = c;
                xs *= scale;

                return xs;
            }

            friend RGB<T>
            operator*(const T& scale, const RGB<T>& c) {
                RGB<T> xs = c;
                xs *= scale;

                return xs;
            }

            RGB<T>&
            operator*=(const RGB<T>& rhs) {
                r_ = std::clamp<T>(r_ * rhs.r_, 0.0, 1.0);
                g_ = std::clamp<T>(g_ * rhs.g_, 0.0, 1.0);
                b_ = std::clamp<T>(b_ * rhs.b_, 0.0, 1.0);

                return *this;
            }

            friend RGB<T>
            operator*(const RGB<T>& lhs, const RGB<T>& rhs) {
                RGB<T> xs = lhs;
                xs *= rhs;

                return xs;
            }

            RGB<T>&
            operator+=(const RGB<T>& rhs) {
                r_ = std::clamp<T>(r_ + rhs.r_, 0.0, 1.0);
                g_ = std::clamp<T>(g_ + rhs.g_, 0.0, 1.0);
                b_ = std::clamp<T>(b_ + rhs.b_, 0.0, 1.0);

                return *this;
            }

            friend RGB<T>
            operator+(const RGB<T>& lhs, const RGB<T>& rhs) {
                RGB<T> xs = lhs;
                xs += rhs;

                return xs;
            }

            RGB<T>&
            gamma() {
                r_ = std::sqrt(r_);
                g_ = std::sqrt(g_);
                b_ = std::sqrt(b_);

                return *this;
            }

            static RGB<T> white() { return RGB<T>(1.0, 1.0, 1.0); }
            static RGB<T> red()   { return RGB<T>(1.0, 0.0, 0.0); }
            static RGB<T> green() { return RGB<T>(0.0, 1.0, 0.0); }
            static RGB<T> blue()  { return RGB<T>(0.0, 0.0, 1.0); }
            static RGB<T> black() { return RGB<T>(0.0, 0.0, 0.0); }
        private:
            T r_;
            T g_;
            T b_;
    };
namespace visitor {
    template <typename T>
    struct red {
        std::uint8_t operator()(const RGB<T>& v) const    { return v.r(); }
    };

    template <typename T>
    struct green {
        std::uint8_t operator()(const RGB<T>& v) const    { return v.b(); }
    };

    template <typename T>
    struct blue {
        std::uint8_t operator()(const RGB<T>& v) const    { return v.g(); }
    };

    template <typename T>
    struct alpha {
        std::uint8_t operator()(const RGB<T>& v) const    { return v.a(); }
    };
} // namespace visitor
} // namespace color
    template <typename T>
    using Color = std::variant<color::RGB<T>>;

    template <typename T>
    void
    write(const std::filesystem::path& path, const std::vector<Color<T>>& colors, Width ws, Height hs) {
        std::FILE * handle = std::fopen(path.c_str(), "wb");
        if (handle == nullptr) {
            throw std::runtime_error("could not open file");
        }

        png_structp png = png_create_write_struct(
                PNG_LIBPNG_VER_STRING
                , NULL
                , NULL
                , NULL
                );
        if (png == nullptr) {
            throw std::runtime_error("png_create_write_struct error");
        }

        png_infop info = png_create_info_struct(png);
        if (info == nullptr) {
            throw std::runtime_error("png_create_info_struct");
        }
        png_init_io(png, handle);
        png_set_IHDR(
                png
                , info
                , static_cast<png_uint_32>(ws)
                , static_cast<png_uint_32>(hs)
                , 8
                , PNG_COLOR_TYPE_RGBA
                , PNG_INTERLACE_NONE
                , PNG_COMPRESSION_TYPE_DEFAULT
                , PNG_FILTER_TYPE_DEFAULT
                );
        png_write_info(png, info);

        png_bytep * rows    = new png_bytep[hs];
        png_uint_32 bytes   = png_get_rowbytes(png, info);

        for (std::size_t h = 0; h < hs; h++) {
            png_byte * row  = new png_byte[bytes];
            rows[h]         = row;
            for (std::size_t w = 0; w < ws; w++) {
                std::size_t index = ((h * ws) + w);

                *row++  = std::visit(  color::visitor::red<T>{}, colors[index]);
                *row++  = std::visit(color::visitor::green<T>{}, colors[index]);
                *row++  = std::visit( color::visitor::blue<T>{}, colors[index]);
                *row++  = std::visit(color::visitor::alpha<T>{}, colors[index]);
            }
        }

        png_write_image(png, rows);
        png_write_end(png, NULL);

        for (std::size_t h = 0; h < hs; h++) {
            delete[] rows[h];
        }
        delete[] rows;

        png_destroy_write_struct(&png, &info);
        std::fclose(handle);
    }
} // namespace figura

#endif // FIGURA_HPP__
