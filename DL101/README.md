Notes
=====

Figure out how to log
Figure out how Knuth counts mems
Experiment between using ints or raw pointers for node pointers
Write setup from text files, lists of options, etc.
Plan out undoing recursion:

    //D1 
    Initialize
    //D2
    search(l) { 
      if no more items in itemList
         print solution
         return // This one always goes to D6
      
      // D3
      choose item i to cover 
      // D4
      cover(i) 
      // D5
      for each node x_l containing i // Try option o containing n
         for each other node j in option o
            cover(j->top)
         search(l+1)
         // D6
         for each other node j in option o going left
            uncover(j->top)
      // D7
      uncover(i)
      
      // D8
      return // This one might exit completely, or might go to D6
      // we can tell which by value of l
      }
   
