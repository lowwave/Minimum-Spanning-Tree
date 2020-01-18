# Minimum Spanning Tree

A **minimum spanning tree** (**MST**) or **minimum weight spanning tree** is a subset of the edges of a connected, edge-weighted undirected graph that connects all the vertices together, without any cycles and with the minimum possible total edge weight.

This program uses Borůvka's algorithm for finding MST. The algorithm begins by finding the minimum-weight edge incident to each vertex of the graph, and adding all of those edges to the forest. Then, it repeats a similar process of finding the minimum-weight edge from each tree constructed so far to a different tree, and adding all of those edges to the forest. Each repetition of this process reduces the number of trees, within each connected component of the graph, to at most half of this former value, so after logarithmically many repetitions the process finishes. When it does, the set of edges it has added forms the minimum spanning forest.

![algoritm_gif](https://upload.wikimedia.org/wikipedia/commons/2/2e/Boruvka%27s_algorithm_%28Sollin%27s_algorithm%29_Anim.gif)

# Implementation

## *Input data*

Data are passed to the program using `graph_data.txt` which must be located in the same folder as your `main.cpp`. The format is the following:
```
<number of vertices> <number of edges>
0 <number of edges attached to vertex #0> <target vertex #1> <length of edge #1> <target vertex #2> <length of edge #2> ...
1 <number of edges attached to vertex #1> <target vertex #1> <length of edge #1> <target vertex #2> <length of edge #2> ...
...
```

Application can create a random graph by using command  `--generate`.

There is also an example file containing the data called `graph_data.txt`.

## *Program arguments*

Application requires exactly one argument which should be one of the following:

  + `--generate` Generates a random graph and saves it to  `graph_data.txt`
  + `--help` Shows possible application arguments
  + `--single` Runs single-threaded implementation using the data from `graph_data.txt` and saves the result to `result.txt`
  + `--parellel` Runs multi-threaded implementation using the data from `graph_data.txt` and saves the result to `result.txt`
  + `--compare` Runs both single-threaded and multi-threaded implementation and compares the results

  ## *Performance measuring*

  You can compare the performance of single-threaded and multi-threaded implementations using the command  `--compare`.
  
  Example of the execution:
  
  ```
  Loading graph from graph_data.txt.
  Finished loading graph from file.
  Starting the single-threaded MST calculation.
  The single-threaded MST calculation took 24846 ms.
  The sum of edge weights is 9987.
  Saving the graph to result.txt.
  Graph is saved to result.txt.

  ================================================

  Loading graph from graph_data.txt.
  Finished loading graph from file.
  Starting the multi-threaded MST calculation.
  The multi-threaded MST calculation took 17450 ms.
  The sum of edge weights is 9987.
  Saving the graph to result.txt.
  Graph is saved to result.txt.

  The multi-threaded simulation was approximately 1.4x faster!
  ```