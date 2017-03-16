#ifndef GRAPHVIEW_H
#define GRAPHVIEW_H

#include <QWidget>

namespace Ui {
class GraphView;
}

class GraphView : public QWidget
{
    Q_OBJECT

public:
    explicit GraphView(QWidget *parent = 0);
    ~GraphView();


private slots:
    void resizeEvent(QResizeEvent *);

private:
    Ui::GraphView *ui;
};

#endif // GRAPHVIEW_H
