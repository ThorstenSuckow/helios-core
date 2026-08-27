/**
 * @file ConceptModelInstanceCollectionRegistry.ixx
 * @brief Generic type-indexed registry for type-erased wrappers.
 */
module;

#include <memory>
#include <concepts>
#include <span>
#include <vector>
#include <utility>


export module helios.core.common.container:ConceptModelInstanceCollectionRegistry;

import helios.core.common.container.types;

export namespace helios::core::common::container {

    /**
     * @brief Stores type-erased wrapper instances grouped by underlying type.
     *
     * @tparam TWrapper Wrapper type that owns one underlying model instance.
     * @tparam TTypeIdProvider Type-id provider exposing `id<T>()` and `value()`.
     */
    template<typename TWrapper, typename TTypeIdProvider>
    class ConceptModelInstanceCollectionRegistry {

        /**
         * @brief Key type for addressing a stored wrapper by type-id and index.
         */
        using ConceptModelCollectionKey = helios::core::common::container::types::ConceptModelCollectionKey<TTypeIdProvider>;

        /**
         * @brief Dense per-type storage of wrapper instances.
         */
        mutable std::vector<std::vector<std::unique_ptr<TWrapper>>> items_;

        /**
         * @brief Type-id indices in first-insertion order for stable collection iteration.
         */
        std::vector<size_t> insertionOrder_;

        /**
         * @brief Maps type-id index to its position inside `insertionOrder_`.
         */
        std::vector<std::size_t> typeIdToInsertionOrder_;

        /**
         * @brief Mutable pointer view for non-const collection access.
         */
        mutable std::vector<std::vector<TWrapper*>> itemView_;

        /**
         * @brief Spans over mutable pointer collections in insertion order.
         */
        mutable std::vector<std::span<TWrapper*>> collectionView_;

        /**
         * @brief Immutable pointer view for const collection access.
         */
        mutable std::vector<std::vector<const TWrapper*>> constItemView_;

        /**
         * @brief Spans over const pointer collections in insertion order.
         */
        mutable std::vector<std::span<const TWrapper*>> constCollectionView_;

        /**
         * @brief Indicates whether cached pointer views must be rebuilt.
         */
        mutable bool needsUpdate_ = false;


        /**
         * @brief Rebuilds cached collection views when storage changed.
         */
        void update() const noexcept {

            if (!needsUpdate_) {
                return;
            }

            itemView_.clear();
            collectionView_.clear();
            constItemView_.clear();
            constCollectionView_.clear();

            itemView_.reserve(insertionOrder_.size());
            constItemView_.reserve(insertionOrder_.size());

            for (const auto insertionIndex : insertionOrder_) {

                auto& collection = itemView_.emplace_back();
                auto& constCollection = constItemView_.emplace_back();

                collection.reserve(items_[insertionIndex].size());
                constCollection.reserve(items_[insertionIndex].size());

                for (const auto& item : items_[insertionIndex]) {
                    collection.push_back(item.get());
                    constCollection.push_back(item.get());
                }
            }


            collectionView_.reserve(itemView_.size());
            constCollectionView_.reserve(constItemView_.size());

            for (std::size_t i = 0; i < itemView_.size(); ++i) {
                auto& collection = itemView_[i];
                auto& constCollection = constItemView_[i];

                collectionView_.emplace_back(collection);
                constCollectionView_.emplace_back(constCollection);
            }

            needsUpdate_ = false;
        }
        
        
        void updateIndices(TTypeIdProvider typeId) {
            const auto typeIdIndex = typeId.value();

            if (items_.size() <= typeIdIndex) {
                items_.resize(typeIdIndex + 1);
            }
            if (typeIdToInsertionOrder_.size() <= typeIdIndex) {
                typeIdToInsertionOrder_.resize(typeIdIndex + 1);
            }

            if (items_[typeIdIndex].empty()) {
                insertionOrder_.push_back(typeIdIndex);
                typeIdToInsertionOrder_[typeIdIndex] = insertionOrder_.size() - 1;
            }
            
        }

        
    public:

        /**
         * @brief Returns all const collections grouped by insertion order.
         *
         * @return Span over spans of const wrapper pointers.
         */
        [[nodiscard]] std::span<const std::span<const TWrapper*>> collections() const noexcept {
            update();
            return constCollectionView_;
        }


