//
// Created by Melchior Grützmann on 2024-02-18.
//

#include "GraphWindow.h"
#include <QtGui>
#include <QApplication>

std::vector<GraphWindow::Graph> GraphWindow::graphs;
std::vector<QString> GraphWindow::names;


void GraphWindow::createGraphs() {
    typedef GraphWindow::Graph  Graph;
    typedef std::pair<int, int>  Edge;
    double weights[] = { 1.0, 1.0, 1.0, 1.0, 1.0,  1.0, 1.0, 1.0, 1.0, 1.0 };
    {
        std::vector<Edge> edges = { {0,1}, {1,2}, {2,3} };
        graphs.emplace_back(edges.begin(), edges.end(), &weights[0], 4u);
        names.emplace_back("P3");
    }
    {
        std::vector<Edge> edges = { {0,1}, {1,2}, {2,0} };
        graphs.emplace_back(edges.begin(), edges.end(), &weights[0], 3u);
        names.emplace_back("K3 = C3");
    }
    {
        std::vector<Edge> edges = { {0,1}, {1,2}, {2,3}, {3,0} };
        graphs.emplace_back(edges.begin(), edges.end(), &weights[0], 4u);
        names.emplace_back("C4");
    }
    {
        std::vector<Edge> edges = { {0,1}, {0,2}, {0,3}, {1,2}, {1,3}, {2,3} };
        graphs.emplace_back(edges.begin(), edges.end(), &weights[0], 4u);
        names.emplace_back("K4");
    }
    {
        std::vector<Edge> edges = { {0,1}, {0,2}, {0,3}, {0,4}, {0,5} };
        graphs.emplace_back(edges.begin(), edges.end(), &weights[0], 6u);
        names.emplace_back("St5");
    }
    {
        std::vector<Edge> edges = { {0,1}, {0,2}, {0,3}, {0,4}, {1,2},{1,3},{1,4}, {2,3},{2,4}, {3,4} };
        graphs.emplace_back(edges.begin(), edges.end(), &weights[0], 5u);
        names.emplace_back("K5");
    }
    {
        std::vector<Edge> edges = { {0,3}, {0,4}, {0,5}, {1,3},{1,4},{1,5}, {2,3}, {2,4}, {2,5} };
        graphs.emplace_back(edges.begin(), edges.end(), &weights[0], 6u);
        names.emplace_back("K3,3");
    }
}

GraphWindow::GraphWindow(QWidget* parent) :QWidget(parent), random(std::random_device()()) {
    createGraphs();
    dice = Dice(0, graphs.size()-1u);
    setWindowTitle("Graphs in Qt");
    workerThread = new QThread();
    worker = new LayoutWorker();
    worker->moveToThread(workerThread);
    connect(worker, SIGNAL(done()), this, SLOT(updateRange()));
    workerThread->start();
    qDebug() << "GUI thread: " << QThread::currentThread();
    switchGraph();
}

GraphWindow::~GraphWindow() {
    if (workerThread!=nullptr) {
        workerThread->quit();
        workerThread->wait();
        delete workerThread;
        workerThread = nullptr;
    }
    if (worker!=nullptr) {
        delete worker;
        worker = nullptr;
    }
    g = nullptr;
}

void GraphWindow::layoutGraph() {
    worker->startLayout(*g, ps);
}

void GraphWindow::switchGraph() {
    size_t choice = dice(random);
    g = &graphs[choice];
    name = names[choice];
    layoutGraph();
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

void GraphWindow::updateRange() {
    range = computeRange(ps);
    update();
}

Rect computeScale(int width, int height, Rect const& range) {
    double dx = 0.95*std::min(width/range.dx, height/range.dy);
    return { range.x0 +(range.dx-width/dx)/2, range.y1() -(range.dy-height/dx)/2, dx, -dx };
}

void GraphWindow::paintEvent(QPaintEvent *paint_event) {
    scale = computeScale(width(), height(), range);
    setWindowTitle("Graphs in Qt –– "+name);
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
            switchGraph();
    }
    event->accept();
}
