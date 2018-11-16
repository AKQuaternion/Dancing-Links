//
//  main.cpp
//  DL101
//
//  Created by Chris Hartman on 11/7/18.
//  Copyright © 2018 Chris Hartman. All rights reserved.
//

#include <cctype>
#include <exception>
#include <functional>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <sstream>
#include <tuple>
#include <utility>
#include <vector>
using std::runtime_error;
using std::function;
using std::setw;
using std::cout;
using std::endl;
using std::istream;
using std::map;
using std::unique_ptr;
using std::make_unique;
using std::string;
using std::to_string;
using std::string_view;
using std::istringstream;
using std::ostringstream;
using std::tie;
using std::move;
using std::pair;
using std::vector;

class ExactCover;
ExactCover exampleA();

using NodePtr = int;
using ItemPtr = int; //!!! Consider type safety here

struct Node {
   int _itm;
#define _len _itm
   int _spare;
#define _aux _spare
   NodePtr _ulink;
   NodePtr _dlink;
   bool operator==(const Node &rhs) const {
      return tie(_itm,_spare,_ulink,_dlink)== tie(rhs._itm,rhs._spare,rhs._ulink,rhs._dlink);
   }
};

struct Item {
   NodePtr _llink;
   NodePtr _rlink;
   bool operator==(const Item &rhs) const {
      return tie(_llink,_rlink) == tie(rhs._llink,rhs._rlink);
   }
};

class ExactCover {
   const static int _root = 0;
   Node & deRef(NodePtr n) {
      return _nodes[size_t(n)];
   }
   
   const Node & deRef(NodePtr n) const {
      return _nodes[size_t(n)];
   }
   
   NodePtr down(NodePtr n) const {
      return deRef(n)._dlink;
   }
   
   NodePtr up(NodePtr n) const {
      return deRef(n)._ulink;
   }
   
   NodePtr right(NodePtr n) const {
      auto r = n+1;
      if (itemOf(r) <= 0)
         r = up(r);
      return r;
   }
   
   NodePtr left(NodePtr n) const {
      auto r = n-1;
      if (itemOf(r) <= 0)
         r = down(r);
      return r;
   }
   
   int itemOf(NodePtr n) const {
      return deRef(n)._itm;
   }
   
