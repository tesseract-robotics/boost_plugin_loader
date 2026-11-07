/**
 *
 * @copyright Copyright (c) 2021, Southwest Research Institute
 *
 * @par License
 * Software License Agreement (Apache License)
 * @par
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * http://www.apache.org/licenses/LICENSE-2.0
 * @par
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef BOOST_PLUGIN_LOADER_PUBLISHED_MAP_H
#define BOOST_PLUGIN_LOADER_PUBLISHED_MAP_H

// STD
#include <atomic>
#include <deque>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace boost_plugin_loader
{
/**
 * @brief A map from string to a value computed once per key, read without a lock
 * @details Readers find the key in an immutable snapshot published through an atomic pointer, so concurrent readers
 * never write to shared memory. A miss computes the value without the lock held, then takes the lock to publish a
 * snapshot with the key added; if another thread published the key first, its value wins and this one is discarded.
 * Values and every snapshot ever published live as long as the map, so a returned reference stays valid.
 *
 * For few keys, each written once and read often: every insert copies the snapshot.
 * @tparam Value The value type
 */
template <typename Value>
class PublishedMap
{
public:
  PublishedMap() = default;
  ~PublishedMap() = default;
  PublishedMap(const PublishedMap&) = delete;
  PublishedMap& operator=(const PublishedMap&) = delete;
  PublishedMap(PublishedMap&&) = delete;
  PublishedMap& operator=(PublishedMap&&) = delete;

  /**
   * @brief The value for @p key, computing it with @p make if there is none
   * @param key The key
   * @param make Called with no arguments, without the lock, on a miss; may be called by several threads that miss the
   * same key at once. If it throws, nothing is stored.
   * @return The stored value; valid for the life of this map
   */
  template <typename Make>
  const Value& findOrInsert(const std::string& key, Make&& make)
  {
    // Acquire pairs with the release in publish, so a published snapshot and its values are visible.
    if (const Snapshot* current = current_.load(std::memory_order_acquire); current != nullptr)
    {
      auto it = current->find(key);
      if (it != current->end())
        return *it->second;
    }

    // Compute without the lock: make may take other locks (the dynamic loader's), which must not be ordered after it.
    Value made = std::forward<Make>(make)();

    std::scoped_lock lock(mutex_);
    const Snapshot* current = current_.load(std::memory_order_relaxed);  // writers are serialised by mutex_
    if (current != nullptr)
    {
      auto it = current->find(key);  // another thread may have published it meanwhile
      if (it != current->end())
        return *it->second;
    }

    const Value& stored = values_.emplace_back(std::move(made));
    auto next = (current != nullptr) ? std::make_unique<Snapshot>(*current) : std::make_unique<Snapshot>();
    next->emplace(key, &stored);
    current_.store(next.get(), std::memory_order_release);
    snapshots_.push_back(std::move(next));
    return stored;
  }

private:
  using Snapshot = std::unordered_map<std::string, const Value*>;

  /** @brief The latest snapshot; null until the first insert */
  std::atomic<const Snapshot*> current_{ nullptr };
  /** @brief Held to publish a snapshot */
  std::mutex mutex_;
  /** @brief Every snapshot published, kept so a reader of an older one stays valid */
  std::vector<std::unique_ptr<const Snapshot>> snapshots_;
  /** @brief The values; a deque, so adding one never moves the others */
  std::deque<Value> values_;
};

}  // namespace boost_plugin_loader

#endif  // BOOST_PLUGIN_LOADER_PUBLISHED_MAP_H
