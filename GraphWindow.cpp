//
// Created by Melchior Grützmann on 2024-02-18.
//

#include "GraphWindow.h"
#include <boost/graph/strong_components.hpp>
#include <QtGui>
#include <QApplication>

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
    auto bad = 0.025;
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
        std::vector<std::string> l0s(edges.size());
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
            boost::put(labels, *v, QString("%1").arg(sizes[*v]));
        std::cout << "Trunkated to " << boost::num_vertices(sg) << " vertices and " << boost::num_edges(sg) << " edges., sizes ";
        std::sort(sizes.begin(), sizes.end());
        unsigned i=0;
        for (auto si=sizes.rbegin(); si!=sizes.rend(); ++si) {
            std::cout << *si << ", ";
            if (++i>accepteds.size())
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

    auto labels = boost::get(boost::vertex_name, *g);
    boost::graph_traits<Graph>::vertex_iterator v, end2;
    for (boost::tie(v, end2)=boost::vertices(*g); v!=end2; ++v) {
        Point const& p1 = ps[*v];
        int px = scale.px(p1[0]);
        int py = scale.py(p1[1]);
        p.drawEllipse(px-1, py-1, 3, 3);
        p.drawText(px, py, boost::get(labels, *v));

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
