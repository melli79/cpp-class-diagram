//
// Created by Melchior Grützmann on 2024-02-18.
//

#ifndef GRAPHWINDOW_HPP
#define GRAPHWINDOW_HPP

#include <QWidget>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/topology.hpp>
#include <boost/graph/topology.hpp>

struct Rect {
    double x0, y0, dx, dy;

    static Rect of(double x0, double y0, double x1, double y1) {
        return { x0, y0, x1-x0, y1-y0 };
    }

    [[nodiscard]]
    double y1() const {
        return y0+dy;
    }

    [[nodiscard]]
    int px(double x) const {
        return int(lround((x-x0)*dx));
    }

    [[nodiscard]]
    int py(double y) const {
        return int(lround((y-y0)*dy));
    }
};

class GraphWindow :public QWidget {
    Q_OBJECT

public:
    explicit GraphWindow(QWidget* parent = nullptr);
    ~GraphWindow() override;

    void layoutGraph();
    void paintEvent(QPaintEvent*) override;
    void keyReleaseEvent(QKeyEvent* event) override;

    typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS, boost::no_property,
            boost::property<boost::edge_weight_t, double>>
        Graph;
    typedef boost::square_topology<>::point_type  Point;
    typedef std::vector<Point> PositionMap;

private:
    Graph* g = nullptr;
    PositionMap ps;
    Rect range = {};
    Rect scale = {};
    bool running = false;
};


#endif //GRAPHWINDOW_HPP
