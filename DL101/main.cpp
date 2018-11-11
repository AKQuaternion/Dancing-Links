//
//  main.cpp
//  DL101
//
//  Created by Chris Hartman on 11/7/18.
//  Copyright © 2018 Chris Hartman. All rights reserved.
//

#include <iostream>
#include <string>
#include <tuple>
#include <utility>
#include <vector>
using std::cout;
using std::endl;
using std::string;
using std::to_string;
using std::move;
using std::pair;
using std::tie;
using std::vector;

class ExactCover;
ExactCover exampleA();

using NodePtr = size_t;
using ItemPtr = size_t; //!!! Consider type safety here

struct Node {
   int _top;
   NodePtr _ulink;
   NodePtr _dlink;
   bool operator==(const Node &rhs) const {
      return tie(_top,_ulink,_dlink)== tie(rhs._top,rhs._ulink,rhs._dlink);
   }
};

struct Item {
   string _name;
   NodePtr _llink;
   NodePtr _rlink;
   bool operator==(const Item &rhs) const {
      return tie(_name,_llink,_rlink) == tie(rhs._name,rhs._llink,rhs._rlink);
   }
};

class ExactCover {
   Node & deRef(NodePtr n) {
      return _nodes[n];
   }
   
   const Node & deRef(NodePtr n) const {
      return _nodes[n];
   }
   
   NodePtr down(NodePtr n) const {
      return deRef(n)._dlink;
   }
   
   NodePtr up(NodePtr n) const {
      return deRef(n)._ulink;
   }
   
   NodePtr right(NodePtr n) const {
      auto r = n+1;
      if (top(r) <= 0)
         r = up(r);
      return r;
   }
   
   NodePtr left(NodePtr n) const {
      auto r = n-1;
      if (top(r) <= 0)
         r = down(r);
      return r;
   }

   int top(NodePtr n) const {
      return deRef(n)._top;
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
         cout << _items[_nodes[n]._top]._name << " ";
         for (auto p = right(n); p != n; p=right(p)) {
            cout << _items[top(p)]._name << " ";
         }
         cout << endl;
      }
   }

public:

   explicit ExactCover(vector<Item> items, vector<Node> nodes)
   :_items(move(items)),_nodes(move(nodes))
   {}
   
   explicit ExactCover(vector<string> items, vector<vector<size_t>> options, size_t primary=0) {
      if(primary==0)
         primary = items.size();
      _items.push_back({"-",0,0});
      _nodes.push_back({0,0,0});
      for(const auto & i : items) {
         auto iNum = _items.size();
         _items.push_back({i,iNum, iNum});
         _nodes.push_back({0,iNum, iNum});
      }
      for(size_t i=0;i<=primary;++i) {
         _items[i]._llink=i-1;
         _items[i]._rlink=i+1;
      }
      _items[0]._llink = primary;
      _items[primary]._rlink = 0;
      auto count=0;
      _nodes.push_back({-count,0,0});
      for(const auto & option : options) {
         auto start = _nodes.size();
         _nodes.back()._dlink = start+option.size()-1;
         for(const auto item : option) {
            auto myIndex = _nodes.size();
            _nodes.push_back({int(item+1),up(item+1),item+1});
            deRef(down(myIndex))._ulink = myIndex;
            deRef(up(myIndex))._dlink = myIndex;
            ++deRef(top(myIndex))._top;
         }
         ++count;
         _nodes.push_back({-count,start,0});
      }
   }
   
   const auto & getItems() {return _items;}
   const auto & getNodes() {return _nodes;}

   void summarize() {
      cout << "Occurences: ";
      for(size_t ii=1;ii<_nodes.size()-1;++ii) {
         if (ii<_items.size()) {
            cout << _items[ii]._name << ":" << _nodes[ii]._top << " ";
         } else {
            if (_nodes[ii]._top <= 0)
               cout << endl << "Option " << -_nodes[ii]._top << ": ";
            else
               cout << _items[_nodes[ii]._top]._name << " ";
         }
      }
      cout << endl;
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
         
      D6: // Try next x_l, return from recursion lands here
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
   vector<string> items;
   for(auto c='a';c <= 'g'; ++c)
      items.emplace_back(1,c);
   vector<vector<size_t>> options;
   for(const string &s : {"ce","adg","bcf","adf","bg","deg"}) {
      vector<size_t> indices;
      for(auto c:s) {
         indices.push_back(c-'a');
      }
      options.push_back(indices);
   }
   return ExactCover(move(items), move(options));
}

ExactCover exampleAHardCoded() {
   return ExactCover(vector<Item>{
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

void exampleATester() {
   auto exA = exampleA();
   auto exAHardCoded = exampleAHardCoded();
   if (exA.getItems() != exAHardCoded.getItems())
      cout << "Items don't match." << endl;
   else
      cout << "Items okay." << endl;
   if (exA.getNodes() != exAHardCoded.getNodes())
      cout << "Nodes don't match." << endl;
   else
      cout << "Nodes okay." << endl;
}

ExactCover nQueens(int n) {
   vector<string> items;
   auto rBase=items.size();
   for(int i=0;i<n;++i)
      items.push_back("r" + to_string(i));
   auto cBase=items.size();
   for(int i=0;i<n;++i)
      items.push_back("c" + to_string(i));
   auto cprBase=items.size();
   for(int i=0;i<=2*(n-1);++i)
      items.push_back("c+r=" + to_string(i));
   auto cmrBase=items.size()+n-1;
   for(int i=-(n-1);i<n;++i)
      items.push_back("c-r=" + to_string(i));

   vector<pair<int,int>> coords;
   vector<vector<size_t>> options;
   for(int c=0;c<n;++c)
      for(int r=0;r<n;++r) {
         vector<size_t> itemIndices;
         itemIndices.push_back(rBase+r);
         itemIndices.push_back(cBase+c);
         itemIndices.push_back(cprBase+r+c);
         itemIndices.push_back(cmrBase+c-r);
         options.push_back(itemIndices);
         coords.push_back({c,r});
      }
   
   return ExactCover(items,options,2*n);
}

int main() {
   exampleATester();
//   auto eA = exampleA();
//   eA.summarize();
//   eA.algorithmD();
   auto fourQ = nQueens(4);
   fourQ.summarize();
   fourQ.algorithmD();
   return 0;
}
