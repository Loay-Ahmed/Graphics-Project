#pragma once

#include <windows.h>
#include <vector>

class Clipping {
  public:
    static void SetClipWindow(int xmin, int ymin, int xmax, int ymax);
    static void ClippingPolygon(HDC hdc, const POINT *points, int n, COLORREF color);
    static void ClippingLine(HDC hdc, int x1, int y1, int x2, int y2, COLORREF color);
    static void ClipLineSquare(HDC hdc, int x1, int y1, int x2, int y2, COLORREF color);
    static void ClipPointSquare(HDC hdc, int x, int y, COLORREF color);
    static void Clippingpoint(HDC hdc, int x, int y, COLORREF color);
    static std::vector<POINT> SutherlandHodgmanClip(const POINT* input, int n);
    static bool inside(int x, int y, int edge);
    static POINT intersect(POINT p1, POINT p2, int edge);
    static int computeCode(int x, int y);

    // Dynamic window boundaries (user can set)
    static int CLIP_X_MIN;
    static int CLIP_Y_MIN;
    static int CLIP_X_MAX;
    static int CLIP_Y_MAX;
    static const int SUTH_EDGES[4]; // Declaration only

    // Cohen-Sutherland region codes
    enum {
        INSIDE = 0, // 0000
        LEFT   = 1, // 0001
        RIGHT  = 2, // 0010
        BOTTOM = 4, // 0100
        TOP    = 8  // 1000
    };

    /**
 * Node - linked-list unit for active edge table
 * Represents an edge in the active edge table with its x-coordinate, maximum y-coordinate, and inverse slope.
 * This structure is used to manage edges during the scanline fill algorithm in polygon clipping.
 */
    struct Node {
        Node* next;
        double x;
        int maxY;
        double Minv; // 1/m
        Node(double x, int maxY, double Minv) : x(x), maxY(maxY), Minv(Minv), next(nullptr) {}
    };
  
    /**
     * @brief - a simple linked list to manage active edges
     * This class provides methods to add, remove, update, and sort edges in the active edge table.
     * It is used during the scanline fill algorithm for polygon clipping.
     */
    struct LinkedList {
        Node* head;
        Node* tail;

        LinkedList() : head(nullptr), tail(nullptr) {}

        /**
         * @brief Adds a new edge to the linked list.
         * @param x The x-coordinate of the edge.
         * @param maxY The maximum y-coordinate of the edge.
         * @param Minv The inverse of the slope of the edge (1/m).
         * This method creates a new Node and adds it to the end of the linked list.
         */
        void add(double x, int maxY, double Minv) {
            Node* newNode = new Node(x, maxY, Minv);
            if (!head) {
                head = tail = newNode;
            } else {
                tail->next = newNode;
                tail = newNode;
            }
        }

        /**
         * @brief Removes edges with a specific maximum y-coordinate from the linked list.
         * @param y The maximum y-coordinate to remove.
         * This method traverses the linked list and removes all nodes with the specified maxY value.
         */
        void removeMaxY(int y) {
            Node* current = head;
            Node* previous = nullptr;
            while (current) {
                if (current->maxY == y) {
                    Node* toDelete = current;
                    if (previous)
                        previous->next = current->next;
                    else
                        head = current->next;

                    if (current == tail)
                        tail = previous;

                    current = current->next;
                    delete toDelete;
                } else {
                    previous = current;
                    current = current->next;
                }
            }
        }

        /**
         * @brief Updates the x-coordinates of all edges in the linked list.
         * This method increments the x-coordinate of each edge by its inverse slope (Minv).
         * It is used to prepare the edges for the next scanline in the polygon fill algorithm.
         */
        void updateX() {
            Node* current = head;
            while (current) {
                current->x = current->x + current->Minv;
                current = current->next;
            }
        }

        /**
         * @brief Sorts the edges in the linked list by their x-coordinates.
         * This method implements a simple bubble sort algorithm to sort the edges based on their x-coordinate.
         * It is used to ensure that edges are processed in the correct order during the scanline fill algorithm.
         */
        void sort() {
            if (!head || !head->next) return;
            bool swapped;
            do {
                swapped = false;
                Node* current = head;
                while (current && current->next) {
                    if (current->x > current->next->x) {
                        std::swap(current->x, current->next->x);
                        std::swap(current->maxY, current->next->maxY);
                        std::swap(current->Minv, current->next->Minv);
                        swapped = true;
                    }
                    current = current->next;
                }
            } while (swapped);
        }

        /**
         * @brief Clears the linked list and deletes all nodes.
         * This method traverses the linked list and deletes each node, effectively clearing the list.
         */
        void clear() {
            Node* current = head;
            while (current) {
                Node* next = current->next;
                delete current;
                current = next;
            }
            head = tail = nullptr;
        }
    };
};

