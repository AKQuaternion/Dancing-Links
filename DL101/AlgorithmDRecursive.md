#  AlgorithmD in recursive form

    
    
    //D1 
    Initialize
    //D2
    void search(int l) { 
    if (noMoreItemsToCover())
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
    


