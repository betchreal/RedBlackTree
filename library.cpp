#include "library.h"

#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>

#define NODE_NOT_FOUND "node not found"
#define T_NIL_DATA ""
#define T_NIL_KEY 0
#define INDENT_VALUE 4

using namespace std;
using json = nlohmann::json;

class JsonHandler;

struct Node {
    int key;
    string data;

    Colour colour;

    shared_ptr<Node> left, right, parent;

    Node(int key, string data, shared_ptr<Node> T_nil) : key(key), data(data), colour(RED), left(T_nil), right(T_nil), parent(T_nil) {}
};

class RedBlackTree {
    friend class JsonHandler;
private:
    shared_ptr<Node> root;
    shared_ptr<Node> T_nil;

    void rotateLeft(shared_ptr<Node> x) {
        shared_ptr<Node> temp = x->right;
        x->right = temp->left;

        if (temp->left != T_nil) {
            temp->left->parent = x;
        }

        temp->parent = x->parent;

        if (x->parent == T_nil) {
            root = temp;
        }
        else if (x == x->parent->left) {
            x->parent->left = temp;
        }
        else {
            x->parent->right = temp;
        }

        temp->left = x;
        x->parent = temp;
    }

    void rotateRight(shared_ptr<Node> x) {
        shared_ptr<Node> temp = x->left;
        x->left = temp->right;

        if (temp->right != T_nil) {
            temp->right->parent = x;
        }

        temp->parent = x->parent;

        if (x->parent == T_nil) {
            root = temp;
        }
        else if (x == x->parent->left) {
            x->parent->left = temp;
        }
        else {
            x->parent->right = temp;
        }

        temp->right = x;
        x->parent = temp;
    }

    void recolour(shared_ptr<Node> node) {
        auto parent = node->parent;
        shared_ptr<Node> uncle = T_nil;

        if (parent == parent->parent->left) {
            uncle = parent->parent->right;
        }
        else {
            uncle = parent->parent->left;
        }

        parent->colour = BLACK;

        if (uncle != T_nil) {
            uncle->colour = BLACK;
        }

        parent->parent->colour = RED;
    }

    void insertFixUp(shared_ptr<Node> node) {
        while (node != root && node->parent->colour == RED) {
            if (node->parent == node->parent->parent->left) {
                shared_ptr<Node> uncle = node->parent->parent->right;
                if (uncle->colour == RED) {
                    recolour(node);
                    node = node->parent->parent;
                }
                else {
                    if (node == node->parent->right) {
                        node = node->parent;
                        rotateLeft(node);
                    }
                    node->parent->colour = BLACK;
                    node->parent->parent->colour = RED;
                    rotateRight(node->parent->parent);
                }
            }
            else {
                shared_ptr<Node> uncle = node->parent->parent->left;
                if (uncle->colour == RED) {
                    recolour(node);
                    node = node->parent->parent;
                }
                else {
                    if (node == node->parent->left) {
                        node = node->parent;
                        rotateRight(node);
                    }
                    node->parent->colour = BLACK;
                    node->parent->parent->colour = RED;
                    rotateLeft(node->parent->parent);
                }
            }
        }
        root->colour = BLACK;
    }

    auto findNode(int key) {
        auto node = root;
        while (node != T_nil) {
            if (key == node->key) {
                return node;
            }
            node = key > node->key ? node->right : node->left;
        }
        return T_nil;
    }

    void transplant(shared_ptr<Node> deletedNode, shared_ptr<Node> newNode) {
        if (deletedNode->parent == T_nil) {
            root = newNode;
        }
        else if (deletedNode == deletedNode->parent->left) {
            deletedNode->parent->left = newNode;
        }
        else {
            deletedNode->parent->right = newNode;
        }
        newNode->parent = deletedNode->parent;
    }

    auto minimum(shared_ptr<Node> node) {
        while (node->left != T_nil) {
            node = node->left;
        }
        return node;
    }

    void deleteFixUp(shared_ptr<Node> node) {
        // Відновлення властивостей t_nil
        shared_ptr<Node> t_nil = nullptr;

        if (node == T_nil) {
            t_nil = node;
        }

        while (node != root && node->colour == BLACK) {
            if (node == node->parent->left) {
                shared_ptr<Node> brother = node->parent->right;

                if (brother->colour == RED) {
                    brother->colour = BLACK;
                    node->parent->colour = RED;
                    rotateLeft(node->parent);
                    brother = node->parent->right;
                }
                if (brother->left->colour == BLACK && brother->right->colour == BLACK) {
                    brother->colour = RED;
                    node = node->parent;
                }
                else {
                    if (brother->right->colour == BLACK) {
                        brother->left->colour = BLACK;
                        brother->colour = RED;
                        rotateRight(brother);
                        brother = node->parent->right;
                    }
                    brother->colour = node->parent->colour;
                    node->parent->colour = BLACK;
                    brother->right->colour = BLACK;
                    rotateLeft(node->parent);
                    node = root;
                }
            }
            else {
                shared_ptr<Node> brother = node->parent->left;
                if (brother->colour == RED) {
                    brother->colour = BLACK;
                    node->parent->colour = RED;
                    rotateRight(node->parent);
                    brother = node->parent->left;
                }
                if (brother->right->colour == BLACK && brother->left->colour == BLACK) {
                    brother->colour = RED;
                    node = node->parent;
                }
                else {
                    if (brother->left->colour == BLACK) {
                        brother->right->colour = BLACK;
                        brother->colour = RED;
                        rotateLeft(brother);
                        brother = node->parent->left;
                    }
                    brother->colour = node->parent->colour;
                    node->parent->colour = BLACK;
                    brother->left->colour = BLACK;
                    rotateRight(node->parent);
                    node = root;
                }
            }
        }
        node->colour = BLACK;

        t_nilMaintain(t_nil);
    }

