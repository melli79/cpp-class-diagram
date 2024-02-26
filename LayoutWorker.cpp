//
// Created by Melchior Grützmann on 2024-02-18.
//

#include "LayoutWorker.h"
#include <boost/graph/fruchterman_reingold.hpp>
#include <boost/graph/boyer_myrvold_planar_test.hpp>
#include <boost/graph/chrobak_payne_drawing.hpp>
#include <boost/graph/planar_canonical_ordering.hpp>
#include <boost/graph/kamada_kawai_spring_layout.hpp>
#include <boost/property_map/property_map.hpp>
#include <random>
#include <vector>

#include "GraphWindow.h"

LayoutWorker::LayoutWorker(graph::PositionMap& ps_) :ps(ps_) {};

LayoutWorker::~LayoutWorker() = default;

void LayoutWorker::startLayout(Graph const* g) {
    if (running)
        return;
    running = true;
    run(*g);
    running = false;
}

void LayoutWorker::run(Graph const& g) {
    qDebug() << "Worker thread: " << QThread::currentThread();
    double r = 1.0;
    size_t N = boost::num_vertices(g);
    ps.resize(N);
    for (int i=0; i<N; ++i) {
        ps[i].x = u01(random);
        ps[i].y = u01(random);
    }
    boost::graph_traits<graph::Graph>::edge_iterator edge, end;
    boost::tie(edge, end) = boost::edges(g);
    std::vector<graph::Edge> edges;
    for (edge; edge!=end; ++edge) {
        edges.emplace_back(boost::source(*edge, g), boost::target(*edge, g));
    }
    auto ug = graph::UGraph(edges.begin(), edges.end(), N);
    typedef std::vector< std::vector< boost::graph_traits<graph::UGraph>::edge_descriptor > >  PlanarEmbeddingStorage;
    typedef boost::iterator_property_map< PlanarEmbeddingStorage::iterator,
                                  boost::property_map<graph::UGraph, boost::vertex_index_t>::type
        >  PlanarEmbedding;
    PlanarEmbeddingStorage storage(N);
    PlanarEmbedding embedding(storage.begin(), boost::get(boost::vertex_index, ug));
    if (boost::boyer_myrvold_planarity_test(boost::boyer_myrvold_params::graph= ug, boost::boyer_myrvold_params::embedding= embedding)) {
        std::cout << "Embedding plane graph..." << std::endl;
        std::vector<graph::UVertex>  ordering;
        boost::planar_canonical_ordering(ug, embedding, std::back_inserter(ordering));

        typedef boost::iterator_property_map<
                graph::PositionMap::iterator,
                boost::property_map<graph::UGraph, boost::vertex_index_t>::type
            >  StraightLines;

        // Compute a straight line drawing
        std::cout << "The layout step." << std::endl;
        boost::chrobak_payne_straight_line_drawing(ug, embedding, ordering.begin(), ordering.end(),
            StraightLines(ps.begin(), get(boost::vertex_index, ug)));
    } else { // the thing is not plane, do the best we can
        std::cout << "Graph is not planar.  Using forceLayout..." << std::endl;
        boost::square_topology squareTopology;
        std::vector<graph::TPoint> ePoints(N);
        for (int i=0; i<N; ++i) {
            ePoints[i][0] = u01(random);
            ePoints[i][1] = u01(random);
        }
        boost::fruchterman_reingold_force_directed_layout(ug, &ePoints[0], squareTopology);
        for (unsigned v=0; v<boost::num_vertices(ug); ++v) {
            ps[v].x = ePoints[v][0];  ps[v].y = ePoints[v][1];
        }
    }
    emit done();
}
