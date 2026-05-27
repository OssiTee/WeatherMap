#pragma once

#include <stdexcept>
#include <string>
#include <utility>
#include <variant>

namespace shared {

    /**
     * @brief Separate type for errors so that std::variant does not confuse
     * the types even if T is std::string.
     */
    struct Error {
        std::string message;
    };

    template <typename T> class Result {
      public:
        // Success factories: overloads for lvalues and rvalues
        // (improves performance by avoiding unnecessary copies)
        static Result success(const T &value) { return Result(value); }

        static Result success(T &&value) { return Result(std::move(value)); }

        // Error factory: accepts a string message
        static Result error(std::string message) {
            return Result(Error{std::move(message)});
        }

        bool isSuccess() const { return std::holds_alternative<T>(m_data); }

        bool isError() const { return std::holds_alternative<Error>(m_data); }

        const T &value() const {
            if (!isSuccess()) {
                throw std::bad_variant_access();
            }
            return std::get<T>(m_data);
        }

        T &value() {
            if (!isSuccess()) {
                throw std::bad_variant_access();
            }
            return std::get<T>(m_data);
        }

        const std::string &errorMessage() const {
            if (!isError()) {
                static const std::string empty;
                return empty;
            }
            return std::get<Error>(m_data).message;
        }

      private:
        // Variant now contains T and Error, not two strings
        std::variant<T, Error> m_data;

        // Private constructors
        explicit Result(const T &value) : m_data(value) {}
        explicit Result(T &&value) : m_data(std::move(value)) {}
        explicit Result(Error error) : m_data(std::move(error)) {}
    };

} // namespace shared
