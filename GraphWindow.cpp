//
// Created by Melchior Grützmann on 2024-02-18.
//

#include "GraphWindow.h"
#include <boost/graph/strong_components.hpp>
#include <QtGui>
#include <QApplication>
#include <cmath>

std::vector<GraphWindow::Graph> GraphWindow::graphs;
std::vector<QString> GraphWindow::names;

typedef std::uniform_int_distribution<unsigned>  Dice;
typedef std::uniform_real_distribution<>  Uniform;
typedef std::poisson_distribution<unsigned>  Poisson;


inline std::map<unsigned, unsigned>::iterator getIterator(std::map<unsigned, unsigned>& accepteds,
        std::vector<unsigned> const& sizes,
        const unsigned su, const unsigned threshold=5) {
    auto it = accepteds.find(su);
    if (it==accepteds.end() && (sizes[su]>1 || accepteds.size()<=threshold)) {
        accepteds[su] = accepteds.size();
        it = accepteds.find(su);
    }
    return it;
}

void GraphWindow::createGraphs() {
    typedef GraphWindow::Graph  Graph;
    typedef graph::Edge  Edge;
    const unsigned N = 100;
    auto d6 = Poisson(3);
    auto bad = 0.02;
    auto u01 = Uniform(0.0, 1.0);
    for (unsigned n=0; n<5; ++n) {
        std::vector<Edge> edges;
        unsigned mdeg = 0;
        for (unsigned v=1; v<N; ++v) {
            unsigned deg = d6(random);
            if (mdeg < deg)
                mdeg = deg;
            for (unsigned k=0; k<deg; ++k) {
                if (u01(random)>bad)
                    edges.emplace_back(v, int(u01(random)*(int((v+9)/10)*10))); // d100(random));
                else // few exceptional dependencies
                    edges.emplace_back(v, v+int(u01(random)*(N-v)));
            }
        }
        auto g = Graph(edges.begin(), edges.end(), N);

        std::vector<int> cc(N);
        auto ug = graph::UGraph(edges.begin(), edges.end(), N);
        (void) boost::connected_components(ug, &cc[0]);
        std::set<unsigned> covered;
        unsigned v1 = 0;
        covered.insert(cc[v1]);
        for (unsigned u=1; u<N; ++u) {
            if (covered.insert(cc[u]).second) {
                boost::add_edge(u, v1, g);
            }
        }

        std::vector<unsigned> sc(N);
        unsigned s = boost::strong_components(g, &sc[0]);
        std::cout << "The directed graph has N=" << N <<" vertices, " << boost::num_edges(g) << " edges"
            <<" and has s=" << s << " strong connected components." << std::endl;

        std::vector<unsigned> sizes(s);
        for (unsigned u=0; u<N; ++u) {
            sizes[sc[u]]++;
        }
        std::set<Edge> strEdges;
        std::map<unsigned, unsigned> accepteds;
        for (unsigned u=0; u<N; ++u) {
            unsigned su = sc[u];
            auto outEdges = boost::out_edges(u, g);
            for (auto e=outEdges.first; e!=outEdges.second; ++e) {
                unsigned sv = sc[e->m_target];
                if (su!=sv) {
                    strEdges.insert(std::make_pair(su, sv));
                }
            }
        }

        auto sg = Graph(strEdges.begin(), strEdges.end(), accepteds.size());
        auto labels = boost::get(boost::vertex_name, sg);
        boost::graph_traits<Graph>::vertex_iterator v, end;
        boost::tie(v, end) = boost::vertices(sg);
        for (; v!=end; ++v)
            boost::put(labels, *v, sizes[*v]);
        std::cout << "Condensed to " << boost::num_vertices(sg) << " vertices and " << boost::num_edges(sg) << " edges., sizes ";
        std::sort(sizes.begin(), sizes.end());
        unsigned i=0;
        for (auto si=sizes.rbegin(); si!=sizes.rend(); ++si) {
            std::cout << *si << ", ";
            if (++i>5)
                break;
        }
        std::cout << std::endl;
        graphs.push_back(sg);
        names.push_back(QString("Project %1").arg(n+1));
    }
}

GraphWindow::GraphWindow(QWidget* parent) :QWidget(parent), random(std::random_device()()) {
    createGraphs();
    dice = Dice(0, graphs.size()-1u);
    setWindowTitle("Graphs in Qt");
    workerThread = new QThread();
    worker = new LayoutWorker(ps);
    worker->moveToThread(workerThread);
    connect(this, &GraphWindow::layoutGraph, worker, &LayoutWorker::startLayout);
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

void GraphWindow::switchGraph() {
    size_t choice = dice(random);
    g = &graphs[choice];
    name = names[choice];
    emit layoutGraph(g);
}

Rect computeRange(std::vector<GraphWindow::Point> const& ps) {
    auto const& p0 = ps[0];
    double x0=p0.x, y0=p0.y;  double x1=x0, y1=y0;
    for (auto const& p :ps) {
        if (p.x<x0)
            x0 = p.x;
        else if (x1<p.x)
            x1 = p.x;
        if (p.y<y0)
            y0 = p.y;
        else if (y1<p.y)
            y1 = p.y;
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

int ild(size_t x) {
    if (x<=1)
        return 0;
    return int(3*log2(x));
}

void GraphWindow::paintEvent(QPaintEvent *paint_event) {
    scale = computeScale(width(), height(), range);
    setWindowTitle("Graphs in Qt –– "+name);
    auto p = QPainter(this);
    p.setBrush(Qt::black);
    for (auto const& pt :ps) {
        p.drawEllipse(scale.px(pt.x)-2, scale.py(pt.y)-2, 5,5);
    }
    boost::graph_traits<Graph>::edge_iterator e, end;
    for (boost::tie(e, end) = boost::edges(*g); e!=end; ++e) {
        Point const& p0 = ps[boost::source(*e, *g)];  Point const& p1 = ps[boost::target(*e, *g)];
        p.drawLine(scale.px(p0.x),scale.py(p0.y), scale.px(p1.x),scale.py(p1.y));
    }

    auto labels = boost::get(boost::vertex_name, *g);
    boost::graph_traits<Graph>::vertex_iterator v, end2;
    for (boost::tie(v, end2)=boost::vertices(*g); v!=end2; ++v) {
        Point const& p1 = ps[*v];
        int px = scale.px(p1.x);
        int py = scale.py(p1.y);
        size_t size = boost::get(labels, *v);
        int d = 3+ild(size);
        p.drawEllipse(px-d/2, py-d/2, d, d);
        p.drawText(px+5, py+5, QString("%1").arg(size));

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