    void t_nilMaintain(shared_ptr<Node> node) {
        if (node == T_nil) {
            T_nil->colour = BLACK;

            T_nil->parent = T_nil;
            T_nil->left = T_nil;
            T_nil->right = T_nil;
        }
    }

public:
    RedBlackTree() {
        T_nil = make_shared<Node>(T_NIL_KEY, T_NIL_DATA, nullptr);
        T_nil->colour = BLACK;
        T_nil->left = T_nil;
        T_nil->right = T_nil;
        T_nil->parent = T_nil;
        root = T_nil;
    }

    shared_ptr<Node> getRoot() const {
        if (root == T_nil) {
            return nullptr;
        }
        return root;
    }

    void insert(int key, const string &data) {
        shared_ptr<Node> node = make_shared<Node>(key, data, T_nil);

        shared_ptr<Node> search = root;
        shared_ptr<Node> parent = T_nil;

        while (search != T_nil) {
            parent = search;
            if (node->key > search->key) {
                search = search->right;
            }
            else {
                search = search->left;
            }
        }

        node->parent = parent;

        if (parent == T_nil) {
            root = node;
        }
        else if (node->key > parent->key) {
            parent->right = node;
        }
        else {
            parent->left = node;
        }

        node->left = T_nil;
        node->right = T_nil;
        node->colour = RED;
        insertFixUp(node);
    }

    tuple<int, string> search(int key) {
        auto node = findNode(key);

        if (node != T_nil) {
            return { node->key, node->data };
        }
        else {
            return { INT_MIN, NODE_NOT_FOUND }; // Вузол не знайдено
        }
    }

    bool remove(int key) {
        auto node = findNode(key);
        if (node == T_nil) return false;

        shared_ptr<Node> successor = node;
        Colour original_colour = successor->colour;
        shared_ptr<Node> x;

        if (node->left == T_nil) {
            x = node->right;
            transplant(node, node->right);
        }
        else if (node->right == T_nil) {
            x = node->left;
            transplant(node, node->left);
        }
        else {
            successor = minimum(node->right);
            original_colour = successor->colour;
            x = successor->right;

            if (successor->parent == node) {
                x->parent = successor;
            }
            else {
                transplant(successor, successor->right);
                successor->right = node->right;
                successor->right->parent = successor;
            }

            transplant(node, successor);
            successor->left = node->left;
            successor->left->parent = successor;
            successor->colour = node->colour;
        }
        if (original_colour == BLACK) {
            deleteFixUp(x);
        }
        return true;
    }
};

class JsonHandler {
public:
    static void saveTreeToJson(const RedBlackTree &tree, const string &filename) {
        json jsonTree;
        serializeNode(tree.root, jsonTree, tree.T_nil);

        ofstream outFile(filename);
        if (outFile.is_open()) {
            outFile << jsonTree.dump(INDENT_VALUE);
            outFile.close();
        }
    }

    static RedBlackTree loadTreeFromJson(const string &filename) {
        ifstream inFile(filename);
        json jsonTree;
        if (inFile.is_open()) {
            inFile >> jsonTree;
            inFile.close();
        } else {
            throw runtime_error("Unable to open file for reading");
        }

        RedBlackTree tree;
        tree.root = deserializeNode(jsonTree, tree.T_nil);
        return tree;
    }

private:
    static void serializeNode(const shared_ptr<Node> &node, json &jsonNode, const shared_ptr<Node> &T_nil) {
        if (node == T_nil) {
            jsonNode = nullptr;
            return;
        }

        jsonNode["key"] = node->key;
        jsonNode["data"] = node->data;
        jsonNode["colour"] = node->colour == RED ? "RED" : "BLACK";

        json left, right;
        serializeNode(node->left, left, T_nil);
        serializeNode(node->right, right, T_nil);

        jsonNode["left"] = left;
        jsonNode["right"] = right;
    }

    static shared_ptr<Node> deserializeNode(const json &jsonNode, const shared_ptr<Node> &T_nil) {
        if (jsonNode.is_null()) {
            return T_nil;
        }

        auto node = make_shared<Node>(jsonNode["key"], jsonNode["data"], T_nil);
        node->colour = jsonNode["colour"] == "RED" ? RED : BLACK;

        node->left = deserializeNode(jsonNode["left"], T_nil);
        node->right = deserializeNode(jsonNode["right"], T_nil);

        if (node->left != T_nil) node->left->parent = node;
        if (node->right != T_nil) node->right->parent = node;

        return node;
    }
};