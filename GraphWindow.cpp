//
// Created by Melchior Grützmann on 2024-02-18.
//

#include "GraphWindow.h"
#include <boost/graph/circle_layout.hpp>
#include <boost/graph/kamada_kawai_spring_layout.hpp>
#include <QtGui>
#include <QApplication>
#include <random>
#include <vector>

GraphWindow::Graph* createGraph() {
    typedef GraphWindow::Graph  Graph;
    typedef std::pair<int, int>  Edge;
    std::vector<Edge> edges = { {0,3}, {3,1}, {1,4}, {4,2}, {2,0} };
    double weights[] = { 1.0, 1.0, 1.0, 1.0, 1.0 };
    return new Graph(edges.begin(), edges.end(), &weights[0], 5u);
}

GraphWindow::GraphWindow(QWidget* parent) :QWidget(parent) {
    g = createGraph();
    layoutGraph();
}

GraphWindow::~GraphWindow() {
    if (g!=nullptr) {
        delete g;
        g = nullptr;
    }
}

Rect computeRange(std::vector<GraphWindow::Point> const& ps) {
    auto const& p0 = ps[0];
    double x0=p0[0], y0=p0[1];  double x1=x0, y1=y0;
    for (auto const& p :ps) {
        if (p[0]<x0)
            x0 = p[0];
        else if (x1<p[0])
            x1 = p[0];
        if (p[1]<y0)
            y0 = p[1];
        else if (y1<p[1])
            y1 = p[1];
    }
    return Rect::of(x0,y0, x1,y1);
}

typedef std::mt19937_64  Random;
typedef std::uniform_real_distribution<>  Uniform;

void GraphWindow::layoutGraph() {
    if (running)
        return;
    running = true;
    auto random = Random(std::random_device()());
    auto u01 = Uniform(0.0, 1.0);
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
    range = computeRange(ps);
    running = false;
}

Rect computeScale(int width, int height, Rect const& range) {
    double dx = 0.95*std::min(width/range.dx, height/range.dy);
    return { range.x0 +(range.dx-width/dx)/2, range.y1() -(range.dy-height/dx)/2, dx, -dx };
}

void GraphWindow::paintEvent(QPaintEvent *paint_event) {
    scale = computeScale(width(), height(), range);
    auto p = QPainter(this);
    p.setBrush(Qt::black);
    for (auto const& pt :ps) {
        p.drawEllipse(scale.px(pt[0])-2, scale.py(pt[1])-2, 5,5);
    }
    boost::graph_traits<Graph>::edge_iterator e, end;
    for (boost::tie(e, end) = boost::edges(*g); e!=end; ++e) {
        Point const& p0 = ps[boost::source(*e, *g)];  Point const& p1 = ps[boost::target(*e, *g)];
        p.drawLine(scale.px(p0[0]),scale.py(p0[1]), scale.px(p1[0]),scale.py(p1[1]));
    }
    p.end();
}

void GraphWindow::keyReleaseEvent(QKeyEvent* event) {
    switch (event->key()) {
        case Qt::Key_Escape:
            case Qt::Key_Q:
            QApplication::exit(0);
        default:
            layoutGraph();
            update();
    }
    event->accept();
}
