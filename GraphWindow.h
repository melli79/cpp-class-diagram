//
// Created by Melchior Grützmann on 2024-02-18.
//

#ifndef GRAPHWINDOW_HPP
#define GRAPHWINDOW_HPP

#include <QWidget>
#include "graph.h"
#include "LayoutWorker.h"

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

    void paintEvent(QPaintEvent*) override;
    void keyReleaseEvent(QKeyEvent* event) override;
    void layoutGraph();

    typedef graph::Graph  Graph;
    typedef graph::Point  Point;
    typedef graph::PositionMap  PositionMap;

    typedef std::mt19937_64  Random;
    typedef std::uniform_int_distribution<size_t>  Dice;

    void switchGraph();
    void createGraphs();

protected slots:
    void updateRange();

private:
    static std::vector<Graph> graphs;
    static std::vector<QString> names;
    Graph* g = nullptr;
    QString name;
    PositionMap ps;
    Rect range = {};
    Rect scale = {};
    QThread* workerThread;
    LayoutWorker* worker;
    Random random;
    Dice dice;
};

#endif //GRAPHWINDOW_HPP
