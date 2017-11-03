#ifndef _LINKEDLIST_H_
#define _LINKEDLIST_H_

template <typename T>
class Node {

public:
    T value;
    Node<T>* next;
    Node (T value) : value (value), next (nullptr) {};
    ~Node() {
        delete(value);
    }
};

template <typename T>
class LinkedList {
    Node<T>* first;
    
public:
    void add(T value) {
        if (! first) {
            Node<T>* n  = new Node<T>(value);
            first = n;
        } else {
            Node<T>* n = first;
            while (n->next) 
                n = n->next;
            Node<T>* newNode = new Node<T>(value);
            n->next = newNode;
        }
    }
    
    T first_value() {
        return first->value;
    }

    template <typename F>
    void for_each(F&& f) {
        Node<T>* n = first;
        while (n) {
            f(n->value);
            n = n->next;
        }
    }

    ~LinkedList() {
        Node<T>* n = first;
        while (n) {
            Node<T>* temp = n;
            n = n->next;
            delete(temp);
        }
    }
};
#endif