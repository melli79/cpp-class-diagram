//
// Created by Melchior Grützmann on 2024-02-18.
//

#ifndef LAYOUTWORKER_H
#define LAYOUTWORKER_H

#include <QtCore>
#include "graph.h"
#include <random>

class LayoutWorker :public QObject {
    Q_OBJECT

public:
    typedef graph::Graph  Graph;
    typedef graph::Point  Point;
    typedef graph::PositionMap  PositionMap;

    explicit LayoutWorker(Graph* g, PositionMap& ps);
    ~LayoutWorker() override;

    typedef std::mt19937_64  Random;
    typedef std::uniform_real_distribution<>  Uniform;

public slots:
    void startLayout();

signals:
    void done();

protected:
    void run();

private:
    Graph* g;
    PositionMap& ps;
    bool running = false;
    Random random = Random(std::random_device()());
    Uniform u01 = Uniform(0.0, 1.0);
};

#endif //LAYOUTWORKER_H
