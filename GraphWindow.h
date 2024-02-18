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

    typedef graph::Graph  Graph;
    typedef graph::Point  Point;
    typedef graph::PositionMap  PositionMap;

protected slots:
    void updateRange();

signals:
    void layoutGraph();

private:
    Graph* g = nullptr;
    PositionMap ps;
    Rect range = {};
    Rect scale = {};
    QThread* workerThread;
    LayoutWorker* worker;
};

#endif //GRAPHWINDOW_HPP