   void hide(NodePtr p) {
      // remove everything else in this row from its column list
      //      cout << "Hide " << p << endl;
      
      for(auto q=right(p); q != p; q=right(q)) {
         //         cout << "Remove " << q << endl;
         deRef(up(q))._dlink = down(q);
         deRef(down(q))._ulink = up(q);
         --deRef(itemOf(q))._len;
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
         ++deRef(itemOf(q))._len;
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
   
   void registerSolution(const vector<NodePtr> & t) {
      
   }
   
   string_view itemNameOf(NodePtr p) const {
      return _names[itemOf(p)];
   }
   
   static string readLine(istream &input) {
      while(true) {
         string line;
         getline(input,line);
         if (!input) {
            if (input.eof())
               return line;
            else
               throw runtime_error("Error reading from input file.");
         }
         
         auto pos = line.find_first_not_of(" \t");
         if (pos != string::npos && line[pos] != '|')
            return line;
      }
   }
   
public:
   explicit ExactCover(istream &input) {
      _items.push_back({0,1});
      _nodes.push_back({0,0,0,0});
      _names.push_back("");
      auto nextItem = 1;
      
      auto line=readLine(input);
      if(line.empty())
         throw runtime_error("No item line in file.");
      istringstream lin(line);

      map<string,int> itemNumbers;

      while (true) {
         string item;
         lin >> item;
         if (!lin && lin.eof())
            break;

         if (item[0]=='|') {
            if (_second != -1)
               throw runtime_error("Item name line contains | twice.");
            _second = nextItem;
            if (item.size()==1)
               continue;
            else
               item = item.substr(1);
         }

         if (item.find_first_of("|:") != string::npos)
             throw runtime_error("| or : used in item name: " + item);
         if (itemNumbers.count(item) != 0)
            throw runtime_error("Duplicate item name in item name line: "+item);
         itemNumbers[item] = nextItem;
         _names.push_back(item);
         _items.push_back({nextItem-1, nextItem+1});
         _nodes.push_back({0,-1,nextItem,nextItem});
         ++nextItem;
      }
      _lastItem = nextItem;
      
      if (_second == -1)
         _second = _lastItem;
      _items.push_back({_lastItem-1, _second});
      _items[_second]._llink = _lastItem;
      _items[_second-1]._rlink = _root;
      _items[_root]._llink = _second-1;

      int nextNode = int(_nodes.size());
      auto options=0;
      _nodes.push_back({-options,-1,-1,-1}); //first spacer
      nextNode++;

      while (true) {
         auto line = readLine(input);
         if(line.empty()) {
            if(nextNode == _lastItem)
               throw runtime_error("No options lines in file.");
            else
               break;
         }
         istringstream lin(line);
         auto prevSpacer = nextNode-1;
         bool optionHasPrimaryItem = false;
         while (true) {
            string item;
            lin >> item;
            if (!lin && lin.eof()) {
               if (!optionHasPrimaryItem)
                  throw runtime_error("Option line has no primary items: " + to_string(options+1));
               deRef(prevSpacer)._dlink = nextNode-1;
               ++options;
               _nodes.push_back({-options,-1,prevSpacer+1,-1});
               ++nextNode;
               break;
            }

            if(itemNumbers.count(item) == 0)
               throw runtime_error("Unknown item in option " + to_string(options+1) + ": " + item);
            auto headerNum = itemNumbers[item];
            if (headerNum < _second)
               optionHasPrimaryItem = true;
            
            if (deRef(headerNum)._aux == options)
               throw runtime_error("Duplicate item in option " + to_string(options+1) + ": " + item);
            auto above=deRef(headerNum)._ulink;
            _nodes.push_back({headerNum,-1,above,headerNum});
            deRef(down(nextNode))._ulink = nextNode;
            deRef(up(nextNode))._dlink = nextNode;
            ++deRef(headerNum)._len;
            deRef(headerNum)._aux = options;
            ++nextNode;
         }
         
      }
   }

   void showOptions(const vector<NodePtr> & t) const {
      for(auto n:t) {
         cout << setw(5) << optionOf(n) << ": ";
         auto p = n;
         do {
            cout << itemNameOf(p) << " ";
            p=right(p);
         } while (p != n);
         cout << endl;
      }
   }
   
   const auto & getItems() const {return _items;}
   const auto & getNodes() const {return _nodes;}
   
   int optionOf(NodePtr n) const {
      while (itemOf(n)>0)
         ++n;
      return -itemOf(n);
   }
   
   void summarize() const {
      cout << "Occurences: ";
      for(NodePtr ii=1;ii<_nodes.size()-1;++ii) {
         if (ii<_items.size()-1) {
            cout << _names[ii] << ":" << itemOf(ii) << " ";
         } else {
            if (itemOf(ii) <= 0)
               cout << endl << "Option " << -itemOf(ii)+1 << ": ";
            else
               cout << _names[itemOf(ii)] << " ";
         }
      }
      cout << endl;
   }
   
   void algorithmD(function<void(const vector<NodePtr> &, const ExactCover *ec)> sol = {}) {
      if (!sol)
         sol = [](auto t, auto ec){
            cout << "Solution: " << endl;
            ec->showOptions(t);
         };
      
      vector<NodePtr> t;
      ItemPtr i;
      //   D1: // Initialize
      //      ItemPtr len = 0;
      //      auto z = _nodes.size()-1;
      
   D2: //Enter level l
      if(_items[0]._rlink == 0) {
         sol(t,this);
         goto D6; //return
      }
      //   D3: // Choose i
      i = _items[0]._rlink;
      //   D4: // Cover i
      cover(i);
      for(t.push_back(down(i));t.back() != i;t.back() = down(t.back())) {
         //      D5: // Try x_l (in our case, t.back())
         for (auto p = right(t.back()); p != t.back(); p=right(p))
            cover(itemOf(p));
         goto D2; //recurse
         
      D6: // Try next x_l, return from recursion lands here
         for(auto p = left(t.back()); p != t.back(); p=left(p))
            uncover(itemOf(p));
         i = itemOf(t.back());
      }
      
   D7: // Backtrack
      uncover(i);
   D8: // Leave level l
      if (t.size()==1)
         return;
      t.pop_back();
      goto D6;
   }
private: //!!! change these to array<>
   vector<Item> _items;
   vector<Node> _nodes;
   vector<string> _names;
   int _second=-1;
   int _lastItem = -1;
};

ExactCover exampleA() {
   string exA = R"(
   a b c d e f g
   c e
   a d g
   b c f
   a d f
   b g
   d e g)";
   auto infile = istringstream(exA);
   return ExactCover(infile);
}
#undef _len
#undef _aux

void exampleATester() {
   static_assert(sizeof(int)==4, "Should have sizeof(int)==4"); //Use int type with that guarantee
   auto exA = exampleA();
   auto v = vector<Item>{
      {7,1},
      {0,2},
      {1,3},
      {2,4},
      {3,5},
      {4,6},
      {5,7},
      {6,0},
      {8,8}
   };
   if (exA.getItems() != v)
      cout << "Items don't match." << endl;
   else
      cout << "Items okay." << endl;
   if (exA.getNodes() != vector<Node>{
      {0,0,0,0},     //dummy
      {2,3,20,12},
      {2,4,24,16},
      {2,2,17,9},
      {3,5,27,13},
      {2,5,28,10},
      {2,3,22,18},
      {3,5,29,14},
      {0,-1,-1,10},
      {3,-1,3,17},
      {5,-1,5,28},
      {-1,-1,9,14},
      {1,-1,1,20},
      {4,-1,4,21},
      {7,-1,7,25},
      {-2,-1,12,18},
      {2,-1,2,24},
      {3,-1,9,3},
      {6,-1,6,22},
      {-3,-1,16,22},
      {1,-1,12,1},
      {4,-1,13,27},
      {6,-1,18,6},
      {-4,-1,20,25},
      {2,-1,16,2},
      {7,-1,14,29},
      {-5,-1,24,29},
      {4,-1,21,4},
      {5,-1,10,5},
      {7,-1,25,7},
      {-6,-1,27,-1}
   })
      cout << "Nodes don't match." << endl;
   else
      cout << "Nodes okay." << endl;
}

void nQueens(int n) {
   ostringstream out;

   for(int i=0;i<n;++i)
      out << " r" + to_string(i);
   for(int i=0;i<n;++i)
      out << " c" + to_string(i);
   out << " | ";
   for(int i=0;i<=2*(n-1);++i)
      out << " c+r=" + to_string(i);
   for(int i=-(n-1);i<n;++i)
      out << " c-r=" + to_string(i);
   out << "\n";
   
   vector<pair<int,int>> coords(1);

   for(int c=0;c<n;++c)
      for(int r=0;r<n;++r) {
         out << "r" << r;
         out << " c" << c;
         out << " c+r=" << c+r;
         out << " c-r=" << c-r << "\n";
         coords.push_back({c,r});
      }
   auto drawSolution = [&](const vector<NodePtr> &t, const ExactCover *ec) {
      vector<vector<int>> b(n,vector<int>(n));
      for(auto node:t) {
         auto i = ec->optionOf(node);
//         cout << "Solopt: " << i << endl;
         b[coords[i].first][coords[i].second] = 1;
      }
      for(const auto & row : b) {
         for(auto c:row) {
            cout << (c?'Q':'.') << ' ';
         }
         cout << endl;
      }
      cout << "--------" << endl;
   };
   cout << out.str() << endl;
   istringstream in(out.str());
   auto fourQ = ExactCover(in);
   fourQ.summarize();
   fourQ.algorithmD(drawSolution);
}

int main() {
   exampleATester();
   auto eA = exampleA();
   eA.summarize();
   eA.algorithmD();
   nQueens(4);
   
   return 0;
}
