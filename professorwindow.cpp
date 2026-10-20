#include "professorwindow.h"
#include <QSqlQueryModel>

#include <QTabWidget>
#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QTableWidget>
#include <QPushButton>
#include <QComboBox>
#include <QHeaderView>

#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

#include <QMessageBox>

ProfessorWindow::ProfessorWindow(int professorId, QWidget* parent)
    : QMainWindow(parent),
    m_professorId(professorId)
{
    m_professorName = loadProfessorName();

    setWindowTitle("Professor Dashboard - Dr. " + m_professorName);
    resize(1000, 650);

    setupTabs();
}


void ProfessorWindow::setupTabs()
{
    QWidget* central = new QWidget(this);
    QVBoxLayout* mainLayout = new QVBoxLayout(central);

    // ===== Header =====
    QWidget* header = new QWidget();
    QVBoxLayout* headerLayout = new QVBoxLayout(header);

    QLabel* welcome = new QLabel("Welcome, Dr. " + m_professorName);
    welcome->setStyleSheet(
        "font-size:18px;"
        "font-weight:bold;"
        "padding:8px;"
    );

    QFrame* line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);

    headerLayout->addWidget(welcome);
    headerLayout->addWidget(line);
    headerLayout->setContentsMargins(5, 5, 5, 5);

    mainLayout->addWidget(header);

    // ===== Tabs =====
    m_tabs = new QTabWidget();
    mainLayout->addWidget(m_tabs, 1); // stretch = 1 مهم جدًا

    setCentralWidget(central);

    // ================= Tabs Content =================
    // My Courses
    m_tabCourses = new QWidget();
    {
        QVBoxLayout* l = new QVBoxLayout(m_tabCourses);

        QLabel* title = new QLabel("My Assigned Courses");
        title->setStyleSheet("font-weight:bold; font-size:14px;");
        l->addWidget(title);

        QTableView* coursesView = new QTableView();
        l->addWidget(coursesView, 1);

        QSqlQueryModel* coursesModel = new QSqlQueryModel(this);
        coursesModel->setQuery(QString(R"(
            SELECT Courses.code, Courses.name
            FROM CourseAssignments
            JOIN Courses ON Courses.id = CourseAssignments.course_id
            WHERE CourseAssignments.professor_id = %1
        )").arg(m_professorId));

        coursesView->setModel(coursesModel);
        coursesView->horizontalHeader()->setStretchLastSection(true);
    }

    // -------- Grades Tab --------
    m_tabGrades = new QWidget();
    m_tabAttendance = new QWidget();
    m_tabSchedule = new QWidget();
    m_tabProfile = new QWidget();

    m_tabs->addTab(m_tabCourses, "My Courses");
    m_tabs->addTab(m_tabGrades, "Grades");
    m_tabs->addTab(m_tabAttendance, "Attendance");
    m_tabs->addTab(m_tabSchedule, "Schedule");
    m_tabs->addTab(m_tabProfile, "Profile");

    // ✅ مرة واحدة فقط
    setCentralWidget(central);
}

// ================== Load Grades ==================
void ProfessorWindow::loadGradesForCourse(int courseId)
{
    m_gradesTable->setRowCount(0);

    QSqlQuery q;
    q.exec("SELECT student_id FROM Students");

    int row = 0;
    while (q.next()) {
        m_gradesTable->insertRow(row);

        m_gradesTable->setItem(row, 0,
            new QTableWidgetItem(q.value(0).toString()));

        // A1, A2, CW, Final, Total
        for (int col = 1; col <= 5; ++col) {
            m_gradesTable->setItem(row, col,
                new QTableWidgetItem("0"));
        }

        m_gradesTable->setItem(row, 6,
            new QTableWidgetItem(QString::number(courseId)));

        row++;
    }
}


QString ProfessorWindow::loadProfessorName()
{
    QSqlQuery q;
    q.prepare("SELECT name FROM Professors WHERE id = ?");
    q.addBindValue(m_professorId);

    if (q.exec() && q.next())
        return q.value(0).toString();

    qDebug() << q.lastError();
    return "Professor";
}


