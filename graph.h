//
// Created by Melchior Grützmann on 2024-02-18.
//

#ifndef GRAPH_H
#define GRAPH_H

#include <QtCore>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/topology.hpp>


namespace graph {
    typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS,
        boost::property<boost::vertex_name_t, size_t>
    >  Graph;
    typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS,
            boost::property<boost::vertex_index_t, unsigned>
        >  UGraph;
    typedef boost::graph_traits<graph::UGraph>::vertex_descriptor  UVertex;
    typedef std::pair<unsigned, unsigned>  Edge;
    typedef boost::square_topology<>::point_type  TPoint;
    struct Point {
        double x, y;
    };
    typedef std::vector<Point> PositionMap;

}

#endif //GRAPH_H