        /**
         * @brief Returns all mutable collections grouped by insertion order.
         *
         * @return Span over spans of mutable wrapper pointers.
         */
        [[nodiscard]] std::span<std::span<TWrapper*>> collections() noexcept {
            update();
            return collectionView_;
        }


        /**
         * @brief Adds a new wrapped model instance and returns its key.
         *
         * @tparam TUnterlying Underlying model type to wrap.
         * @tparam Args Constructor argument pack for `TUnterlying`.
         * @param args Arguments forwarded to the model constructor.
         * @return Key identifying the inserted wrapper instance.
         */
        template<typename TUnterlying, typename... Args>
        ConceptModelCollectionKey add(Args&&... args) {
            
            TWrapper wrapper{TUnterlying{std::forward<Args>(args)...}};

            const auto typeId = TTypeIdProvider::template id<TUnterlying>();
            updateIndices(typeId);
            const auto typeIdIndex = typeId.value(); 

            items_[typeIdIndex].push_back(std::make_unique<TWrapper>(std::move(wrapper)));

            needsUpdate_ = true;

            return ConceptModelCollectionKey{typeId, items_[typeIdIndex].size() - 1};
        }

        /**
         * @brief Add a pre-fabricated wrapper to the collection.
         *
         * @details The wrapper must provide a typeId() function that returns a
         * TTypeIdProvider value to properly compute the returned key.
         *
         * @param wrapper The wrapper to add
         *
         * @return The key under which the wrapper is identified.
         */
        ConceptModelCollectionKey add(TWrapper&& wrapper) requires requires (const TWrapper& candidate) {{ candidate.typeId() } -> std::same_as<TTypeIdProvider>;} {

            const auto typeId = wrapper.typeId();
            const auto typeIdIndex = typeId.value();
            
            updateIndices(typeId);
            items_[typeIdIndex].push_back(std::make_unique<TWrapper>(std::move(wrapper)));

            needsUpdate_ = true;

            return ConceptModelCollectionKey{typeId, items_[typeIdIndex].size() - 1};
        }


        /**
         * @brief Returns all const items for one underlying type.
         *
         * @tparam TUnderlying Underlying model type.
         * @return Span of const wrapper pointers, or empty span if none exist.
         */
        template<typename TUnderlying>
        [[nodiscard]] std::span<const TWrapper*> items() const {
            update();
            const auto idx = TTypeIdProvider::template id<TUnderlying>().value();
            if (items_.size() <= idx || items_[idx].empty()) {
                return {};
            }
            return constCollectionView_[typeIdToInsertionOrder_[idx]];
        }

        /**
         * @brief Returns all mutable items for one underlying type.
         *
         * @tparam TUnderlying Underlying model type.
         * @return Span of mutable wrapper pointers, or empty span if none exist.
         */
        template<typename TUnderlying>
        [[nodiscard]] std::span<TWrapper*> items() {
            update();
            const auto idx = TTypeIdProvider::template id<TUnderlying>().value();
            if (items_.size() <= idx || items_[idx].empty()) {
                return {};
            }
            return collectionView_[typeIdToInsertionOrder_[idx]];
        }

        /**
         * @brief Returns all mutable items for the given runtime type-id.
         *
         * @param typeId Runtime type-id provided by `TTypeIdProvider`.
         * @return Span of mutable wrapper pointers, or empty span if none exist.
         */
        [[nodiscard]] std::span<TWrapper*> items(TTypeIdProvider typeId) noexcept {
            update();
            const auto idx = typeId.value();
            if (items_.size() <= idx || items_[idx].empty()) {
                return {};
            }
            return collectionView_[typeIdToInsertionOrder_[idx]];
        }

        /**
         * @brief Returns a mutable item by key.
         *
         * @param key Compound key containing type-id and per-type index.
         * @return Pointer to the wrapper, or `nullptr` if the key is out of range.
         */
        [[nodiscard]] TWrapper* item(ConceptModelCollectionKey key) noexcept {
            return const_cast<TWrapper*>(std::as_const(*this).item(key));
        }

        /**
         * @brief Returns a non-mutable item by key.
         *
         * @param key Compound key containing type-id and per-type index.
         * @return Pointer to the wrapper, or `nullptr` if the key is out of range.
         */
        [[nodiscard]] const TWrapper* item(ConceptModelCollectionKey key) const noexcept {
            const auto typeId = key.typeId.value();
            const auto idx = key.index;

            if (items_.size() <= typeId || items_[typeId].size() <= idx) {
                return nullptr;
            }
            return items_[typeId][idx].get();
        }

    };


}
