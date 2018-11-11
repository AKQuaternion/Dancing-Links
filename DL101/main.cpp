//
//  main.cpp
//  DL101
//
//  Created by Chris Hartman on 11/7/18.
//  Copyright © 2018 Chris Hartman. All rights reserved.
//

#include <iostream>
#include <string>
#include <utility>
#include <vector>
using std::cout;
using std::endl;
using std::string;
using std::move;
using std::vector;

class ExactCover {
   using NodePtr = size_t;
   using ItemPtr = size_t;
   
   struct Node {
      int _top;
      NodePtr _ulink;
      NodePtr _dlink;
   };
   
   struct Item {
      string _name;
      NodePtr _llink;
      NodePtr _rlink;
   };
   
   Node & deRef(NodePtr n) {
      return _nodes[n];
   }
   
   NodePtr down(NodePtr n) {
      return deRef(n)._dlink;
   }
   
   NodePtr up(NodePtr n) {
      return deRef(n)._ulink;
   }
   
   auto top(NodePtr n) {
      return deRef(n)._top;
   }
   
   NodePtr right(NodePtr n) {
      auto r = n+1;
      if (top(r) <= 0)
         r = up(r);
      return r;
   }
   
   NodePtr left(NodePtr n) {
      auto r = n-1;
      if (top(r) <= 0)
         r = down(r);
      return r;
   }
//      friend ostream& operator<<(ostream &os, NodePtr n) {
//         return os << n._val;
//      }
//      friend bool operator==(NodePtr lhs, NodePtr rhs) {
//         return lhs._val == rhs._val;
//      }
//      friend bool operator!=(NodePtr lhs, NodePtr rhs) {
//         return !(lhs == rhs);
//      }



public:

   explicit ExactCover(vector<Item> items, vector<Node> nodes) :_items(move(items)),_nodes(move(nodes))
   {}
   
   void summarize() {
      cout << "Occurences: ";
      for(size_t ii=1;ii<_nodes.size()-1;++ii) {
         if (ii<_items.size()) {
            cout << _nodes[ii]._top << _items[ii]._name << " ";
         } else {
            if (_nodes[ii]._top <= 0)
               cout << endl << "Option " << -_nodes[ii]._top << ": ";
            else
               cout << _items[_nodes[ii]._top]._name;
         }
      }
      cout << endl;
   }
   
   void hide(NodePtr p) {
      // remove everything else in this row from its column list
//      cout << "Hide " << p << endl;
      
      for(auto q=right(p); q != p; q=right(q)) {
//         cout << "Remove " << q << endl;
         deRef(up(q))._dlink = down(q);
         deRef(down(q))._ulink = up(q);
         --deRef(top(q))._top;
      }
   }
   
   void cover(ItemPtr i) {
      // for each option using this item (including this one)
      //     remove every other item in the option from its column
      // remove this item from item list

//      cout << "Cover " << i << endl;
      for(auto p = down(i);p != i; p=down(p))
         hide(p);
      auto l = _items[i]._llink;
      auto r = _items[i]._rlink;
      _items[l]._rlink = r;
      _items[r]._llink = l;
   }
   
   void unhide(NodePtr p) {
      for(auto q=left(p); q != p; q=left(q)) {
         deRef(up(q))._dlink = q;
         deRef(down(q))._ulink = q;
         ++deRef(top(q))._top;
      }
   }
      
   void uncover(ItemPtr i) {
      auto l = _items[i]._llink;
      auto r = _items[i]._rlink;
      _items[l]._rlink = i;
      _items[r]._llink = i;
      for(auto p = up(i);p != i;p=up(p))
         unhide(p);
   }

   void showSolution(const vector<NodePtr> t) {
      cout << "Solution: " << endl;
      for(auto n:t) {
         cout << _items[_nodes[n]._top]._name;
         for (auto p = right(n); p != n; p=right(p)) {
               cout << _items[top(p)]._name;
         }
         cout << endl;
      }
   }
   
   void algorithmD() {
      vector<NodePtr> t;
      ItemPtr i;
//   D1: // Initialize
//      ItemPtr len = 0;
//      auto z = _nodes.size()-1;
      
   D2: //Enter level l
      if(_items[0]._rlink == 0) {
         showSolution(t);
         goto D6; //return
      }
//   D3: // Choose i
      i = _items[0]._rlink;
//   D4: // Cover i
      cover(i);
      for(t.push_back(down(i));t.back() != i;t.back() = down(t.back())) {
//      D5: // Try x_l (in our case, t.back())
         for (auto p = right(t.back()); p != t.back(); p=right(p))
            cover(top(p));
         goto D2; //recurse
         
      D6: // Try next x_l
         for(auto p = left(t.back()); p != t.back(); p=left(p))
               uncover(top(p));
         i = top(t.back());
      }


   D7: // Backtrack
      uncover(i);
   D8: // Leave level l
      if (t.size()==1)
         return;
      t.pop_back();
      goto D6;
   }
private:
   vector<Item> _items;
   vector<Node> _nodes;
};

ExactCover exampleA() {

   return ExactCover({
      {"-",7,1},
      {"a",0,2},
      {"b",1,3},
      {"c",2,4},
      {"d",3,5},
      {"e",4,6},
      {"f",5,7},
      {"g",6,0},
   },{
      {0,0,0},
      {2,20,12},
      {2,24,16},
      {2,17,9},
      {3,27,13},
      {2,28,10},
      {2,22,18},
      {3,29,14},
      {0,0,10},
      {3,3,17},
      {5,5,28},
      {-1,9,14},
      {1,1,20},
      {4,4,21},
      {7,7,25},
      {-2,12,18},
      {2,2,24},
      {3,9,3},
      {6,6,22},
      {-3,16,22},
      {1,12,1},
      {4,13,27},
      {6,18,6},
      {-4,20,25},
      {2,16,2},
      {7,14,29},
      {-5,24,29},
      {4,21,4},
      {5,10,5},
      {7,25,7},
      {-6,27,0}
   });
}

int main() {
   auto eA = exampleA();
   eA.summarize();
   eA.algorithmD();
   return 0;
}
