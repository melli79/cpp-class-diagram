//
// Created by Melchior Grützmann on 2024-02-18.
//

#include "LayoutWorker.h"
#include <boost/graph/kamada_kawai_spring_layout.hpp>
#include <random>

LayoutWorker::LayoutWorker(Graph *g_, PositionMap &ps_) :QObject(), g(g_), ps(ps_) {
}

LayoutWorker::~LayoutWorker() {
    g = nullptr;
}

void LayoutWorker::startLayout() {
    if (running)
        return;
    running = true;
    run();
    running = false;
}

void LayoutWorker::run() {
    qDebug() << "Worker thread: " << QThread::currentThread();
    boost::square_topology squareTopology;
    double r = 1.0;
    size_t n = boost::num_vertices(*g);
    ps.resize(n);
    for (int i=0; i<n; ++i) {
        ps[i][0] = u01(random);
        ps[i][1] = u01(random);
    }
    boost::kamada_kawai_spring_layout(*g, &ps[0], boost::get(boost::edge_weight, *g), squareTopology,
        boost::side_length(1.0));
    emit done();
}
