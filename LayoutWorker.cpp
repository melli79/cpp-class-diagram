//
// Created by Melchior Grützmann on 2024-02-18.
//

#include "LayoutWorker.h"
#include <boost/graph/fruchterman_reingold.hpp>
#include <boost/graph/kamada_kawai_spring_layout.hpp>
#include <random>

#include "GraphWindow.h"

LayoutWorker::LayoutWorker() = default;

LayoutWorker::~LayoutWorker() = default;

void LayoutWorker::startLayout(Graph const& g, PositionMap& ps) {
    if (running)
        return;
    running = true;
    run(g, ps);
    running = false;
}

void LayoutWorker::run(Graph const& g, PositionMap& ps) {
    qDebug() << "Worker thread: " << QThread::currentThread();
    boost::square_topology squareTopology;
    double r = 1.0;
    size_t n = boost::num_vertices(g);
    ps.resize(n);
    for (int i=0; i<n; ++i) {
        ps[i][0] = u01(random);
        ps[i][1] = u01(random);
    }
    boost::graph_traits<graph::Graph>::edge_iterator edge, end;
    boost::tie(edge, end) = boost::edges(g);
    std::vector<graph::Edge> edges;
    std::vector<double> weights(std::distance(edge, end));
    for (; edge!=end; ++edge)
        edges.emplace_back(boost::source(*edge, g), boost::target(*edge, g));
    for (auto w : weights)
        w = 1.0;
    auto ug = graph::UGraph(edges.begin(), edges.end(), &weights[0], boost::num_vertices(g));
    // boost::kamada_kawai_spring_layout(ug, &ps[0], boost::get(boost::edge_weight, ug), squareTopology,
    //     boost::side_length(1.0));
    boost::fruchterman_reingold_force_directed_layout(ug, &ps[0], squareTopology);
    emit done();
}
