/**
 * @file ConceptModelRegistry.ixx
 * @brief Generic type-indexed registry for type-erased wrappers.
 */
module;

#include <cassert>
#include <memory>
#include <span>
#include <vector>
#include <utility>


export module helios.core.container.ConceptModelRegistry;


export namespace helios::core::container {

    /**
     * @brief Generic type-indexed registry for type-erased wrapper instances.
     *
     * @details ConceptModelRegistry stores instances of a type-erased wrapper
     * `TWrapperType` (e.g. System, Manager) and indexes them by a compile-time
     * `IdProvider` (e.g. SystemTypeId, ResourceTypeId). This provides:
     *
     * - **O(1) lookup** by concrete type via `item<TConcreteType>()`
     * - **Insertion-order iteration** via `items()` for deterministic processing
     * - **Type-safe registration** via `add<TConcreteType>(args...)` or `add<TConcreteType>(TWrapperType&&)`
     *   returning a reference to the underlying concrete instance
     *
     * The registry is used as the backend for `SystemRegistry` and
     * `ManagerRegistry`.
     *
     * ## Instantiations
     *
     * | Alias | TWrapperType | IdProvider |
     * |-------|------|------------|
     * | `SystemRegistry` | `System` | `SystemTypeId` |
     * | `ManagerRegistry` | `Manager` | `ResourceTypeId` |
     *
     * @tparam TWrapperType The type-erased wrapper type (must support `underlying()`).
     * @tparam IdProvider Compile-time ID generator (must provide `id<TConcreteType>().value()`).
     *
     * @see SystemRegistry
     * @see ManagerRegistry
     *
     * @todo this impl needs to make sure that memory allocations / moves do not invalidate
     * the fastAccess in ResourceRegistry - we are using vector with fixed pointers for now to make sure
     * reallocation does not invalidate addresses
     */
    template<typename TWrapperType, typename IdProvider>
    class ConceptModelRegistry {

        /**
         * @brief Owning storage for wrapped TWrapperType instances, indexed by type ID.
         */
        mutable std::vector<std::unique_ptr<TWrapperType>> items_;

        /**
         * @brief Cached raw pointers to the underlying concrete instances.
         */
        std::vector<void*> underlyingAnyT_;

        /**
         * @brief Records insertion order for deterministic iteration.
         */
        std::vector<size_t> insertionOrder_;

        /**
         * @brief Lazily-built view of TWrapperType pointers in insertion order.
         */
        mutable std::vector<TWrapperType*> itemView_;

        /**
         * @brief Dirty flag indicating itemView_ needs rebuilding.
         */
        mutable bool needsUpdate_ = false;

        /**
         * @brief Rebuilds the insertion-ordered view if dirty.
         */
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
         * @brief Registers and wraps a concrete instance of type TConcreteType.
         *
         * @details Constructs TConcreteType in-place from the forwarded arguments, wraps it
         * in an TWrapperType, and stores it at the index determined by `IdProvider::id<TConcreteType>()`.
         * Returns a reference to the underlying concrete TConcreteType instance.
         *
         * @tparam TConcreteType The concrete type to register.
         * @tparam Args Constructor argument types for TConcreteType.
         *
         * @param args Arguments forwarded to the TConcreteType constructor.
         *
         * @return Reference to the registered TConcreteType instance.
         *
         * @pre TConcreteType must not already be registered.
         */
        template<typename TConcreteType, typename... Args>
        TConcreteType& add(Args&&... args) {

            assert(!has<TConcreteType>() && "TWrapperType already registered.");

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
         * @brief Registers a pre-built wrapper instance for concrete type TConcreteType.
         *
         * @details Stores an already-constructed `TWrapperType` at the slot determined
         * by `IdProvider::id<TConcreteType>()` and returns the underlying `TConcreteType` reference.
         *
         * This overload is useful when wrapper construction needs custom
         * arguments that are not expressed as `TConcreteType{args...}` (for example,
         * pre-wrapped systems with injected dependencies).
         *
         * @tparam TConcreteType The concrete type represented by `wrapper`.
         *
         * @param wrapper Pre-built type-erased wrapper owning a `TConcreteType` instance.
         *
         * @return Reference to the registered TConcreteType instance.
         *
         * @pre `wrapper` must contain a concrete `TConcreteType` instance.
         * @pre TConcreteType should not already be registered.
         */
        template<typename TConcreteType>
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
         * @brief Checks whether a concrete type TConcreteType is registered.
         *
         * @tparam TConcreteType The type to check.
         *
         * @return True if TConcreteType has been added.
         */
        template<typename TConcreteType>
        [[nodiscard]] bool has() const {
            return item<TConcreteType>() != nullptr;
        }

        /**
         * @brief Returns a pointer to the registered instance of type TConcreteType.
         *
         * @tparam TConcreteType The concrete type to look up.
         *
         * @return Const Pointer to TConcreteType, or nullptr if not registered.
         */
        template<typename TConcreteType>
        [[nodiscard]] const TConcreteType* item() const {

            const auto idx = IdProvider::template id<TConcreteType>().value();
            if (items_.size() <= idx || underlyingAnyT_.size() <= idx || !underlyingAnyT_[idx]) {
                return nullptr;
            }

            return static_cast<const TConcreteType*>(underlyingAnyT_[idx]);
        }

        /**
         * @brief Returns a pointer to the registered instance of type TConcreteType.
         *
         * @tparam TConcreteType The concrete type to look up.
         *
         * @return Pointer to TConcreteType, or nullptr if not registered.
         */
        template<typename TConcreteType>
        [[nodiscard]] TConcreteType* item()  {
            return const_cast<TConcreteType*>(std::as_const(*this).template item<TConcreteType>());
        }

        /**
         * @brief Returns the item registered with the typeId-value, if any.
         *
         * @param typeId The typeId of the component that should be returned.
         *
         * @return A pointer to the item identified by typeId, or nullptr if not found.
         */
        [[nodiscard]] TWrapperType* item(IdProvider typeId) noexcept {
            const auto idx = typeId.value();
            return items_.size() > idx && underlyingAnyT_.size() > idx && underlyingAnyT_[idx] ? items_[idx].get() : nullptr;
        }

    };


}

