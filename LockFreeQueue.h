#pragma once

#include <Windows.h>

#include <string>
#include <stdexcept>


// implement a lock-free queue based on algorithm by John D. Valois
template <typename T>
class LockFreeQueue
{
private:
    // internal base node structure...
    template <typename U>
    struct Node
    {
        typedef Node<U>     _MyType;
        typedef T           value_type;

        Node() : next(NULL), value()
        {
        }

        Node(value_type v) : next(NULL), value(v)
        {
        }

        _MyType*    next;
        value_type  value;
    };

public:
    // typedefs...
    typedef LockFreeQueue<T>            _MyType;
    typedef Node<T>                     _Node;
    typedef typename _Node::value_type  value_type;

public:
    // constructor and destructor...
    LockFreeQueue(void) : _head(), _tail()
    {
        _head = _tail = new _Node;
    }

    ~LockFreeQueue(void)
    {
    }

public:
    // exception...
    class empty_queue : public std::runtime_error
    {
    public:
        explicit empty_queue(const std::string& message)
            : runtime_error(message)
        {
        }

        explicit empty_queue(const char* message)
            : runtime_error(message)
        {
        }
    };

public:
    // public methods...
    void EnQueue(value_type value)
    {
        _Node* pNewNode = NULL;
        try
        {
            pNewNode = new _Node(value);
        }
        catch (const std::bad_alloc&)
        {
            return;
        }

        _Node* pNode = (_tail);
        _Node* pOldNode = pNode;

        do
        {
            while (pNode->next)
            {
                pNode = pNode->next;
            }
        } while(::InterlockedCompareExchangePointer(reinterpret_cast<PVOID volatile*>(&(pNode->next)), pNewNode, NULL));

        ::InterlockedCompareExchangePointer(reinterpret_cast<PVOID volatile*>(&_tail), pNewNode, pOldNode);
    }

    value_type DeQueue()
    {
        _Node* pHead = NULL;
        do
        {
            pHead = _head;
            if (NULL == pHead->next)
            {
                throw empty_queue("Try to dequeue from an empty queue.");
            }
        } while (pHead != ::InterlockedCompareExchangePointer(reinterpret_cast<PVOID volatile*>(&_head), pHead->next, pHead));

        value_type ret = pHead->next->value;
        delete pHead;
        return ret;
    }

private:
    _Node* volatile _head;
    _Node* volatile _tail;
};
