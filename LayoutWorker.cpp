//
// Created by Melchior Grützmann on 2024-02-18.
//

#include "LayoutWorker.h"
#include <boost/graph/kamada_kawai_spring_layout.hpp>
#include <random>

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
    boost::kamada_kawai_spring_layout(g, &ps[0], boost::get(boost::edge_weight, g), squareTopology,
        boost::side_length(1.0));
    emit done();
}
