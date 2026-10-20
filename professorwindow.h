#ifndef PROFESSORWINDOW_H
#define PROFESSORWINDOW_H

#include <QMainWindow>

class QTabWidget;
class QWidget;
class QTableWidget; 

class ProfessorWindow : public QMainWindow
{
public:
    explicit ProfessorWindow(int professorId, QWidget* parent = nullptr);

private:
    int m_professorId;
    QString m_professorName;  


    QTabWidget* m_tabs;

    QWidget* m_tabCourses;
    QWidget* m_tabGrades;
    QWidget* m_tabAttendance;
    QWidget* m_tabSchedule;
    QWidget* m_tabProfile;

    QTableWidget* m_gradesTable; 



    QString loadProfessorName();
    void setupTabs();
    void loadGradesForCourse(int courseId);
};

#endif
