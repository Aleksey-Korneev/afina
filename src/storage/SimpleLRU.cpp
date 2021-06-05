#include "SimpleLRU.h"

namespace Afina {
namespace Backend {

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Put(const std::string &key, const std::string &value)
{
    // No need to search if total weight is too large.
    if (key.size() + value.size() > _max_size) {
        return false;
    }
    if (_lru_index.find(key) == _lru_index.end()) {
        return _Put(key, value);
    }
    return _UpdateValue(_lru_index.find(key)->second.get(), value);
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::PutIfAbsent(const std::string &key, const std::string &value)
{
    // No need to search if total weight is too large.
    if (key.size() + value.size() > _max_size) {
        return false;
    }
    if (_lru_index.find(key) == _lru_index.end()) {
        return _Put(key, value);
    }
    return false;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Set(const std::string &key, const std::string &value)
{
    // No need to search if total weight is too large.
    if (key.size() + value.size() > _max_size) {
        return false;
    }
    if (_lru_index.find(key) == _lru_index.end()) {
        return false;
    }
    return _UpdateValue(_lru_index.find(key)->second.get(), value);
}
// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Delete(const std::string &key)
{
    // No need to search if key weight is too large.
    if (key.size() > _max_size) {
        return false;
    }
    if (_lru_index.find(key) == _lru_index.end()) {
        return false;
    }
    _DeleteNode(_lru_index.find(key)->second.get());
    return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Get(const std::string &key, std::string &value)
{
    // No need to search if total weight is too large.
    if (key.size() + value.size() > _max_size) {
        return false;
    }
    if (_lru_index.find(key) == _lru_index.end()) {
        return false;
    }
    lru_node &node = _lru_index.find(key)->second.get();
    value = node.value;
    _MoveNode(node);
    return true;
}

bool SimpleLRU::_Put(const std::string &key, const std::string &value)
{
    std::size_t size = key.size() + value.size();
    // Already checked that (key + value) size is acceptable.
    while (_size + size > _max_size) {
        _DeleteNode(std::ref(*(_lru_head)));
    }
    lru_node *new_node = new lru_node { key, value, _lru_tail, nullptr };
    if (_lru_head) {
        _lru_tail->next = std::unique_ptr<lru_node>(new_node);
    } else {
        _lru_head = std::unique_ptr<lru_node>(new_node);
    }
    _size += size;
    _lru_tail = new_node;
    _lru_index.emplace(std::cref(new_node->key), std::ref(*new_node));
    return true;
}

bool SimpleLRU::_UpdateValue(lru_node &node, const std::string &value)
{
    std::size_t old_value_size = node.value.size();
    std::size_t value_size = value.size();
    std::size_t old_size = old_value_size + node.key.size();
    if (value_size + node.key.size() > _max_size) {
        return false;
    }
    //node.value = value;
    _MoveNode(node);
    while (_size + value_size > _max_size + old_value_size) {
        _DeleteNode(std::ref(*(_lru_head)));
    }
    _size += value_size - old_value_size;
    node.value = value;
    return true;
}

void SimpleLRU::_DeleteNode(lru_node &node) {
    std::size_t size = node.key.size() + node.value.size();
    _size -= size;
    _lru_index.erase(node.key);
    if (_lru_head.get() == _lru_tail) {
        _lru_head = nullptr;
        _lru_tail = nullptr;
    } else if (!node.prev) {
        _lru_head = std::move(_lru_head->next);
        _lru_head->prev = nullptr;
    } else if (!node.next) {
        _lru_tail = _lru_tail->prev;
        _lru_tail->next = nullptr;
    } else {
        lru_node *prev_node = node.prev;
        prev_node->next = std::move(node.next);
        prev_node->next->prev = prev_node;
    }
}

void SimpleLRU::_MoveNode(lru_node &node)
{
    if (node.next) {
        // Change next
        lru_node *ptr = node.next->prev;
        ptr->next->prev = ptr->prev;
        if (!(ptr->prev)) {
            //Change head and head
            _lru_tail->next = std::move(_lru_head);
            _lru_head = std::move(ptr->next);
        } else {
            //Change tail and prev
            _lru_tail->next = std::move(ptr->prev->next);
            ptr->prev->next = std::move(ptr->next);
        }
        ptr->prev = _lru_tail;
        _lru_tail = ptr;
        ptr->next = nullptr;
    }
}

} // namespace Backend
} // namespace Afina
