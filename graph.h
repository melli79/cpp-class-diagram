//
// Created by Melchior Grützmann on 2024-02-18.
//

#ifndef GRAPH_H
#define GRAPH_H

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/topology.hpp>


namespace graph {
    typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS, boost::no_property,
        boost::property<boost::edge_weight_t, double>>
    Graph;
    typedef boost::square_topology<>::point_type  Point;
    typedef std::vector<Point> PositionMap;

}

#endif //GRAPH_H
