module;

#include <algorithm>
#include <vector>
#include <cstddef>

export module helios.core.common.DynamicBitSet;


export namespace helios::core::common {
    class DynamicBitSet {

        std::vector<std::uint64_t> pages_;

        std::size_t count_ = 0;

        [[nodiscard]] inline std::size_t toPage(const std::size_t idx) const noexcept {
            return idx / 64;
        }

        [[nodiscard]] inline std::uint8_t toBit(const std::size_t idx) const noexcept {
            return (idx % 64);
        }

    public:

        void set(const std::size_t idx) {

            if (get(idx)) {
                return;
            }

            const auto page = toPage(idx);
            const auto bit = toBit(idx);

            if (page >= pages_.size()) {
                pages_.resize(page + 1);
            }

            count_++;
            pages_[page] |= (std::uint64_t{1} << bit);
        }

        void clear(const std::size_t idx) noexcept {
            if (!get(idx)) {
                return;
            }
            const auto page = toPage(idx);

            if (page >= pages_.size()) {
                return;
            }

            const auto bit = toBit(idx);
            count_--;
            pages_[page] &= ~(std::uint64_t{1} << bit);
        }

        [[nodiscard]] bool get(const std::size_t idx) const noexcept {
            const auto page = toPage(idx);

            if (pages_.size() <= page) {
                return false;
            }

            const auto bit = toBit(idx);
            return pages_[page] & (std::uint64_t{1} << bit);
        }

        void clear() noexcept {
            std::ranges::fill(pages_, std::uint64_t{0}  );
            count_ = 0;
        }

        [[nodiscard]] std::size_t count() const noexcept {
            return count_;
        }

        void reserve(const std::size_t capacity) noexcept {
            pages_.reserve(capacity);
        }
    };


}