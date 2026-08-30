/**
 * @file ConceptModelRegistry.ixx
 * @brief Generic type-indexed registry for type-erased wrappers.
 */
module;

#include <cassert>
#include <exception>
#include <memory>
#include <span>
#include <utility>
#include <vector>

export module helios.core.common.container:ConceptModelRegistry;

export namespace helios::core::common::container {

/**
 * @brief Type-indexed registry for type-erased wrapper instances.
 * @tparam TWrapperType Wrapper type exposing `underlying()`.
 * @tparam IdProvider Type-id provider exposing `id<T>().value()`.
 */
template <typename TWrapperType, typename IdProvider>
class ConceptModelRegistry {

    mutable std::vector<std::unique_ptr<TWrapperType>> items_;
    std::vector<void*> underlyingAnyT_;
    std::vector<size_t> insertionOrder_;
    mutable std::vector<TWrapperType*> itemView_;
    mutable bool needsUpdate_ = false;

    void update() const {

        if (!needsUpdate_) {
            return;
        }

        itemView_.clear();
        itemView_.reserve(insertionOrder_.size());

        for (const auto insertionIndex : insertionOrder_) {
            itemView_.push_back(items_[insertionIndex].get());
        }

        needsUpdate_ = false;
    }

public:
    ConceptModelRegistry() = default;
    ConceptModelRegistry(const ConceptModelRegistry&) = delete;
    ConceptModelRegistry& operator=(const ConceptModelRegistry&) = delete;

    ConceptModelRegistry(ConceptModelRegistry&&) noexcept = default;
    ConceptModelRegistry& operator=(ConceptModelRegistry&&) noexcept = default;

    /**
     * @brief Returns a read-only span of registered items in insertion order.
     *
     * @return Span of const TWrapperType pointers.
     */
    [[nodiscard]] std::span<TWrapperType* const> items() const noexcept {
        update();
        return itemView_;
    }

    /**
     * @brief Returns a mutable span of registered items in insertion order.
     *
     * @return Span of TWrapperType pointers.
     */
    [[nodiscard]] std::span<TWrapperType*> items() noexcept {
        update();
        return itemView_;
    }

    /**
     * @brief Registers a concrete instance and returns the underlying object.
     * @tparam TConcreteType Concrete type to register.
     * @tparam Args Constructor argument types.
     * @param args Constructor arguments forwarded to `TConcreteType`.
     */
    template <typename TConcreteType, typename... Args>
    TConcreteType& add(Args&&... args) {

        if (has<TConcreteType>()) [[unlikely]] {
            assert(false && "TWrapperType already registered.");
            std::terminate();
        }

        TWrapperType wrapper{TConcreteType{std::forward<Args>(args)...}};

        const auto idx = IdProvider::template id<TConcreteType>().value();

        if (items_.size() <= idx) {
            items_.resize(idx + 1);
        }
        if (underlyingAnyT_.size() <= idx) {
            underlyingAnyT_.resize(idx + 1);
        }

        items_[idx] = std::make_unique<TWrapperType>(std::move(wrapper));

        void* rawUnderlying = items_[idx]->underlying();
        underlyingAnyT_[idx] = rawUnderlying;

        insertionOrder_.push_back(idx);

        needsUpdate_ = true;
        return *static_cast<TConcreteType*>(rawUnderlying);
    }

    /**
     * @brief Registers a pre-built wrapper and returns the underlying object.
     * @tparam TConcreteType Concrete type represented by `wrapper`.
     * @param wrapper Pre-built wrapper instance.
     */
    template <typename TConcreteType>
    TConcreteType& add(TWrapperType&& wrapper) {
        const auto idx = IdProvider::template id<TConcreteType>().value();

        if (items_.size() <= idx) {
            items_.resize(idx + 1);
        }
        if (underlyingAnyT_.size() <= idx) {
            underlyingAnyT_.resize(idx + 1);
        }

        assert(!has<TConcreteType>() && "TWrapperType already registered.");

        items_[idx] = std::make_unique<TWrapperType>(std::move(wrapper));

        void* rawUnderlying = items_[idx]->underlying();
        underlyingAnyT_[idx] = rawUnderlying;

        insertionOrder_.push_back(idx);
        needsUpdate_ = true;

        return *static_cast<TConcreteType*>(rawUnderlying);
    }

    /**
     * @brief Checks whether a concrete type `TConcreteType` is registered.
     *
     * @tparam TConcreteType The type to check.
     *
     * @return True if `TConcreteType` has been added.
     */
    template <typename TConcreteType>
    [[nodiscard]] bool has() const {
        return item<TConcreteType>() != nullptr;
    }

    /**
     * @brief Returns a pointer to the registered instance of type `TConcreteType`.
     *
     * @tparam TConcreteType The concrete type to look up.
     *
     * @return Pointer to const `TConcreteType`, or nullptr if not registered.
     */
    template <typename TConcreteType>
    [[nodiscard]] const TConcreteType* item() const {

        const auto idx = IdProvider::template id<TConcreteType>().value();
        if (items_.size() <= idx || underlyingAnyT_.size() <= idx || !underlyingAnyT_[idx]) {
            return nullptr;
        }

        return static_cast<const TConcreteType*>(underlyingAnyT_[idx]);
    }

    /**
     * @brief Returns a pointer to the registered instance of type `TConcreteType`.
     *
     * @tparam TConcreteType The concrete type to look up.
     *
     * @return Pointer to `TConcreteType`, or nullptr if not registered.
     */
    template <typename TConcreteType>
    [[nodiscard]] TConcreteType* item() {
        return const_cast<TConcreteType*>(std::as_const(*this).template item<TConcreteType>());
    }

    /**
     * @brief Returns the item registered with the given type-id value, if any.
     *
     * @param typeId Type id used to look up the stored wrapper.
     *
     * @return A pointer to the item identified by typeId, or nullptr if not found.
     */
    [[nodiscard]] TWrapperType* item(IdProvider typeId) noexcept {
        const auto idx = typeId.value();
        return items_.size() > idx && underlyingAnyT_.size() > idx && underlyingAnyT_[idx] ? items_[idx].get()
                                                                                           : nullptr;
    }
};

} // namespace helios::core::common::container
